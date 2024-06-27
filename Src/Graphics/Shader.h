#pragma once

#include <string>
// Figure out if we can forward declare the vec structs somehow to skip this include
#include <glm/glm.hpp>

class CShader
{
public:
    unsigned int Id = 0;
    // constructor generates the shader on the fly
    // ------------------------------------------------------------------------
    CShader(unsigned int id) : Id(id) {}
    // activate the shader
    // ------------------------------------------------------------------------
    void Use();
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
};

#define SetVarUniform(var) SetUniform(#var, var);
