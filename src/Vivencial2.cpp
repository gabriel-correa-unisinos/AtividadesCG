// Câmera e Iluminação básica
//
// Este código serve como base para toda a disciplina.
// Ele implementa:
// - OpenGL moderno (pipeline programável)
// - Um modelo geométrico simples (cubo)
// - Transformações de modelo, visualização (câmera) e projeção
// - Uma câmera no estilo FPS (yaw + pitch)
//
// A partir deste exemplo, novos conceitos serão adicionados gradualmente
// (iluminação, materiais, texturas, visibilidade, sombras, etc.)

#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <cmath>

using namespace std;

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

Light lights[3];

float radius = 0.5f;

GLFWwindow *Window = nullptr;
GLuint Shader_programm = 0;
GLuint Vao_cubo = 0;
GLuint Vao_esfera = 0;
int Num_vertices_esfera = 0; // Precisamos guardar a quantidade de vértices gerados

int WIDTH = 800;
int HEIGHT = 600;

float Tempo_entre_frames = 0.0f;

// -----------------------------
// Parâmetros da câmera virtual
// -----------------------------

float Cam_speed = 10.0f;
float Cam_yaw_speed = 30.0f;
glm::vec3 Cam_pos(0.0f, 0.0f, 2.0f);
float Cam_yaw = -90.0f;
float Cam_pitch = 0.0f;

double lastX = WIDTH / 2.0;
double lastY = HEIGHT / 2.0;
bool primeiro_mouse = true;

// -----------------------------
// Callbacks de janela e entrada
// -----------------------------

