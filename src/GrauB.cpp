/* Hello Triangle - código adaptado de https://learnopengl.com/#!Getting-started/Hello-Triangle
 *
 * Adaptado por Rossana Baptista Queiroz
 * para as disciplinas de Processamento Gráfico/Computação Gráfica - Unisinos
 * Versão inicial: 7/4/2017
 * Última atualização em 07/03/2025
 */

#include <iostream>
#include <string>
#include <assert.h>

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "utils/LoadSimpleObj.hpp"
#include "utils/LoadTexture.hpp"
#include "utils/SceneTypes.hpp"
#include "utils/LoadScene.hpp"

// ==================== Constants ====================
const GLuint WINDOW_WIDTH = 1920;
const GLuint WINDOW_HEIGHT = 1024;

const float CAMERA_DEFAULT_SPEED = 0.3f;
const float OBJECT_SCALE_STEP = 0.05f;
const float OBJECT_SCALE_MIN = 0.05f;
const float LIGHT_INTENSITY_STEP = 0.1f;
const float OBJECT_POSITION_STEP = 0.1f;
const float LIGHT_POSITION_STEP = 0.1f;
const float OBJECT_ROTATION_STEP = 5.0f;

const float VIEWPORT_FOV = 45.0f;
const float VIEWPORT_NEAR = 0.1f;
const float VIEWPORT_FAR = 100.0f;

const float ATTENUATION_Kc = 1.0f;
const float ATTENUATION_Kl = 0.09f;
const float ATTENUATION_Kq = 0.032f;

// ==================== Enums ====================
enum LightIndex
{
    KEY_LIGHT = 0,
    FILL_LIGHT = 1,
    BACK_LIGHT = 2,
    NUM_LIGHTS = 3
};

// ==================== Structures ====================
struct Trajectory
{
    std::vector<glm::vec3> controlPoints;
    int currentPoint = 0;
    float speed = 1.5f;
    bool enabled = true;
};

struct Light
{
    glm::vec3 position;
    glm::vec3 color;
    float intensity;
    bool enabled;
};

// ==================== Global State ====================
int g_selectedObjectIndex = 0;
std::vector<SceneObject> *g_sceneObjectsPtr = nullptr;
GLuint g_shaderProgram = 0;
int g_selectedLight = KEY_LIGHT;

Light g_lights[NUM_LIGHTS] = {
    {glm::vec3(0.0f, 3.0f, 3.0f), glm::vec3(1.0f), 1.0f, true},
    {glm::vec3(3.0f, 2.0f, 0.0f), glm::vec3(1.0f), 0.5f, false},
    {glm::vec3(-3.0f, 2.0f, 0.0f), glm::vec3(1.0f), 0.5f, false}
};

glm::vec3 g_cameraPos = glm::vec3(0.0f, 0.0f, 5.0f);
glm::vec3 g_cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 g_cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float g_cameraSpeed = CAMERA_DEFAULT_SPEED;

// ==================== Function Prototypes ====================
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);
void inicializaShaders();
void drawBoundingBox(
    const BoundingBox &bbox,
    const glm::mat4 &model,
    GLint modelLoc,
    GLuint shaderID);
void updateObjectAnimation(SceneObject &object, float time);

// ==================== Helper Functions ====================

// Set up all lighting uniforms for the current frame
void setupLightingUniforms(GLuint shaderID)
{
    std::glUniform1f(std::glGetUniformLocation(shaderID, "Kc"), ATTENUATION_Kc);
    std::glUniform1f(std::glGetUniformLocation(shaderID, "Kl"), ATTENUATION_Kl);
    std::glUniform1f(std::glGetUniformLocation(shaderID, "Kq"), ATTENUATION_Kq);

    for (int i = 0; i < NUM_LIGHTS; ++i)
    {
        std::string idx = std::to_string(i);

        std::glUniform3fv(
            std::glGetUniformLocation(shaderID, ("lights[" + idx + "].position").c_str()),
            1,
            glm::value_ptr(g_lights[i].position));

        std::glUniform3fv(
            std::glGetUniformLocation(shaderID, ("lights[" + idx + "].color").c_str()),
            1,
            glm::value_ptr(g_lights[i].color));

        std::glUniform1f(
            std::glGetUniformLocation(shaderID, ("lights[" + idx + "].intensity").c_str()),
            g_lights[i].intensity);

        std::glUniform1i(
            std::glGetUniformLocation(shaderID, ("lights[" + idx + "].enabled").c_str()),
            g_lights[i].enabled);
    }
}

