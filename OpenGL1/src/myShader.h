#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/fwd.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader
{
public:
    // Constructor reads and builds the shader
    Shader(const GLchar* vertexPath, const GLchar* fragmentPath, const GLchar* geometryPath);
    Shader(const GLchar* vertexPath, const GLchar* fragmentPath);

    // The program ID
    GLuint ID;

    void use();
    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec3(const std::string& name, glm::vec3 value) const;
    void setMat4(const std::string& name, glm::mat4 value) const;
    void checkCompileErrors(unsigned int shader, std::string type);
};
#endif