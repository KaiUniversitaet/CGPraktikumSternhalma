#include <iostream>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// GLM Mathematics
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

// Other Libs
#include <SOIL.h>

// Other includes
#include "Shader.h"
#include "Camera.h"
#include "GamePieceHandle.h"


// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void doCameraMovement();
GLuint loadCubemap(vector<const GLchar*> faces);

// Camera
Camera camera(glm::vec3(0.0f, 1.0f, 2.0f));

// Keyboard
bool keys[1024];

// Mouse
GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;

// Light attributes
glm::vec3 lightPos(-2.0f, 1.0f, 2.0f);

// Frames and time
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

// Window dimensions
const GLuint WIDTH = 1000, HEIGHT = 600;

// Distance between the spots where the pieces are being located
const GLfloat cposX = 0.397f / 3.0f;
const GLfloat cposZ = 0.92f / 8.0f;

// Andimation prototypes
void animation();
void animationProc(GamePieceHandle& pm, GamePieceHandle::DIRECTION dir, int index, GLfloat deltaTime, GLfloat durationJump, GLfloat distance);

// Animation globals
bool animate = false;
GamePieceHandle::DIRECTION jumpdir = GamePieceHandle::NEAST;

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_SAMPLES, 8);

	glEnable(GL_MULTISAMPLE);

	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Sternhalma", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);

	glewExperimental = GL_TRUE;
	glewInit();
	glViewport(0, 0, WIDTH, HEIGHT);
	glEnable(GL_DEPTH_TEST);

	Shader brettShader("brettShader.vert", "brettShader.frag");
	Shader skyboxShader("skyboxShader.vert", "skyboxShader.frag");

	// FIGUREN
	GLfloat posX = 0.4f / 3.0f, posZ = 0.92f / 8.0f;
	GamePieceHandle piecesHandle("gamepiece.obj", "objShader.vert", "objShader.frag", 1.0f, 1.0f);
	// rote Figuren
	piecesHandle.addPiece(glm::vec3(0.0f, 0.03f, 8 * posZ), GamePieceHandle::RED);
	piecesHandle.addPiece(glm::vec3(-posX, 0.03f, 6 * posZ), GamePieceHandle::RED);
	piecesHandle.addPiece(glm::vec3(posX / 2, 0.03f, 5 * posZ), GamePieceHandle::RED);
	piecesHandle.addPiece(glm::vec3(2 * posX, 0.03f, 4 * posZ), GamePieceHandle::RED);
	piecesHandle.addPiece(glm::vec3(posX / 2, 0.03f, 7 * posZ), GamePieceHandle::RED);
	piecesHandle.addPiece(glm::vec3(-posX, 0.03f, 4 * posZ), GamePieceHandle::RED);
	// blaue Figuren
	piecesHandle.addPiece(glm::vec3(5 * -posX, 0.03f, 4 * -posZ), GamePieceHandle::BLUE);
	piecesHandle.addPiece(glm::vec3(7 * -posX / 2, 0.03f, 3 * -posZ), GamePieceHandle::BLUE);
	piecesHandle.addPiece(glm::vec3(4 * -posX, 0.03f, 2 * -posZ), GamePieceHandle::BLUE);
	piecesHandle.addPiece(glm::vec3(7 * -posX / 2, 0.03f, 1 * -posZ), GamePieceHandle::BLUE);
	piecesHandle.addPiece(glm::vec3(4 * -posX, 0.03f, 0.0f), GamePieceHandle::BLUE);
	piecesHandle.addPiece(glm::vec3(4 * -posX / 2, 0.03f, 4 * -posZ), GamePieceHandle::BLUE);
	// grüne Figuren
	piecesHandle.addPiece(glm::vec3(6 * posX, 0.03f, 4 * -posZ), GamePieceHandle::GREEN);
	piecesHandle.addPiece(glm::vec3(7 * posX / 2, 0.03f, 3 * -posZ), GamePieceHandle::GREEN);
	piecesHandle.addPiece(glm::vec3(4 * posX, 0.03f, 2 * -posZ), GamePieceHandle::GREEN);
	piecesHandle.addPiece(glm::vec3(7 * posX / 2, 0.03f, 1 * -posZ), GamePieceHandle::GREEN);
	piecesHandle.addPiece(glm::vec3(4 * posX, 0.03f, 0.0f), GamePieceHandle::GREEN);
	piecesHandle.addPiece(glm::vec3(8 * posX / 2, 0.03f, 4 * -posZ), GamePieceHandle::GREEN);

	// Skybox
	vector<const GLchar*> faces;
	faces.push_back("skybox/right.jpg");
	faces.push_back("skybox/left.jpg");
	faces.push_back("skybox/top.jpg");
	faces.push_back("skybox/bottom.jpg");
	faces.push_back("skybox/back.jpg");
	faces.push_back("skybox/front.jpg");
	GLuint cubemapTexture = loadCubemap(faces);

	// Vertex data and buffers and attribute pointers for the game board and the sky box
	GLfloat X = 1.0f, Y = 0.03f, Z = 1.0f;
	GLfloat brettVertices[] = {
		// position		// brett		// normalen
		-X, -Y, -Z,		1.0f, 0.0f,		0.0f,  0.0f, -1.0f,	// back
		X, -Y, -Z,		0.0f, 0.0f,		0.0f,  0.0f, -1.0f,
		X,  Y, -Z,		0.0f, 0.5f,		0.0f,  0.0f, -1.0f,
		X,  Y, -Z,		0.0f, 0.5f,		0.0f,  0.0f, -1.0f,
		-X,  Y, -Z,		1.0f, 0.5f,		0.0f,  0.0f, -1.0f,
		-X, -Y, -Z,		1.0f, 0.0f,		0.0f,  0.0f, -1.0f,

		-X, -Y,  Z,		0.0f, 0.0f,		0.0f,  0.0f,  1.0f,	// front
		X, -Y,  Z,		1.0f, 0.0f,		0.0f,  0.0f,  1.0f,
		X,  Y,  Z,		1.0f, 0.5f,		0.0f,  0.0f,  1.0f,
		X,  Y,  Z,		1.0f, 0.5f,		0.0f,  0.0f,  1.0f,
		-X,  Y,  Z,		0.0f, 0.5f,		0.0f,  0.0f,  1.0f,
		-X, -Y,  Z,		0.0f, 0.0f,		0.0f,  0.0f,  1.0f,

		-X,  Y,  Z,		1.0f, 0.5f,		-1.0f,  0.0f,  0.0f,	// left
		-X,  Y, -Z,		0.0f, 0.5f,		-1.0f,  0.0f,  0.0f,
		-X, -Y, -Z,		0.0f, 0.0f,		-1.0f,  0.0f,  0.0f,
		-X, -Y, -Z,		0.0f, 0.0f,		-1.0f,  0.0f,  0.0f,
		-X, -Y,  Z,		1.0f, 0.0f,		-1.0f,  0.0f,  0.0f,
		-X,  Y,  Z,		1.0f, 0.5f,		-1.0f,  0.0f,  0.0f,

		X,  Y,  Z,		0.0f, 0.5f,		1.0f,  0.0f,  0.0f,	// right
		X,  Y, -Z,		1.0f, 0.5f,		1.0f,  0.0f,  0.0f,
		X, -Y, -Z,		1.0f, 0.0f,		1.0f,  0.0f,  0.0f,
		X, -Y, -Z,		1.0f, 0.0f,		1.0f,  0.0f,  0.0f,
		X, -Y,  Z,		0.0f, 0.0f,		1.0f,  0.0f,  0.0f,
		X,  Y,  Z,		0.0f, 0.5f,		1.0f,  0.0f,  0.0f,

		-X, -Y, -Z,		0.0f, 0.0f,		0.0f, -1.0f,  0.0f,	// down
		X, -Y, -Z,		1.0f, 0.0f,		0.0f, -1.0f,  0.0f,
		X, -Y,  Z,		1.0f, 0.5f,		0.0f, -1.0f,  0.0f,
		X, -Y,  Z,		1.0f, 0.5f,		0.0f, -1.0f,  0.0f,
		-X, -Y,  Z,		0.0f, 0.5f,		0.0f, -1.0f,  0.0f,
		-X, -Y, -Z,		0.0f, 0.0f,		0.0f, -1.0f,  0.0f,

		-X,  Y, -Z,		0.0f, 0.5f,		0.0f,  1.0f,  0.0f,	// up
		X,  Y, -Z,		1.0f, 0.5f,		0.0f,  1.0f,  0.0f,
		X,  Y,  Z,		1.0f, 1.0f,		0.0f,  1.0f,  0.0f,
		X,  Y,  Z,		1.0f, 1.0f,		0.0f,  1.0f,  0.0f,
		-X,  Y,  Z,		0.0f, 1.0f,		0.0f,  1.0f,  0.0f,
		-X,  Y, -Z,		0.0f, 0.5f,		0.0f,  1.0f,  0.0f
	};

	GLfloat skyboxVertices[] = {
		// Positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f,  1.0f
	};

	// BOARD: VAO und VBO
	GLuint VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(brettVertices), brettVertices, GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	// Color attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	// Normal attribute
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0); 


	// SKYBOX: VAO und VBO
	GLuint skyboxVBO, skyboxVAO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);


	// TEXTURE for BOARD
	GLuint textureHalma;
	glGenTextures(1, &textureHalma);
	glBindTexture(GL_TEXTURE_2D, textureHalma); 

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width, height;
	unsigned char* image = SOIL_load_image("brett.png", &width, &height, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);

	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0); 


	// Game loop
	while (!glfwWindowShouldClose(window))
	{
		// Set frame time
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// EVENT CHECKS and processing 
		glfwPollEvents();
		doCameraMovement();
		// Check if animation was requested, only if there isn't already one running
		if (!animate) animation();
		// Only process animation if activated
		if (animate) animationProc(piecesHandle, jumpdir, 0, deltaTime, 0.2f, cposX * 2);

		// Clear the colorbuffer and depthbuffer
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// SKYBOX
		// Activate and draw Skybox
		glDepthMask(GL_FALSE);
		skyboxShader.Use();
		glm::mat4 model;
		glm::mat4 skyboxModel = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));
		glm::mat4 view;
		view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
		glm::mat4 projection;
		projection = glm::perspective(camera.Zoom, (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 1000.0f);
		GLint viewLoc = glGetUniformLocation(skyboxShader.Program, "view");
		GLint projLoc = glGetUniformLocation(skyboxShader.Program, "projection");
		GLint modelLoc = glGetUniformLocation(skyboxShader.Program, "model");
		// Pass the matrices to the shader
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(skyboxModel));
		glBindVertexArray(skyboxVAO);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthMask(GL_TRUE);

		// GAME BOARD
		// Activate shader brett
		brettShader.Use();
		view = camera.GetViewMatrix();
		GLint lightPosLoc = glGetUniformLocation(brettShader.Program, "lightPos");
		GLint viewPosLoc = glGetUniformLocation(brettShader.Program, "viewPos");
		GLint lightColorLoc = glGetUniformLocation(brettShader.Program, "lightColor");
		glUniform3f(lightPosLoc, lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(viewPosLoc, camera.Position.x, camera.Position.y, camera.Position.z);
		glm::vec3 lightColor = glm::vec3(1.0f);
		glUniform3f(lightColorLoc, lightColor.x, lightColor.y, lightColor.z);

		// Bind Textures using texture units
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureHalma);
		glUniform1i(glGetUniformLocation(brettShader.Program, "halmaTexture"), 0);

		// Create camera transformation
		// Get the uniform locations
		modelLoc = glGetUniformLocation(brettShader.Program, "model");
		viewLoc = glGetUniformLocation(brettShader.Program, "view");
		projLoc = glGetUniformLocation(brettShader.Program, "projection");
		// Pass the matrices to the shader
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

		// Draw brett
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);

		// PIECES      (transformation and drawing is capsulated in the handle class)
		// Draw pieces
		piecesHandle.drawPieces(view, projection, lightPos, camera);


		glfwSwapBuffers(window);
	}

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glfwTerminate();
	return 0;
}

