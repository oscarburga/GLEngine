#pragma once

#include <string>
// Figure out if we can forward declare the vec structs somehow to skip this include
#include <glm/glm.hpp>

class CShader
{
public:
    unsigned int id = 0;
    // constructor generates the shader on the fly
    // ------------------------------------------------------------------------
    CShader(const char* vertexPath, const char* fragmentPath);
    // activate the shader
    // ------------------------------------------------------------------------
    void use();
    // utility uniform functions
    // ------------------------------------------------------------------------
    void SetUniform(const std::string& name, bool value) const;
    void SetUniform(const std::string& name, int value) const;
    void SetUniform(const std::string& name, float value) const;
    void SetUniform(const std::string& name, const glm::vec2& value) const;
    void SetUniform(const std::string& name, const glm::vec3& value) const;
    void SetUniform(const std::string& name, const glm::vec4& value) const;
    void SetUniform(const std::string& name, const glm::mat2& value) const;
    void SetUniform(const std::string& name, const glm::mat3& value) const;
    void SetUniform(const std::string& name, const glm::mat4& value) const;

private:
    inline static char infoLog[1024] = {};
    static void CheckShaderCompilation(unsigned int shader, const char* shaderPath = nullptr);
    static void CheckProgramLinkSuccess(unsigned int program);
};

#define SetVarUniform(var) SetUniform(#var, var);
