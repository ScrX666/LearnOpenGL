#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_internal.h"

#include "Camera.h"
#include "myShader.h"

#include "myTexture.h"
#include "myModel.h"



void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void renderScene(const Shader& shader);
void renderModel(Shader& shader, Model customModel);
void renderCube();
void renderQuad();
void renderSphere();
void renderPointLight(Shader& shader, Model model, glm::vec3 lightpos);
void renderObjectMatrix(Shader& shader);

// settings
const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// meshes
unsigned int planeVAO;


Texture woodDiffuse("textures/brickwall.jpg");
//const char* normalTexture = "textures/brickwall_normal.jpg";
//const char* baseColor_pbr = "textures/pbr_baseColor.png";
//const char* normal_pbr = "textures/pbr_normal.png";
//const char* orm_pbr = "textures/pbr_ORM.png";
Texture normalTexture("textures/brickwall_normal.jpg");
Texture baseColor_pbr("textures/pbr_baseColor.png");
Texture normal_pbr("textures/pbr_normal.png");
Texture orm_pbr("textures/pbr_ORM.png");

bool bRotateLight = false;
bool bRenderCube = true;
bool bdShadow = true;
bool bpShadow = false;
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
	//glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	glfwSwapInterval(1);
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

	// build and compile shaders
	// -------------------------
	Shader simpleDepthShader("shader/simpleDepth.vs", "shader/simpleDepth.fs");
	Shader debugDepthQuad("shader/quad_depth.vs", "shader/quad_depth.fs");
	Shader shadowMapShader("shader/shadowMap.vs", "shader/shadowMap.fs");
	Shader lightShader("shader/light.vs", "shader/light.fs");
	Shader objectShader("shader/object.vs", "shader/object.fs");
	Shader pointLightShader("shader/pointLightShadow.vs", "shader/pointLightShadow.fs", "shader/pointLightShadow.gs");
	Shader pbrDirectShader("shader/pbrDirect.vs", "shader/pbrDirect.fs");

	Model model_light("Assets/Light/pointLight.obj");
	Model model_cyborg("Assets/cyborg/cyborg.obj");

	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------
	glm::vec3 pos1(25.0f, -0.5f, 25.0f);
	glm::vec3 pos2(-25.0f, -0.5f, 25.0f);
	glm::vec3 pos3(-25.0f, -0.5f, -25.0f);
	glm::vec3 pos4(25.0f, -0.5f, -25.0f);

	glm::vec2 uv1(25.0f, 0.0f);
	glm::vec2 uv2(0.0f, 0.0f);
	glm::vec2 uv3(0.0f, 25.0f);
	glm::vec2 uv4(25.0f, 25.0f);


	glm::vec3 nm(0.0f, 1.0f, 0.0f);

	// calculate tangent/bitangent vectors of both triangles 1
	glm::vec3 tangent1;
	glm::vec3 tangent2;
	glm::vec3 edge1 = pos2 - pos1;
	glm::vec3 edge2 = pos3 - pos1;
	glm::vec2 deltaUV1 = uv2 - uv1;
	glm::vec2 deltaUV2 = uv3 - uv1;
	
	float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
	tangent1 = f * (deltaUV2.y * edge1 - deltaUV1.y * edge2);
	

	// - triangle 2 re-calculate
	edge1 = pos3 - pos1;
	edge2 = pos4 - pos1;
	deltaUV1 = uv3 - uv1;
	deltaUV2 = uv4 - uv1;

	f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
	tangent2 = f * (deltaUV2.y * edge1 - deltaUV1.y * edge2);
	

	GLfloat planeVertices[] = {
    // Positions            // Normal         // TexCoords  // Tangent                          // Bitangent
    pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent1.x, tangent1.y, tangent1.z, 
    pos2.x, pos2.y, pos2.z, nm.x, nm.y, nm.z, uv2.x, uv2.y, tangent1.x, tangent1.y, tangent1.z,
    pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent1.x, tangent1.y, tangent1.z,

    pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent2.x, tangent2.y, tangent2.z,
    pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent2.x, tangent2.y, tangent2.z,
    pos4.x, pos4.y, pos4.z, nm.x, nm.y, nm.z, uv4.x, uv4.y, tangent2.x, tangent2.y, tangent2.z,
};
	// plane VAO
	unsigned int planeVBO;
	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glBindVertexArray(planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(8 * sizeof(GLfloat)));
	glBindVertexArray(0);


	// load textures
	// -------------
	unsigned int woodTexture = woodDiffuse.loadTexture();
	unsigned int woodNormolTexture = normalTexture.loadTexture();
	unsigned int pbrBaseColorTexture = baseColor_pbr.loadTexture();
	unsigned int pbrNormalTexture = normal_pbr.loadTexture();
	unsigned int pbrORM = orm_pbr.loadTexture();

	// configure depth map FBO
	// -----------------------
	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
	unsigned int depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);
	// create depth texture
	unsigned int depthMap;
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	// attach depth texture as FBO's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Configure depth CubeMap FBO
	GLuint depthCubeMapFBO;
	glGenFramebuffers(1, &depthCubeMapFBO);
	// Create depth cubemap texture
	GLuint depthCubeMap;
	glGenTextures(1, &depthCubeMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubeMap);
	for (GLuint i = 0; i < 6; ++i)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	// Attach cubemap as depth map FBO's color buffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthCubeMapFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubeMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	// shader configuration
	// --------------------
	shadowMapShader.use();
	shadowMapShader.setInt("diffuseTexture", 0);
	shadowMapShader.setInt("normalTexture", 1);
	shadowMapShader.setInt("shadowMap", 2);
	shadowMapShader.setInt("depthCubeMap", 3);
	debugDepthQuad.use();
	debugDepthQuad.setInt("depthMap", 0);

	pbrDirectShader.use();
	pbrDirectShader.setInt("albedoMap", 0);
	pbrDirectShader.setInt("normalMap", 1);
	pbrDirectShader.setInt("ormMap", 2);


	// lighting info
	// -------------
	glm::vec3 lightPos(-2.0f, 4.0f, -1.0f);

	// -----------------IMGUI SetUp--------------------
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

	ImGui_ImplOpenGL3_Init("#version 420");
	// Setup Dear ImGui style
	// ImGui::StyleColorsDark();
	ImGui::StyleColorsLight();
	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	bool show_demo_window = true;
	bool show_shadowmap = true;


	// PBR
	// lights
	// ------
	glm::vec3 lightPositions[] = {
		glm::vec3(-10.0f,  22.0f, 10.0f),
		glm::vec3(10.0f,  22.0f, 10.0f),
		glm::vec3(-10.0f, 2.0f, 10.0f),
		glm::vec3(10.0f, 2.0f, 10.0f),
	};
	glm::vec3 lightColors[] = {
		glm::vec3(300.0f, 300.0f, 300.0f),
		glm::vec3(300.0f, 300.0f, 300.0f),
		glm::vec3(300.0f, 300.0f, 300.0f),
		glm::vec3(300.0f, 300.0f, 300.0f),
		glm::vec3(300.0f, 300.0f, 300.0f),
		glm::vec3(300.0f, 300.0f, 300.0f)
	};
	int nrRows = 7;
	int nrColumns = 7;
	float spacing = 2.5;
	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Settings");


		// UI - light pos
		ImGui::Text("Light Position\n");
		static float p_pos[3]{ 8.0f, 8.0f, 0.0f };
		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::SliderFloat("X", p_pos, -50.0f, 50.0f, "%.3f");
		ImGui::PopItemWidth();
		ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
		ImGui::SliderFloat("Y", p_pos + 1, -50.0f, 50.0f, "%.3f");
		ImGui::PopItemWidth();
		ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
		ImGui::SliderFloat("Z", p_pos + 2, -50.0f, 50.0f, "%.3f");
		ImGui::PopItemWidth();
		ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
		ImGui::Text("Position");
		static float p0_Intensity = 1.0f;
		ImGui::SliderFloat("PointLight Intensity", &p0_Intensity, 0.0f, 5.0f, "%.2f");
		// cast to vec3
		glm::vec3 pointLight_pos = glm::vec3(p_pos[0], p_pos[1], p_pos[2]);

		// per-frame time logic
		// --------------------
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		// -----
		processInput(window);

		// render
		// ------
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// 1. render depth of scene to texture (from light's perspective)
		// --------------------------------------------------------------
		glm::mat4 lightProjection, lightView;
		glm::mat4 lightSpaceMatrix;
		lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 7.5f);
		lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
		lightSpaceMatrix = lightProjection * lightView;
		// render scene from light's point of view
		simpleDepthShader.use();
		simpleDepthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		//render all opaque object in depth-map
		renderScene(simpleDepthShader);
		renderModel(simpleDepthShader, model_cyborg);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// 0. Create depth cubemap transformation matrices
		float near_plane = 1.0f;
		float far_plane = 25.0f;
		glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, near_plane, far_plane);
		std::vector<glm::mat4> shadowTransforms;
		shadowTransforms.push_back(shadowProj * glm::lookAt(pointLight_pos, pointLight_pos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
		shadowTransforms.push_back(shadowProj * glm::lookAt(pointLight_pos, pointLight_pos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
		shadowTransforms.push_back(shadowProj * glm::lookAt(pointLight_pos, pointLight_pos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
		shadowTransforms.push_back(shadowProj * glm::lookAt(pointLight_pos, pointLight_pos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
		shadowTransforms.push_back(shadowProj * glm::lookAt(pointLight_pos, pointLight_pos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
		shadowTransforms.push_back(shadowProj * glm::lookAt(pointLight_pos, pointLight_pos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));

		// 2 : render point light shader
		pointLightShader.use();
		for (GLuint i = 0; i < 6; ++i)
			pointLightShader.setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
		pointLightShader.setVec3("lightPos", pointLight_pos);
		pointLightShader.setFloat("far_plane", far_plane);

		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthCubeMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		renderScene(pointLightShader);
		if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cout << "Cubemap FBO not complete!" << std::endl;
		}
		// 在渲染后检查一个面的深度值
		float depthValue;
		glReadPixels(SHADOW_WIDTH/2, SHADOW_HEIGHT/2, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depthValue);
		//std::cout << "Center depth value: " << depthValue << std::endl;

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// reset viewport
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// 2. render scene as normal using the generated depth/shadow map  
		// --------------------------------------------------------------
		shadowMapShader.use();
		glm::mat4 projection = glm::perspective(glm::radians(camera.Fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		shadowMapShader.setMat4("projection", projection);
		shadowMapShader.setMat4("view", view);
		// set light uniforms
		shadowMapShader.setVec3("viewPos", camera.Position);
		shadowMapShader.setVec3("lightPos", lightPos);
		shadowMapShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

		ImGui::Checkbox("Directional Light shadow", &bdShadow);
		shadowMapShader.setInt("b_dshadow", bdShadow);
		ImGui::Checkbox("Point Light shadow", &bpShadow);
		shadowMapShader.setInt("b_pshadow", bpShadow);

		ImGui::Checkbox("Rotate the Light", &bRotateLight);
		if (bRotateLight)
		{
			//rotate the light 
			float angle = 2.0f * 3.14f * static_cast<float>(glfwGetTime()) / 5.0f;
			pointLight_pos.z = 10 * cos(angle);
			pointLight_pos.x = 10 * sin(angle);
		}
		// Add Color Picker
		static float lightColor[3] = { 1.0f, 1.0f, 1.0f }; // RGB values
		ImGui::ColorEdit3("PointLight Color", lightColor, ImGuiColorEditFlags_Float);

		// Convert to glm::vec3 when passing to shader
		glm::vec3 light_color = glm::vec3(lightColor[0], lightColor[1], lightColor[2]);
		// Pass to shader

		shadowMapShader.setVec3("pointLights[0].color", light_color);
		shadowMapShader.setVec3("pointLights[0].position", pointLight_pos);
		shadowMapShader.setVec3("pointLights[0].ambient", glm::vec3(0.05f, 0.05f, 0.05f));
		shadowMapShader.setVec3("pointLights[0].diffuse", glm::vec3(0.8f, 0.8f, 0.8f));
		shadowMapShader.setFloat("pointLights[0].intensity", p0_Intensity);
		shadowMapShader.setVec3("pointLights[0].specular", glm::vec3(1.0f, 1.0f, 1.0f));
		shadowMapShader.setFloat("pointLights[0].constant", 1.0f);
		shadowMapShader.setFloat("pointLights[0].linear", 0.09f);
		shadowMapShader.setFloat("pointLights[0].quadratic", 0.032f);

		shadowMapShader.setFloat("far_plane", far_plane);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, woodTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, woodNormolTexture);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubeMap);
		renderScene(shadowMapShader);
		// render the custom model
		renderModel(shadowMapShader, model_cyborg);
		
		// render Depth map to quad for visual debugging
		// ---------------------------------------------
		//debugDepthQuad.use();
		//debugDepthQuad.setFloat("near_plane", near_plane);
		//debugDepthQuad.setFloat("far_plane", far_plane);
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, depthMap);
		//renderQuad();



		//float orient[3] = {pos[0],pos[1],pos[2]};
		//ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		//ImGui::DragFloat("Pitch", orient, 0.1f, -180.0f, 180.0f, "%.1f");
		//ImGui::PopItemWidth();
		//ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
		//ImGui::DragFloat("Roll", orient + 1, 0.1f, -180.0f, 180.0f, "%.1f");
		//ImGui::PopItemWidth();
		//ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
		//ImGui::DragFloat("Yaw", orient + 2, 0.1f, -180.0f, 180.0f, "%.1f");
		//ImGui::PopItemWidth();
		//ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
		//ImGui::Text("Orientation");
		static float baseColor[3] = { 0.5f, 0.0f, 0.0f }; // RGB values
		ImGui::ColorEdit3("PBR Base Color", baseColor, ImGuiColorEditFlags_Float);

		// Convert to glm::vec3 when passing to shader
		glm::vec3 pbr_basecolor = glm::vec3(baseColor[0], baseColor[1], baseColor[2]);

		renderPointLight(lightShader, model_light, pointLight_pos);

		pbrDirectShader.use();
		pbrDirectShader.setMat4("projection", projection);
		pbrDirectShader.setMat4("view", view);
		pbrDirectShader.setVec3("camPos", camera.Position);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, pbrBaseColorTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, pbrNormalTexture);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, pbrORM);
		// render rows*column number of spheres with varying metallic/roughness values scaled by rows and columns respectively
		glm::mat4 model = glm::mat4(1.0f);
		for (int row = 0; row < nrRows; ++row)
		{
			pbrDirectShader.setFloat("metallicRange", (float)row / (float)nrRows);
			for (int col = 0; col < nrColumns; ++col)
			{
				// we clamp the roughness to 0.05 - 1.0 as perfectly smooth surfaces (roughness of 0.0) tend to look a bit off
				// on direct lighting.
				pbrDirectShader.setFloat("roughnessRange", glm::clamp((float)col / (float)nrColumns, 0.05f, 1.0f));

				model = glm::mat4(1.0f);
				model = glm::translate(model, glm::vec3(
					(col - (nrColumns / 2)) * spacing,
					(row - (nrRows / 2)) * spacing,
					0.0f
				));
				model = glm::translate(model, glm::vec3(0, 12, 0));
				pbrDirectShader.setMat4("model", model);
				renderSphere();
			}
		}
		
		for (unsigned int i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); ++i)
		{
			glm::vec3 newPos = lightPositions[i] + glm::vec3(sin(glfwGetTime() * 5.0) * 5.0, 0.0, 0.0);
			newPos = lightPositions[i];
			pbrDirectShader.setVec3("lightPositions[" + std::to_string(i) + "]", newPos);
			pbrDirectShader.setVec3("lightColors[" + std::to_string(i) + "]", lightColors[i]);
			model = glm::mat4(1.0f);
			model = glm::translate(model, newPos);
			model = glm::scale(model, glm::vec3(0.5f));
			pbrDirectShader.setMat4("model", model);
			renderSphere();
		}


		ImGui::Checkbox("Show Demo Sample", &show_demo_window);
		// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
		if (show_demo_window)
		{
			ImGui::ShowDemoWindow();
		}
		ImGui::Checkbox("Render Cube", &bRenderCube);
		if (ImGui::Button("Show ShadowMap"))
		{
			show_shadowmap = !show_shadowmap;
		}
		if (show_shadowmap)
		{
			ImGui::Image((ImTextureID)(intptr_t)depthMap, ImVec2(128, 128), ImVec2(0, 1), ImVec2(1, 0), ImVec4(1, 1, 1, 1));
		}
		

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
		ImGui::End();

		// Rendering
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	glDeleteVertexArrays(1, &planeVAO);
	glDeleteBuffers(1, &planeVBO);
	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();
	return 0;
}

void renderObjectMatrix(Shader& shader)
{
	
}

// renders the 3D scene
// --------------------
void renderScene(const Shader& shader)
{

	// floor
	glm::mat4 model = glm::mat4(1.0f);
	shader.setMat4("model", model);
	glBindVertexArray(planeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	// cubes
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 1.5f, 0.0));
	model = glm::scale(model, glm::vec3(0.5f));
	shader.setMat4("model", model);
	if (bRenderCube)
		renderCube();
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(2.0f, 0.0f, 1.0));
	model = glm::scale(model, glm::vec3(0.5f));
	shader.setMat4("model", model);
	if (bRenderCube)
		renderCube();

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 2.0));
	model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
	model = glm::scale(model, glm::vec3(0.25));
	shader.setMat4("model", model);
	if (bRenderCube)
		renderCube();
}

