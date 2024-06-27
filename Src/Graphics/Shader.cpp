#include "Shader.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <fstream>
#include <sstream>
#include <iostream>

// CShader::CShader(const char* vertexPath, const char* fragmentPath)
// {
//     const char* paths[] = { vertexPath, fragmentPath };
//     const GLuint shaderTypes[] = { GL_VERTEX_SHADER, GL_FRAGMENT_SHADER };
//     GLuint compiledShaders[2] = {};
//     std::string shaderCode;
//     shaderCode.reserve(1024);
// 
//     // shader Program
//     Id = glCreateProgram();
//     for (int i = 0; i < 2; i++)
//     {
//         // shaderCode.clear();
//         std::stringstream shaderStream;
// 		// open at end, read as binary since we just want the flat raw content, don't care about any text format.
//         std::ifstream shaderFile(paths[i], std::ios_base::ate | std::ios_base::binary); 
//         if (!shaderFile)
//         {
//             std::cerr << "Failed to open shader file " << paths[i] << '\n';
//             std::abort();
//         }
//         size_t codeLen = shaderFile.tellg();
//         shaderCode.resize(codeLen + 1); // +1 for null terminator
//         shaderFile.seekg(0);
//         shaderFile.read(shaderCode.data(), codeLen);
//         shaderCode.back() = 0; // null-terminated string
//         shaderFile.close();
// 
//         GLuint shader = glCreateShader(shaderTypes[i]);
//         const char* shaderStr = shaderCode.c_str();
//         glShaderSource(shader, 1, &shaderStr, nullptr);
//         glCompileShader(shader);
//         CheckShaderCompilation(shader, paths[i]);
//         glAttachShader(Id, shader);
//     }
// 
//     glLinkProgram(Id);
//     CheckProgramLinkSuccess(Id);
// 
//     for (int i = 0; i < 2; i++)
//     {
// 		glDeleteShader(compiledShaders[i]);
//     }
// }

void CShader::Use()
{
    glUseProgram(Id);
}

void CShader::SetUniform(const std::string& name, bool value) const
{
    SetUniform(name, int(value));
}

void CShader::SetUniform(const std::string& name, int value) const
{
    glUniform1i(glGetUniformLocation(Id, name.c_str()), value);
}

void CShader::SetUniform(const std::string& name, float value) const
{
    glUniform1f(glGetUniformLocation(Id, name.c_str()), value);
}

void CShader::SetUniform(const std::string& name, const glm::vec2& value) const
{
    glUniform2f(glGetUniformLocation(Id, name.c_str()), value.x, value.y);
}

void CShader::SetUniform(const std::string& name, const glm::vec3& value) const
{
    glUniform3fv(glGetUniformLocation(Id, name.c_str()), 1, &value[0]);
}

void CShader::SetUniform(const std::string& name, const glm::vec4& value) const
{
    glUniform4fv(glGetUniformLocation(Id, name.c_str()), 1, &value[0]);
}

void CShader::SetUniform(const std::string& name, const glm::mat2& value) const
{
    glUniformMatrix2fv(glGetUniformLocation(Id, name.c_str()), 1, GL_FALSE, &value[0][0]);
}

void CShader::SetUniform(const std::string& name, const glm::mat3& value) const
{
    glUniformMatrix3fv(glGetUniformLocation(Id, name.c_str()), 1, GL_FALSE, &value[0][0]);
}

void CShader::SetUniform(const std::string& name, const glm::mat4& value) const
{
    glUniformMatrix4fv(glGetUniformLocation(Id, name.c_str()), 1, GL_FALSE, &value[0][0]);
}

