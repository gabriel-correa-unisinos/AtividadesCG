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

struct Trajectory
{
    std::vector<glm::vec3> controlPoints;
    int currentPoint = 0;
    float speed = 1.5f;
    bool enabled = true;
};

struct SceneObject
{
    glm::vec3 basePosition;
    glm::vec3 currentPosition;

    float scale = 1.0f;

    Trajectory trajectory;
};

// Protótipo da função de callback de teclado
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);
void updateTrajectory(SceneObject &object, float deltaTime);
void drawTrajectory(const std::vector<glm::vec3> &points,
                    GLint modelLoc,
                    const glm::vec3 &globalPosition);

// Protótipos das funções
int setupShader();
int setupGeometry();

// Dimensões da janela (pode ser alterado em tempo de execução)
const GLuint WIDTH = 1920, HEIGHT = 1024;

// Código fonte do Vertex Shader (em GLSL): ainda hardcoded
const GLchar *vertexShaderSource = "#version 450\n"
                                   "layout (location = 0) in vec3 position;\n"
                                   "layout (location = 1) in vec3 normal;\n"
                                   "layout (location = 2) in vec2 texCoord;\n"
                                   "uniform mat4 model;\n"
                                   "uniform mat4 view;\n"
                                   "uniform mat4 projection;\n"
                                   "out vec3 FragNormal;\n"
                                   "out vec2 FragTexCoord;\n"
                                   "out vec4 finalColor;\n"
                                   "out vec2 vTexCoord;"
                                   "void main()\n"
                                   "{\n"
                                   //...pode ter mais linhas de código aqui!
                                   "gl_Position = projection * view * model * vec4(position, 1.0);\n"
                                   "FragNormal = mat3(transpose(inverse(model))) * normal;\n"
                                   "FragTexCoord = texCoord;\n"
                                   "vTexCoord = texCoord;\n"
                                   "}\0";

// Códifo fonte do Fragment Shader (em GLSL): ainda hardcoded
const GLchar *fragmentShaderSource = "#version 450\n"
                                     "in vec3 FragNormal;\n"
                                     "in vec2 FragTexCoord;\n"

                                     "uniform sampler2D texture1;\n"
                                     "uniform sampler2D diffuseTexture;\n"
                                     "out vec4 color;\n"
                                     "void main()\n"
                                     "{\n"
                                     "vec3 lightDir = normalize(vec3(1.0,1.0,1.0));\n"
                                     "float diffuse = max(dot(normalize(FragNormal),lightDir),0.2);\n"
                                     "vec4 texColor = texture(texture1,FragTexCoord);\n"
                                     "color = texColor * diffuse;\n"
                                     "}\n\0";