void renderModel(Shader& shader, Model customModel)
{
	glm::mat4 model = glm::mat4(1);
	model = translate(model, glm::vec3(4, -0.5, 0));
	shader.setMat4("model", model);
	customModel.Draw(shader);
}



// renderCube() renders a 1x1 3D cube in NDC.
// -------------------------------------------------
unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
void renderCube()
{
	// initialize (if necessary)
	if (cubeVAO == 0)
	{
		float vertices[] = {
			// back face
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			 1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
			 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
			// front face
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			 1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
			 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			// left face
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			// right face
			 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			 1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
			 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
			// bottom face
			-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			 1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
			 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			-1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			// top face
			-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			 1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			 1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
			 1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			-1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
		};
		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &cubeVBO);
		// fill buffer
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		// link vertex attributes
		glBindVertexArray(cubeVAO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	// render Cube
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
	if (quadVAO == 0)
	{
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

void renderPointLight(Shader& shader, Model custom_model, glm::vec3 lightpos)
{
	shader.use();
	glm::mat4 projection = glm::perspective(glm::radians(camera.Fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
	glm::mat4 view = camera.GetViewMatrix();
	shader.setMat4("projection", projection);
	shader.setMat4("view", view);
	glm::mat4 model = glm::mat4(1);
	model = glm::scale(model, glm::vec3(0.2f));
	model = glm::translate(model, lightpos);
	shader.setMat4("model", model);
	custom_model.Draw(shader);
}

unsigned int sphereVAO = 0;
unsigned int indexCount;
void renderSphere()
{
	if (sphereVAO == 0)
	{
		glGenVertexArrays(1, &sphereVAO);

		unsigned int vbo, ebo;
		glGenBuffers(1, &vbo);
		glGenBuffers(1, &ebo);

		std::vector<glm::vec3> positions;
		std::vector<glm::vec2> uv;
		std::vector<glm::vec3> normals;
		std::vector<unsigned int> indices;

		const unsigned int X_SEGMENTS = 64;
		const unsigned int Y_SEGMENTS = 64;
		const float PI = 3.14159265359f;
		for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
		{
			for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
			{
				float xSegment = (float)x / (float)X_SEGMENTS;
				float ySegment = (float)y / (float)Y_SEGMENTS;
				float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
				float yPos = std::cos(ySegment * PI);
				float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

				positions.push_back(glm::vec3(xPos, yPos, zPos));
				uv.push_back(glm::vec2(xSegment, ySegment));
				normals.push_back(glm::vec3(xPos, yPos, zPos));
			}
		}

		bool oddRow = false;
		for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
		{
			if (!oddRow) // even rows: y == 0, y == 2; and so on
			{
				for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
				{
					indices.push_back(y * (X_SEGMENTS + 1) + x);
					indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
				}
			}
			else
			{
				for (int x = X_SEGMENTS; x >= 0; --x)
				{
					indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
					indices.push_back(y * (X_SEGMENTS + 1) + x);
				}
			}
			oddRow = !oddRow;
		}
		indexCount = static_cast<unsigned int>(indices.size());

		std::vector<float> data;
		for (unsigned int i = 0; i < positions.size(); ++i)
		{
			data.push_back(positions[i].x);
			data.push_back(positions[i].y);
			data.push_back(positions[i].z);
			if (normals.size() > 0)
			{
				data.push_back(normals[i].x);
				data.push_back(normals[i].y);
				data.push_back(normals[i].z);
			}
			if (uv.size() > 0)
			{
				data.push_back(uv[i].x);
				data.push_back(uv[i].y);
			}
		}
		glBindVertexArray(sphereVAO);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
		unsigned int stride = (3 + 2 + 3) * sizeof(float);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
	}

	glBindVertexArray(sphereVAO);
	glDrawElements(GL_TRIANGLE_STRIP, indexCount, GL_UNSIGNED_INT, 0);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		camera.ProcessKeyboard(DOWN, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		camera.ProcessKeyboard(UP, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

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

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

