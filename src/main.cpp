/**
 * @file main.cpp
 * @brief OpenGL Solar System Simulation
 */

#include "../include/glad/glad.h"
#include "../include/GLFW/glfw3.h"
#include "../include/glm/glm/glm.hpp"
#include "../include/glm/glm/gtc/matrix_transform.hpp"
#include "../include/glm/glm/gtc/type_ptr.hpp"
#include "../include/stb_image.h"

#include "../include/shader.h"
#include "../include/camera.h"
#include "../include/config.h"
#include "../include/planet.h"
#include "../include/scenario.h"

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <memory>
#include <optional>
#include <thread>
#include <chrono>

// ===================== Globals =====================
unsigned int SCR_WIDTH = 1280, SCR_HEIGHT = 720;
Camera camera;
float lastX = 640, lastY = 360;
bool firstMouse = true;
float deltaTime = 0.0f, lastFrame = 0.0f;

bool fullscreen = false, f11_pressed = false;
int last_window_x = 100, last_window_y = 100, last_window_width = 1280, last_window_height = 720;

float simulationSpeed = 1.0f;
float accumulatedSimTime = 0.0f;

CelestialBody* cameraLockedTo = nullptr;
std::string lockedBodyName = "None";
std::map<std::string, CelestialBody*> bodyMap;
std::vector<std::string> lockablePlanetNames;
int currentLockIndex = -1;
float lockedCameraDistance = 10.0f, lockedCameraOrbitYaw = -90.0f, lockedCameraOrbitPitch = 0.0f;

Config config;
float lastTimeForFPS = 0.0;
int nbFrames = 0;

// Skybox cube vertices
float skyboxVertices[] = {
    -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f
};

// ===================== Function Prototypes =====================
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
unsigned int loadTexture(const char* path);
unsigned int loadCubemap(std::vector<std::string> faces);
void lockCameraToBody(const std::string& name);

// ===================== GLFW Callbacks =====================
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = (float)xpos;
        lastY = (float)ypos;
        firstMouse = false;
    }

    float xoffset = (float)xpos - lastX;
    float yoffset = lastY - (float)ypos;
    lastX = (float)xpos;
    lastY = (float)ypos;

    if (!cameraLockedTo)
        camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (!cameraLockedTo)
        camera.ProcessMouseScroll((float)yoffset);
}

// ===================== Input =====================
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (!cameraLockedTo)
    {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera.ProcessKeyboard(FORWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera.ProcessKeyboard(BACKWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera.ProcessKeyboard(LEFT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera.ProcessKeyboard(RIGHT, deltaTime);
    }
}

// ===================== Key Callback =====================
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // Toggle fullscreen (F11)
    if (key == GLFW_KEY_F11 && action == GLFW_PRESS)
    {
        f11_pressed = true;
        fullscreen = !fullscreen;
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        if (fullscreen)
        {
            glfwGetWindowPos(window, &last_window_x, &last_window_y);
            glfwGetWindowSize(window, &last_window_width, &last_window_height);
            glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
        }
        else
        {
            glfwSetWindowMonitor(window, nullptr, last_window_x, last_window_y, last_window_width, last_window_height, mode->refreshRate);
        }
    }

    // Camera lock/unlock (Tab)
    if (key == GLFW_KEY_TAB && action == GLFW_PRESS)
    {
        if (!lockablePlanetNames.empty())
        {
            currentLockIndex = (currentLockIndex + 1) % lockablePlanetNames.size();
            lockCameraToBody(lockablePlanetNames[currentLockIndex]);
        }
    }

    // Simulation speed
    if (key == GLFW_KEY_UP && (action == GLFW_PRESS || action == GLFW_REPEAT))
        simulationSpeed *= 1.1f;
    if (key == GLFW_KEY_DOWN && (action == GLFW_PRESS || action == GLFW_REPEAT))
        simulationSpeed /= 1.1f;
}

// ===================== Lock Camera =====================
void lockCameraToBody(const std::string& name)
{
    auto it = bodyMap.find(name);
    if (it != bodyMap.end())
    {
        cameraLockedTo = it->second;
        lockedBodyName = name;
        lockedCameraDistance = cameraLockedTo->radius * 5.0f;
        camera.Zoom = ZOOM;

        glm::vec3 dir = glm::normalize(camera.Position - glm::vec3(cameraLockedTo->currentModelMatrix[3]));
        lockedCameraOrbitYaw = glm::degrees(atan2(dir.z, dir.x));
        lockedCameraOrbitPitch = std::clamp(static_cast<float>(glm::degrees(asin(dir.y))), -89.0f, 89.0f);

        auto lockIt = std::find(lockablePlanetNames.begin(), lockablePlanetNames.end(), name);
        currentLockIndex = (lockIt != lockablePlanetNames.end()) ? std::distance(lockablePlanetNames.begin(), lockIt) : -1;
    }
}

// ===================== Load Texture =====================
unsigned int loadTexture(const char* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);

    if (data)
    {
        std::cout << "Texture loaded successfully: " << path
                  << " (" << width << "x" << height
                  << ", channels: " << nrChannels << ")" << std::endl;

        GLenum format;
        if (nrChannels == 1)
            format = GL_RED;
        else if (nrChannels == 3)
            format = GL_RGB;
        else if (nrChannels == 4)
            format = GL_RGBA;
        else
        {
            std::cerr << "Unknown format for texture: " << path << std::endl;
            format = GL_RGB;
        }

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cerr << "ERROR: Failed to load texture at path: " << path << std::endl;
        std::cerr << "Working directory: ";

        #ifdef _WIN32
        system("cd");
        #else
        system("pwd");
        #endif

        glBindTexture(GL_TEXTURE_2D, textureID);
        unsigned char pinkData[] = {255, 105, 180, 255};
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, pinkData);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    return textureID;
}