bool rotateX = false, rotateY = false, rotateZ = false;
glm::vec3 position(0.0f, 0.0f, -5.0f);
float scale = 1.0f;
std::vector<glm::vec3> cubePositions;

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

    Mesh mesh = loadSimpleOBJ("../assets/Modelos3D/Suzanne.obj");

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
    GLuint shaderID = setupShader();

    // Gerando um buffer simples, com a geometria de um triângulo

    glUseProgram(shaderID);

    glm::mat4 model = glm::mat4(1); // matriz identidade;
    GLint modelLoc = glGetUniformLocation(shaderID, "model");
    //
    model = glm::rotate(model, /*(GLfloat)glfwGetTime()*/ glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    GLuint viewLoc = glGetUniformLocation(shaderID, "view");
    GLuint projLoc = glGetUniformLocation(shaderID, "projection");

    glEnable(GL_DEPTH_TEST);

    mesh.submeshes[0].material.textureId = loadTexture("../assets/Modelos3D/Suzanne.png");

    std::vector<SceneObject> sceneObjects;

    SceneObject obj1;
    SceneObject obj2;

    obj1.trajectory.controlPoints.push_back(glm::vec3(-1.2f, -0.8f, 0.0f));
    obj1.trajectory.controlPoints.push_back(glm::vec3(1.2f, -0.8f, 0.0f));
    obj1.trajectory.controlPoints.push_back(glm::vec3(0.0f, 1.0f, 0.0f));

    obj1.currentPosition = obj1.trajectory.controlPoints[0];
    obj1.scale = 0.3f;

    obj2.trajectory.controlPoints.push_back(glm::vec3(3.0f, 1.5f, 0.0f));
    obj2.trajectory.controlPoints.push_back(glm::vec3(4.5f, 0.0f, 0.0f));
    obj2.trajectory.controlPoints.push_back(glm::vec3(3.0f, -1.5f, 0.0f));
    obj2.trajectory.controlPoints.push_back(glm::vec3(1.5f, 0.0f, 0.0f));

    obj2.currentPosition = obj2.trajectory.controlPoints[0];
    obj2.scale = 0.3f;
    obj2.trajectory.speed = 1.2f;

    sceneObjects.push_back(obj1);
    sceneObjects.push_back(obj2);
    float lastFrame = glfwGetTime();

    // Loop da aplicação - "game loop"
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        // Checa se houveram eventos de input (key pressed, mouse moved etc.) e chama as funções de callback correspondentes
        glfwPollEvents();

        // Limpa o buffer de cor
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // cor de fundo
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glLineWidth(10);
        glPointSize(20);

        float angle = (GLfloat)glfwGetTime();

        model = glm::mat4(1);
        model = glm::translate(model, position);
        if (rotateX)
        {
            model = glm::rotate(model, angle, glm::vec3(1.0f, 0.0f, 0.0f));
        }
        else if (rotateY)
        {
            model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));
        }
        else if (rotateZ)
        {
            model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f));
        }

        model = glm::scale(model, glm::vec3(scale));

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        glBindVertexArray(mesh.submeshes[0].VAO);
        glm::mat4 view = glm::translate(
            glm::mat4(1.0f),
            glm::vec3(0.0f, 0.0f, -3.0f) // "afasta" a câmera
        );

        glm::mat4 projection = glm::perspective(
            glm::radians(45.0f),
            (float)WIDTH / HEIGHT,
            0.1f,
            100.0f);

        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        for (auto &object : sceneObjects)
        {
            updateTrajectory(object, deltaTime);

            // 1. Desenha a trajetória
            drawTrajectory(object.trajectory.controlPoints, modelLoc, position);

            // 2. Agora redesenha o estado correto do objeto
            glBindVertexArray(mesh.submeshes[0].VAO);

            glm::mat4 model = glm::mat4(1.0f);

            model = glm::translate(model, object.currentPosition);
            model = glm::translate(model, position);

            if (rotateX)
                model = glm::rotate(model, angle, glm::vec3(1, 0, 0));
            else if (rotateY)
                model = glm::rotate(model, angle, glm::vec3(0, 1, 0));
            else if (rotateZ)
                model = glm::rotate(model, angle, glm::vec3(0, 0, 1));

            model = glm::scale(model, glm::vec3(object.scale));

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, mesh.submeshes[0].material.textureId);

            glUniform1i(
                glGetUniformLocation(shaderID, "diffuseTexture"),
                0);

            glDrawArrays(GL_TRIANGLES, 0, mesh.submeshes[0].nVertices);
        }
        glBindVertexArray(0);

        // Troca os buffers da tela
        glfwSwapBuffers(window);
    }
    // Pede pra OpenGL desalocar os buffers
    glDeleteVertexArrays(1, &mesh.submeshes[0].VAO);
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

    // ================= ROTAÇÃO =================
    if (key == GLFW_KEY_X && action == GLFW_PRESS)
    {
        rotateX = true;
        rotateY = false;
        rotateZ = false;
    }

    if (key == GLFW_KEY_Y && action == GLFW_PRESS)
    {
        rotateX = false;
        rotateY = true;
        rotateZ = false;
    }

    if (key == GLFW_KEY_Z && action == GLFW_PRESS)
    {
        rotateX = false;
        rotateY = false;
        rotateZ = true;
    }

    // ================= MOVIMENTO =================
    if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {
        // WASD → eixo X/Z
        if (key == GLFW_KEY_W)
            position.z -= 0.1f;
        if (key == GLFW_KEY_S)
            position.z += 0.1f;
        if (key == GLFW_KEY_A)
            position.x -= 0.1f;
        if (key == GLFW_KEY_D)
            position.x += 0.1f;

        // I/J → eixo Y
        if (key == GLFW_KEY_I)
            position.y += 0.1f;
        if (key == GLFW_KEY_J)
            position.y -= 0.1f;

        // ================= ESCALA =================
        if (key == GLFW_KEY_LEFT_BRACKET)
            scale -= 0.1f;
        if (key == GLFW_KEY_RIGHT_BRACKET)
            scale += 0.1f;

        // Evitar escala negativa ou zero
        if (scale < 0.1f)
            scale = 0.1f;
    }
}

