#include "Graphics\window.h"
#include "Camera\camera.h"
#include "Shaders\shader.h"
#include "Model Loading\mesh.h"
#include "Model Loading\texture.h"
#include "Model Loading\meshLoaderObj.h"

#include "ImGui/imgui.h";
#include "ImGui/imgui_impl_glfw_gl3.h";
#include <math.h>

#define MIN(X,Y) ((X<Y)?X:Y)
#define MAX(X,Y) ((X>Y)?X:Y)

void processKeyboardInput();
void processCarMovement();
void processAnimation();

// AABB Collision Detection
void BBox(Mesh p, int n_vert, Vertex& p_max, Vertex& p_min);
bool intersect(Vertex aMin, Vertex aMax, Vertex bMin, Vertex bMax, float aSize, float bSize);

float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f;

Window window("RPG Game", 800, 800);
Camera camera;

glm::vec3 lightColor = glm::vec3(1.0f);
glm::vec3 lightPos = glm::vec3(-180.0f, 100.0f, -200.0f);

float xPos = -15.0f;
float zPos = 70.0f;
float carRot = 180.0f;
float dolzPos = -20.0f;
float dolRot = -70.0f;
bool isMoving = true;
bool isAnimating = false;

int main()
{
	// Building and compiling shader program
	Shader shader("Shaders/vertex_shader.glsl", "Shaders/fragment_shader.glsl");
	Shader sunShader("Shaders/sun_vertex_shader.glsl", "Shaders/sun_fragment_shader.glsl");
	Shader lakeShader("Shaders/lake_vertex_shader.glsl", "Shaders/fragment_shader.glsl");

	// Textures
	GLuint tex = loadBMP("Resources/Textures/lake.bmp");
	GLuint tex2 = loadBMP("Resources/Textures/tree.bmp");
	GLuint tex3 = loadBMP("Resources/Textures/hills.bmp");
	GLuint tex4 = loadBMP("Resources/Textures/grass.bmp");
	GLuint tex5 = loadBMP("Resources/Textures/orange.bmp");
	GLuint tex6 = loadBMP("Resources/Textures/dolphin.bmp");

	glEnable(GL_DEPTH_TEST);

	// Test custom mesh loading
	std::vector<Vertex> vert;
	vert.push_back(Vertex());
	vert[0].pos = glm::vec3(10.5f, 10.5f, 0.0f);
	vert[0].textureCoords = glm::vec2(1.0f, 1.0f);

	vert.push_back(Vertex());
	vert[1].pos = glm::vec3(10.5f, -10.5f, 0.0f);
	vert[1].textureCoords = glm::vec2(1.0f, 0.0f);

	vert.push_back(Vertex());
	vert[2].pos = glm::vec3(-10.5f, -10.5f, 0.0f);
	vert[2].textureCoords = glm::vec2(0.0f, 0.0f);

	vert.push_back(Vertex());
	vert[3].pos = glm::vec3(-10.5f, 10.5f, 0.0f);
	vert[3].textureCoords = glm::vec2(0.0f, 1.0f);

	vert[0].normals = glm::normalize(glm::cross(vert[1].pos - vert[0].pos, vert[3].pos - vert[0].pos));
	vert[1].normals = glm::normalize(glm::cross(vert[2].pos - vert[1].pos, vert[0].pos - vert[1].pos));
	vert[2].normals = glm::normalize(glm::cross(vert[3].pos - vert[2].pos, vert[1].pos - vert[2].pos));
	vert[3].normals = glm::normalize(glm::cross(vert[0].pos - vert[3].pos, vert[2].pos - vert[3].pos));

	std::vector<int> ind = { 0, 1, 3,
		1, 2, 3 };

	std::vector<Texture> textures;
	textures.push_back(Texture());
	textures[0].id = tex;
	textures[0].type = "texture_diffuse";

	std::vector<Texture> textures2;
	textures2.push_back(Texture());
	textures2[0].id = tex2;
	textures2[0].type = "texture_diffuse";

	std::vector<Texture> textures3;
	textures3.push_back(Texture());
	textures3[0].id = tex3;
	textures3[0].type = "texture_diffuse";

	std::vector<Texture> textures4;
	textures4.push_back(Texture());
	textures4[0].id = tex4;
	textures4[0].type = "texture_diffuse";

	std::vector<Texture> textures5;
	textures5.push_back(Texture());
	textures5[0].id = tex5;
	textures5[0].type = "texture_diffuse";

	std::vector<Texture> textures6;
	textures6.push_back(Texture());
	textures6[0].id = tex6;
	textures6[0].type = "texture_diffuse";

	Mesh mesh(vert, ind, textures3);

	// Create Obj files & add their textures
	MeshLoaderObj loader;

	Mesh sun = loader.loadObj("Resources/Models/sphere.obj");

	Mesh player = loader.loadObj("Resources/Models/player.obj", textures5);
	Mesh dolphin = loader.loadObj("Resources/Models/dolphin.obj", textures6);

	Mesh terrain = loader.loadObj("Resources/Models/plane.obj", textures4);
	Mesh lake = loader.loadObj("Resources/Models/plane1.obj", textures);

	Mesh bigTree = loader.loadObj("Resources/Models/tree.obj", textures2);
	Mesh smallTree = loader.loadObj("Resources/Models/tree02.obj", textures2);

	Mesh hillsOne = loader.loadObj("Resources/Models/stone_with_moss_1.obj", textures3);
	Mesh hillsTwo = loader.loadObj("Resources/Models/stone_with_moss_3.obj", textures3);
	Mesh hillsThree = loader.loadObj("Resources/Models/stone_3.obj", textures3);
	Mesh hillsFour = loader.loadObj("Resources/Models/stone_with_moss_7.obj", textures3);
	Mesh hillsFive = loader.loadObj("Resources/Models/stone_2.obj", textures3);

	unsigned int counter = 0;

	// Imgui initialization
	ImGui::CreateContext();
	ImGui_ImplGlfwGL3_Init(window.getWindow(), false);
	ImGui::StyleColorsDark();
	bool show_demo_window = true;
	bool show_another_window = false;
	ImVec4 bg_color = ImVec4(0.2f, 0.8f, 1.0f, 1.0f);

	// Check if we close the window or press the escape button
	while (!window.isPressed(GLFW_KEY_ESCAPE) && glfwWindowShouldClose(window.getWindow()) == 0)
	{
		window.clear();
		ImGui_ImplGlfwGL3_NewFrame();
		glClearColor(bg_color.x, bg_color.y, bg_color.z, bg_color.w);

		// Get FPS & ms
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		counter++;
		if (currentFrame >= 1.0 / 3.0) {
			std::string FPS = std::to_string((1.0 / deltaTime) * counter);
			std::string ms = std::to_string((deltaTime / counter) * 1000);
			std::string title = "RPG Game - " + FPS + " FPS / " + ms + " ms";
			glfwSetWindowTitle(window.getWindow(), title.c_str());
			lastFrame = currentFrame;
			counter = 0;
		}

		processKeyboardInput();
		processCarMovement();
		processAnimation();

		// Sun - light source
		sunShader.use();
		glm::mat4 ProjectionMatrix = glm::perspective(90.0f, window.getWidth() * 1.0f / window.getHeight(), 0.1f, 10000.0f);
		glm::mat4 ViewMatrix = glm::lookAt(camera.getCameraPosition(), camera.getCameraPosition() + camera.getCameraViewDirection(), camera.getCameraUp());
		GLuint MatrixID = glGetUniformLocation(sunShader.getId(), "MVP");
		glm::mat4 ModelMatrix = glm::mat4(1.0);
		ModelMatrix = glm::translate(ModelMatrix, lightPos);
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		sun.draw(sunShader);

		GLuint MatrixID2 = glGetUniformLocation(shader.getId(), "MVP");
		GLuint ModelMatrixID = glGetUniformLocation(shader.getId(), "model");

		// Player
		shader.use();
		ModelMatrix = glm::mat4(1.0);
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(xPos, -19.5f, zPos));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(10.0f, 10.0f, 10.0f));
		ModelMatrix = glm::rotate(ModelMatrix, carRot, glm::vec3(0.0f, -19.5f, 0.0f));
		MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
		glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniform3f(glGetUniformLocation(shader.getId(), "lightColor"), lightColor.x, lightColor.y, lightColor.z);
		glUniform3f(glGetUniformLocation(shader.getId(), "lightPos"), lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(glGetUniformLocation(shader.getId(), "viewPos"), camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);

		Vertex playerMin, playerMax;
		BBox(player, player.vertices.size(), playerMax, playerMin);
		player.draw(shader);
		playerMin.pos.x += xPos;
		playerMin.pos.y += -22.0f;
		playerMin.pos.z += zPos;
		playerMax.pos.x += xPos;
		playerMax.pos.y += -22.0f;
		playerMax.pos.z += zPos;

		// Dolphin
		shader.use();
		ModelMatrix = glm::mat4(1.0);
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-20.0f, -20.0f, dolzPos));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(10.0f, 10.0f, 10.0f));
		ModelMatrix = glm::rotate(ModelMatrix, -dolRot, glm::vec3(-19.5f, 0.0f, 0.0f));
		MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
		glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniform3f(glGetUniformLocation(shader.getId(), "lightColor"), lightColor.x, lightColor.y, lightColor.z);
		glUniform3f(glGetUniformLocation(shader.getId(), "lightPos"), lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(glGetUniformLocation(shader.getId(), "viewPos"), camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);
		dolphin.draw(shader);


		// Mountains
		shader.use();
		ModelMatrix = glm::mat4(1.0);
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(50.0f, -22.0f, 0.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(70.0f, 70.0f, 70.0f));
		MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
		glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniform3f(glGetUniformLocation(shader.getId(), "lightColor"), lightColor.x, lightColor.y, lightColor.z);
		glUniform3f(glGetUniformLocation(shader.getId(), "lightPos"), lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(glGetUniformLocation(shader.getId(), "viewPos"), camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);
		hillsOne.draw(shader);
		Vertex hillsOne_min, hillsOne_max;
		BBox(hillsFour, hillsFour.vertices.size(), hillsOne_max, hillsOne_min);
		hillsOne_min.pos.x += 50.0f;
		hillsOne_min.pos.y += -22.0f;
		hillsOne_min.pos.z += 0.0f;
		hillsOne_max.pos.x += 50.0f;
		hillsOne_max.pos.y += -22.0f;
		hillsOne_max.pos.z += 0.0f;


		if (intersect(playerMin, playerMax, hillsOne_min, hillsOne_max, 10.0f, 70.0f)) {
			isMoving = false;
		}

		ModelMatrix = glm::mat4(1.0);
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-75.0f, -22.0f, 0.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(25.0f, 25.0f, 25.0f));
		MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
		glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniform3f(glGetUniformLocation(shader.getId(), "lightColor"), lightColor.x, lightColor.y, lightColor.z);
		glUniform3f(glGetUniformLocation(shader.getId(), "lightPos"), lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(glGetUniformLocation(shader.getId(), "viewPos"), camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);
		hillsTwo.draw(shader);
		ModelMatrix = glm::mat4(1.0);
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-85.0f, -22.0f, -50.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(30.0f, 30.0f, 30.0f));
		MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
		glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniform3f(glGetUniformLocation(shader.getId(), "lightColor"), lightColor.x, lightColor.y, lightColor.z);
		glUniform3f(glGetUniformLocation(shader.getId(), "lightPos"), lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(glGetUniformLocation(shader.getId(), "viewPos"), camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);

		hillsThree.draw(shader);
		ModelMatrix = glm::mat4(1.0);
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(10.0f, -22.0f, -50.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(30.0f, 30.0f, 30.0f));
		MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
		glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniform3f(glGetUniformLocation(shader.getId(), "lightColor"), lightColor.x, lightColor.y, lightColor.z);
		glUniform3f(glGetUniformLocation(shader.getId(), "lightPos"), lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(glGetUniformLocation(shader.getId(), "viewPos"), camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);
		hillsThree.draw(shader);

		ModelMatrix = glm::mat4(1.0);
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-65.0f, -22.0f, 35.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(15.0f, 15.0f, 15.0f));
		MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
		glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniform3f(glGetUniformLocation(shader.getId(), "lightColor"), lightColor.x, lightColor.y, lightColor.z);
		glUniform3f(glGetUniformLocation(shader.getId(), "lightPos"), lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(glGetUniformLocation(shader.getId(), "viewPos"), camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);
		hillsFive.draw(shader);

		// Trees
		ModelMatrix = glm::mat4(1.0);
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-40.0f, -22.0f, -50.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(3.0f, 3.0f, 3.0f));
		MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
		glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniform3f(glGetUniformLocation(shader.getId(), "lightColor"), lightColor.x, lightColor.y, lightColor.z);
		glUniform3f(glGetUniformLocation(shader.getId(), "lightPos"), lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(glGetUniformLocation(shader.getId(), "viewPos"), camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);
		bigTree.draw(shader);

		Vertex bigTree_min, bigTree_max;
		BBox(hillsFour, hillsFour.vertices.size(), bigTree_max, bigTree_min);
		bigTree_min.pos.x += -40.0f;
		bigTree_min.pos.y += -22.0f;
		bigTree_min.pos.z += -50.0f;
		bigTree_max.pos.x += -40.0f;
		bigTree_max.pos.y += -22.0f;
		bigTree_max.pos.z += -50.0f;

		if (intersect(playerMin, playerMax, bigTree_min, bigTree_max, 10.0f, 3.0f)) {
			isMoving = false;
		}

		ModelMatrix = glm::mat4(1.0);
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-90.0f, -22.0f, 40.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(0.5f, 0.5f, 0.5f));
		MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
		glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniform3f(glGetUniformLocation(shader.getId(), "lightColor"), lightColor.x, lightColor.y, lightColor.z);
		glUniform3f(glGetUniformLocation(shader.getId(), "lightPos"), lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(glGetUniformLocation(shader.getId(), "viewPos"), camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);
		smallTree.draw(shader);

		Vertex smallTree_min, smallTree_max;
		BBox(hillsFour, hillsFour.vertices.size(), smallTree_max, smallTree_min);
		smallTree_min.pos.x += -90.0f;
		smallTree_min.pos.z += 40.0f;
		smallTree_min.pos.y += -22.0f;
		smallTree_max.pos.x += -90.0f;
		smallTree_max.pos.y += -22.0f;
		smallTree_max.pos.z += 40.0f;

		if (intersect(playerMin, playerMax, smallTree_min, smallTree_max, 10.0f, 0.5f)) {
			isMoving = false;
		}

		// Terrain
		ModelMatrix = glm::mat4(1.0);
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f, -22.25f, 0.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(1.25f, 1.25f, 1.25f));
		MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
		glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniform3f(glGetUniformLocation(shader.getId(), "lightColor"), lightColor.x, lightColor.y, lightColor.z);
		glUniform3f(glGetUniformLocation(shader.getId(), "lightPos"), lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(glGetUniformLocation(shader.getId(), "viewPos"), camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);
		terrain.draw(shader);

		// Lake
		lakeShader.use();
		ModelMatrix = glm::mat4(1.0);
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-20.0f, -22.0f, 0.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(0.15f, 0.15f, 0.15f));
		MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
		glUniformMatrix4fv(MatrixID2, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniform3f(glGetUniformLocation(lakeShader.getId(), "lightColor"), lightColor.x, lightColor.y, lightColor.z);
		glUniform3f(glGetUniformLocation(lakeShader.getId(), "lightPos"), lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(glGetUniformLocation(lakeShader.getId(), "viewPos"), camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);
		lake.draw(lakeShader);

		// ImGui window
		{
			ImGui::Begin("RGP GAME TOOLS");
			ImGui::Text("Window Size: 800x800");

			ImGui::Text("11 Objects in the scene");

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

			ImGui::ColorEdit3("clear color", (float*)&bg_color);
			ImGui::End();
		}

		ImGui::Render();
		ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());

		// Update window
		window.update();
	}

	ImGui_ImplGlfwGL3_Shutdown();
	ImGui::DestroyContext();
}

void processAnimation() {
	if (window.isPressed(GLFW_KEY_N)) {
		isAnimating = true;
	}
	if (window.isPressed(GLFW_KEY_M)) {
		isAnimating = false;
		dolRot = -70.0f;
		dolzPos = -20.0f;
	}
	if (isAnimating && dolRot <= 20.0f) {
		dolRot += 0.7f;
		dolzPos += 0.3f;
	}
}
void processCarMovement() {
	if (window.isPressed(GLFW_KEY_O)) {
		isMoving = true;
		xPos = -15.0f;
		zPos = 70.0f;
		carRot = 180.0f;
	}
	if (!isMoving)
		return;

	if (window.isPressed(GLFW_KEY_LEFT))
	{
		xPos -= 0.5f;
		//carRot += 0.3f;
		carRot = 90.0f;
	}
	if (window.isPressed(GLFW_KEY_RIGHT)) {
		xPos += 0.5f;
		//carRot -= 0.3f;
		carRot = -90.0f;
	}

	if (window.isPressed(GLFW_KEY_UP)) {
		zPos -= 0.5f;
		carRot = -180.0f;
	}

	if (window.isPressed(GLFW_KEY_DOWN))
		zPos += 0.5f;
}
void processKeyboardInput()
{
	float cameraSpeed = 30 * deltaTime;
	// Translation
	if (window.isPressed(GLFW_KEY_W))
		camera.keyboardMoveFront(cameraSpeed);
	if (window.isPressed(GLFW_KEY_S))
		camera.keyboardMoveBack(cameraSpeed);
	if (window.isPressed(GLFW_KEY_A))
		camera.keyboardMoveLeft(cameraSpeed);
	if (window.isPressed(GLFW_KEY_D))
		camera.keyboardMoveRight(cameraSpeed);
	if (window.isPressed(GLFW_KEY_R))
		camera.keyboardMoveUp(cameraSpeed);
	if (window.isPressed(GLFW_KEY_F))
		camera.keyboardMoveDown(cameraSpeed);

	// Rotation
	if (window.isPressed(GLFW_KEY_J))
		camera.rotateOy(cameraSpeed);
	if (window.isPressed(GLFW_KEY_L))
		camera.rotateOy(-cameraSpeed);
	if (window.isPressed(GLFW_KEY_I))
		camera.rotateOx(cameraSpeed);
	if (window.isPressed(GLFW_KEY_K))
		camera.rotateOx(-cameraSpeed);
}

void BBox(Mesh p, int n_vert, Vertex& p_max, Vertex& p_min)
{
	p_min.pos.x = p.vertices[0].pos.x;
	p_min.pos.y = p.vertices[0].pos.y;
	p_min.pos.z = p.vertices[0].pos.z;

	p_max.pos.x = p.vertices[0].pos.x;
	p_max.pos.y = p.vertices[0].pos.y;
	p_max.pos.z = p.vertices[0].pos.z;

	for (int i = 1; i < n_vert; i++)
	{
		p_min.pos.x = MIN(p_min.pos.x, p.vertices[i].pos.x);
		p_min.pos.y = MIN(p_min.pos.y, p.vertices[i].pos.y);
		p_min.pos.z = MIN(p_min.pos.z, p.vertices[i].pos.z);

		p_max.pos.x = MAX(p_max.pos.x, p.vertices[i].pos.x);
		p_max.pos.y = MAX(p_max.pos.y, p.vertices[i].pos.y);
		p_max.pos.z = MAX(p_max.pos.z, p.vertices[i].pos.z);
	}
}

// Get intersection
bool intersect(Vertex aMin, Vertex aMax, Vertex bMin, Vertex bMax, float aSize, float bSize) {
	return (aMin.pos.x <= (bMax.pos.x + bSize) && (aMax.pos.x + aSize) >= bMin.pos.x) &&
		(aMin.pos.y <= (bMax.pos.y + bSize) && (aMax.pos.y + aSize) >= bMin.pos.y) &&
		(aMin.pos.z <= (bMax.pos.z + bSize) && (aMax.pos.z + aSize) >= bMin.pos.z);
}
