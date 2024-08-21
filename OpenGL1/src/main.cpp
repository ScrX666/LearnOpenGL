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


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
unsigned int loadTexture(char const* path);

glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

// settings
const unsigned int SCR_WIDTH = 1000;
const unsigned int SCR_HEIGHT = 600;

//Camera
glm::vec3 cameraPos  = glm::vec3(0.0f, 0.0f, 3.0f);
float deltaTime = 0.0f; // 当前帧与上一帧的时间差
float lastFrame = 0.0f; // 上一帧的时间
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

int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

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

	// ---------------------configure ImGui---------------------------------
	//IMGUI_CHECKVERSION();
	//ImGui::CreateContext(NULL);     // Setup Dear ImGui context
	//ImGui::StyleColorsDark();       // Setup Dear ImGui style
	//ImGui_ImplGlfw_InitForOpenGL(window, true);     // Setup Platform/Renderer backends
	//ImGui_ImplOpenGL3_Init("#version 330");
	
	
	// build and compile our shader program
	// ------------------------------------
	Shader cubeObject("shader/object.vs", "shader/object.fs");
	Shader lightShader("shader/light.vs", "shader/light.fs");

	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ----------------------- load mesh data ----------------------------
	float vertices[] = {
		// positions          // normals           // texture coords
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
	};

	// world space positions of our cubes
	glm::vec3 cubePositions[] = {
		glm::vec3(0.0f,  0.0f,  0.0f),
		glm::vec3(2.0f,  5.0f, -15.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.0f, -12.3f),
		glm::vec3(2.4f, -0.4f, -3.5f),
		glm::vec3(-1.7f,  3.0f, -7.5f),
		glm::vec3(1.3f, -2.0f, -2.5f),
		glm::vec3(1.5f,  2.0f, -2.5f),
		glm::vec3(1.5f,  0.2f, -1.5f),
		glm::vec3(-1.3f,  1.0f, -1.5f)
	};

	//---------------------------------------- setting VAO/VBO ---------------------------------------------------
	//object VAO
	unsigned int VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	// object position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// object normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// object texcoord attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	cubeObject.use(); // don't forget to activate/use the shader before setting uniforms!
	
	//light VAO
	unsigned int lightVAO;
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);
	
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// light position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	

	// -------------------------------------- load texture -----------------------------------------------------
	unsigned int diffuseTex = loadTexture("textures/container2.png");
	unsigned int specularTex = loadTexture("textures/container2_specular.png");
	unsigned int emissionTex = loadTexture("textures/matrix.jpg");

	cubeObject.use();
	// set texture glActiveTexture ID
	cubeObject.setInt("material.diffuse", 0);
	cubeObject.setInt("material.specular", 1);
	//cubeObject.setInt("material.emissionTex", 2);

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		
		// per-frame time logic
		// --------------------
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		
		// input
		// -----
		processInput(window);

		// clear
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		//-------------------------------------------- passing parameters (cube)--------------------------------------------------
		cubeObject.use();
		cubeObject.setVec3("viewPos", camera.Position);
		// directional light
		cubeObject.setVec3("dirLight.direction", glm::vec3(-0.2f, -1.0f, -0.3f));
		cubeObject.setVec3("dirLight.ambient", glm::vec3(0.05f, 0.05f, 0.05f));
		cubeObject.setVec3("dirLight.diffuse", glm::vec3(0.4f, 0.4f, 0.4f));
		cubeObject.setVec3("dirLight.specular", glm::vec3(0.5f, 0.5f, 0.5f));
		// point light 1
		cubeObject.setVec3("pointLights[0].position", pointLightPositions[0]);
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

		//spot light
		cubeObject.setVec3("spotLight.position", camera.Position);
		cubeObject.setVec3("spotLight.direction", camera.Front);
		cubeObject.setVec3("spotLight.ambient", glm::vec3(0.0f, 0.0f, 0.0f));
		cubeObject.setVec3("spotLight.diffuse", glm::vec3(1.0f, 1.0f, 1.0f));
		cubeObject.setVec3("spotLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));
		cubeObject.setFloat("spotLight.constant", 1.0f);
		cubeObject.setFloat("spotLight.linear", 0.09f);
		cubeObject.setFloat("spotLight.quadratic", 0.032f);
		cubeObject.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
		cubeObject.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));

		cubeObject.setFloat("material.shininess", 32);

		//mvp
		//projection
		glm::mat4 projection(1.0f);
		float aspect = static_cast<float>(SCR_WIDTH / SCR_HEIGHT);
		projection = glm::perspective(glm::radians(camera.Fov), aspect , 0.1f, 100.0f);
		cubeObject.setMat4("projection", projection);
		//view
		glm::mat4 view(1.0f);
		view = camera.GetViewMatrix();
		// to vs shader
		cubeObject.setMat4("view", view);
		//model
		glm::mat4 model = (1.0f);
		//cubeObject.setMat4("model", model);

		// model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.0f));
		

		//bind texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, diffuseTex);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, specularTex);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, emissionTex);

		//-------------------------------------------- RENDER (cube)--------------------------------------------------
		glBindVertexArray(VAO);
		for (unsigned int i = 0; i < 10; i++)
		{
			// calculate the model matrix for each object and pass it to shader before drawing
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, cubePositions[i]);
			float angle = 20.0f * i;
			model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
			cubeObject.setMat4("model", model);

			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		//-------------------------------------------- passing parameters (light)--------------------------------------------------
		lightShader.use();
		lightShader.setMat4("projection", projection);
		lightShader.setMat4("view", view);
		lightShader.setVec3("lightColor", glm::vec3(1));


		//rotate the light 
		// lightPos.x = 1.0f + sin(glfwGetTime()) * 2.0f;
		// lightPos.y = sin(glfwGetTime() / 2.0f) * 1.0f;
		//model
		model = (1.0f);
		model = translate(model, lightPos);
		model = glm::scale(model, glm::vec3(0.2f));
		lightShader.setMat4("model", model);
		//-------------------------------------------- RENDER (light)--------------------------------------------------
		glBindVertexArray(lightVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		
		// ----------------------------------ImGui init--------------------------------------
		//ImGui_ImplOpenGL3_NewFrame();
		//ImGui_ImplGlfw_NewFrame();
		//ImGui::NewFrame();

		//ImGui::Begin("UI Box", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
		//ImGui::Checkbox("Enable Mouse Control", &camera.canMouseControl);
		//if(!camera.canMouseControl)
		//	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		//else
		//	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		//ImGui::SliderFloat("shininess", &shininess, 1, 128);
		//ImGui::End();
		//ImGui::ShowDemoWindow();
		//ImGui::Render();
		//ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


		
		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	/*glDeleteVertexArrays(1, &lightVAO);*/
	
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

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
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


