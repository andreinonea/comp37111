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
				float ft, float it, float ut, float rt,
				bool vsync, bool play, bool show_active_particles);
void DrawFPS (unsigned font, float x, float y, float scale, int value);
void DrawHelp (unsigned font, float x, float y, float scale);
void DrawControls (unsigned font, float x, float y, float scale);

enum CONTROLS
{
	ALL_WAIT_TIME = 0,
	GRAVITY_FORCE,
	EXPLOSION_MASS,
	PROJECTILE_MASS,
	EXPLOSION_VELOCITY,
	EXPLOSION_LIFESPAN,
	PROJECTILE_VELOCITY,
	PROJECTILE_LIFESPAN,
	TIMESCALE,
	CONTROLS_SIZE
};
std::vector<std::pair<std::string, float>> controls;
unsigned selected = CONTROLS_SIZE - 1;

unsigned total_draw_calls = 0;
unsigned fireworks_draw_calls = 0;
unsigned water_draw_calls = 0;
unsigned skybox_draw_calls = 0;
unsigned text_draw_calls = 0;

// DEBUG!
double timescale = 1.0;

struct Resolution
{
	int w, h;
};

#define REPEAT 0x01U
#define RANDOM 0x02U
#define ALL 0x04U
unsigned show_type = 0x00U;

struct WaitTimes
{
	double repeat = 0.4;
	double random = 0.2;
	double all = 2.0;
} wait;
struct LastTimes
{
	double repeat = 0.4;
	double random = 0.2;
	double all = 2.0;
	double timescale = 0.0;
} last;

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void key_callback(GLFWwindow *window,
				  int key, int scancode, int action, int mods);

void processInput(GLFWwindow *window, float deltaTime);

unsigned int loadTexture(const char *path);
unsigned int loadCubemap(std::vector<std::string>);

void FireFireworks (unsigned type);

Resolution screen = { 1024, 720 };
bool vsync = true;
bool pause = false;
bool show_active_particles = false;
bool show_stats = false;
bool show_controls = false;
bool show_water = false;
unsigned count = 0;
int dir_forward = 1;
std::list<std::shared_ptr<FireworkParticle>> particlePool;
std::ofstream debug_file;

const std::vector<glm::vec3> emitters = {
		{ -4.0f, -5.0f, -70.0f },
		{ -4.0f, -5.0f,   8.0f },
		{ -4.0f, -5.0f,  95.0f }
};

