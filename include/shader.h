#ifndef SHADER_H
#define SHADER_H
#include "glad/glad.h"
#include "glm/glm/glm.hpp"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader
{
public:
    unsigned int ID;
    Shader(const char *vertexPath, const char *fragmentPath);
    void use();

    /** @brief Sets a boolean uniform. */
    void setBool(const std::string &name, bool value) const;
    /** @brief Sets an integer uniform. */
    void setInt(const std::string &name, int value) const;
    /** @brief Sets a float uniform. */
    void setFloat(const std::string &name, float value) const;
    /** @brief Sets a vec3 uniform (using glm::vec3). */
    void setVec3(const std::string &name, const glm::vec3 &value) const;
    /** @brief Sets a vec3 uniform (using 3 float values). */
    void setVec3(const std::string &name, float x, float y, float z) const;
    /** @brief Sets a mat3 uniform (using glm::mat3). */
    void setMat3(const std::string &name, const glm::mat3 &mat) const;
    /** @brief Sets a mat4 uniform (using glm::mat4). */
    void setMat4(const std::string &name, const glm::mat4 &mat) const;

private:
    void checkCompileErrors(unsigned int shader, std::string type);
};
#endif
