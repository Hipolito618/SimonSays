#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Texture.h"
#include "Camera.h"
#include "ModelOBJ.h"

#include <iostream>

using namespace std;
using namespace glm;

// Dimensiones de ventana
unsigned int w = 800;
unsigned int h = 600;

// Identificadores de buffers y texturas
unsigned int VAO, lightCubeVAO, VBO;
unsigned int diffuseMap;
unsigned int specularMap;

// Instanciación de los 5 objetos ModelOBJ para la consola Simón (piezas independientes)
ModelOBJ base("Modelos/base.obj");
ModelOBJ botonRojo("Modelos/boton_rojo.obj");
ModelOBJ botonVerde("Modelos/boton_verde.obj");
ModelOBJ botonAmarillo("Modelos/boton_amarillo.obj");
ModelOBJ botonAzul("Modelos/boton_azul.obj");

// Profundidad de hundimiento físico al pulsar un botón (la altura del botón es 0.125 en el modelo)
const float desplazamientoPulsado = -0.06f;

// Color de fondo de la ventana (gris azulado pizarra moderno para alto contraste)
vec4 backgroundColor = vec4(0.18f, 0.20f, 0.24f, 1.0f);

// Parámetros de la cámara fija en perspectiva isométrica (Zoom y distancia bloqueados)
const float orbitRadius    = 0.36f;                    // Distancia fija al centro (sin zoom)
const float orbitElevation = glm::radians(42.0f);      // Inclinación isométrica fija (visión clara de los botones)
float orbitAzimuth         = glm::radians(90.0f);      // Ángulo horizontal (solo rotación / girar)

// Control de cámara enfocado de cerca en la consola (perspectiva cenital en primer plano)
Camera camera(vec3(0.0f, 0.24f, 0.27f));
float lastX = 800.0f / 2.0f;
float lastY = 600.0f / 2.0f;
bool firstMouse = true;

// Control de tiempo
float deltaTime = 0.0f;
float lastTime  = 0.0f;

// Posiciones de los cubos en la escena 3D
vec3 posCube[] = {
    vec3( 0.0f,  0.0f,   0.0f),
    vec3( 2.0f,  5.0f, -15.0f),
    vec3(-1.5f, -2.2f,  -2.5f),
    vec3(-3.8f, -2.0f, -12.3f),
    vec3( 2.4f, -0.4f,  -3.5f),
    vec3(-1.7f,  3.0f,  -7.5f),
    vec3( 1.3f, -2.0f,  -2.5f),
    vec3( 1.5f,  2.0f,  -2.5f),
    vec3( 1.5f,  0.2f,  -1.5f),
    vec3(-1.3f,  1.0f,  -1.5f)
};

// Posición del cubo fuente de luz independiente
vec3 posCubeLight[] = {
    vec3(1.2f, 1.0f, 2.0f)
};

// Geometría completa de un cubo 3D: Posición (3), Normales (3) y Coordenadas UV (2) -> 36 vértices
float vertices[] = {
    // Posición            // Normales           // UV (Textura)
    // Cara trasera (Normal: 0, 0, -1)
    -0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   0.0f, 0.0f,
     0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,   0.0f,  0.0f, -1.0f,   0.0f, 1.0f,

    // Cara frontal (Normal: 0, 0, 1)
    -0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,   0.0f, 0.0f,

    // Cara izquierda (Normal: -1, 0, 0)
    -0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,   1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,   0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,   1.0f, 0.0f,

    // Cara derecha (Normal: 1, 0, 0)
     0.5f,  0.5f,  0.5f,   1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
     0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
     0.5f,  0.5f, -0.5f,   1.0f,  0.0f,  0.0f,   1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
     0.5f,  0.5f,  0.5f,   1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,   1.0f,  0.0f,  0.0f,   0.0f, 0.0f,

    // Cara inferior (Normal: 0, -1, 0)
    -0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,   0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,   1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,   1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,   1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,   0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,   0.0f, 1.0f,

    // Cara superior (Normal: 0, 1, 0)
    -0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,   0.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,   0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,   1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,   0.0f,  1.0f,  0.0f,   1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,   1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,   0.0f,  1.0f,  0.0f,   0.0f, 1.0f
};

// Prototipos de función
void InitGLFWVersion();
bool GladLoad();
void Framebuffer_Size_Callback(GLFWwindow* window, int width, int height);
void Mouse_callback(GLFWwindow* window, double xpos, double ypos);
void Scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void ProcessInput(GLFWwindow* window);
void CameraInput(GLFWwindow* window);
void UpdateCameraOrbitalPosition();
void GeneracionBuffer();
void TransformCamera(Shader ourShader);
void TransformCubo(Shader ourShader, GLFWwindow* window);
void updateWindow(GLFWwindow* window, Shader ourShader, Shader ourLight, Texture outTexture);