Camera camera (glm::vec3 (180.181000f, 178.723328f, 204.241592f));
float lastX = screen.w / 2.0f;
float lastY = screen.h / 2.0f;
bool firstMouse = true;

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
	// glfwWindowHint (GLFW_SAMPLES, 4);
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
	int success = Text::LoadFont (FONT_PATH "ARIAL.TTF", &arialFont);

	Shader textShader (SHADER_PATH "text_default.vert",
					   SHADER_PATH "text_default.frag");

	Quad waterQuad;
	Shader waterShader(SHADER_PATH "quad_textured_default.vert",
					   SHADER_PATH "quad_textured_default.frag");

	Cube skybox;
	Shader skyShader(SHADER_PATH "skybox.vert",
					 SHADER_PATH "skybox.frag");

	Cube lightSource;
	Shader fireworkShader(SHADER_PATH "fireworks.vert",
						  SHADER_PATH "fireworks.frag");

	controls.resize (CONTROLS_SIZE);
	controls[TIMESCALE] = std::make_pair<std::string, float>
		("Timescale: ", 1.0f);
	controls[PROJECTILE_LIFESPAN] = std::make_pair<std::string, float>
		("Projectile base lifespan: ", 5.0f);
	controls[PROJECTILE_VELOCITY] = std::make_pair<std::string, float>
		("Projectile base velocity: ", 75.0f);
	controls[EXPLOSION_LIFESPAN] = std::make_pair<std::string, float>
		("Explosion base lifespan: ", 6.0f);
	controls[EXPLOSION_VELOCITY] = std::make_pair<std::string, float>
		("Explosion base velocity: ", 15.0f);
	controls[PROJECTILE_MASS] = std::make_pair<std::string, float>
		("Projectile mass: ", 3.0f);
	controls[EXPLOSION_MASS] = std::make_pair<std::string, float>
		("Explosion mass: ", 0.3f);
	controls[GRAVITY_FORCE] = std::make_pair<std::string, float>
		("Gravity force (Y-axis): ", -9.8f);
	controls[ALL_WAIT_TIME] = std::make_pair<std::string, float>
		("Wait time (all mode '2'): ", 2.1f);

	std::srand((unsigned)std::time(nullptr));
	double t = 0.0;
	// double timescale = 1.0;
	double update_rate = 0.05;
	double dt = update_rate * timescale;
	double accumulator = 0.0;
	double alpha = 0.0;

	double inputTime = -1.0;
	double updateTime = -1.0;
	double renderTime = -1.0;
	double sceneRenderTime = -1.0;
	double textRenderTime = -1.0;
	double startTime = -1.0;

	double FPS = 0.0;
	double frameCount = 0.0;
	double lastFrame = glfwGetTime ();

	// glEnable (GL_CULL_FACE);
	glEnable (GL_DEPTH_TEST);
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {		
		double newFrame = glfwGetTime();
		double frameTime = newFrame - lastFrame;
		lastFrame = newFrame;
		FPS = 1 / frameTime;

		accumulator += (frameTime > 0.25) ? 0.25 : frameTime;

		// Input
		startTime = glfwGetTime ();
		processInput(window, static_cast<float>(frameTime));
		inputTime = glfwGetTime () - startTime;

		// Update
		startTime = glfwGetTime ();
		total_draw_calls = 0;
		fireworks_draw_calls = 0;
		water_draw_calls = 0;
		skybox_draw_calls = 0;
		text_draw_calls = 0;
		int winWidth, winHeight;
		glfwGetWindowSize (window, &winWidth, &winHeight);

		glm::mat4 projection = glm::perspective (glm::radians (camera.Zoom),
										(float) winWidth / (float) winHeight,
										0.1f, 1000.0f);
		glm::mat4 view = camera.GetViewMatrix ();
		glm::mat4 model;

		if (!pause)
		{
			dt = update_rate * static_cast<double>(controls[TIMESCALE].second);
			while (accumulator >= update_rate)
			{
				if (timescale)
				{
					FireFireworks (show_type);
					std::vector<std::shared_ptr<FireworkParticle>> deadParticles;
					std::vector<std::shared_ptr<FireworkParticle>> babyParticles;
					for (auto& f : particlePool)
					{
						if (!f->active)
						{
							if (f->tracked && debug_file.is_open ())
								debug_file.close ();
							continue;
						}

						if (f->lifetime <= 0.0f)
						{
							f->active = false;
							deadParticles.push_back (f);

							if (f->recursionLevel <= 0)
								continue;

							float deltaBoom;
							deltaBoom = rand () / (float) RAND_MAX * 20.0f;
							unsigned boomSize = f->explosionFactor
								+ static_cast<int>(deltaBoom);
							for (unsigned i = 0; i < boomSize; i++)
							{
								FireworkChildParticle *e = new FireworkChildParticle;

								float dlife = rand () / (float) RAND_MAX * 2.0f;
								e->lifespan = controls[EXPLOSION_LIFESPAN].second
									+ dlife;
								e->lifetime = e->lifespan;
								e->active = true;
								e->child = true;

								e->position = f->position;
								e->scale = { 0.4f, 0.4f, 0.4f };

								e->rgba = f->rgba;

								float dx = rand () / (float) RAND_MAX * 2.0f - 1.0f;
								float dy = rand () / (float) RAND_MAX;
								float dz = rand () / (float) RAND_MAX * 2.0f - 1.0f;

								glm::vec3 explosionDirection (dx, dy, dz);
								e->mass = controls[EXPLOSION_MASS].second;
								e->acceleration = e->mass
									* glm::vec3 (0.0f,
												controls[GRAVITY_FORCE].second,
												0.0f);
								e->velocity = explosionDirection * 15.0f;

								e->recursionLevel = --f->recursionLevel;
								//e->explosionFactor = f->explosionFactor / 1.5;

								babyParticles.push_back (
									std::shared_ptr<FireworkParticle> (e));
							}
							continue;
						}

						f->previous.alpha = f->rgba.w;
						f->previous.lifetime = f->lifetime;
						f->previous.position = f->position;
						f->previous.velocity = f->velocity;

						f->lifetime -= static_cast<float>(dt);
						f->velocity += f->acceleration
							* static_cast<float>(dt);
						f->position += f->velocity
							* static_cast<float>(dt);
						if (f->tracked)
							debug_file << t << ','
							<< f->position.x << ','
							<< f->position.y << ','
							<< f->position.z << '\n';
						if (!f->child)
						{
							if (!f->tracked)
							{
								f->scale.y = f->velocity.y / 5.0f;
								if (f->velocity.y <= 0.0f)
									f->lifetime = 0.0f;
							}
						}
						else
						{
							f->rgba.w -= static_cast<float>(dt)
								* glm::exp (0.0005f * (f->lifespan - f->lifetime));
							if (f->rgba.w < 0.2f)
								f->lifetime = 0.0f;
						}
					}
					for (auto& p : deadParticles)
						particlePool.remove (p);
					for (auto& p : babyParticles)
						particlePool.push_back (p);
				}
				accumulator -= update_rate;
				t += dt;
			}
		}
		else accumulator = 0.0;
		alpha = accumulator / dt;
		updateTime = glfwGetTime () - startTime;

		// Rendering
		startTime = glfwGetTime ();
		glClearColor(0.035f, 0.031f, 0.004f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Fireworks
		float draw_alpha = 0.0f;
		float draw_lifetime = 0.0f;
		glm::vec3 draw_position (0.0f);
		glm::vec3 draw_velocity (0.0f);
		glm::vec4 draw_color (0.0f);

		glEnable (GL_CULL_FACE);
		fireworkShader.use ();
		fireworkShader.setMat4 ("view", view);
		fireworkShader.setMat4 ("projection", projection);
		for (auto& p : particlePool)
		{
			if (!p->active)
				continue;

			draw_alpha = glm::mix (p->previous.alpha, p->rgba.w, alpha);
			draw_color = glm::vec4(p->rgba.x, p->rgba.y, p->rgba.z, draw_alpha);
			draw_lifetime = glm::mix (p->previous.lifetime, p->lifetime, alpha);
			draw_position = glm::mix (p->previous.position, p->position, alpha);
			draw_velocity = glm::mix (p->previous.velocity, p->velocity, alpha);

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

			
			fireworkShader.setVec4 ("u_lightColor", p->rgba);
			fireworkShader.setMat4 ("model", model);

			lightSource.Draw ();
			fireworks_draw_calls++;
		}
		glDisable (GL_CULL_FACE);

		// Water plane
		if (show_water)
		{
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
		}

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
		sceneRenderTime = glfwGetTime () - startTime;

		// Stats 
		startTime = glfwGetTime ();
		glEnable (GL_CULL_FACE);
		glm::vec4 textColor (1.0f, 1.0f, 1.0f, 1.0f);
		glm::mat4 orthografic = glm::ortho (0.0f,
									   (float) winWidth,
									   0.0f,
									   (float) winHeight);
		textShader.use ();
		textShader.setVec4 ("textColor", textColor);
		textShader.setMat4 ("projection", orthografic);

		
		if (! show_controls)
			DrawHelp (arialFont,
					  static_cast<float>(winWidth / 2) - 150.0f,
					  static_cast<float>(winHeight) - 25.0f,
					  0.5f);
		else
			DrawControls (arialFont,
						  10.0f,
						  25.0f,
						  0.5f);

		if (show_stats)
		{
		DrawFPS (arialFont,
				 static_cast<float>(winWidth) - 150.0f,
				 static_cast<float>(winHeight) - 25.0f,
				 0.5f,
				 static_cast<int>(FPS));
		DrawStats (arialFont,
				   10.0f,
				   static_cast<float>(winHeight) - 25.0f,
				   0.5f,
				   static_cast<float>(frameTime),
				   static_cast<float>(inputTime),
				   static_cast<float>(updateTime),
				   static_cast<float>(renderTime),
				   vsync, pause, show_active_particles);
		}
		glDisable (GL_CULL_FACE);
		textRenderTime = glfwGetTime () - startTime;
		renderTime = sceneRenderTime + textRenderTime;

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
		camera.ProcessKeyboard(Camera::Camera_Movement::FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(Camera::Camera_Movement::LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(Camera::Camera_Movement::BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(Camera::Camera_Movement::RIGHT, deltaTime);
	if (glfwGetKey (window, GLFW_KEY_SPACE) == GLFW_PRESS)
		camera.ProcessKeyboard (Camera::Camera_Movement::UP, deltaTime);
	if (glfwGetKey (window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		camera.ProcessKeyboard (Camera::Camera_Movement::DOWN, deltaTime);
}

void key_callback(GLFWwindow *window,
				  int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_UP && (action == GLFW_REPEAT || action == GLFW_PRESS)
			&& show_controls)
		selected = (selected + 1) % static_cast<unsigned>(controls.size());
	if (key == GLFW_KEY_DOWN && (action == GLFW_REPEAT || action == GLFW_PRESS)
			&& show_controls)
		selected = (selected + controls.size () - 1) % controls.size ();
	if (key == GLFW_KEY_LEFT && (action == GLFW_REPEAT || action == GLFW_PRESS)
			&& show_controls)
		controls[selected].second -= 0.1f;
	if (key == GLFW_KEY_RIGHT && (action == GLFW_REPEAT || action == GLFW_PRESS)
			&& show_controls)
		controls[selected].second += 0.1f;
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (key == GLFW_KEY_F3 && action == GLFW_PRESS)
		show_stats = !show_stats;
	if (key == GLFW_KEY_H && action == GLFW_PRESS)
		show_controls = !show_controls;
	if (key == GLFW_KEY_R && action == GLFW_PRESS)
		show_water = !show_water;
	if (key == GLFW_KEY_TAB && action == GLFW_PRESS && show_stats)
		show_active_particles = !show_active_particles;
	if (key == GLFW_KEY_V && action == GLFW_PRESS)
		vsync = !vsync, glfwSwapInterval (vsync);
	if (key == GLFW_KEY_P && action == GLFW_PRESS)
		pause = !pause;
	if (key == GLFW_KEY_1 && action == GLFW_PRESS)
		show_type ^= REPEAT;
	if (key == GLFW_KEY_2 && action == GLFW_PRESS)
		show_type ^= RANDOM;
	if (key == GLFW_KEY_3 && action == GLFW_PRESS)
		show_type ^= ALL;
	if (key == GLFW_KEY_F && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
		glm::vec3 firePos;
		float station = rand () / (float) RAND_MAX * 3.0f;
		if (station < 1.0f)
			firePos = emitters[0];
		else if (station < 2.0f)
			firePos = emitters[1];
		else
			firePos = emitters[2];

		FireworkParticle *p = new FireworkParticle;
		float dlife = rand () / (float) RAND_MAX * 3.0f;

		p->lifespan = controls[PROJECTILE_LIFESPAN].second + dlife;
		p->lifetime = p->lifespan;
		p->active = true;

		p->mass = controls[PROJECTILE_MASS].second;
		p->acceleration = p->mass
			* glm::vec3 (0.0f, controls[GRAVITY_FORCE].second, 0.0f);

		p->position = firePos;
		p->scale = { 0.2f, 5.0f, 0.2f };

		float dr = rand () / (float) RAND_MAX * 0.34f + 0.66f;
		float dg = rand () / (float) RAND_MAX * 0.34f + 0.66f;
		float db = rand () / (float) RAND_MAX * 0.34f + 0.66f;
		p->rgba = glm::vec4 (dr, dg, db, 1.0f);

		float base_speed = controls[PROJECTILE_VELOCITY].second;
		float dx = rand () / (float) RAND_MAX * 20.0f - 10.0f;
		float dy = rand () / (float) RAND_MAX * 15.0f;
		float dz = rand () / (float) RAND_MAX * 20.0f - 10.0f;
		glm::vec3 impulse (dx, base_speed + dy, dz);
		p->velocity = impulse;

		particlePool.push_back (std::shared_ptr<FireworkParticle> (p));
	}
	if (key == GLFW_KEY_T && action == GLFW_PRESS)
	{
		if (!debug_file.is_open ())
			debug_file.open ("pipilica.txt");

		FireworkParticle *p = new FireworkParticle;

		p->lifespan = 5.0f;
		p->lifetime = p->lifespan;
		p->active = true;
		p->tracked = true;

		p->mass = controls[PROJECTILE_MASS].second;
		p->acceleration = p->mass
			* glm::vec3 (0.0f, controls[GRAVITY_FORCE].second, 0.0f);

		p->position = emitters[0];
		p->scale = { 0.2f, 5.0f, 0.2f };
		p->rgba = glm::vec4 (1.0f, 1.0f, 1.0f, 1.0f);

		glm::vec3 impulse (5.0f, 40.0f, 60.0f);
		p->velocity = impulse;

		particlePool.push_back (std::shared_ptr<FireworkParticle> (p));
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

void FireFireworks (unsigned type)
{
	if (type & REPEAT && glfwGetTime () - last.repeat >= wait.repeat)
	{
		glm::vec3 firePos = emitters[count];

		FireworkParticle *p = new FireworkParticle;
		float dlife = rand () / (float) RAND_MAX * 3.0f;

		p->lifespan = controls[PROJECTILE_LIFESPAN].second + dlife;
		p->lifetime = p->lifespan;
		p->active = true;

		p->mass = controls[PROJECTILE_MASS].second;
		p->acceleration = p->mass
			* glm::vec3 (0.0f, controls[GRAVITY_FORCE].second, 0.0f);

		p->position = firePos;
		p->scale = { 0.2f, 5.0f, 0.2f };

		float dr = rand () / (float) RAND_MAX * 0.34f + 0.66f;
		float dg = rand () / (float) RAND_MAX * 0.34f + 0.66f;
		float db = rand () / (float) RAND_MAX * 0.34f + 0.66f;
		p->rgba = glm::vec4 (dr, dg, db, 1.0f);

		float base_speed = controls[PROJECTILE_VELOCITY].second;
		float dx = rand () / (float) RAND_MAX * 1.0f - 0.5f;
		// float dy = rand () / (float) RAND_MAX * 15.0f;
		float dz = rand () / (float) RAND_MAX * 1.0f - 0.5f;
		glm::vec3 impulse (dx, base_speed, dz);
		p->velocity = impulse;

		particlePool.push_back (std::shared_ptr<FireworkParticle> (p));
		if (count == 2) dir_forward = -1;
		else if (count == 0) dir_forward = 1;
		count += 1 * dir_forward;
		last.repeat = glfwGetTime ();
	}
	if (type & RANDOM && glfwGetTime () - last.random >= wait.random)
	{
		glm::vec3 firePos;
		float station = rand () / (float) RAND_MAX * 3.0f;
		if (station < 1.0f)
			firePos = emitters[0];
		else if (station < 2.0f)
			firePos = emitters[1];
		else
			firePos = emitters[2];

		FireworkParticle *p = new FireworkParticle;
		float dlife = rand () / (float) RAND_MAX * 3.0f;

		p->lifespan = controls[PROJECTILE_LIFESPAN].second + dlife;
		p->lifetime = p->lifespan;
		p->active = true;

		p->mass = controls[PROJECTILE_MASS].second;
		p->acceleration = p->mass
			* glm::vec3 (0.0f, controls[GRAVITY_FORCE].second, 0.0f);

		p->position = firePos;
		p->scale = { 0.2f, 5.0f, 0.2f };

		float dr = rand () / (float) RAND_MAX * 0.34f + 0.66f;
		float dg = rand () / (float) RAND_MAX * 0.34f + 0.66f;
		float db = rand () / (float) RAND_MAX * 0.34f + 0.66f;
		p->rgba = glm::vec4 (dr, dg, db, 1.0f);

		float base_speed = controls[PROJECTILE_VELOCITY].second;
		float dx = rand () / (float) RAND_MAX * 20.0f - 10.0f;
		float dy = rand () / (float) RAND_MAX * 15.0f;
		float dz = rand () / (float) RAND_MAX * 20.0f - 10.0f;
		glm::vec3 impulse (dx, base_speed + dy, dz);
		p->velocity = impulse;

		particlePool.push_back (std::shared_ptr<FireworkParticle> (p));
		last.random = glfwGetTime ();
	}
	wait.all = controls[ALL_WAIT_TIME].second;
	if (type & ALL && glfwGetTime () - last.all >= wait.all)
	{
		constexpr unsigned MAX_LOAD = 50;
		for (int count = 0; count < MAX_LOAD; count++)
		{
			glm::vec3 firePos = emitters[count % 3];

			FireworkParticle *p = new FireworkParticle;
			float dlife = rand () / (float) RAND_MAX * 3.0f;

			p->lifespan = controls[PROJECTILE_LIFESPAN].second + dlife;
			p->lifetime = p->lifespan;
			p->active = true;

			p->mass = controls[PROJECTILE_MASS].second;
			p->acceleration = p->mass
				* glm::vec3 (0.0f, controls[GRAVITY_FORCE].second, 0.0f);

			p->position = firePos;
			p->scale = { 0.2f, 5.0f, 0.2f };

			float dr = rand () / (float) RAND_MAX * 0.34f + 0.66f;
			float dg = rand () / (float) RAND_MAX * 0.34f + 0.66f;
			float db = rand () / (float) RAND_MAX * 0.34f + 0.66f;
			p->rgba = glm::vec4 (dr, dg, db, 1.0f);

			float base_speed = controls[PROJECTILE_VELOCITY].second;
			float dx = rand () / (float) RAND_MAX * 5.0f - 2.5f;
			float dy = rand () / (float) RAND_MAX * 15.0f;
			float dz = rand () / (float) RAND_MAX * 5.0f - 2.5f;
			glm::vec3 impulse (dx, base_speed + dy, dz);
			p->velocity = impulse;

			particlePool.push_back (std::shared_ptr<FireworkParticle> (p));
		}
		last.all = glfwGetTime ();
	}
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
	std::cout << "Pause: \t\t" << (pause ? "ON" : "OFF") << '\n';
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

void DrawFPS (unsigned font, float x, float y, float scale, int value)
{
	std::stringstream s;
	s << "FPS: " << value;
	Text::Draw (font, s.str (), x, y, scale, glm::vec4 (0.0f));
	text_draw_calls++;
}

void DrawStats (unsigned font, float x, float y, float scale,
				float ft, float it, float ut, float rt,
				bool vsync, bool pause, bool show_active_particles)
{
	int total_mem_kb, used_mem_kb, available_mem_kb;
	glGetIntegerv (GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX,
				  &total_mem_kb);
	glGetIntegerv (GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX,
				  &available_mem_kb);
	used_mem_kb = total_mem_kb - available_mem_kb;

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

	std::stringstream s;
	total_draw_calls = fireworks_draw_calls
		+ water_draw_calls
		+ skybox_draw_calls
		+ text_draw_calls
		+ 9;
	s << "Draw calls: " << total_draw_calls;
	Text::Draw (font, s.str (), x, y, scale, glm::vec4 (0.0f));
	y -= 25;

	for (unsigned i = 0; i < 4; i++)
	{
		std::stringstream s;
		s << std::fixed;
		s.precision (2);
		s << texts[i] << values[i];
		Text::Draw (font, s.str (), x, y, scale, glm::vec4 (0.0f));
		y -= 25;
	}

	std::stringstream ss;
	ss << "MEM: " << used_mem_kb / 1024 << "/" << total_mem_kb / 1024 << " MB";
	Text::Draw (font, ss.str (), x, y, scale, glm::vec4 (0.0f));
	y -= 25;

	std::stringstream s0;
	s0 << "Vsync: " << (vsync ? "ON" : "OFF");
	Text::Draw (font, s0.str (), x, y, scale, glm::vec4 (0.0f));
	y -= 25;

	std::stringstream s1;
	s1 << "Pause: " << (pause ? "ON" : "OFF");
	Text::Draw (font, s1.str (), x, y, scale, glm::vec4 (0.0f));
	y -= 25;

	std::stringstream s2;
	s2 << "Active particles: " << (show_active_particles ?
		std::to_string (particlePool.size ()) : "OFF");
	Text::Draw (font, s2.str (), x, y, scale, glm::vec4 (0.0f));
}

void DrawHelp (unsigned font, float x, float y, float scale)
{
	std::stringstream s;
	s << "Press H to show controls";
	Text::Draw (font, s.str (), x, y, scale, glm::vec4 (0.0f));
	text_draw_calls++;
}

void DrawControls (unsigned font, float x, float y, float scale)
{
	std::stringstream s;
	s << ">";
	Text::Draw (font, s.str (), x, y + selected * 25.0f, scale,
				glm::vec4 (0.0f));
	text_draw_calls++;

	for (auto control : controls)
	{
		std::stringstream s;
		s << control.first << control.second;
		Text::Draw (font, s.str (), x + 20.0f, y, scale, glm::vec4 (0.0f));
		y += 25;
		text_draw_calls++;
	}
}