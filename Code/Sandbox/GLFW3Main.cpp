#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <stb_image.h>

#include <fmt/core.h>
//#include <spdlog/spdlog.h>

#ifndef _WIN32
#include <cassert>
#include <cmath>
#include <cstdlib>
#endif
#include <ctime>

#include <fstream>
#include <string>
#include <sstream>
#include <vector>

#include <Debug.h>
#include <Shader.h>
#include <Camera.h>
#include <VertexArray.h>
#include <Primitives/Cube.h>
#include <Primitives/Quad.h>
#include <ParticleSystem.h>


struct Resolution
{
	int w, h;
};

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

void processInput(GLFWwindow *window, float deltaTime);

unsigned int loadTexture(const char *path);
unsigned int loadCubemap(std::vector<std::string>);

Resolution screen = { 1024, 720 };
bool vsync = false;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = screen.w / 2.0f;
float lastY = screen.h / 2.0f;
bool firstMouse = true;

const std::vector<glm::vec3> launchPositions = {
		{ -4.0f, -5.0f, -70.0f },
		{ -4.0f, -5.0f,   8.0f },
		{ -4.0f, -5.0f,  95.0f }
};

std::vector<Particle> particlePool;
int numParticles = 1000;
int nextFreeParticleIndex = numParticles - 1;

int main(int argc, char **argv)
{


    /* Initialize GLFW3 */
    if (!glfwInit())
	{
		fmt::print("GLFW3 could not be initialiazed!\n");
    	return -1;
	}

    /* Create a windowed mode window and its OpenGL context */
	GLFWwindow* window = glfwCreateWindow(1024, 720, "Hello World", NULL, NULL);
    if (!window)
    {
		fmt::print("GLFW3 window could not be initialiazed!\n");
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

	// Initialize GLEW
	if (glewInit() != GLEW_OK)
	{
		fmt::print("GLEW could not be initialiazed!\n");
		glfwTerminate();
		return -1;
	}

	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	// Print OpenGL version
	fmt::print("{0}\n", glGetString(GL_VERSION));
	glfwSwapInterval(vsync);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

#ifdef R_OPENGL_DEBUG
	// Enable OpenGL debugging
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

	int flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
	{
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(glDebugOutput, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	}

	// glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	// glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
#endif
	
	std::vector<std::string> cubemapFaces = {
		"right.jpg",
		"left.jpg",
		"top.jpg",
		"bottom.jpg",
		"front.jpg",
		"back.jpg"
	};

	unsigned int cubeMapId = loadCubemap(cubemapFaces);
	stbi_set_flip_vertically_on_load(true);
	unsigned int waterTexture = loadTexture("water_edited.jpg");

	Quad water;
	Shader waterShader("textured_cube.vert", "textured_cube.frag");

	Cube skybox;
	Shader skyShader("skybox.vert", "skybox.frag");

	Cube lightSource;
	Shader fireworkShader("fireworks.vert", "fireworks.frag");
	glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
	fireworkShader.setVec3("u_lightColor", lightColor);

	particlePool.resize(numParticles);

	std::srand((unsigned)std::time(nullptr));
	double t = 0;
	double dt = 0.05;

	float deltaTime = 0.0f;
	float lastFrame = 0.0f;

	glEnable(GL_DEPTH_TEST);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window, deltaTime);

		for (int index = numParticles - 1; index > -1; index--)
		{
			Particle& p = particlePool[index];
			if (!p.active)
				continue;

			p.position += glm::vec3(0.0f, 5.0f, 0.0f) * deltaTime;
		}

		int winWidth, winHeight;
		glfwGetWindowSize(window, &winWidth, &winHeight);
		glViewport(0, 0, winWidth, winHeight);

		glClearColor(0.035f, 0.031f, 0.004f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
												(float)winWidth / (float)winHeight,
												0.1f, 200.0f
		);
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 model;
		glm::mat4 mvp;
		
		// Fireworks
		for (int index = numParticles - 1; index > -1; index--)
		{
			Particle p = particlePool[index];
			if (! p.active)
				continue;			

			model = glm::mat4(1.0f);
			model = glm::translate(model, p.position);
			// model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, .0f, 0.0f));
			// model = glm::scale(model, glm::vec3(0.1f, 0.2f, 0.1f));
			model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));

			mvp = projection * view * model;
			fireworkShader.setMat4("mvp", mvp);
			fireworkShader.use();
			lightSource.Draw();
		}

		// Water plane
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, -5.0f, 0.0f));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(300.0f, 300.0f, 0.0f));

		mvp = projection * view * model;
		waterShader.setMat4("mvp", mvp);
		waterShader.use();
		water.Draw();


		// Skybox
		glDepthFunc(GL_LEQUAL);
		model = glm::mat4(1.0f);
		view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
		mvp = projection * view * model;
		skyShader.setMat4("mvp", mvp);
		skyShader.use();
		skybox.Draw();
		glDepthMask(GL_LESS);
	
        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window, float deltaTime)
{
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (key == GLFW_KEY_SPACE && action == GLFW_REPEAT || action == GLFW_PRESS)
	{
		glm::vec3 firePos;
		float station = rand() / (float)RAND_MAX * 3.0f;
		if (station < 1.0f)
			firePos = launchPositions[0];
		else if (station < 2.0f)
			firePos = launchPositions[1];
		else
			firePos = launchPositions[2];

		Particle& currentParticle = particlePool[nextFreeParticleIndex--];
		currentParticle.position = firePos;
		currentParticle.active = true;
	}
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

unsigned int loadTexture(char const *path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;
		else ASSERT(0);

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);;
	}
	else
		fmt::print("Texture failed to load at path: {}\n", path);
	stbi_image_free(data);

	return textureID;
}

// loads a cubemap texture from 6 individual texture faces
// order:
// +X (right)
// -X (left)
// +Y (top)
// -Y (bottom)
// +Z (front) 
// -Z (back)
// -------------------------------------------------------
unsigned int loadCubemap(std::vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		else
			fmt::print("Cubemap texture failed to load at path: {}\n", faces[i]);
		stbi_image_free(data);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}