#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


//#include "imgui.h"
//#include "imgui_impl_glfw.h"
//#include "imgui_impl_opengl3.h"

#include "Camera.h"
#include "myShader.h"
#include "stb_image.h"
#include "myModel.h"


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
unsigned int loadTexture(char const* path);
unsigned int loadCubemap(std::vector<std::string> faces);

glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

// settings
const unsigned int SCR_WIDTH = 1000;
const unsigned int SCR_HEIGHT = 600;

//Camera
glm::vec3 cameraPos  = glm::vec3(0.0f, 0.0f, 55.0f);
float deltaTime = 0.0f; // 当前帧与上一帧的时间差
float lastFrame = 0.0f; // 上一帧的时间
int nbFrames = 0;
Camera camera(cameraPos);
//rotate
bool firstMouse = true;
double lastX = SCR_WIDTH / 2;
double lastY = SCR_HEIGHT /2;


glm::vec3 pointLightPositions[] = {
	glm::vec3(0.7f,  0.2f,  2.0f),
	glm::vec3(2.3f, -3.3f, -4.0f),
	glm::vec3(-4.0f,  2.0f, -12.0f),
	glm::vec3(0.0f,  0.0f, -3.0f)
};

void updateFPS() {
	float currentFrame = static_cast<float>(glfwGetTime());
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;
	nbFrames++;

	static float lastTime = 0.0f; // 记录上次更新时间
	if (currentFrame - lastTime >= 1.0f) {
		printf("%d FPS:   %f ms/frame\n",nbFrames, 1000.0f / nbFrames);
		nbFrames = 0;
		lastTime = currentFrame;
	}
}