// load Skybox faces
GLuint loadCubemap(vector<const GLchar*> faces)
{
	GLuint textureID;
	glGenTextures(1, &textureID);
	glActiveTexture(GL_TEXTURE0);

	int width, height;
	unsigned char* image;

	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	for (GLuint i = 0; i < faces.size(); i++)
	{
		image = SOIL_load_image(faces[i], &width, &height, 0, SOIL_LOAD_RGB);
		glTexImage2D(
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
			GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image
			);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return textureID;
}


void doCameraMovement()
{

	if (keys[GLFW_KEY_W])
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (keys[GLFW_KEY_S])
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (keys[GLFW_KEY_A])
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (keys[GLFW_KEY_D])
		camera.ProcessKeyboard(RIGHT, deltaTime);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
		{
			keys[key] = true;
		}
		else if (action == GLFW_RELEASE)
		{
			keys[key] = false;
		}
	}
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	GLfloat xoffset = xpos - lastX;
	GLfloat yoffset = lastY - ypos; 

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}


void animation() {

	if (keys[GLFW_KEY_I]) {
		if (!animate) {
			jumpdir = GamePieceHandle::NEAST;
			animate = true;
		}
	}
	else if (keys[GLFW_KEY_H]) {
		if (!animate) {
			jumpdir = GamePieceHandle::WEST;
			animate = true;
		}
	}
	else if (keys[GLFW_KEY_M]) {
		if (!animate) {
			jumpdir = GamePieceHandle::SEAST;
			animate = true;
		}
	}
	else if (keys[GLFW_KEY_U]) {
		if (!animate) {
			jumpdir = GamePieceHandle::NWEST;
			animate = true;
		}
	}
	else if (keys[GLFW_KEY_K]) {
		if (!animate) {
			jumpdir = GamePieceHandle::EAST;
			animate = true;
		}
	}
	else if (keys[GLFW_KEY_N]) {
		if (!animate) {
			jumpdir = GamePieceHandle::SWEST;
			animate = true;
		}
	}
}

void animationProc(GamePieceHandle& pm, GamePieceHandle::DIRECTION dir, int index, GLfloat deltaTime, GLfloat durationJump, GLfloat distance) {
	// If animation has finished it will return FALSE, hence the next loop round can check for new animation requests
	animate = pm.animate(0, dir, deltaTime, durationJump, distance);
}