// Set up material uniforms for a submesh
void setupMaterialUniforms(GLuint shaderID, const Submesh &submesh)
{
    std::glUniform1f(
        std::glGetUniformLocation(shaderID, "Ka"),
        submesh.material.Ka.x);

    std::glUniform1f(
        std::glGetUniformLocation(shaderID, "Kd"),
        submesh.material.Kd.x);

    std::glUniform1f(
        std::glGetUniformLocation(shaderID, "Ks"),
        submesh.material.Ks.x);

    std::glUniform1f(
        std::glGetUniformLocation(shaderID, "shininess"),
        submesh.material.shininess);

    std::glUniform3fv(
        std::glGetUniformLocation(shaderID, "objectColor"),
        1,
        glm::value_ptr(submesh.material.Kd));

    std::glUniform1i(
        std::glGetUniformLocation(shaderID, "useTexture"),
        submesh.material.textureId != 0);

    std::glActiveTexture(GL_TEXTURE0);
    std::glBindTexture(GL_TEXTURE_2D, submesh.material.textureId);

    std::glUniform1i(
        std::glGetUniformLocation(shaderID, "diffuseTexture"),
        0);
}

// Render a single submesh
void renderSubmesh(const Submesh &submesh)
{
    std::glBindVertexArray(submesh.VAO);
    std::glDrawArrays(
        GL_TRIANGLES,
        0,
        submesh.nVertices);
}

// Função MAIN
int main()
{
    // Inicialização da GLFW
    glfwInit();
    GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Ola 3D -- Gabriel Corrêa!", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    // Fazendo o registro da função de callback para a janela GLFW
    glfwSetKeyCallback(window, key_callback);

    // GLAD: carrega todos os ponteiros de funções da OpenGL
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
    }

    // Obtendo as informações de versão
    const GLubyte *renderer = glGetString(GL_RENDERER);
    const GLubyte *version = glGetString(GL_VERSION);
    std::cout << "Renderer: " << renderer << std::endl;
    std::cout << "OpenGL version supported " << version << std::endl;

    // Definindo as dimensões da viewport com as mesmas dimensões da janela da aplicação
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    // Compilando e buildando o programa de shader
    inicializaShaders();

    GLuint shaderID = g_shaderProgram;
    glUseProgram(shaderID);

    glm::mat4 model = glm::mat4(1);
    GLint modelLoc = glGetUniformLocation(shaderID, "model");
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    GLuint viewLoc = glGetUniformLocation(shaderID, "view");
    GLuint projLoc = glGetUniformLocation(shaderID, "proj");

    glEnable(GL_DEPTH_TEST);

    SceneData scene = loadScene("../scene.json");

    std::vector<SceneObject> sceneObjects = scene.objects;
    g_sceneObjectsPtr = &sceneObjects;

    g_cameraPos = scene.camera.position;
    g_cameraFront = scene.camera.front;
    g_cameraUp = scene.camera.up;

    for (size_t i = 0; i < scene.lights.size() && i < NUM_LIGHTS; ++i)
    {
        g_lights[i].position = scene.lights[i].position;
        g_lights[i].color = scene.lights[i].color;
        g_lights[i].intensity = scene.lights[i].intensity;
        g_lights[i].enabled = scene.lights[i].enabled;
    }

    // Loop da aplicação - "game loop"
    while (!glfwWindowShouldClose(window))
    {
        // Checa se houveram eventos de input (key pressed, mouse moved etc.) e chama as funções de callback correspondentes
        glfwPollEvents();

        // Limpa o buffer de cor
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glLineWidth(10);
        glPointSize(20);

        glm::mat4 view = glm::lookAt(
            g_cameraPos,
            g_cameraPos + g_cameraFront,
            g_cameraUp);

        glm::mat4 projection = glm::perspective(
            glm::radians(VIEWPORT_FOV),
            (float)WINDOW_WIDTH / WINDOW_HEIGHT,
            VIEWPORT_NEAR,
            VIEWPORT_FAR);

        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        glUniform3fv(
            glGetUniformLocation(shaderID, "viewPos"),
            1,
            glm::value_ptr(g_cameraPos));

        setupLightingUniforms(shaderID);

        for (size_t i = 0; i < sceneObjects.size(); ++i)
        {
            SceneObject &object = sceneObjects[i];
            bool isSelected = i == g_selectedObjectIndex;

            float time = glfwGetTime();
            updateObjectAnimation(object, time);

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, object.position);
            model = glm::rotate(model, glm::radians(object.rotation.x), glm::vec3(1, 0, 0));
            model = glm::rotate(model, glm::radians(object.rotation.y), glm::vec3(0, 1, 0));
            model = glm::rotate(model, glm::radians(object.rotation.z), glm::vec3(0, 0, 1));
            model = glm::scale(model, object.scale);

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

            // Render all submeshes for this object
            for (auto &submesh : object.mesh.submeshes)
            {
                setupMaterialUniforms(shaderID, submesh);
                renderSubmesh(submesh);
            }

            if (isSelected)
            {
                drawBoundingBox(object.mesh.bbox, model, modelLoc, shaderID);
            }
        }
        glBindVertexArray(0);

        // Troca os buffers da tela
        glfwSwapBuffers(window);
    }

    // Finaliza a execução da GLFW, limpando os recursos alocados por ela
    glfwTerminate();
    return 0;
}