// Esta função está basntante hardcoded - objetivo é compilar e "buildar" um programa de
//  shader simples e único neste exemplo de código
//  O código fonte do vertex e fragment shader está nos arrays vertexShaderSource e
//  fragmentShader source no iniçio deste arquivo
//  A função retorna o identificador do programa de shader
int setupShader()
{
    // Vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // Checando erros de compilação (exibição via log no terminal)
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    }
    // Fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // Checando erros de compilação (exibição via log no terminal)
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    }
    // Linkando os shaders e criando o identificador do programa de shader
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // Checando por erros de linkagem
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
                  << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

// Esta função está bastante harcoded - objetivo é criar os buffers que armazenam a
// geometria de um triângulo
// Apenas atributo coordenada nos vértices
// 1 VBO com as coordenadas, VAO com apenas 1 ponteiro para atributo
// A função retorna o identificador do VAO
int setupGeometry()
{
    // Aqui setamos as coordenadas x, y e z do triângulo e as armazenamos de forma
    // sequencial, já visando mandar para o VBO (Vertex Buffer Objects)
    // Cada atributo do vértice (coordenada, cores, coordenadas de textura, normal, etc)
    // Pode ser arazenado em um VBO único ou em VBOs separados
    GLfloat vertices[] = {
        // Cada linha: x, y, z, r, g, b

        // ===== FACE FRENTE =====
        -0.5,
        -0.5,
        0.5,
        1,
        0,
        0,
        0.5,
        -0.5,
        0.5,
        1,
        0,
        0,
        0.5,
        0.5,
        0.5,
        1,
        0,
        0,

        -0.5,
        -0.5,
        0.5,
        1,
        0,
        0,
        0.5,
        0.5,
        0.5,
        1,
        0,
        0,
        -0.5,
        0.5,
        0.5,
        1,
        0,
        0,

        // ===== FACE TRÁS =====
        -0.5,
        -0.5,
        -0.5,
        0,
        1,
        0,
        0.5,
        0.5,
        -0.5,
        0,
        1,
        0,
        0.5,
        -0.5,
        -0.5,
        0,
        1,
        0,

        -0.5,
        -0.5,
        -0.5,
        0,
        1,
        0,
        -0.5,
        0.5,
        -0.5,
        0,
        1,
        0,
        0.5,
        0.5,
        -0.5,
        0,
        1,
        0,

        // ===== ESQUERDA =====
        -0.5,
        -0.5,
        -0.5,
        0,
        0,
        1,
        -0.5,
        -0.5,
        0.5,
        0,
        0,
        1,
        -0.5,
        0.5,
        0.5,
        0,
        0,
        1,

        -0.5,
        -0.5,
        -0.5,
        0,
        0,
        1,
        -0.5,
        0.5,
        0.5,
        0,
        0,
        1,
        -0.5,
        0.5,
        -0.5,
        0,
        0,
        1,

        // ===== DIREITA =====
        0.5,
        -0.5,
        -0.5,
        1,
        1,
        0,
        0.5,
        0.5,
        0.5,
        1,
        1,
        0,
        0.5,
        -0.5,
        0.5,
        1,
        1,
        0,

        0.5,
        -0.5,
        -0.5,
        1,
        1,
        0,
        0.5,
        0.5,
        -0.5,
        1,
        1,
        0,
        0.5,
        0.5,
        0.5,
        1,
        1,
        0,

        // ===== TOPO =====
        -0.5,
        0.5,
        -0.5,
        1,
        0,
        1,
        -0.5,
        0.5,
        0.5,
        1,
        0,
        1,
        0.5,
        0.5,
        0.5,
        1,
        0,
        1,

        -0.5,
        0.5,
        -0.5,
        1,
        0,
        1,
        0.5,
        0.5,
        0.5,
        1,
        0,
        1,
        0.5,
        0.5,
        -0.5,
        1,
        0,
        1,

        // ===== BASE =====
        -0.5,
        -0.5,
        -0.5,
        0,
        1,
        1,
        0.5,
        -0.5,
        0.5,
        0,
        1,
        1,
        -0.5,
        -0.5,
        0.5,
        0,
        1,
        1,

        -0.5,
        -0.5,
        -0.5,
        0,
        1,
        1,
        0.5,
        -0.5,
        -0.5,
        0,
        1,
        1,
        0.5,
        -0.5,
        0.5,
        0,
        1,
        1,
    };

    GLuint VBO, VAO;

    // Geração do identificador do VBO
    glGenBuffers(1, &VBO);

    // Faz a conexão (vincula) do buffer como um buffer de array
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // Envia os dados do array de floats para o buffer da OpenGl
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Geração do identificador do VAO (Vertex Array Object)
    glGenVertexArrays(1, &VAO);

    // Vincula (bind) o VAO primeiro, e em seguida  conecta e seta o(s) buffer(s) de vértices
    // e os ponteiros para os atributos
    glBindVertexArray(VAO);

    // Para cada atributo do vertice, criamos um "AttribPointer" (ponteiro para o atributo), indicando:
    //  Localização no shader * (a localização dos atributos devem ser correspondentes no layout especificado no vertex shader)
    //  Numero de valores que o atributo tem (por ex, 3 coordenadas xyz)
    //  Tipo do dado
    //  Se está normalizado (entre zero e um)
    //  Tamanho em bytes
    //  Deslocamento a partir do byte zero

    // Atributo posição (x, y, z)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid *)0);
    glEnableVertexAttribArray(0);

    // Atributo cor (r, g, b)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid *)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    // Observe que isso é permitido, a chamada para glVertexAttribPointer registrou o VBO como o objeto de buffer de vértice
    // atualmente vinculado - para que depois possamos desvincular com segurança
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Desvincula o VAO (é uma boa prática desvincular qualquer buffer ou array para evitar bugs medonhos)
    glBindVertexArray(0);

    return VAO;
}

