#include "../include/glad/glad.h"
#include "../include/GLFW/glfw3.h"
#include "../include/glm/glm/glm.hpp"
#include "../include/glm/glm/gtc/matrix_transform.hpp"
#include "../include/glm/glm/gtc/type_ptr.hpp"
#include "../include/stb_image.h"

#include "../include/shader.h"
#include "../include/camera.h"
#include "../include/planet.h"

#include <iostream>
#include <vector>

// ===================== Globals =====================
unsigned int SCR_WIDTH = 1280, SCR_HEIGHT = 720;
Camera camera(glm::vec3(0.0f, 5.0f, 20.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// ===================== Callbacks =====================
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0,0,width,height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if(firstMouse){ lastX = xpos; lastY = ypos; firstMouse = false; }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

// ==================== Global vars for eclipse/lunar eclipse =====================
bool togglePressed = false;
bool eclipseMode = false;        // كسوف
float speedFactor = 1.0f;
bool isFrozen = false;
float frozenTime = 0.0f;

bool lunarTogglePressed = false;
bool lunarEclipseMode = false;   // خسوف
float frozenLunarTime = 0.0f;

// ===================== Input =====================
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

    // ===== Solar Eclipse (G) =====
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS && !togglePressed)
    {
        togglePressed = true;
        eclipseMode = true;
        lunarEclipseMode = false;
        isFrozen = false;
        speedFactor = 3.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_RELEASE)
        togglePressed = false;

    // ===== Lunar Eclipse (H) =====
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS && !lunarTogglePressed)
    {
        lunarTogglePressed = true;
        lunarEclipseMode = true;
        eclipseMode = false;
        isFrozen = false;
        speedFactor = 3.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_RELEASE)
        lunarTogglePressed = false;

    // ===== Exit Eclipse Modes (J) =====
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
    {
        eclipseMode = false;
        lunarEclipseMode = false;
        isFrozen = false;
        speedFactor = 1.0f;
        std::cout << "EXIT ECLIPSE / LUNAR ECLIPSE MODE\n";
    }
}