// Função de callback de teclado - só pode ter uma instância (deve ser estática se
// estiver dentro de uma classe) - É chamada sempre que uma tecla for pressionada
// ou solta via GLFW
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    // ================= SELEÇÃO DE OBJETO =================
    if (key == GLFW_KEY_TAB && action == GLFW_PRESS)
    {
        if (g_sceneObjectsPtr && !g_sceneObjectsPtr->empty())
        {
            g_selectedObjectIndex =
                (g_selectedObjectIndex + 1) % g_sceneObjectsPtr->size();

            std::cout << "Objeto selecionado: "
                      << g_selectedObjectIndex
                      << std::endl;
        }
    }

    // ================= SELEÇÃO DE LUZ =================
    if (key == GLFW_KEY_1 && action == GLFW_PRESS)
        g_selectedLight = KEY_LIGHT;

    if (key == GLFW_KEY_2 && action == GLFW_PRESS)
        g_selectedLight = FILL_LIGHT;

    if (key == GLFW_KEY_3 && action == GLFW_PRESS)
        g_selectedLight = BACK_LIGHT;

    if (key == GLFW_KEY_L && action == GLFW_PRESS)
        g_lights[g_selectedLight].enabled = !g_lights[g_selectedLight].enabled;

    if (!(action == GLFW_PRESS || action == GLFW_REPEAT))
        return;

    // ================= CÂMERA =================
    if (key == GLFW_KEY_W)
        g_cameraPos += g_cameraSpeed * g_cameraFront;

    if (key == GLFW_KEY_S)
        g_cameraPos -= g_cameraSpeed * g_cameraFront;

    if (key == GLFW_KEY_A)
        g_cameraPos -= glm::normalize(glm::cross(g_cameraFront, g_cameraUp)) * g_cameraSpeed;

    if (key == GLFW_KEY_D)
        g_cameraPos += glm::normalize(glm::cross(g_cameraFront, g_cameraUp)) * g_cameraSpeed;

    if (key == GLFW_KEY_Q)
        g_cameraPos.y += g_cameraSpeed;

    if (key == GLFW_KEY_E)
        g_cameraPos.y -= g_cameraSpeed;

    // ================= LUZ SELECIONADA =================
    if (key == GLFW_KEY_EQUAL)
        g_lights[g_selectedLight].intensity += LIGHT_INTENSITY_STEP;

    if (key == GLFW_KEY_MINUS)
        g_lights[g_selectedLight].intensity -= LIGHT_INTENSITY_STEP;

    if (g_lights[g_selectedLight].intensity < 0.0f)
        g_lights[g_selectedLight].intensity = 0.0f;

    if (key == GLFW_KEY_UP)
        g_lights[g_selectedLight].position.y += LIGHT_POSITION_STEP;

    if (key == GLFW_KEY_DOWN)
        g_lights[g_selectedLight].position.y -= LIGHT_POSITION_STEP;

    if (key == GLFW_KEY_LEFT)
        g_lights[g_selectedLight].position.x -= LIGHT_POSITION_STEP;

    if (key == GLFW_KEY_RIGHT)
        g_lights[g_selectedLight].position.x += LIGHT_POSITION_STEP;

    if (key == GLFW_KEY_PAGE_UP)
        g_lights[g_selectedLight].position.z += LIGHT_POSITION_STEP;

    if (key == GLFW_KEY_PAGE_DOWN)
        g_lights[g_selectedLight].position.z -= LIGHT_POSITION_STEP;

    // ================= OBJETO SELECIONADO =================
    if (g_sceneObjectsPtr && !g_sceneObjectsPtr->empty())
    {
        SceneObject &selectedObject = (*g_sceneObjectsPtr)[g_selectedObjectIndex];

        // Translação do objeto
        if (key == GLFW_KEY_I)
            selectedObject.position.y += OBJECT_POSITION_STEP;

        if (key == GLFW_KEY_K)
            selectedObject.position.y -= OBJECT_POSITION_STEP;

        if (key == GLFW_KEY_J)
            selectedObject.position.x -= OBJECT_POSITION_STEP;

        if (key == GLFW_KEY_U)
            selectedObject.position.x += OBJECT_POSITION_STEP;

        if (key == GLFW_KEY_O)
            selectedObject.position.z += OBJECT_POSITION_STEP;

        if (key == GLFW_KEY_P)
            selectedObject.position.z -= OBJECT_POSITION_STEP;

        // Escala uniforme
        if (key == GLFW_KEY_RIGHT_BRACKET)
            selectedObject.scale += glm::vec3(OBJECT_SCALE_STEP);

        if (key == GLFW_KEY_LEFT_BRACKET)
            selectedObject.scale -= glm::vec3(OBJECT_SCALE_STEP);

        if (selectedObject.scale.x < OBJECT_SCALE_MIN)
            selectedObject.scale = glm::vec3(OBJECT_SCALE_MIN);

        // Rotação do objeto
        if (key == GLFW_KEY_X)
            selectedObject.rotation.x += OBJECT_ROTATION_STEP;

        if (key == GLFW_KEY_Y)
            selectedObject.rotation.y += OBJECT_ROTATION_STEP;

        if (key == GLFW_KEY_Z)
            selectedObject.rotation.z += OBJECT_ROTATION_STEP;
    }
}

