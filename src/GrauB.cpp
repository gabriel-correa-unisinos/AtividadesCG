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

using namespace std;

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

struct Light
{
    glm::vec3 position;
    glm::vec3 color;
    float intensity;
    bool enabled;
};

enum
{
    KEY_LIGHT = 0,
    FILL_LIGHT = 1,
    BACK_LIGHT = 2
};

int selectedObjectIndex = 0;
std::vector<SceneObject> *sceneObjectsPtr = nullptr;
GLuint Shader_programm = 0;
int selectedLight = KEY_LIGHT;

Light lights[3] = {
    {glm::vec3(0.0f, 3.0f, 3.0f), glm::vec3(1.0f), 1.0f, true},
    {glm::vec3(3.0f, 2.0f, 0.0f), glm::vec3(1.0f), 0.5f, false},
    {glm::vec3(-3.0f, 2.0f, 0.0f), glm::vec3(1.0f), 0.5f, false}};

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 5.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float cameraSpeed = 0.3f;

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);

// Protótipos das funções

void inicializaShaders();
void drawBoundingBox(
    const BoundingBox &bbox,
    const glm::mat4 &model,
    GLint modelLoc,
    GLuint shaderID);
void updateObjectAnimation(SceneObject &object, float time);

// Dimensões da janela (pode ser alterado em tempo de execução)
const GLuint WIDTH = 1920,
             HEIGHT = 1024;

// Código fonte do Vertex Shader (em GLSL): ainda hardcoded