void redimensionaCallback(GLFWwindow *window, int w, int h)
{
    WIDTH = w;
    HEIGHT = h;
    glViewport(0, 0, w, h);
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    if (primeiro_mouse)
    {
        lastX = xpos;
        lastY = ypos;
        primeiro_mouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensibilidade = 0.1f;
    xoffset *= sensibilidade;
    yoffset *= sensibilidade;

    Cam_yaw += xoffset;
    Cam_pitch += yoffset;

    if (Cam_pitch > 89.0f)
        Cam_pitch = 89.0f;
    if (Cam_pitch < -89.0f)
        Cam_pitch = -89.0f;
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
}

// -----------------------------
// Inicialização do OpenGL
// -----------------------------

void inicializaOpenGL()
{
    glfwInit();

    Window = glfwCreateWindow(WIDTH, HEIGHT, "Exemplo Base - CG em Tempo Real", nullptr, nullptr);
    glfwMakeContextCurrent(Window);

    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    glfwSetFramebufferSizeCallback(Window, redimensionaCallback);
    glfwSetCursorPosCallback(Window, mouse_callback);
    glfwSetKeyCallback(Window, key_callback);
    glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    cout << "Placa de vídeo: " << glGetString(GL_RENDERER) << endl;
    cout << "Versão do OpenGL: " << glGetString(GL_VERSION) << endl;
}

// -----------------------------
// Inicialização da geometria
// -----------------------------

void inicializaCubo()
{
    float points[] = {
        // posição              // normal

        // Frente (0,0,1)
        0.5, 0.5, 0.5, 0, 0, 1,
        0.5, -0.5, 0.5, 0, 0, 1,
        -0.5, -0.5, 0.5, 0, 0, 1,
        0.5, 0.5, 0.5, 0, 0, 1,
        -0.5, -0.5, 0.5, 0, 0, 1,
        -0.5, 0.5, 0.5, 0, 0, 1,

        // Trás (0,0,-1)
        0.5, 0.5, -0.5, 0, 0, -1,
        0.5, -0.5, -0.5, 0, 0, -1,
        -0.5, -0.5, -0.5, 0, 0, -1,
        0.5, 0.5, -0.5, 0, 0, -1,
        -0.5, -0.5, -0.5, 0, 0, -1,
        -0.5, 0.5, -0.5, 0, 0, -1,

        // Esquerda (-1,0,0)
        -0.5, -0.5, 0.5, -1, 0, 0,
        -0.5, 0.5, 0.5, -1, 0, 0,
        -0.5, -0.5, -0.5, -1, 0, 0,
        -0.5, -0.5, -0.5, -1, 0, 0,
        -0.5, 0.5, -0.5, -1, 0, 0,
        -0.5, 0.5, 0.5, -1, 0, 0,

        // Direita (1,0,0)
        0.5, -0.5, 0.5, 1, 0, 0,
        0.5, 0.5, 0.5, 1, 0, 0,
        0.5, -0.5, -0.5, 1, 0, 0,
        0.5, -0.5, -0.5, 1, 0, 0,
        0.5, 0.5, -0.5, 1, 0, 0,
        0.5, 0.5, 0.5, 1, 0, 0,

        // Baixo (0,-1,0)
        -0.5, -0.5, 0.5, 0, -1, 0,
        0.5, -0.5, 0.5, 0, -1, 0,
        0.5, -0.5, -0.5, 0, -1, 0,
        0.5, -0.5, -0.5, 0, -1, 0,
        -0.5, -0.5, -0.5, 0, -1, 0,
        -0.5, -0.5, 0.5, 0, -1, 0,

        // Cima (0,1,0)
        -0.5, 0.5, 0.5, 0, 1, 0,
        0.5, 0.5, 0.5, 0, 1, 0,
        0.5, 0.5, -0.5, 0, 1, 0,
        0.5, 0.5, -0.5, 0, 1, 0,
        -0.5, 0.5, -0.5, 0, 1, 0,
        -0.5, 0.5, 0.5, 0, 1, 0};
    GLuint VBO;
    glGenVertexArrays(1, &Vao_cubo);
    glGenBuffers(1, &VBO);

    glBindVertexArray(Vao_cubo);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

void inicializaEsfera()
{
    std::vector<float> points;

    // Resolução da esfera (quanto maior, mais redonda, porém mais pesada)
    int stacks = 30;  // Fatias horizontais (latitude)
    int sectors = 30; // Fatias verticais (longitude)
    float radius = 0.5f;
    const float PI = 3.14159265359f;

    for (int i = 0; i < stacks; ++i)
    {
        float phi1 = PI * float(i) / stacks;
        float phi2 = PI * float(i + 1) / stacks;

        for (int j = 0; j < sectors; ++j)
        {
            float theta1 = 2.0f * PI * float(j) / sectors;
            float theta2 = 2.0f * PI * float(j + 1) / sectors;

            // Função auxiliar (lambda) para calcular e adicionar um vértice e sua normal
            auto addVertex = [&](float p, float t)
            {
                // Coordenadas esféricas para cartesianas
                float x = radius * sin(p) * cos(t);
                float y = radius * cos(p);
                float z = radius * sin(p) * sin(t);

                // 1. Adiciona a Posição
                points.push_back(x);
                points.push_back(y);
                points.push_back(z);

                // 2. Adiciona a Normal
                // Para uma esfera centrada na origem (0,0,0), a direção da normal
                // é exatamente a posição do vértice dividida pelo raio (normalizada).
                points.push_back(x / radius);
                points.push_back(y / radius);
                points.push_back(z / radius);
            };

            // Um "quadrado" na superfície da esfera é formado por 2 triângulos.
            // Triângulo 1
            addVertex(phi1, theta1);
            addVertex(phi2, theta1);
            addVertex(phi1, theta2);

            // Triângulo 2
            addVertex(phi1, theta2);
            addVertex(phi2, theta1);
            addVertex(phi2, theta2);
        }
    }

    // Calcula quantos vértices reais foram gerados (cada vértice tem 6 floats: 3 pos + 3 norm)
    Num_vertices_esfera = points.size() / 6;

    // A partir daqui, é a mesma lógica do seu cubo
    GLuint VBO;
    glGenVertexArrays(1, &Vao_esfera);
    glGenBuffers(1, &VBO);

    glBindVertexArray(Vao_esfera);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // Nota: Usamos points.data() para pegar o ponteiro do vector e points.size() para o tamanho
    glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(float), points.data(), GL_STATIC_DRAW);

    // Atributo 0: Posição
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // Atributo 1: Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

// -----------------------------
// Shaders
// -----------------------------

GLuint compilaShader(const char *source, GLenum type)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    return shader;
}

void inicializaShaders()
{
    const char *vertex_shader = R"(
        #version 450

        layout(location = 0) in vec3 vertex_posicao;
        layout(location = 1) in vec3 vertex_normal;

        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 proj;

        out vec3 fragPos;
        out vec3 normal;

        void main()
        {
            vec4 worldPos = model * vec4(vertex_posicao, 1.0);
            fragPos = worldPos.xyz;

            normal = mat3(transpose(inverse(model))) * vertex_normal;

            gl_Position = proj * view * worldPos;
        }
    )";

    const char *fragment_shader = R"(
        #version 450

in vec3 fragPos;
in vec3 normal;

out vec4 frag_colour;

uniform vec3 viewPos;
uniform vec3 objectColor;

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

    vec3 result=
        (ambient+diffuse)
        *objectColor
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

// -----------------------------
// Transformação de modelo
// -----------------------------

void transformacaoGenerica(float Tx, float Ty, float Tz,
                           float Sx, float Sy, float Sz,
                           float Rx, float Ry, float Rz)
{
    glm::mat4 transform(1.0f);

    transform = glm::translate(transform, glm::vec3(Tx, Ty, Tz));
    transform = glm::rotate(transform, glm::radians(Rz), glm::vec3(0, 0, 1));
    transform = glm::rotate(transform, glm::radians(Ry), glm::vec3(0, 1, 0));
    transform = glm::rotate(transform, glm::radians(Rx), glm::vec3(1, 0, 0));
    transform = glm::scale(transform, glm::vec3(Sx, Sy, Sz));

    GLuint loc = glGetUniformLocation(Shader_programm, "model");
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(transform));
}