// ===================== Load Cubemap =====================
unsigned int loadCubemap(std::vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            GLenum format = (nrChannels == 3) ? GL_RGB : GL_RGBA;
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cerr << "Cubemap texture failed to load at path: " << faces[i] << "\n";
            stbi_image_free(data);
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

// ===================== Main =====================
int main()
{
    // --- Load config ---
    config = loadConfig("config.ini");
    SCR_WIDTH = config.width;
    SCR_HEIGHT = config.height;
    fullscreen = config.startFullscreen;
    last_window_width = config.width;
    last_window_height = config.height;
    lastX = SCR_WIDTH / 2.0f;
    lastY = SCR_HEIGHT / 2.0f;

    // --- GLFW ---
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "SolarSystem", NULL, NULL);
    if (!window) { std::cout << "Failed to create window\n"; return -1; }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) { std::cout << "Failed to initialize GLAD\n"; return -1; }

    glEnable(GL_DEPTH_TEST);

    // --- Shaders ---
    Shader lightingShader("lighting.vert","lighting.frag");
    lightingShader.use();
    lightingShader.setInt("texture_diffuse1",0);

    // --- Load textures ---
    unsigned int sunTex = loadTexture("textures/sun.jpg");
    unsigned int earthTex = loadTexture("textures/earth.jpg");
    unsigned int moonTex = loadTexture("textures/moon.jpg");

    // --- Planets ---
    Planet sun(2.0f,64,64);
    Planet earth(0.5f,64,64);
    Planet moon(0.14f,32,32);

    glm::vec3 cameraPos(0.0f,5.0f,20.0f);
    glm::mat4 projection = glm::perspective(glm::radians(45.0f),(float)SCR_WIDTH/SCR_HEIGHT,0.1f,100.0f);

    while(!glfwWindowShouldClose(window))
    {
        glClearColor(0.01f,0.01f,0.01f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view = glm::lookAt(cameraPos, glm::vec3(0,0,0), glm::vec3(0,1,0));

        lightingShader.use();
        lightingShader.setVec3("lightPos", glm::vec3(0,0,0));
        lightingShader.setVec3("viewPos", cameraPos);
        lightingShader.setMat4("view", view);
        lightingShader.setMat4("projection", projection);

        // --- Sun ---
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, sunTex);
        glm::mat4 model = glm::mat4(1.0f);
        lightingShader.setMat4("model", model);
        sun.draw();

        // --- Earth ---
        glBindTexture(GL_TEXTURE_2D, earthTex);
        float t = (float)glfwGetTime();
        model = glm::translate(glm::mat4(1.0f), glm::vec3(10*cos(t),0,10*sin(t)));
        lightingShader.setMat4("model", model);
        earth.draw();

        // --- Moon ---
        glBindTexture(GL_TEXTURE_2D, moonTex);
        glm::vec3 earthPos = glm::vec3(10*cos(t),0,10*sin(t));
        model = glm::translate(glm::mat4(1.0f), earthPos + glm::vec3(1.0f*cos(t*2),0,1.0f*sin(t*2)));
        lightingShader.setMat4("model", model);
        moon.draw();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;

}