void updateTrajectory(SceneObject &object, float deltaTime)
{
    Trajectory &trajectory = object.trajectory;

    if (!trajectory.enabled || trajectory.controlPoints.size() < 2)
        return;

    glm::vec3 target = trajectory.controlPoints[trajectory.currentPoint];

    glm::vec3 direction = target - object.currentPosition;
    float distance = glm::length(direction);

    if (distance < 0.05f)
    {
        trajectory.currentPoint =
            (trajectory.currentPoint + 1) % trajectory.controlPoints.size();

        return;
    }

    direction = glm::normalize(direction);

    object.currentPosition += direction * trajectory.speed * deltaTime;
}

void drawTrajectory(
    const std::vector<glm::vec3> &points,
    GLint modelLoc,
    const glm::vec3 &globalPosition)
{
    if (points.size() < 2)
        return;

    GLuint vao, vbo;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(
        GL_ARRAY_BUFFER,
        points.size() * sizeof(glm::vec3),
        points.data(),
        GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)0);
    glEnableVertexAttribArray(0);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, globalPosition);

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    glLineWidth(3.0f);
    glDrawArrays(GL_LINE_LOOP, 0, points.size());

    glPointSize(8.0f);
    glDrawArrays(GL_POINTS, 0, points.size());

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
}