// -----------------------------
// Câmera (matriz de visualização)
// -----------------------------

void especificaMatrizVisualizacao()
{
    glm::vec3 front;
    front.x = cos(glm::radians(Cam_yaw)) * cos(glm::radians(Cam_pitch));
    front.y = sin(glm::radians(Cam_pitch));
    front.z = sin(glm::radians(Cam_yaw)) * cos(glm::radians(Cam_pitch));
    front = glm::normalize(front);

    glm::mat4 view = glm::lookAt(Cam_pos, Cam_pos + front, glm::vec3(0, 1, 0));

    GLuint loc = glGetUniformLocation(Shader_programm, "view");
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(view));
}

// -----------------------------
// Projeção
// -----------------------------

void especificaMatrizProjecao()
{
    glm::mat4 proj = glm::perspective(glm::radians(67.0f),
                                      (float)WIDTH / HEIGHT,
                                      0.1f, 100.0f);

    GLuint loc = glGetUniformLocation(Shader_programm, "proj");
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(proj));
}

void inicializaCamera()
{
    especificaMatrizVisualizacao();
    especificaMatrizProjecao();
}

// -----------------------------
// Entrada de teclado
// -----------------------------

void trataTeclado()
{
    float velocidade = Cam_speed * Tempo_entre_frames;
    static bool key1Pressed = false;
    static bool key2Pressed = false;
    static bool key3Pressed = false;

    glm::vec3 frente;
    frente.x = cos(glm::radians(Cam_yaw)) * cos(glm::radians(Cam_pitch));
    frente.y = sin(glm::radians(Cam_pitch));
    frente.z = sin(glm::radians(Cam_yaw)) * cos(glm::radians(Cam_pitch));
    frente = glm::normalize(frente);

    glm::vec3 direita = glm::normalize(glm::cross(frente, glm::vec3(0, 1, 0)));

    if (glfwGetKey(Window, GLFW_KEY_W) == GLFW_PRESS)
        Cam_pos += frente * velocidade;
    if (glfwGetKey(Window, GLFW_KEY_S) == GLFW_PRESS)
        Cam_pos -= frente * velocidade;
    if (glfwGetKey(Window, GLFW_KEY_A) == GLFW_PRESS)
        Cam_pos -= direita * velocidade;
    if (glfwGetKey(Window, GLFW_KEY_D) == GLFW_PRESS)
        Cam_pos += direita * velocidade;
    if (glfwGetKey(Window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(Window, true);

    if (glfwGetKey(Window, GLFW_KEY_1) == GLFW_PRESS)
    {
        if (!key1Pressed)
        {
            lights[KEY_LIGHT].enabled =
                !lights[KEY_LIGHT].enabled;

            key1Pressed = true;
        }
    }
    else
    {
        key1Pressed = false;
    }

    // Luz preenchimento
    if (glfwGetKey(Window, GLFW_KEY_2) == GLFW_PRESS)
    {
        if (!key2Pressed)
        {
            lights[FILL_LIGHT].enabled =
                !lights[FILL_LIGHT].enabled;

            key2Pressed = true;
        }
    }
    else
    {
        key2Pressed = false;
    }

    // Luz de fundo
    if (glfwGetKey(Window, GLFW_KEY_3) == GLFW_PRESS)
    {
        if (!key3Pressed)
        {
            lights[BACK_LIGHT].enabled =
                !lights[BACK_LIGHT].enabled;

            key3Pressed = true;
        }
    }
    else
    {
        key3Pressed = false;
    }
}

// -----------------------------
// Renderização
// -----------------------------

void defineMaterial(float r, float g, float b,
                    float ka, float kd, float ks,
                    float shininess)
{
    // cout << ks << endl;
    glUniform3f(glGetUniformLocation(Shader_programm, "objectColor"),
                r, g, b);

    glUniform1f(glGetUniformLocation(Shader_programm, "Ka"), ka);
    glUniform1f(glGetUniformLocation(Shader_programm, "Kd"), kd);
    glUniform1f(glGetUniformLocation(Shader_programm, "Ks"), ks);
    glUniform1f(glGetUniformLocation(Shader_programm, "shininess"), shininess);
}

void inicializaRenderizacao()
{
    float tempo_anterior = glfwGetTime();

    glEnable(GL_DEPTH_TEST);

    while (!glfwWindowShouldClose(Window))
    {
        float tempo_atual = glfwGetTime();
        Tempo_entre_frames = tempo_atual - tempo_anterior;
        tempo_anterior = tempo_atual;

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(Shader_programm);
        inicializaCamera();

        glBindVertexArray(Vao_cubo);

        for (int i = 0; i < 3; i++)
        {
            std::string idx =
                std::to_string(i);

            glUniform3fv(
                glGetUniformLocation(
                    Shader_programm,
                    ("lights[" + idx + "].position").c_str()),
                1,
                glm::value_ptr(
                    lights[i].position));

            glUniform3fv(
                glGetUniformLocation(
                    Shader_programm,
                    ("lights[" + idx + "].color").c_str()),
                1,
                glm::value_ptr(
                    lights[i].color));

            glUniform1f(
                glGetUniformLocation(
                    Shader_programm,
                    ("lights[" + idx + "].intensity").c_str()),
                lights[i].intensity);

            glUniform1i(
                glGetUniformLocation(
                    Shader_programm,
                    ("lights[" + idx + "].enabled").c_str()),
                lights[i].enabled);
        }

        // Configuração da Atenuação ---
        // Estes valores representam uma luz que cobre uma distância de cerca de 50 unidades.
        // Você pode ajustá-los para fazer a luz ir mais ou menos longe.
        glUniform1f(glGetUniformLocation(Shader_programm, "Kc"), 1.0f);   // Constante
        glUniform1f(glGetUniformLocation(Shader_programm, "Kl"), 0.09f);  // Linear
        glUniform1f(glGetUniformLocation(Shader_programm, "Kq"), 0.032f); // Quadrática

        defineMaterial(
            1.0f, 0.6f, 0.2f, // cor base
            0.1f,             // Ka
            0.7f,             // Kd
            1.0f,             // Ks
            32.0f             // shininess
        );
        // transformacaoGenerica(0, 0, 0, 1, 1, 1, 0, 0, 0);
        // glDrawArrays(GL_TRIANGLES, 0, 36);

        // 2. Desenha a Esfera
        glBindVertexArray(Vao_esfera);
        transformacaoGenerica(0, 0, 0, 1, 1, 1, 0, 0, 0);
        glDrawArrays(GL_TRIANGLES, 0, Num_vertices_esfera); // <-- Usa a variável dinâmica que criamos

        glfwSwapBuffers(Window);
        glfwPollEvents();
        trataTeclado();
    }

    glfwTerminate();
}

void inicializaIluminacao3Pontos(
    glm::vec3 center,
    float radius)
{
    float dist = radius * 4.0f;

    // Luz principal
    lights[KEY_LIGHT].position =
        center + glm::vec3(
                     dist,
                     dist,
                     dist);

    lights[KEY_LIGHT].color =
        glm::vec3(1.0f);

    lights[KEY_LIGHT].intensity = 1.0f;
    lights[KEY_LIGHT].enabled = true;

    // Luz preenchimento
    lights[FILL_LIGHT].position =
        center + glm::vec3(
                     -dist,
                     dist * 0.5f,
                     dist);

    lights[FILL_LIGHT].color =
        glm::vec3(1.0f);

    lights[FILL_LIGHT].intensity = 0.4f;
    lights[FILL_LIGHT].enabled = true;

    // Luz de fundo
    lights[BACK_LIGHT].position =
        center + glm::vec3(
                     0,
                     dist,
                     -dist);

    lights[BACK_LIGHT].color =
        glm::vec3(1.0f);

    lights[BACK_LIGHT].intensity = 0.8f;
    lights[BACK_LIGHT].enabled = true;
}

// -----------------------------
// Função principal
// -----------------------------

int main()
{
    inicializaOpenGL();
    // inicializaCubo();
    inicializaEsfera();
    inicializaShaders();
    inicializaIluminacao3Pontos(
        glm::vec3(0, 0, 0),
        0.5f);
    inicializaRenderizacao();
    return 0;
}