// Função MAIN
int main()
{
    // Inicialização da GLFW
    glfwInit();
    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Ola 3D -- Gabriel Corrêa!", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    // Fazendo o registro da função de callback para a janela GLFW
    glfwSetKeyCallback(window, key_callback);

    // GLAD: carrega todos os ponteiros d funções da OpenGL
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
    }

    // Obtendo as informações de versão
    const GLubyte *renderer = glGetString(GL_RENDERER); /* get renderer string */
    const GLubyte *version = glGetString(GL_VERSION);   /* version as a string */
    cout << "Renderer: " << renderer << endl;
    cout << "OpenGL version supported " << version << endl;

    // Definindo as dimensões da viewport com as mesmas dimensões da janela da aplicação
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    // Compilando e buildando o programa de shader

    // Gerando um buffer simples, com a geometria de um triângulo

    inicializaShaders();

    GLuint shaderID = Shader_programm;
    glUseProgram(shaderID);

    glm::mat4 model = glm::mat4(1); // matriz identidade;
    GLint modelLoc = glGetUniformLocation(shaderID, "model");
    //
    model = glm::rotate(model, /*(GLfloat)glfwGetTime()*/ glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    GLuint viewLoc = glGetUniformLocation(shaderID, "view");
    GLuint projLoc = glGetUniformLocation(shaderID, "proj");

    glEnable(GL_DEPTH_TEST);

    SceneData scene = loadScene("../scene.json");

    std::vector<SceneObject> sceneObjects = scene.objects;
    sceneObjectsPtr = &sceneObjects;

    cameraPos = scene.camera.position;
    cameraFront = scene.camera.front;
    cameraUp = scene.camera.up;

    for (int i = 0; i < scene.lights.size() && i < 3; i++)
    {
        lights[i].position = scene.lights[i].position;
        lights[i].color = scene.lights[i].color;
        lights[i].intensity = scene.lights[i].intensity;
        lights[i].enabled = scene.lights[i].enabled;
    }

    // Loop da aplicação - "game loop"
    while (!glfwWindowShouldClose(window))
    {
        // Checa se houveram eventos de input (key pressed, mouse moved etc.) e chama as funções de callback correspondentes
        glfwPollEvents();

        // Limpa o buffer de cor
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // cor de fundo
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glLineWidth(10);
        glPointSize(20);

        glm::mat4 view = glm::lookAt(
            cameraPos,
            cameraPos + cameraFront,
            cameraUp);

        glm::mat4 projection = glm::perspective(
            glm::radians(45.0f),
            (float)WIDTH / HEIGHT,
            0.1f,
            100.0f);

        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        glUniform3fv(
            glGetUniformLocation(shaderID, "viewPos"),
            1,
            glm::value_ptr(cameraPos));

        glUniform1f(glGetUniformLocation(shaderID, "Kc"), 1.0f);
        glUniform1f(glGetUniformLocation(shaderID, "Kl"), 0.09f);
        glUniform1f(glGetUniformLocation(shaderID, "Kq"), 0.032f);

        for (int i = 0; i < 3; i++)
        {
            std::string idx = std::to_string(i);

            glUniform3fv(
                glGetUniformLocation(shaderID, ("lights[" + idx + "].position").c_str()),
                1,
                glm::value_ptr(lights[i].position));

            glUniform3fv(
                glGetUniformLocation(shaderID, ("lights[" + idx + "].color").c_str()),
                1,
                glm::value_ptr(lights[i].color));

            glUniform1f(
                glGetUniformLocation(shaderID, ("lights[" + idx + "].intensity").c_str()),
                lights[i].intensity);

            glUniform1i(
                glGetUniformLocation(shaderID, ("lights[" + idx + "].enabled").c_str()),
                lights[i].enabled);
        }

        for (int i = 0; i < sceneObjects.size(); i++)
        {
            SceneObject &object = sceneObjects[i];
            bool isSelected = i == selectedObjectIndex;

            float time = glfwGetTime();

            updateObjectAnimation(object, time);

            glm::mat4 model = glm::mat4(1.0f);

            model = glm::translate(model, object.position);
            model = glm::rotate(model, glm::radians(object.rotation.x), glm::vec3(1, 0, 0));
            model = glm::rotate(model, glm::radians(object.rotation.y), glm::vec3(0, 1, 0));
            model = glm::rotate(model, glm::radians(object.rotation.z), glm::vec3(0, 0, 1));
            model = glm::scale(model, object.scale);

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

            // desenho normal
            for (auto &submesh : object.mesh.submeshes)
            {
                glUniform1f(
                    glGetUniformLocation(shaderID, "Ka"),
                    submesh.material.Ka.x);

                glUniform1f(
                    glGetUniformLocation(shaderID, "Kd"),
                    submesh.material.Kd.x);

                glUniform1f(
                    glGetUniformLocation(shaderID, "Ks"),
                    submesh.material.Ks.x);

                glUniform1f(
                    glGetUniformLocation(shaderID, "shininess"),
                    submesh.material.shininess);

                glUniform3fv(
                    glGetUniformLocation(shaderID, "objectColor"),
                    1,
                    glm::value_ptr(submesh.material.Kd));

                glUniform1i(
                    glGetUniformLocation(shaderID, "useTexture"),
                    submesh.material.textureId != 0);

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, submesh.material.textureId);

                glUniform1i(
                    glGetUniformLocation(shaderID, "diffuseTexture"),
                    0);

                glBindVertexArray(submesh.VAO);

                glDrawArrays(
                    GL_TRIANGLES,
                    0,
                    submesh.nVertices);
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
    // Pede pra OpenGL desalocar os buffers
    // glDeleteVertexArrays(1, &mesh.VAO);
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
        if (sceneObjectsPtr && !sceneObjectsPtr->empty())
        {
            selectedObjectIndex =
                (selectedObjectIndex + 1) % sceneObjectsPtr->size();

            std::cout << "Objeto selecionado: "
                      << selectedObjectIndex
                      << std::endl;
        }
    }

    // ================= SELEÇÃO DE LUZ =================
    if (key == GLFW_KEY_1 && action == GLFW_PRESS)
        selectedLight = KEY_LIGHT;

    if (key == GLFW_KEY_2 && action == GLFW_PRESS)
        selectedLight = FILL_LIGHT;

    if (key == GLFW_KEY_3 && action == GLFW_PRESS)
        selectedLight = BACK_LIGHT;

    if (key == GLFW_KEY_L && action == GLFW_PRESS)
        lights[selectedLight].enabled = !lights[selectedLight].enabled;

    if (!(action == GLFW_PRESS || action == GLFW_REPEAT))
        return;

    // ================= CÂMERA =================
    if (key == GLFW_KEY_W)
        cameraPos += cameraSpeed * cameraFront;

    if (key == GLFW_KEY_S)
        cameraPos -= cameraSpeed * cameraFront;

    if (key == GLFW_KEY_A)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

    if (key == GLFW_KEY_D)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

    if (key == GLFW_KEY_Q)
        cameraPos.y += cameraSpeed;

    if (key == GLFW_KEY_E)
        cameraPos.y -= cameraSpeed;

    // ================= LUZ SELECIONADA =================
    if (key == GLFW_KEY_EQUAL)
        lights[selectedLight].intensity += 0.1f;

    if (key == GLFW_KEY_MINUS)
        lights[selectedLight].intensity -= 0.1f;

    if (lights[selectedLight].intensity < 0.0f)
        lights[selectedLight].intensity = 0.0f;

    if (key == GLFW_KEY_UP)
        lights[selectedLight].position.y += 0.1f;

    if (key == GLFW_KEY_DOWN)
        lights[selectedLight].position.y -= 0.1f;

    if (key == GLFW_KEY_LEFT)
        lights[selectedLight].position.x -= 0.1f;

    if (key == GLFW_KEY_RIGHT)
        lights[selectedLight].position.x += 0.1f;

    if (key == GLFW_KEY_PAGE_UP)
        lights[selectedLight].position.z += 0.1f;

    if (key == GLFW_KEY_PAGE_DOWN)
        lights[selectedLight].position.z -= 0.1f;

    // ================= OBJETO SELECIONADO =================
    if (sceneObjectsPtr && !sceneObjectsPtr->empty())
    {
        SceneObject &selectedObject =
            (*sceneObjectsPtr)[selectedObjectIndex];

        // translação do objeto
        if (key == GLFW_KEY_I)
            selectedObject.position.y += 0.1f;

        if (key == GLFW_KEY_K)
            selectedObject.position.y -= 0.1f;

        if (key == GLFW_KEY_J)
            selectedObject.position.x -= 0.1f;

        if (key == GLFW_KEY_U)
            selectedObject.position.x += 0.1f;

        if (key == GLFW_KEY_O)
            selectedObject.position.z += 0.1f;

        if (key == GLFW_KEY_P)
            selectedObject.position.z -= 0.1f;

        // escala uniforme
        if (key == GLFW_KEY_RIGHT_BRACKET)
            selectedObject.scale += glm::vec3(0.05f);

        if (key == GLFW_KEY_LEFT_BRACKET)
            selectedObject.scale -= glm::vec3(0.05f);

        if (selectedObject.scale.x < 0.05f)
            selectedObject.scale = glm::vec3(0.05f);

        // rotação do objeto
        if (key == GLFW_KEY_X)
            selectedObject.rotation.x += 5.0f;

        if (key == GLFW_KEY_Y)
            selectedObject.rotation.y += 5.0f;

        if (key == GLFW_KEY_Z)
            selectedObject.rotation.z += 5.0f;
    }
}

// Esta função está basntante hardcoded - objetivo é compilar e "buildar" um programa de
//  shader simples e único neste exemplo de código
//  O código fonte do vertex e fragment shader está nos arrays vertexShaderSource e
//  fragmentShader source no iniçio deste arquivo
//  A função retorna o identificador do programa de shader

// Esta função está bastante harcoded - objetivo é criar os buffers que armazenam a
// geometria de um triângulo
// Apenas atributo coordenada nos vértices
// 1 VBO com as coordenadas, VAO com apenas 1 ponteiro para atributo
// A função retorna o identificador do VAO

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

    Shader_programm = glCreateProgram();
    glAttachShader(Shader_programm, vs);
    glAttachShader(Shader_programm, fs);
    glLinkProgram(Shader_programm);

    glDeleteShader(vs);
    glDeleteShader(fs);
}

void drawBoundingBox(
    const BoundingBox &bbox,
    const glm::mat4 &model,
    GLint modelLoc,
    GLuint shaderID)
{
    glm::vec3 min = bbox.min;
    glm::vec3 max = bbox.max;

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
        float angle = time * object.animation.speed;

        object.position.x =
            object.animation.center.x +
            cos(angle) * object.animation.radius;

        object.position.z =
            object.animation.center.z +
            sin(angle) * object.animation.radius;

        object.position.y =
            object.animation.center.y;
    }
}