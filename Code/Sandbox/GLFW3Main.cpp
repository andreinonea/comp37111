#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Gui/Text.h>
#include <stb_image.h>

#include <iostream>
#include <iomanip>

#ifndef _WIN32
#include <cassert>
#include <cmath>
#include <cstdlib>
#endif
#include <ctime>

#include <list>
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


#ifndef DEFAULT_TEXTURE_PATH
#define TEXTURE_PATH ""
#else
#define TEXTURE_PATH DEFAULT_TEXTURE_PATH
#endif

#define GPU_MATRIX_COMPUTE

void printGLInfo();
void printFlagsInfo();
void printOperationTimes (float ft, float it, float ut, float rt);
void DrawStats (unsigned font, float x, float y, float scale,
				float ft, float it, float ut, float rt);
void DrawFPS (unsigned font, float x, float y, float scale, int value);
void DrawFlags (unsigned font, float x, float y, float scale,
		   bool vsync, bool play, bool show_active_particles);

unsigned total_draw_calls = 0;
unsigned fireworks_draw_calls = 0;
unsigned water_draw_calls = 0;
unsigned skybox_draw_calls = 0;
unsigned text_draw_calls = 0;

struct Resolution
{
	int w, h;
};

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void key_callback(GLFWwindow *window,
				  int key, int scancode, int action, int mods);

void processInput(GLFWwindow *window, float deltaTime);

unsigned int loadTexture(const char *path);
unsigned int loadCubemap(std::vector<std::string>);

Resolution screen = { 1024, 720 };
bool vsync = false;
bool show_active_particles = false;

Camera camera(glm::vec3(180.181000f, 178.723328f, 204.241592f));
float lastX = screen.w / 2.0f;
float lastY = screen.h / 2.0f;
bool firstMouse = true;

const std::vector<glm::vec3> emitters = {
		{ -4.0f, -5.0f, -70.0f },
		{ -4.0f, -5.0f,   8.0f },
		{ -4.0f, -5.0f,  95.0f }
};


std::list<std::shared_ptr<FireworkParticle>> particlePool;
bool play = true;