// ============================================
// Función Principal (main)
// ============================================
int main()
{
    InitGLFWVersion();

    GLFWwindow* window = glfwCreateWindow(w, h, "Juego Simon Says", NULL, NULL);
    if (window == NULL)
    {
        cout << "Falla al crear ventana" << endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!GladLoad())
    {
        return -1;
    }

    // Inicializar posición y orientación orbital bloqueada al centro del modelo
    UpdateCameraOrbitalPosition();

    // Registro de callbacks de GLFW
    glfwSetFramebufferSizeCallback(window, Framebuffer_Size_Callback);
    glfwSetCursorPosCallback(window, Mouse_callback);
    glfwSetScrollCallback(window, Scroll_callback);

    // Ocultar y capturar el cursor para control FPS
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Habilitar la prueba de profundidad (Z-Buffer)
    glEnable(GL_DEPTH_TEST);

    // Carga y compilación de los shaders
    Shader ourShader("vertexShader.vs", "fragmentShader.fs");
    Shader ourLight("vertexLight.vl", "fragmentLight.fl");

    Texture outTexture;

    // Configuración de los buffers (VAO, lightCubeVAO, VBO)
    GeneracionBuffer();

    // Configuración de buffers OpenGL para los modelos OBJ
    base.setupBuffers();
    botonRojo.setupBuffers();
    botonVerde.setupBuffers();
    botonAmarillo.setupBuffers();
    botonAzul.setupBuffers();

    // Carga de la textura atlas compartida de Simón con filtrado nítido (Pixel Art / Nearest)
    unsigned int texturaSimon = outTexture.loadTextureID("Imagenes/textura_simon.png", 2);
    glBindTexture(GL_TEXTURE_2D, texturaSimon);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    diffuseMap  = texturaSimon;
    specularMap = diffuseMap;

    // Asignación de las unidades de textura a los samplers del material
    ourShader.use();
    ourShader.setInt("material.diffuse", 0);
    ourShader.setInt("material.specular", 1);

    // Bucle principal de renderizado
    updateWindow(window, ourShader, ourLight, outTexture);

    // Liberación de memoria y recursos de buffers
    glDeleteVertexArrays(1, &VAO);
    glDeleteVertexArrays(1, &lightCubeVAO);
    glDeleteBuffers(1, &VBO);

    glfwTerminate();
    return 0;
}

// Inicialización de la versión y perfil de GLFW
void InitGLFWVersion()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
}

// Inicialización del cargador de punteros OpenGL (GLAD)
bool GladLoad()
{
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        cout << "Falla al inicializar GLAD" << endl;
        return false;
    }
    return true;
}

// Callback de ajuste de dimensiones de la ventana
void Framebuffer_Size_Callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    w = width;
    h = height;
}

