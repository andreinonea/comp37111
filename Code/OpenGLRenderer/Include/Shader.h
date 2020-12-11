#ifndef __SHADER_H__
#define __SHADER_H__

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <glm/glm.hpp>

class Shader
{
public:
    Shader(const char* vertexPath, const char* fragmentPath);
    ~Shader();

    void use();

    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float v1) const;
    void setFloat4f(const std::string& name, float v1, float v2, float v3, float v4) const;
    void setVec2(const std::string& name, float x, float y) const;
    void setVec2(const std::string& name, const glm::vec2& value) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setVec3(const std::string& name, float x, float y, float z) const;
    void setVec4(const std::string& name, const glm::vec4& value) const;
    void setVec4(const std::string& name, float x, float y, float z, float w) const;
    void setMat2(const std::string& name, const glm::mat2& mat) const;
    void setMat3(const std::string& name, const glm::mat3& mat) const;
    void setMat4(const std::string& name, const glm::mat4& mat) const;

private:
    unsigned m_id;

private:
    int checkErrors(unsigned shader, std::string type) const;
    int getUniformLocation(const char *name) const;
};

#endif // !__SHADER_H__