int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);
#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	//读取鼠标
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	//鼠标旋转输入
	glfwSetCursorPosCallback(window, mouse_callback);
	//鼠标缩放
	glfwSetScrollCallback(window,scroll_callback);
	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//解锁60帧
	glfwSwapInterval(0);  // 0 表示关闭 V-Sync，1 表示打开 V-Sync

	// ---------------------configure ImGui---------------------------------
	//IMGUI_CHECKVERSION();
	//ImGui::CreateContext(NULL);     // Setup Dear ImGui context
	//ImGui::StyleColorsDark();       // Setup Dear ImGui style
	//ImGui_ImplGlfw_InitForOpenGL(window, true);     // Setup Platform/Renderer backends
	//ImGui_ImplOpenGL3_Init("#version 330");
	
	
	// build and compile our shader program
	// ------------------------------------
	Shader cubeObject("shader/object.vs", "shader/object.fs");
	Shader planeShader("shader/plane.vs", "shader/plane.fs");
	Shader lightShader("shader/light.vs", "shader/light.fs");
	Shader screenShader("shader/screen.vs", "shader/screen.fs");
	Shader skyboxShader("shader/skybox.vs", "shader/skybox.fs");

	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ----------------------- load mesh data ----------------------------

	float skyboxVertices[] = {
		// positions          
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
	float planeVertices[] = {
		// positions          // texture Coords 
		 5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
		-5.0f, -0.5f,  5.0f,  0.0f, 0.0f,
		-5.0f, -0.5f, -5.0f,  0.0f, 2.0f,

		 5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
		-5.0f, -0.5f, -5.0f,  0.0f, 2.0f,
		 5.0f, -0.5f, -5.0f,  2.0f, 2.0f
	};
	float transparentVertices[] = {
		// positions         // texture Coords (swapped y coordinates because texture is flipped upside down)
		0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
		0.0f, -0.5f,  0.0f,  0.0f,  1.0f,
		1.0f, -0.5f,  0.0f,  1.0f,  1.0f,

		0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
		1.0f, -0.5f,  0.0f,  1.0f,  1.0f,
		1.0f,  0.5f,  0.0f,  1.0f,  0.0f
	};

	std::vector<glm::vec3> vegetation
	{
		glm::vec3(-1.5f, 0.0f, -0.48f),
		glm::vec3(1.5f, 0.0f, 0.51f),
		glm::vec3(0.0f, 0.0f, 0.7f),
		glm::vec3(-0.3f, 0.0f, -2.3f),
		glm::vec3(0.5f, 0.0f, -0.6f)
	};

	float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
	// positions   // texCoords
	-1.0f,  1.0f,  0.0f, 1.0f,
	-1.0f, -1.0f,  0.0f, 0.0f,
	 1.0f, -1.0f,  1.0f, 0.0f,

	-1.0f,  1.0f,  0.0f, 1.0f,
	 1.0f, -1.0f,  1.0f, 0.0f,
	 1.0f,  1.0f,  1.0f, 1.0f
	};
	//线框模式查看

	// skybox VAO
	unsigned int skyboxVAO, skyboxVBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	// transparent VAO
	unsigned int transparentVAO, transparentVBO;
	glGenVertexArrays(1, &transparentVAO);
	glGenBuffers(1, &transparentVBO);
	glBindVertexArray(transparentVAO);
	glBindBuffer(GL_ARRAY_BUFFER, transparentVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(transparentVertices), transparentVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glBindVertexArray(0);

	

	Model planetModel("Assets/planet/planet.obj");
	Model rockModel("Assets/rock/rock.obj");

	std::vector<std::string> faces
	{
		"textures/skybox/right.jpg",
		"textures/skybox/left.jpg",
		"textures/skybox/top.jpg",
		"textures/skybox/bottom.jpg",
		"textures/skybox/front.jpg",
		"textures/skybox/back.jpg"
	};
	std::vector<std::string> stars
	{
		"textures/skybox/star/StarSkybox041.png",
		"textures/skybox/star/StarSkybox042.png",
		"textures/skybox/star/StarSkybox043.png",
		"textures/skybox/star/StarSkybox044.png",
		"textures/skybox/star/StarSkybox045.png",
		"textures/skybox/star/StarSkybox046.png"
	};
	unsigned int windowTexture = loadTexture("textures/blending_transparent_window.png");

	unsigned int cubemapTexture = loadCubemap(stars);

	skyboxShader.use();
	skyboxShader.setInt("skybox", 0);
	// shader configuration
// --------------------
	planeShader.use();
	planeShader.setInt("texture1", 0);
	//加载小行星带
	unsigned int amount = 100000;
	glm::mat4* modelMatrices;
	modelMatrices = new glm::mat4[amount];
	srand(static_cast<unsigned int>(glfwGetTime())); // initialize random seed
	float radius = 50.0;
	float offset = 2.5f;
	for (unsigned int i = 0; i < amount; i++)
	{
		glm::mat4 model = glm::mat4(1.0f);
		// 1. translation: displace along circle with 'radius' in range [-offset, offset]
		float angle = (float)i / (float)amount * 360.0f;
		float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float x = sin(angle) * radius + displacement;
		displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float y = displacement * 0.4f; // keep height of asteroid field smaller compared to width of x and z
		displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float z = cos(angle) * radius + displacement;
		model = glm::translate(model, glm::vec3(x, y, z));

		// 2. scale: Scale between 0.05 and 0.25f
		float scale = static_cast<float>((rand() % 20) / 100.0 + 0.05);
		model = glm::scale(model, glm::vec3(scale));

		// 3. rotation: add random rotation around a (semi)randomly picked rotation axis vector
		float rotAngle = static_cast<float>((rand() % 360));
		model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));

		// 4. now add to list of matrices
		modelMatrices[i] = model;
	}
	//绑定小行星带的VAO
	unsigned int buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, amount * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);

	// set transformation matrices as an instance vertex attribute (with divisor 1)
	// note: we're cheating a little by taking the, now publicly declared, VAO of the model's mesh(es) and adding new vertexAttribPointers
	// normally you'd want to do this in a more organized fashion, but for learning purposes this will do.
	// -----------------------------------------------------------------------------------------------------------------------------------
	unsigned int instanceWindowVAO = transparentVAO;
	glBindVertexArray(instanceWindowVAO);
	// set attribute pointers for matrix (4 times vec4)
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
	glEnableVertexAttribArray(6);
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

	glVertexAttribDivisor(3, 1);
	glVertexAttribDivisor(4, 1);
	glVertexAttribDivisor(5, 1);
	glVertexAttribDivisor(6, 1);

	glBindVertexArray(0);
	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		
		// per-frame time logic
		// --------------------
		updateFPS();

		// input
		// -----
		processInput(window);

		//glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		//glEnable(GL_DEPTH_TEST); // enable depth testing (is disabled for rendering screen-space quad)

		// make sure we clear the framebuffer's content
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//-------------------------------------------- passing parameters (cube)--------------------------------------------------
		cubeObject.use();
		cubeObject.setVec3("viewPos", camera.Position);
		// directional light
		cubeObject.setVec3("dirLight.direction", glm::vec3(-0.2f, -1.0f, -0.3f));
		cubeObject.setVec3("dirLight.ambient", glm::vec3(0.05f, 0.05f, 0.05f));
		cubeObject.setVec3("dirLight.diffuse", glm::vec3(0.4f, 0.4f, 0.4f));
		cubeObject.setVec3("dirLight.specular", glm::vec3(0.5f, 0.5f, 0.5f));
		cubeObject.setFloat("material.shininess", 32);
		// point light 1
		cubeObject.setVec3("pointLights[0].position", lightPos);
		cubeObject.setVec3("pointLights[0].ambient", glm::vec3(0.05f, 0.05f, 0.05f));
		cubeObject.setVec3("pointLights[0].diffuse", glm::vec3(0.8f, 0.8f, 0.8f));
		cubeObject.setVec3("pointLights[0].specular", glm::vec3(1.0f, 1.0f, 1.0f));
		cubeObject.setFloat("pointLights[0].constant", 1.0f);
		cubeObject.setFloat("pointLights[0].linear", 0.09f);
		cubeObject.setFloat("pointLights[0].quadratic", 0.032f);
		// point light 2
		cubeObject.setVec3("pointLights[1].position", pointLightPositions[1]);
		cubeObject.setVec3("pointLights[1].ambient", glm::vec3(0.05f, 0.05f, 0.05f));
		cubeObject.setVec3("pointLights[1].diffuse", glm::vec3(0.8f, 0.8f, 0.8f));
		cubeObject.setVec3("pointLights[1].specular", glm::vec3(1.0f, 1.0f, 1.0f));
		cubeObject.setFloat("pointLights[1].constant", 1.0f);
		cubeObject.setFloat("pointLights[1].linear", 0.09f);
		cubeObject.setFloat("pointLights[1].quadratic", 0.032f);
		// point light 3
		cubeObject.setVec3("pointLights[2].position", pointLightPositions[2]);
		cubeObject.setVec3("pointLights[2].ambient", glm::vec3(0.05f, 0.05f, 0.05f));
		cubeObject.setVec3("pointLights[2].diffuse", glm::vec3(0.8f, 0.8f, 0.8f));
		cubeObject.setVec3("pointLights[2].specular", glm::vec3(1.0f, 1.0f, 1.0f));
		cubeObject.setFloat("pointLights[2].constant", 1.0f);
		cubeObject.setFloat("pointLights[2].linear", 0.09f);
		cubeObject.setFloat("pointLights[2].quadratic", 0.032f);
		// point light 4
		cubeObject.setVec3("pointLights[3].position", pointLightPositions[3]);
		cubeObject.setVec3("pointLights[3].ambient", glm::vec3(0.05f, 0.05f, 0.05f));
		cubeObject.setVec3("pointLights[3].diffuse", glm::vec3(0.8f, 0.8f, 0.8f));
		cubeObject.setVec3("pointLights[3].specular", glm::vec3(1.0f, 1.0f, 1.0f));
		cubeObject.setFloat("pointLights[3].constant", 1.0f);
		cubeObject.setFloat("pointLights[3].linear", 0.09f);
		cubeObject.setFloat("pointLights[3].quadratic", 0.032f);
		//mvp
		//projection
		glm::mat4 projection(1.0f);
		float aspect = static_cast<float>(SCR_WIDTH) / static_cast<float>(SCR_HEIGHT);
		projection = glm::perspective(glm::radians(camera.Fov), aspect, 0.1f, 1000.0f);
		//view
		glm::mat4 view(1.0f);
		view = camera.GetViewMatrix();
		cubeObject.use();
		cubeObject.setMat4("projection", projection);
		cubeObject.setMat4("view", view);
		// 加载行星
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, -3.0f, 0.0f));
		model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));

		cubeObject.setMat4("model", model);

		planetModel.Draw(cubeObject);

		// 绘制小行星
		planeShader.use();
		planeShader.setMat4("projection", projection);
		planeShader.setMat4("view", view);
		model = glm::mat4(1.0f);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, windowTexture); // note: we also made the textures_loaded vector public (instead of private) from the model class.

		// Draw Instance
		glBindVertexArray(instanceWindowVAO);
		glDrawArraysInstanced(GL_TRIANGLES, 0, 6, amount);
		glBindVertexArray(0);

		// draw skybox as last
		glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
		skyboxShader.use();
		view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
		skyboxShader.setMat4("view", view);
		skyboxShader.setMat4("projection", projection);
		// skybox cube
		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS); // set depth function back to default


		
		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------

	//glDeleteRenderbuffers(1, &rbo);
	//glDeleteFramebuffers(1, &framebuffer);
	glDeleteBuffers(1, &skyboxVBO);
	glDeleteBuffers(1, &buffer);
	glDeleteBuffers(1, &transparentVAO);
	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD,deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD,deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT,deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT,deltaTime);
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		camera.ProcessKeyboard(DOWN,deltaTime);
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		camera.ProcessKeyboard(UP,deltaTime);
	if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
		if(camera.canMouseControl)
			camera.canMouseControl = false;
		
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	
	if(firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; 
	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset,yoffset,firstMouse);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

unsigned int loadTexture(char const* path)
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

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}


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
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
			);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}