// Compila um shader individual
GLuint compilaShader(const char *source, GLenum type)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);

        std::cerr << "Erro ao compilar shader: "
                  << (type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT")
                  << "\n"
                  << infoLog
                  << std::endl;
    }
    return shader;
}

void inicializaShaders()
{
    const char *vertex_shader = R"(
        #version 450

        layout(location = 0) in vec3 vertex_posicao;
        layout(location = 1) in vec3 vertex_normal;
        layout(location = 2) in vec2 vertex_texcoord;

        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 proj;

        out vec3 fragPos;
        out vec3 normal;
        out vec2 texCoord;


        void main()
        {
            vec4 worldPos = model * vec4(vertex_posicao, 1.0);
            fragPos = worldPos.xyz;

            normal = mat3(transpose(inverse(model))) * vertex_normal;
            texCoord = vertex_texcoord;

            gl_Position = proj * view * worldPos;
        }
    )";

    const char *fragment_shader = R"(
        #version 450

in vec3 fragPos;
in vec3 normal;
in vec2 texCoord;

out vec4 frag_colour;

uniform vec3 viewPos;
uniform vec3 objectColor;

uniform sampler2D diffuseTexture;
uniform bool useTexture;

uniform float Ka;
uniform float Kd;
uniform float Ks;
uniform float shininess;

uniform float Kc;
uniform float Kl;
uniform float Kq;

