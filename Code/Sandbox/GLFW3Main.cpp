#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <fmt/core.h>
//#include <spdlog/spdlog.h>

#include <cassert>
#include <cmath>

#include <fstream>
#include <string>
#include <sstream>

#include <VertexBuffer.h>
#include <IndexBuffer.h>

#define R_OPENGL_DEBUG // enable OpenGL debugging

#ifndef R_OPENGL_DEBUG
#define NDEBUG
#endif

//TODO: move this into another class
#ifdef _WIN32
	#define ASSERT(x) if (!(x)) __debugbreak()
#else
	#define ASSERT(x) assert(x);
#endif

void APIENTRY glDebugOutput(unsigned int source,
							unsigned int type,
							unsigned int id,
							unsigned int severity,
							int length,
							const char* msg,
							const void* data)
{
	// ignore non-significant error/warning codes
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

	const char* _source;
    const char* _type;
    const char* _severity;

    switch (source) 
	{
		case GL_DEBUG_SOURCE_API:
			_source = "API";
			break;

		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
			_source = "WINDOW SYSTEM";
			break;

		case GL_DEBUG_SOURCE_SHADER_COMPILER:
			_source = "SHADER COMPILER";
			break;

		case GL_DEBUG_SOURCE_THIRD_PARTY:
			_source = "THIRD PARTY";
			break;

		case GL_DEBUG_SOURCE_APPLICATION:
			_source = "APPLICATION";
			break;

		case GL_DEBUG_SOURCE_OTHER:
			_source = "UNKNOWN";
			break;

		default:
			_source = "UNKNOWN";
			break;
    }

    switch (type) 
	{
		case GL_DEBUG_TYPE_ERROR:
			_type = "ERROR";
			break;

		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
			_type = "DEPRECATED BEHAVIOR";
			break;

		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
			_type = "UDEFINED BEHAVIOR";
			break;

		case GL_DEBUG_TYPE_PORTABILITY:
			_type = "PORTABILITY";
			break;

		case GL_DEBUG_TYPE_PERFORMANCE:
			_type = "PERFORMANCE";
			break;

		case GL_DEBUG_TYPE_OTHER:
			_type = "OTHER";
			break;

		case GL_DEBUG_TYPE_MARKER:
			_type = "MARKER";
			break;

		default:
			_type = "UNKNOWN";
			break;
    }

    switch (severity) 
	{
		case GL_DEBUG_SEVERITY_HIGH:
			_severity = "HIGH";
			break;

		case GL_DEBUG_SEVERITY_MEDIUM:
			_severity = "MEDIUM";
			break;

		case GL_DEBUG_SEVERITY_LOW:
			_severity = "LOW";
			break;

		case GL_DEBUG_SEVERITY_NOTIFICATION:
			_severity = "NOTIFICATION";
			break;

		default:
			_severity = "UNKNOWN";
			break;
    }

    // ignore notification severity (you can add your own ignores)
    // + Adds __debugbreak if _DEBUG is defined (automatic in visual studio)
    // note: __debugbreak is specific for MSVC, won't work with gcc/clang
    // -> in that case remove it and manually set breakpoints
    if (std::strcmp(_severity, "NOTIFICATION") != 0) 
	{
		fmt::print("OpenGL error [{0}]: {1} of {2} severity, raised from {3}: {4}\n",
            id, _type, _severity, _source, msg);
		ASSERT(0);
    }
}


struct ShaderSources
{
	std::string vertSource;
	std::string fragSource;
};

static ShaderSources ParseShader(const std::string& shaderPath)
{
	std::ifstream shader(shaderPath);

	enum class ShaderType
	{
		NONE = -1, VERTEX = 0, FRAGMENT = 1
	};
	
	ShaderType type = ShaderType::NONE;
	std::stringstream buffers[2];
	std::string line;

	while(getline(shader, line))
	{
		if (line.find("#shader") != std::string::npos)
		{
			if (line.find("vertex") != std::string::npos)
				type = ShaderType::VERTEX;
			else if (line.find("fragment") != std::string::npos)
				type = ShaderType::FRAGMENT;
		}
		else
			buffers[(int) type] << line << "\n";
	}

	return { buffers[0].str(), buffers[1].str() };
}

