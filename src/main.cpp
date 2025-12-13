#include "../include/glad/glad.h"
#include "../include/GLFW/glfw3.h"
#include "../include/glm/glm/glm.hpp"
#include "../include/glm/glm/gtc/matrix_transform.hpp"
#include "../include/glm/glm/gtc/type_ptr.hpp"
#include "../include/stb_image.h"

#include "../include/shader.h"
#include "../include/camera.h"
#include "../include/planet.h"
#include "scenario.h"

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
float earthSelfRotation = 0.0f;

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
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);

    // ===== Solar Eclipse (G) =====
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS && !togglePressed)
    {
        togglePressed = true;
        eclipseMode = true;
        lunarEclipseMode = false;
        isFrozen = false;
        speedFactor = 3.0f;

        // camera.Position = glm::vec3(0.0f);
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
    Shader orbitShader("shaders/orbit.vert", "shaders/orbit.frag");

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

    // --- Orbit Path for Earth ---
    std::vector<glm::vec3> earthOrbitVertices;
    int orbitSegments = 100;
    float earthOrbitRadius = 10.0f;

    for (int i = 0; i < orbitSegments; ++i)
    {
        float angle = 2.0f * glm::pi<float>() * i / orbitSegments;
        float x = earthOrbitRadius * cos(angle);
        float z = earthOrbitRadius * sin(angle);
        earthOrbitVertices.push_back(glm::vec3(x, 0.0f, z));
    }

    unsigned int orbitVAO, orbitVBO;
    glGenVertexArrays(1, &orbitVAO);
    glGenBuffers(1, &orbitVBO);

    glBindVertexArray(orbitVAO);
    glBindBuffer(GL_ARRAY_BUFFER, orbitVBO);
    glBufferData(GL_ARRAY_BUFFER, earthOrbitVertices.size() * sizeof(glm::vec3), &earthOrbitVertices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

    glBindVertexArray(0);



    // --- Orbit Path for Moon ---
    std::vector<glm::vec3> moonOrbitVertices;
    int moonOrbitSegments = 100;
    float moonOrbitRadius = 2.0f;

    for (int i = 0; i < moonOrbitSegments; ++i)
    {
        float angle = 2.0f * glm::pi<float>() * i / moonOrbitSegments;
        float x = moonOrbitRadius * cos(angle);
        float z = moonOrbitRadius * sin(angle);
        moonOrbitVertices.push_back(glm::vec3(x, 0.0f, z));
    }
    unsigned int moonOrbitVAO, moonOrbitVBO;
    glGenVertexArrays(1, &moonOrbitVAO);
    glGenBuffers(1, &moonOrbitVBO);

    glBindVertexArray(moonOrbitVAO);
    glBindBuffer(GL_ARRAY_BUFFER, moonOrbitVBO);
    glBufferData(GL_ARRAY_BUFFER, moonOrbitVertices.size() * sizeof(glm::vec3), &moonOrbitVertices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

    glBindVertexArray(0);

    // --- Planets ---
    Planet sun(2.0f,64,64);
    Planet earth(0.5f,64,64);
    Planet moon(0.14f,32,32);

    Scenario scenario = loadScenario_SolarSystemBasic();

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

        CelestialBody* moonBody = nullptr;
        for (auto& body : scenario.bodies) {
            if (body.name == "Moon") {
                moonBody = &body;
                break;
            }
        }

    // ======================= Planet Movement  =======================
        float t;
        if(isFrozen)
            t = frozenTime;
        else
            t = currentFrame * speedFactor;

        earthSelfRotation += deltaTime * 50.0f;
        float earthOrbitRadius = 10.0f;
        float earthSpeed = 1.0f;

        glm::vec3 sunPos = glm::vec3(0.0f , 0.0f , 0.0f);

        glm::vec3 earthPos = glm::vec3(
            earthOrbitRadius * cos(t * earthSpeed),
            0.0f,
            earthOrbitRadius * sin(t * earthSpeed)
        );
        glm::mat4 earthModel = glm::translate(glm::mat4(1.0f), earthPos);
        earthModel = glm::rotate(earthModel, glm::radians(earthSelfRotation),glm::vec3(0.0f, 1.0f, 0.0f));

        float moonOrbitRadius = 2.0f;
        float moonSpeed = 3.0f;

        glm::vec3 moonPos = earthPos + glm::vec3(
            moonOrbitRadius * cos(t * moonSpeed),
            0.0f,
            moonOrbitRadius * sin(t * moonSpeed)
        );
        glm::mat4 moonModel = glm::translate(glm::mat4(1.0f), moonPos);

        // =======================  moon size after eclipse  =======================
        float moonRadius = 0.135f;
        if (eclipseMode)
        {
            static float originalMoonRadius = 0.1f;

            glm::vec3 camPos = camera.Position;
            glm::vec3 toEarth = glm::normalize(earthPos - camPos);
            glm::vec3 toMoon = glm::normalize(moonPos - camPos);
            glm::vec3 toSun = glm::normalize(sunPos - camPos);

            float dotCameraMoon = glm::dot(camera.Front, toMoon);


            glm::vec3 earthToMoon = moonPos - earthPos;
            glm::vec3 earthToCam = camPos - earthPos;

            float projectionLength = glm::dot(earthToCam, glm::normalize(earthToMoon));

            float dotSunMoon = glm::dot(toSun, toMoon);

            float distCamToEarth = glm::length(earthPos - camPos);
            float distCamToMoon = glm::length(moonPos - camPos);
            float distEarthToMoon = glm::length(earthToMoon);

            bool isCameraBetween = (distCamToEarth < distEarthToMoon) && (projectionLength > 0);

            bool isMoonInFrontOfSun = (dotSunMoon > 0.95f);

            bool isLookingAtMoon = (dotCameraMoon > 0.7f);

            glm::vec3 camToEarthDir = glm::normalize(earthPos - camPos);
            float dotCamFrontToEarth = glm::dot(camera.Front, camToEarthDir);
            bool isEarthBehindCamera = (dotCamFrontToEarth < 0);

            if (isCameraBetween && isMoonInFrontOfSun && isLookingAtMoon && isEarthBehindCamera)
            {
                moonRadius = 0.6f;

                float distCamToMoon = glm::length(moonPos - camPos);
                float desiredDistance = distCamToMoon + 1000.0f;

                glm::vec3 camToMoonDir = glm::normalize(moonPos - camPos);
                moonPos = camPos + camToMoonDir * desiredDistance;

                glm::mat4 moonModel = glm::mat4(1.0f);
                moonModel = glm::translate(moonModel, moonPos);
                moonModel = glm::scale(moonModel, glm::vec3(moonRadius));
                planetShader.setMat4("model", moonModel);

                if (moonBody && moonBody->mesh) {
                    moonBody->mesh->draw();
                }

            }
            else
            {
                moonRadius = originalMoonRadius;
            }
        }
        if (moonBody && moonBody->mesh) {
            moonBody->mesh = std::make_unique<Planet>(moonRadius, 32, 32);
            planetShader.setMat4("model", moonModel);
            moonBody->mesh->draw();
            }

        bool earthInShadow = false;
        if(eclipseMode)
        {
            // Sun → Earth
            glm::vec3 SE = earthPos - sunPos;
            // Sun → Moon
            glm::vec3 SM = moonPos - sunPos;

            float distance = glm::length(glm::cross(SE, SM)) / glm::length(SE);

            bool earthBetween = glm::dot(SE, SM) > 0 && glm::length(SM) < glm::length(SE);

            if(distance < 0.5f && earthBetween)
            {
                earthInShadow = true;
                std::cout << "EARTH IN SHADOW (SOLAR ECLIPSE)\n";
            }
        }

        // ==================================================
        //                  الكسوف Solar Eclipse
        // ==================================================
        if (eclipseMode && !isFrozen)
        {
            glm::vec3 sunPos(0.0f, 0.0f, 0.0f);

            glm::vec3 ES = sunPos - earthPos;
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

        planetShader.use();
        planetShader.setBool("isShadowed", earthInShadow);
        planetShader.setVec3("lightPos", sunPos);
        planetShader.setVec3("viewPos", camera.Position);

        planetShader.setMat4("model", earthModel);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, earthTex);
        earth.draw();

        planetShader.setMat4("model", moonModel);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, moonTex);

        if (moonBody && moonBody->mesh) {
            planetShader.setMat4("model", moonModel);
            moonBody->mesh->draw();
        }

        // --- Draw Earth Orbit ---
        orbitShader.use();
        orbitShader.setMat4("view", view);
        orbitShader.setMat4("projection", projection);
        orbitShader.setVec3("color", glm::vec3(0.8f, 0.8f, 0.8f));

        glBindVertexArray(orbitVAO);
        glDrawArrays(GL_LINE_LOOP, 0, earthOrbitVertices.size());
        glBindVertexArray(0);


        orbitShader.use();
        orbitShader.setMat4("view", view);
        orbitShader.setMat4("projection", projection);
        orbitShader.setVec3("color", glm::vec3(0.5f, 0.5f, 1.0f));

        glBindVertexArray(moonOrbitVAO);
        std::vector<glm::vec3> transformedMoonVertices;
        transformedMoonVertices.reserve(moonOrbitVertices.size());

        for (auto& v : moonOrbitVertices)
            transformedMoonVertices.push_back(v + earthPos);

        glBindBuffer(GL_ARRAY_BUFFER, moonOrbitVBO);
        glBufferData(GL_ARRAY_BUFFER, transformedMoonVertices.size() * sizeof(glm::vec3), &transformedMoonVertices[0], GL_DYNAMIC_DRAW);

        glDrawArrays(GL_LINE_LOOP, 0, transformedMoonVertices.size());
        glBindVertexArray(0);

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