struct Light
{
    vec3 position;
    vec3 color;
    float intensity;
    bool enabled;
};

uniform Light lights[3];

void main()
{
    vec3 N=normalize(normal);
    vec3 V=normalize(viewPos-fragPos);

    vec3 ambient=vec3(0);
    vec3 diffuse=vec3(0);
    vec3 specular=vec3(0);

    for(int i=0;i<3;i++)
    {
        if(!lights[i].enabled)
            continue;

        vec3 L=
            normalize(
                lights[i].position-fragPos
            );

        vec3 R=
            reflect(-L,N);

        float d=
            length(
                lights[i].position-fragPos
            );

        float attenuation=
            1.0/
            (
                Kc+
                Kl*d+
                Kq*d*d
            );

        ambient+=
            Ka*
            lights[i].color*
            0.1;

        float diff=
            max(dot(N,L),0);

        diffuse+=
            Kd*
            diff*
            lights[i].color*
            lights[i].intensity*
            attenuation;

        float spec=
            pow(
                max(dot(V,R),0),
                shininess
            );

        specular+=
            Ks*
            spec*
            lights[i].color*
            lights[i].intensity*
            attenuation;
    }

    vec3 baseColor = objectColor;

    if (useTexture)
    {
        baseColor = texture(diffuseTexture, texCoord).rgb;
    }

    vec3 result=
        (ambient+diffuse) *
        baseColor
        +specular;

    frag_colour=
        vec4(result,1);
        }
    )";

    GLuint vs = compilaShader(vertex_shader, GL_VERTEX_SHADER);
    GLuint fs = compilaShader(fragment_shader, GL_FRAGMENT_SHADER);

    g_shaderProgram = glCreateProgram();
    glAttachShader(g_shaderProgram, vs);
    glAttachShader(g_shaderProgram, fs);
    glLinkProgram(g_shaderProgram);

    glDeleteShader(vs);
    glDeleteShader(fs);
}

void drawBoundingBox(
    const BoundingBox &bbox,
    const glm::mat4 &model,
    GLint modelLoc,
    GLuint shaderID)
{
    const glm::vec3 min = bbox.min;
    const glm::vec3 max = bbox.max;

    glm::vec3 vertices[] = {
        {min.x, min.y, min.z},
        {max.x, min.y, min.z},
        {max.x, max.y, min.z},
        {min.x, max.y, min.z},

        {min.x, min.y, max.z},
        {max.x, min.y, max.z},
        {max.x, max.y, max.z},
        {min.x, max.y, max.z}};

    unsigned int indices[] = {
        0, 1, 1, 2, 2, 3, 3, 0,
        4, 5, 5, 6, 6, 7, 7, 4,
        0, 4, 1, 5, 2, 6, 3, 7};

    GLuint VAO, VBO, EBO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(glm::vec3),
        (void *)0);

    glEnableVertexAttribArray(0);

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    glUniform1i(glGetUniformLocation(shaderID, "useTexture"), 0);

    glUniform3f(
        glGetUniformLocation(shaderID, "objectColor"),
        0.2f, 0.8f, 1.0f);

    glUniform1f(glGetUniformLocation(shaderID, "Ka"), 1.0f);
    glUniform1f(glGetUniformLocation(shaderID, "Kd"), 1.0f);
    glUniform1f(glGetUniformLocation(shaderID, "Ks"), 0.0f);
    glUniform1f(glGetUniformLocation(shaderID, "shininess"), 1.0f);

    glLineWidth(2.0f);

    glDrawElements(
        GL_LINES,
        24,
        GL_UNSIGNED_INT,
        0);

    glBindVertexArray(0);

    glDeleteBuffers(1, &EBO);
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
}

void updateObjectAnimation(SceneObject &object, float time)
{
    if (!object.animation.enabled)
        return;

    if (object.animation.type == "circle")
    {
        const float angle = time * object.animation.speed;

        object.position.x =
            object.animation.center.x +
            std::cos(angle) * object.animation.radius;

        object.position.z =
            object.animation.center.z +
            std::sin(angle) * object.animation.radius;

        object.position.y =
            object.animation.center.y;
    }
}
