#include "Shader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

Shader::Shader(const char* vertexPath, const char* fragmentPath)
{
    const char* paths[] = { vertexPath, fragmentPath };
    const GLuint shaderTypes[] = { GL_VERTEX_SHADER, GL_FRAGMENT_SHADER };
    GLuint compiledShaders[2] = {};
    std::string shaderCode;
    shaderCode.reserve(1024);

    // shader Program
    id = glCreateProgram();
    for (int i = 0; i < 2; i++)
    {
        // shaderCode.clear();
        std::stringstream shaderStream;
		// open at end, read as binary since we just want the flat raw content, don't care about any text format.
        std::ifstream shaderFile(paths[i], std::ios_base::ate | std::ios_base::binary); 
        if (!shaderFile)
        {
            std::cerr << "Failed to open shader file " << paths[i] << '\n';
            std::abort();
        }
        size_t codeLen = shaderFile.tellg();
        shaderCode.resize(codeLen + 1); // +1 for null terminator
        shaderFile.seekg(0);
        shaderFile.read(shaderCode.data(), codeLen);
        shaderCode.back() = 0; // null-terminated string
        shaderFile.close();

        GLuint shader = glCreateShader(shaderTypes[i]);
        const char* shaderStr = shaderCode.c_str();
        glShaderSource(shader, 1, &shaderStr, nullptr);
        glCompileShader(shader);
        CheckShaderCompilation(shader, paths[i]);
        glAttachShader(id, shader);
    }

    glLinkProgram(id);
    CheckProgramLinkSuccess(id);

    for (int i = 0; i < 2; i++)
    {
		glDeleteShader(compiledShaders[i]);
    }
}

void Shader::use()
{
    glUseProgram(id);
}

void Shader::SetUniform(const std::string& name, bool value) const
{
    glUniform1i(glGetUniformLocation(id, name.c_str()), (int)value);
}

void Shader::SetUniform(const std::string& name, int value) const
{
    glUniform1i(glGetUniformLocation(id, name.c_str()), value);
}

void Shader::SetUniform(const std::string& name, float value) const
{
    glUniform1f(glGetUniformLocation(id, name.c_str()), value);
}

void Shader::CheckShaderCompilation(unsigned int shader, const char* shaderPath)
{
	int success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(shader, sizeof(infoLog), nullptr, infoLog);
		std::cerr << "Error - Shader " << shader << " compilation failed\n" << infoLog << '\n';
        if (shaderPath)
            std::cerr << "Shader Path: " << shaderPath << '\n';
		std::abort();
	}
}

void Shader::CheckProgramLinkSuccess(unsigned int program)
{
	int success;
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(program, sizeof(infoLog), nullptr, infoLog);
		std::cerr << "Program " << program << " failed to link:\n" << infoLog << '\n';
		std::abort();
	}
}