// Configuración de punteros de atributos y buffers
void GeneracionBuffer()
{
    glGenVertexArrays(1, &VAO);
    glGenVertexArrays(1, &lightCubeVAO);
    glGenBuffers(1, &VBO);

    // Enlazar VBO con los datos del cubo
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // 1. Configuración del VAO para los cubos principales (Posición, Normal, UV)
    glBindVertexArray(VAO);

    // Atributo 0: Posición (3 floats, stride = 8 * sizeof(float), offset = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Atributo 1: Normal (3 floats, stride = 8 * sizeof(float), offset = 3 * sizeof(float))
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Atributo 2: Coordenadas UV (2 floats, stride = 8 * sizeof(float), offset = 6 * sizeof(float))
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // 2. Configuración del lightCubeVAO para la lámpara (comparte el mismo VBO)
    glBindVertexArray(lightCubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // Atributo 0: Posición solamente (para el cubo de luz blanca)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

// Bucle de renderizado
void updateWindow(GLFWwindow* window, Shader ourShader, Shader ourLight, Texture outTexture)
{
    while (!glfwWindowShouldClose(window))
    {
        // Cálculo de delta time para movimientos independientes del framerate
        float currentTime = static_cast<float>(glfwGetTime());
        deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        // Procesar teclado
        ProcessInput(window);

        // Limpieza de buffers de color y profundidad (gris azulado pizarra moderno)
        glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Activar texturas difusa y especular
        outTexture.ViewTextureMap(0, diffuseMap);
        outTexture.ViewTextureMap(1, specularMap);

        // Activar el shader principal de los objetos
        ourShader.use();

        // Configuración de la linterna / Spotlight vinculada a la cámara (cono amplio para cubrir la consola de cerca)
        ourShader.setVec3("light.position", camera.Position);
        ourShader.setVec3("light.direction", camera.Front);
        ourShader.setFloat("light.cutoff", cos(radians(35.0f)));

        // Coeficientes de atenuación para distancia
        ourShader.setFloat("light.constant", 1.0f);
        ourShader.setFloat("light.lineal", 0.09f);
        ourShader.setFloat("light.quadratic", 0.032f);

        // Propiedades de la luz (luz ambiental y difusa suavizadas para resaltar brillo de botones)
        ourShader.setVec3("light.ambient", 0.55f, 0.55f, 0.55f);
        ourShader.setVec3("light.diffuse", 0.7f, 0.7f, 0.7f);
        ourShader.setVec3("light.specular", 0.4f, 0.4f, 0.4f);

        // Posición de la vista y brillo del material
        ourShader.setVec3("viewPos", camera.Position);
        ourShader.setFloat("material.shininess", 32.0f);

        // Configurar matrices de proyección y vista en ourShader
        TransformCamera(ourShader);

        // Renderizar los cubos en sus diferentes posiciones con rotación
        TransformCubo(ourShader, window);


        // Intercambio de buffers y consulta de eventos de GLFW
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

// Configuración de matrices Projection y View para la cámara (fijada al centro del modelo)
void TransformCamera(Shader ourShader)
{
    mat4 projection = perspective(radians(camera.Zoom), (float)w / (float)h, 0.01f, 100.0f);
    ourShader.setMat4("projection", projection);

    mat4 view = lookAt(camera.Position, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
    ourShader.setMat4("view", view);
}

// Renderizado ordenado de la consola Simón y sus 4 botones con animación e iluminación interactiva
void TransformCubo(Shader ourShader, GLFWwindow* window)
{
    // Posición centrada en el origen y escala uniforme (70-80% de la pantalla)
    mat4 modelBase = mat4(1.0f);
    modelBase = translate(modelBase, vec3(0.0f, 0.0f, 0.0f));
    modelBase = scale(modelBase, vec3(0.11f));

    // Asegurar que la textura atlas difusa y especular estén activas
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuseMap);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, specularMap);

    // 1. Objeto base.obj: No se mueve en nada (matriz estática) ni se ilumina (brillo estándar 1.0f)
    ourShader.setFloat("brillo", 1.0f);
    ourShader.setMat4("model", modelBase);
    base.draw();

    // 2. Botón Verde (Asignado a tecla 'Q')
    bool verdePresionado = (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS);
    mat4 modelVerde = verdePresionado ? translate(modelBase, vec3(0.0f, desplazamientoPulsado, 0.0f)) : modelBase;
    ourShader.setFloat("brillo", verdePresionado ? 2.4f : 0.45f);
    ourShader.setMat4("model", modelVerde);
    botonVerde.draw();

    // 3. Botón Rojo (Asignado a tecla 'W')
    bool rojoPresionado = (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS);
    mat4 modelRojo = rojoPresionado ? translate(modelBase, vec3(0.0f, desplazamientoPulsado, 0.0f)) : modelBase;
    ourShader.setFloat("brillo", rojoPresionado ? 2.4f : 0.45f);
    ourShader.setMat4("model", modelRojo);
    botonRojo.draw();

    // 4. Botón Amarillo (Asignado a tecla 'E')
    bool amarilloPresionado = (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS);
    mat4 modelAmarillo = amarilloPresionado ? translate(modelBase, vec3(0.0f, desplazamientoPulsado, 0.0f)) : modelBase;
    ourShader.setFloat("brillo", amarilloPresionado ? 2.4f : 0.45f);
    ourShader.setMat4("model", modelAmarillo);
    botonAmarillo.draw();

    // 5. Botón Azul (Asignado a tecla 'R')
    bool azulPresionado = (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS);
    mat4 modelAzul = azulPresionado ? translate(modelBase, vec3(0.0f, desplazamientoPulsado, 0.0f)) : modelBase;
    ourShader.setFloat("brillo", azulPresionado ? 2.4f : 0.45f);
    ourShader.setMat4("model", modelAzul);
    botonAzul.draw();
}

// Control de entrada de teclado general
void ProcessInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    CameraInput(window);
}

// Actualización de la posición orbital de la cámara basada en coordenadas esféricas
void UpdateCameraOrbitalPosition()
{
    camera.Position.x = orbitRadius * cos(orbitElevation) * cos(orbitAzimuth);
    camera.Position.y = orbitRadius * sin(orbitElevation);
    camera.Position.z = orbitRadius * cos(orbitElevation) * sin(orbitAzimuth);
    camera.Front      = normalize(vec3(0.0f, 0.0f, 0.0f) - camera.Position);
}

// Control de entrada de la cámara: Solo rotación horizontal (girar) manteniendo vista isométrica fija
void CameraInput(GLFWwindow* window)
{
    float orbitSpeed = 1.8f * deltaTime;

    // A y D / Flechas Izquierda y Derecha: Girar horizontalmente alrededor de la consola
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    {
        orbitAzimuth += orbitSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    {
        orbitAzimuth -= orbitSpeed;
    }

    UpdateCameraOrbitalPosition();
}

// Callback de movimiento del ratón: Solo girar horizontalmente (sin alterar la inclinación isométrica)
void Mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    lastX = xpos;
    lastY = ypos;

    // Solo girar horizontalmente (Azimuth)
    float mouseSens = 0.003f;
    orbitAzimuth -= xoffset * mouseSens;

    UpdateCameraOrbitalPosition();
}

// Callback de scroll del ratón: Bloqueado (sin zoom ni alejamiento)
void Scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    // Zoom bloqueado para mantener la perspectiva isométrica fija y consistente
}