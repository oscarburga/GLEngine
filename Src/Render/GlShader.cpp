#include "GlShader.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <fstream>
#include <sstream>
#include <iostream>

void CGlShader::Use()
{
    glUseProgram(Id);
}

void CGlShader::SetUniform(const std::string& name, bool value) const
{
    SetUniform(name, int(value));
}

void CGlShader::SetUniform(const std::string& name, int value) const
{
    glUniform1i(glGetUniformLocation(Id, name.c_str()), value);
}

void CGlShader::SetUniform(const std::string& name, float value) const
{
    glUniform1f(glGetUniformLocation(Id, name.c_str()), value);
}

void CGlShader::SetUniform(const std::string& name, const glm::vec2& value) const
{
    glUniform2f(glGetUniformLocation(Id, name.c_str()), value.x, value.y);
}

void CGlShader::SetUniform(const std::string& name, const glm::vec3& value) const
{
    glUniform3fv(glGetUniformLocation(Id, name.c_str()), 1, &value[0]);
}

void CGlShader::SetUniform(const std::string& name, const glm::vec4& value) const
{
    glUniform4fv(glGetUniformLocation(Id, name.c_str()), 1, &value[0]);
}

void CGlShader::SetUniform(const std::string& name, const glm::mat2& value) const
{
    glUniformMatrix2fv(glGetUniformLocation(Id, name.c_str()), 1, GL_FALSE, &value[0][0]);
}

void CGlShader::SetUniform(const std::string& name, const glm::mat3& value) const
{
    glUniformMatrix3fv(glGetUniformLocation(Id, name.c_str()), 1, GL_FALSE, &value[0][0]);
}

void CGlShader::SetUniform(const std::string& name, const glm::mat4& value) const
{
    glUniformMatrix4fv(glGetUniformLocation(Id, name.c_str()), 1, GL_FALSE, &value[0][0]);
}