// ===================== Load Texture =====================
unsigned int loadTexture(const char* path)
{
    unsigned int textureID;
    glGenTextures(1,&textureID);

    int width,height,nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path,&width,&height,&nrChannels,0);
    if(data)
    {
        GLenum format = (nrChannels == 3) ? GL_RGB : GL_RGBA;
        glBindTexture(GL_TEXTURE_2D,textureID);
        glTexImage2D(GL_TEXTURE_2D,0,format,width,height,0,format,GL_UNSIGNED_BYTE,data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cerr << "Failed to load texture: " << path << std::endl;
        stbi_image_free(data);
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
    stbi_set_flip_vertically_on_load(false);

    for(unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if(data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load: " << faces[i] << std::endl;
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

// ===================== Eclipse Check =====================
bool isEclipse(const glm::vec3& sunPos, const glm::vec3& earthPos, const glm::vec3& moonPos)
{
    glm::vec3 SE = earthPos - sunPos;
    glm::vec3 SM = moonPos - sunPos;

    float distance = glm::length(glm::cross(SE, SM)) / glm::length(SE);

    bool inBetween =
        glm::dot(SE, SM) > 0 &&
        glm::length(SM) < glm::length(SE);

    return (distance < 0.5f && inBetween);
}

// ===================== Main =====================
int main()
{
    // --- GLFW Init ---
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH,SCR_HEIGHT,"SolarSystem",NULL,NULL);
    if(!window){ std::cout<<"Failed to create window\n"; return -1; }
    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){ std::cout<<"Failed to initialize GLAD\n"; return -1; }

    glEnable(GL_DEPTH_TEST);

    // --- Shaders ---
    Shader sunShader("shaders/emissive.vert","shaders/emissive.frag");
    Shader planetShader("shaders/lighting.vert","shaders/lighting.frag");
    Shader skyboxShader("shaders/skybox.vert","shaders/skybox.frag");

    // --- Textures ---
    unsigned int sunTex   = loadTexture("textures/sun.jpg");
    unsigned int earthTex = loadTexture("textures/earth.jpg");
    unsigned int moonTex  = loadTexture("textures/moon.jpg");

    std::vector<std::string> faces
    {
        "textures/skybox/right.jpg",
        "textures/skybox/left.jpg",
        "textures/skybox/top.jpg",
        "textures/skybox/bottom.jpg",
        "textures/skybox/front.jpg",
        "textures/skybox/back.jpg"
    };
    unsigned int cubemapTexture = loadCubemap(faces);

    // --- Skybox VAO/VBO ---
    float skyboxVertices[] = {
    // ----------- Back face -----------
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        // ----------- Left face -----------
        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        // ----------- Right face -----------
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        // ----------- Front face -----------
        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        // ----------- Top face -----------
        -1.0f,  1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        // ----------- Bottom face -----------
        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f,  1.0f,
        1.0f, -1.0f,  1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f
    };

    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);

    // --- Planets ---
    Planet sun(2.0f,64,64);
    Planet earth(0.5f,64,64);
    Planet moon(0.14f,32,32);

    // ===================== RENDER LOOP =====================
    while(!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.01f,0.01f,0.01f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),(float)SCR_WIDTH/SCR_HEIGHT,0.1f,100.0f);

    // ======================= Planet Movement  =======================
        float t;
        if(isFrozen)
            t = frozenTime;
        else
            t = currentFrame * speedFactor;

        // earth`s rotation
        float earthOrbitRadius = 10.0f;
        float earthSpeed = 1.0f;

        glm::vec3 earthPos = glm::vec3(
            earthOrbitRadius * cos(t * earthSpeed),
            0.0f,
            earthOrbitRadius * sin(t * earthSpeed)
        );
        glm::mat4 earthModel = glm::translate(glm::mat4(1.0f), earthPos);

        // moon`s rotation
        float moonOrbitRadius = 2.0f;
        float moonSpeed = 3.0f;

        glm::vec3 moonPos = earthPos + glm::vec3(
            moonOrbitRadius * cos(t * moonSpeed),
            0.0f,
            moonOrbitRadius * sin(t * moonSpeed)
        );
        glm::mat4 moonModel = glm::translate(glm::mat4(1.0f), moonPos);

        // ==================================================
        //                  الكسوف Solar Eclipse
        // ==================================================
        if (eclipseMode && !isFrozen)
        {
            glm::vec3 sunPos(0.0f, 0.0f, 0.0f);

            // Earth → Sun
            glm::vec3 ES = sunPos - earthPos;
            // Earth → Moon
            glm::vec3 EM = moonPos - earthPos;

            float distance = glm::length(glm::cross(ES, EM)) / glm::length(ES);

            bool moonBetween =
                glm::dot(ES, EM) > 0 &&
                glm::length(EM) < glm::length(ES);

            if (distance < 0.3f && moonBetween)
            {
                isFrozen = true;
                frozenTime = t;
                std::cout << "SOLAR ECLIPSE OCCURRED\n";
            }
        }

        // ==================================================
        //                  الخسوف Lunar Eclipse
        // ==================================================
        if (lunarEclipseMode && !isFrozen)
        {
            glm::vec3 sunPos(0.0f, 0.0f, 0.0f);

            // Sun → Earth
            glm::vec3 SE = earthPos - sunPos;
            // Sun → Moon
            glm::vec3 SM = moonPos - sunPos;

            float distance = glm::length(glm::cross(SE, SM)) / glm::length(SE);

            bool earthBetween =
                glm::dot(SE, SM) > 0 &&
                glm::length(SE) < glm::length(SM);

            if (distance < 0.3f && earthBetween)
            {
                isFrozen = true;
                frozenTime = t;
                std::cout << "LUNAR ECLIPSE OCCURRED\n";
            }
        }

        // ======================= draw planet =======================
        sunShader.use();
        sunShader.setMat4("view", view);
        sunShader.setMat4("projection", projection);
        sunShader.setMat4("model", glm::mat4(1.0f));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, sunTex);
        sun.draw();

        planetShader.use();
        planetShader.setInt("texture_diffuse1",0);
        planetShader.setVec3("lightPos",glm::vec3(0,0,0));
        planetShader.setVec3("viewPos",camera.Position);
        planetShader.setMat4("view", view);
        planetShader.setMat4("projection", projection);

        planetShader.setMat4("model", earthModel);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, earthTex);
        earth.draw();

        planetShader.setMat4("model", moonModel);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, moonTex);

        moon.draw();

        // ======================= Skybox =======================
        glDepthFunc(GL_LEQUAL);
        skyboxShader.use();
        glm::mat4 skyboxView = glm::mat4(glm::mat3(camera.GetViewMatrix()));
        skyboxShader.setMat4("view", skyboxView);
        skyboxShader.setMat4("projection", projection);
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES,0,36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