int main(int argc, char **argv)
{
    /* Initialize GLFW3 */
    if (!glfwInit())
	{
		std::cout << "GLFW3 could not be initialiazed!" << '\n';
    	return -1;
	}

    /* Create a windowed mode window and its OpenGL context */
	GLFWwindow* window = glfwCreateWindow(screen.w,
										  screen.h,
										  "Hello World",
										  NULL,
										  NULL);
    if (!window)
    {
		std::cout << "GLFW3 window could not be initialiazed!" << '\n';
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

	// Initialize GLEW
	if (glewInit() != GLEW_OK)
	{
		std::cout << "GLEW could not be initialiazed!" << '\n';
		glfwTerminate();
		return -1;
	}

#ifdef __APPLE__
	glfwWindowHint (GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

#ifdef R_OPENGL_DEBUG
	// Enable OpenGL debugging
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

	int flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
	{
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(glDebugOutput, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE,
							  0, nullptr, GL_TRUE);
	}

	// glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	// glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
#endif

	printGLInfo ();
	glfwWindowHint (GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwSetFramebufferSizeCallback (window, framebuffer_size_callback);
	glfwSetKeyCallback (window, key_callback);
	glfwSetCursorPosCallback (window, mouse_callback);
	glfwSetScrollCallback (window, scroll_callback);
	glfwSetInputMode (window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	
	std::vector<std::string> cubemapFaces = {
		TEXTURE_PATH "right.jpg",
		TEXTURE_PATH "left.jpg",
		TEXTURE_PATH "top.jpg",
		TEXTURE_PATH "bottom.jpg",
		TEXTURE_PATH "front.jpg",
		TEXTURE_PATH "back.jpg"
	};

	unsigned int cubeMapId = loadCubemap(cubemapFaces);
	stbi_set_flip_vertically_on_load(true);
	unsigned int waterTexture = loadTexture(TEXTURE_PATH "water_edited.jpg");

	Text::Init ();
	unsigned arialFont;
	int caca = Text::LoadFont (FONT_PATH "ARIAL.TTF", &arialFont);

	Shader textShader (SHADER_PATH "text_default.vert",
					   SHADER_PATH "text_default.frag");
	glm::mat4 orthografic = glm::ortho (0.0f,
									   (float) screen.w,
									   0.0f,
									   (float) screen.h);
	textShader.use ();
	textShader.setMat4 ("projection", orthografic);

	Quad waterQuad;
	Shader waterShader(SHADER_PATH "quad_textured_default.vert",
					   SHADER_PATH "quad_textured_default.frag");

	Cube skybox;
	Shader skyShader(SHADER_PATH "skybox.vert",
					 SHADER_PATH "skybox.frag");

	Cube lightSource;
	Shader fireworkShader(SHADER_PATH "fireworks.vert",
						  SHADER_PATH "fireworks.frag");

	std::srand((unsigned)std::time(nullptr));
	double t = 0.0;
	double timestep = 0.05;
	double accumulator = 0.0;

	double FPS = 0.0;
	double frameCount = 0.0;
	double frameTime = 0.0;
	double lastFrame = 0.0;

	double inputTime = -1;
	double updateTime = -1;
	double renderTime = -1;
	double startTime = -1;

	// glEnable (GL_CULL_FACE);
	glEnable (GL_DEPTH_TEST);
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
		glfwSwapInterval (vsync);
		
		total_draw_calls = 0;
		fireworks_draw_calls = 0;
		water_draw_calls = 0;
		skybox_draw_calls = 0;
		text_draw_calls = 0;

		double newFrame = glfwGetTime();
		frameTime = newFrame - lastFrame;
		lastFrame = newFrame;
		FPS = 1 / frameTime;

		accumulator += frameTime;

		startTime = glfwGetTime ();
		processInput(window, static_cast<float>(frameTime));
		inputTime = glfwGetTime () - startTime;

		startTime = glfwGetTime ();
		while (accumulator >= timestep)
		{
			if (play)
			{
				std::vector<std::shared_ptr<FireworkParticle>> deadParticles;
				std::vector<std::shared_ptr<FireworkParticle>> babyParticles;
				for (auto& f : particlePool)
				{
					if (!f->active)
						continue;

					if (f->lifetime <= 0.0f)
					{
						f->active = false;
						deadParticles.push_back (f);

						if (f->recursionLevel == 0)
							continue;

						float deltaBoom;
						deltaBoom = rand () / (float) RAND_MAX * 10.0f - 5.0f;
						unsigned boomSize = f->explosionFactor
							+ static_cast<int>(deltaBoom);
						for (unsigned i = 0; i < boomSize; i++)
						{
							FireworkChildParticle *e = new FireworkChildParticle;

							float dlife = rand () / (float) RAND_MAX * 2.5f;
							e->lifetime = 1.0f + dlife;
							e->active = true;
							e->child = true;

							e->position = f->position;
							e->scale = { 0.2f, 0.3f, 0.2f };

							e->rgba = f->rgba;

							float dx = rand () / (float) RAND_MAX;
							float dy = rand () / (float) RAND_MAX;
							float dz = rand () / (float) RAND_MAX;

							glm::vec3 explosionDirection (dx, dy, dz);
							e->mass = 0.3f;
							e->acceleration = GRAVITY * e->mass;
							e->velocity = explosionDirection * 10.0f;

							e->recursionLevel = f->recursionLevel - 1;
							e->explosionFactor = f->explosionFactor / 2;

							babyParticles.push_back (
								std::shared_ptr<FireworkParticle> (e));
						}

						continue;
					}

					f->lifetime -= static_cast<float>(timestep);
					f->rgba.w -= 0.05f * static_cast<float>(timestep);
					if (f->rgba.w < 0.0f)
						f->lifetime = 0.0f;
					f->velocity += f->acceleration
										* static_cast<float>(timestep);
					f->position += f->velocity
										* static_cast<float>(timestep);
					if (!f->child)
					{
						f->scale.y = f->velocity.y / 5.0f;
						if (f->velocity.y <= 0.0f)
							f->lifetime = 0.0f;
					}
				}
				for (auto& p : deadParticles)
					particlePool.remove (p);
				for (auto& p : babyParticles)
					particlePool.push_back (p);
			}
			accumulator -= timestep;
			t += timestep;
		}
		updateTime = glfwGetTime () - startTime;

		startTime = glfwGetTime ();
		int winWidth, winHeight;
		glfwGetWindowSize(window, &winWidth, &winHeight);
		//glViewport(0, 0, winWidth, winHeight);

		glClearColor(0.035f, 0.031f, 0.004f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
											(float)winWidth / (float)winHeight,
											0.1f, 1000.0f);
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 model;
		
		// Rendering

		// Fireworks
		glEnable (GL_CULL_FACE);
		for (auto& p : particlePool)
		{
			if (!p->active)
				continue;

			model = glm::mat4 (1.0f);
			model = glm::translate (model, p->position);
			model = glm::rotate (model,
								 p->rotation.x,
								 glm::vec3 (1.0f, 0.0f, 0.0f));
			model = glm::rotate (model,
								 p->rotation.y,
								 glm::vec3 (0.0f, 1.0f, 0.0f));
			model = glm::rotate (model,
								 p->rotation.z,
								 glm::vec3 (0.0f, 0.0f, 1.0f));
			model = glm::scale (model, p->scale);

			fireworkShader.use ();
			fireworkShader.setVec4 ("u_lightColor", p->rgba);
			fireworkShader.setMat4 ("model", model);
			fireworkShader.setMat4 ("view", view);
			fireworkShader.setMat4 ("projection", projection);

			lightSource.Draw ();
			fireworks_draw_calls++;
		}
		glDisable (GL_CULL_FACE);

		// Water plane
		model = glm::mat4 (1.0f);
		model = glm::translate (model, glm::vec3 (0.0f, -5.0f, 0.0f));
		model = glm::rotate (model,
							 glm::radians (90.0f),
							 glm::vec3 (1.0f, 0.0f, 0.0f));
		model = glm::scale (model, glm::vec3 (300.0f, 300.0f, 0.0f));

		waterShader.use ();
		glBindTexture (GL_TEXTURE_2D, waterTexture);
		waterShader.setMat4 ("model", model);
		waterShader.setMat4 ("view", view);
		waterShader.setMat4 ("projection", projection);
		waterQuad.Draw ();	
		water_draw_calls++;

		// Skybox
		glDepthFunc (GL_LEQUAL);
		model = glm::mat4 (1.0f);
		view = glm::mat4 (glm::mat3 (camera.GetViewMatrix ()));

		skyShader.use ();
		skyShader.setMat4 ("model", model);
		skyShader.setMat4 ("view", view);
		skyShader.setMat4 ("projection", projection);

		skybox.Draw ();
		skybox_draw_calls++;
		glDepthMask (static_cast<bool>(GL_LESS));

		// Stats
		glEnable (GL_CULL_FACE);
		glm::vec4 textColor (1.0f, 1.0f, 1.0f, 1.0f);
		textShader.use ();
		textShader.setVec4 ("textColor", textColor);
		DrawFPS (arialFont,
				 static_cast<float>(winWidth) - 150.0f,
				 static_cast<float>(winHeight) - 25.0f,
				 0.5f,
				 static_cast<int>(FPS));
		DrawFlags (arialFont,
				   10.0f,
				   static_cast<float>(winHeight) - 200.0f,
				   0.5f,
				  vsync, play, show_active_particles);
		DrawStats (arialFont,
				   10.0f,
				   static_cast<float>(winHeight) - 25.0f,
				   0.5f,
				   static_cast<float>(frameTime),
				   static_cast<float>(inputTime),
				   static_cast<float>(updateTime),
				   static_cast<float>(renderTime));
		glDisable (GL_CULL_FACE);

		renderTime = glfwGetTime () - startTime;

		// t += timestep;

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();

		// printOperationTimes (frameTime, inputTime, updateTime, renderTime);
    }

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window, float deltaTime)
{
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(Camera::Camera_Movement::FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(Camera::Camera_Movement::LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(Camera::Camera_Movement::BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(Camera::Camera_Movement::RIGHT, deltaTime);
}

void key_callback(GLFWwindow *window,
				  int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (key == GLFW_KEY_P && action == GLFW_PRESS)
		play = !play;
	if (key == GLFW_KEY_V && action == GLFW_PRESS)
		vsync = !vsync;
	if (key == GLFW_KEY_TAB && action == GLFW_PRESS)
		show_active_particles = !show_active_particles;
	if (key == GLFW_KEY_SPACE && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
		glm::vec3 firePos;
		float station = rand() / (float)RAND_MAX * 3.0f;
		if (station < 1.0f)
			firePos = emitters[0];
		else if (station < 2.0f)
			firePos = emitters[1];
		else
			firePos = emitters[2];

		FireworkParticle *p = new FireworkParticle;
		float dlife = rand() / (float)RAND_MAX * 3.0f;

		p->lifetime = 2.0f + dlife;
		p->active = true;

		p->mass = 1.0f;
		p->acceleration = GRAVITY * p->mass;

		p->position = firePos;
		p->scale = { 0.2f, 5.0f, 0.2f };
		
		float dr = rand () / (float) RAND_MAX * 0.34f + 0.66f;
		float dg = rand () / (float) RAND_MAX * 0.34f + 0.66f;
		float db = rand () / (float) RAND_MAX * 0.34f + 0.66f;
		p->rgba = glm::vec4 (dr, dg, db, 1.0f);

		float dx = rand() / (float)RAND_MAX * 2.0f;
		float dz = rand() / (float)RAND_MAX * 2.0f;
		glm::vec3 impulse(dx, 70.0f, dz);
		p->velocity = impulse;

		particlePool.push_back(std::shared_ptr<FireworkParticle>(p));
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
		lastX = static_cast<float>(xpos);
		lastY = static_cast<float>(ypos);
		firstMouse = false;
	}

	float xoffset = static_cast<float>(xpos) - lastX;
	float yoffset = lastY - static_cast<float>(ypos); // reversed since y-coordinates go from bottom to top

	lastX = static_cast<float>(xpos);
	lastY = static_cast<float>(ypos);

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

unsigned int loadTexture(const char *path)
{
	unsigned int textureId;
	glGenTextures(1, &textureId);

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

		glBindTexture(GL_TEXTURE_2D, textureId);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else
		std::cout << "Texture failed to load at path: "
			<< path << '\n';
	stbi_image_free(data);

	return textureId;
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
			std::cout << "Cubemap texture failed to load at path: "
				<< faces[i] << '\n';

		stbi_image_free(data);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

void printGLInfo()
{
	std::cout << "Version: \t" << glGetString (GL_VERSION) << '\n';
	std::cout << "Vendor: \t" << glGetString (GL_VENDOR) << '\n';
	std::cout << "Renderer: \t" << glGetString (GL_RENDERER) << '\n' << '\n';
}

void printFlagsInfo ()
{
	std::cout << "Vsync: \t\t" << (vsync ? "ON" : "OFF") << '\n';
	std::cout << "Pause: \t\t" << (play ? "OFF" : "ON") << '\n';
	std::cout << "Show active particules: \t"
		<< (show_active_particles ? "ON" : "OFF") << '\n' << '\n';
}

void printOperationTimes (float ft, float it, float ut, float rt)
{
	std::cout << std::fixed;
	std::cout.precision (2);
	std::cout << "Frame (ms): \t" << ft * 1000 << '\n';
	std::cout << "Input (ms): \t" << it * 1000 << '\n';
	std::cout << "Update (ms): \t" << ut * 1000 << '\n';
	std::cout << "Render (ms): \t" << rt * 1000 << '\n' << '\n';
	std::cout.unsetf (std::ios_base::floatfield);
}

void DrawStats (unsigned font, float x, float y, float scale,
				float ft, float it, float ut, float rt)
{
	std::string texts[4] = {
		"Frame (ms): ",
		"Update (ms): ",
		"Render (ms): ",
		"Input (ms): "
	};

	float values[4] = {
		ft * 1000,
		ut * 1000,
		rt * 1000,
		it * 1000
	};

	for (unsigned i = 0; i < 4; i++)
	{
		std::stringstream s;
		s << std::fixed;
		s.precision (2);
		s << texts[i] << values[i];
		Text::Draw (font, s.str (), x, y, scale, glm::vec4 (0.0f));
		text_draw_calls++;
		y -= 25;
	}
	y -= 25;
	std::stringstream s;
	total_draw_calls = fireworks_draw_calls
						+ water_draw_calls
						+ skybox_draw_calls
						+ text_draw_calls
						+ 1;
	s << "Draw calls: " << total_draw_calls;
	Text::Draw (font, s.str (), x, y, scale, glm::vec4 (0.0f));
}

void DrawFPS (unsigned font, float x, float y, float scale, int value)
{
	std::stringstream s;
	s << std::fixed;
	s.precision (2);
	s << "FPS: " << value;
	Text::Draw (font, s.str (), x, y, scale, glm::vec4 (0.0f));
}

void DrawFlags (unsigned font, float x, float y, float scale,
		   bool vsync, bool play, bool show_active_particles)
{
	std::stringstream s0;
	s0 << "Vsync: " << (vsync ? "ON" : "OFF");
	Text::Draw (font, s0.str (), x, y, scale, glm::vec4 (0.0f));
	y -= 25;

	std::stringstream s1;
	s1 << "Pause: " << (play ? "OFF" : "ON");
	Text::Draw (font, s1.str (), x, y, scale, glm::vec4 (0.0f));
	y -= 25;

	std::stringstream s2;
	s2 << "Active particles: " << (show_active_particles ?
		std::to_string(particlePool.size()) : "OFF");
	Text::Draw (font, s2.str (), x, y, scale, glm::vec4 (0.0f));
}