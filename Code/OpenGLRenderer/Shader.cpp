#include <Shader.h>
#include <Platform.h>

#include <GL/glew.h>
#include <fmt/core.h>

Shader::Shader(const char* vertexPath, const char* fragmentPath)
{
    std::string vertexCode;
    std::string fragmentCode;

    std::ifstream vShaderFile;
    std::ifstream fShaderFile;

    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try
    {
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);

        std::stringstream vShaderStream, fShaderStream;
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();

        vShaderFile.close();
        fShaderFile.close();

        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    }
    catch (std::ifstream::failure& e)
    {
        fmt::print("ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ\n");
    }

    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    unsigned int vertex, fragment;

    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    checkErrors(vertex, "VERTEX");

    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    checkErrors(fragment, "FRAGMENT");

    m_id = glCreateProgram();
    glAttachShader(m_id, vertex);
    glAttachShader(m_id, fragment);
    glLinkProgram(m_id);
    checkErrors(m_id, "PROGRAM");

    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

Shader::~Shader()
{
    glDeleteProgram(m_id);
}

void
Shader::use()
{
    glUseProgram(m_id);
}

void
Shader::setBool(const std::string& name, bool value) const
{
    glUniform1i(getUniformLocation(name.c_str()), (int)value);
}

void
Shader::setInt(const std::string& name, int value) const
{
    glUniform1i(getUniformLocation(name.c_str()), value);
}

void
Shader::setFloat(const std::string& name, float v1) const
{
    glUniform1f(getUniformLocation(name.c_str()), v1);
}

void
Shader::setFloat4f(const std::string& name, float v1, float v2, float v3, float v4) const
{
    glUniform4f(getUniformLocation(name.c_str()), v1, v2, v3, v4);
}

void
Shader::setVec2(const std::string& name, float x, float y) const
{
    glUniform2f(getUniformLocation(name.c_str()), x, y);
}
void
Shader::setVec2(const std::string& name, const glm::vec2& value) const
{
    glUniform2fv(getUniformLocation(name.c_str()), 1, &value[0]);
}
void
Shader::setVec3(const std::string& name, const glm::vec3& value) const
{
    glUniform3fv(getUniformLocation(name.c_str()), 1, &value[0]);
}
void
Shader::setVec3(const std::string& name, float x, float y, float z) const
{
    glUniform3f(getUniformLocation(name.c_str()), x, y, z);
}
void
Shader::setVec4(const std::string& name, const glm::vec4& value) const
{
    glUniform4fv(getUniformLocation(name.c_str()), 1, &value[0]);
}
void
Shader::setVec4(const std::string& name, float x, float y, float z, float w) const
{
    glUniform4f(getUniformLocation(name.c_str()), x, y, z, w);
}
void
Shader::setMat2(const std::string& name, const glm::mat2& mat) const
{
    glUniformMatrix2fv(getUniformLocation(name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
void
Shader::setMat3(const std::string& name, const glm::mat3& mat) const
{
    glUniformMatrix3fv(getUniformLocation(name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
void
Shader::setMat4(const std::string& name, const glm::mat4& mat) const
{
    glUniformMatrix4fv(getUniformLocation(name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

int
Shader::checkErrors(unsigned shader, std::string type) const
{
    int success;
    char infoLog[1024];
    if (type != "PROGRAM")
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            fmt::print("ERROR::SHADER_COMPILATION_ERROR of type: {}\n{}\n -- --------------------------------------------------- -- \n");
        }
    }
    else
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            fmt::print("ERROR::PROGRAM_LINKING_ERROR of type: {}\n{}\n -- --------------------------------------------------- -- \n");
        }
    }
    return success;
}

int
Shader::getUniformLocation(const char *name) const
{
    int loc = glGetUniformLocation(m_id, name);
    ASSERT(loc != -1);
    return loc;
}