#pragma once

#include <string>
// Figure out if we can forward declare the vec structs somehow to skip this include
#include <glm/glm.hpp>
#include "GlIdTypes.h"

class CGlShader
{
public:
    unsigned int Id = 0;
    // constructor generates the shader on the fly
    // ------------------------------------------------------------------------
    CGlShader(unsigned int id) : Id(id) {}
    // activate the shader
    // ------------------------------------------------------------------------
    void Use();
    // utility uniform functions
    // ------------------------------------------------------------------------
    void SetUniform(GlUniformLocs::Loc loc, bool value) const;
    void SetUniform(GlUniformLocs::Loc loc, int value) const;
    void SetUniform(GlUniformLocs::Loc loc, float value) const;
    void SetUniform(GlUniformLocs::Loc loc, const glm::vec2& value) const;
    void SetUniform(GlUniformLocs::Loc loc, const glm::vec3& value) const;
    void SetUniform(GlUniformLocs::Loc loc, const glm::vec4& value) const;
    void SetUniform(GlUniformLocs::Loc loc, const glm::mat2& value) const;
    void SetUniform(GlUniformLocs::Loc loc, const glm::mat3& value) const;
    void SetUniform(GlUniformLocs::Loc loc, const glm::mat4& value) const;

#ifndef NDEBUG
    void SetUniform(const std::string& name, bool value) const;
    void SetUniform(const std::string& name, int value) const;
    void SetUniform(const std::string& name, float value) const;
    void SetUniform(const std::string& name, const glm::vec2& value) const;
    void SetUniform(const std::string& name, const glm::vec3& value) const;
    void SetUniform(const std::string& name, const glm::vec4& value) const;
    void SetUniform(const std::string& name, const glm::mat2& value) const;
    void SetUniform(const std::string& name, const glm::mat3& value) const;
    void SetUniform(const std::string& name, const glm::mat4& value) const;
#endif
};

#define SetVarUniform(var) SetUniform(#var, var);
