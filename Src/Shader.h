#pragma once

#include <glad/glad.h>

#include <string>

class Shader
{
public:
    unsigned int id = 0;
    // constructor generates the shader on the fly
    // ------------------------------------------------------------------------
    Shader(const char* vertexPath, const char* fragmentPath);
    // activate the shader
    // ------------------------------------------------------------------------
    void use();
    // utility uniform functions
    // ------------------------------------------------------------------------
    void SetUniform(const std::string& name, bool value) const;
    void SetUniform(const std::string& name, int value) const;
    void SetUniform(const std::string& name, float value) const;

private:
    inline static char infoLog[1024] = {};
    static void CheckShaderCompilation(unsigned int shader, const char* shaderPath = nullptr);
    static void CheckProgramLinkSuccess(unsigned int program);
};