static void PrintShaderError(unsigned int shader, unsigned int type)
{
	int result;
	glGetShaderiv(shader, type, &result);
	if (!result)
	{
		int logLength;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
		char* message = (char*) alloca(logLength * sizeof(char));
		glGetShaderInfoLog(shader, logLength, &logLength, message);
		fmt::print("Failed to compile shader {0}: {1}\n", shader, message);
		glDeleteShader(shader);
	}
}

static void PrintProgramError(unsigned int program, unsigned int type)
{
	int result;
	glGetProgramiv(program, type, &result);
	if (!result)
	{
		int logLength;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
		char* message = (char*) alloca(logLength * sizeof(char));
		glGetProgramInfoLog(program, logLength, &logLength, message);
		std::string op = type == GL_LINK_STATUS ? "link" : "validate";
		fmt::print("Failed to {0} program {1}: {2}\n", op, program, message);
	}
}

static unsigned int CompileShader(unsigned int type, const std::string& source)
{
	unsigned int shader = glCreateShader(type);
	const char* src = source.c_str();
	glShaderSource(shader, 1, &src, nullptr);
	glCompileShader(shader);
	PrintShaderError(shader, GL_COMPILE_STATUS);

	return shader;
}

static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
{
	unsigned int program = glCreateProgram();
	unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
	unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

	glAttachShader(program, vs);
	glAttachShader(program, fs);
	
	glLinkProgram(program);
	PrintProgramError(program, GL_LINK_STATUS);

	glValidateProgram(program);
	PrintProgramError(program, GL_VALIDATE_STATUS);

	glDeleteShader(vs);
	glDeleteShader(fs);

	return program;
}


int main(int argc, char **argv)
{
    GLFWwindow* window;

    /* Initialize GLFW3 */
    if (!glfwInit())
	{
		fmt::print("GLFW3 could not be initialiazed!\n");
    	return -1;
	}

#ifdef R_OPENGL_DEBUG
	// Enable OpenGL debugging
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	
	int flags;
	glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
	{
    	glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); 
		glDebugMessageCallback(glDebugOutput, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	}
	
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
		fmt::print("GLFW3 window could not be initialiazed!\n");
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

//	glfwSwapInterval(1);

	// Initialize GLEW
	if (glewInit() != GLEW_OK)
	{
		fmt::print("GLEW could not be initialiazed!\n");
		glfwTerminate();
		return -1;
	}

	// Print OpenGL version
	fmt::print("{0}\n", glGetString(GL_VERSION));

	// Basic triangle draw in modern OpenGL
	float positions[] = {
		-0.5f, -0.5f,
		 0.5f, -0.5f,
		 0.5f,  0.5f,
		-0.5f,  0.5f,
	};

	unsigned int indices[] = {
		0, 1, 2,
		2, 3, 0
	};
	

	unsigned int vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	VertexBuffer vbo(positions, 4 * 2);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);

	IndexBuffer ibo(indices, 6);

	ShaderSources s = ParseShader("/usr/share/comp37111/sandbox/shaders/default.pse");
	unsigned int shader = CreateShader(s.vertSource, s.fragSource);

	int location = glGetUniformLocation(shader, "u_Color");
	ASSERT(location != -1);
	glUseProgram(shader);

	glUseProgram(0);
	glBindVertexArray(0);
	vbo.Unbind();
	ibo.Unbind();
	
	float r = 0.0f, g = 0.0f, b = 0.0f;
	float increment = 0.01f;
	float phase;

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

		//phase = std::cos(increment) * M_PI;
		phase = 2.0f * M_PI / 3.0f;

		r = (std::sin(increment) + 1.0f) / 2.0f;
		g = (std::sin(increment + phase) + 1.0f) / 2.0f;
		b = (std::sin(increment + phase * 2.0f) + 1.0f) / 2.0f;

		increment += M_PI / 180;
		
		glUseProgram(shader);
		glUniform4f(location, r, g, b, 1.0f);

		glBindVertexArray(vao);
		vbo.Bind();
		ibo.Bind();

		// Draw call for triangle
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

	glDeleteProgram(shader);

    glfwTerminate();
    return 0;
}
