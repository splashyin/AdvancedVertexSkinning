#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include "Lamp.h"
#include "Skeleton.h"
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/dual_quaternion.hpp>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include <iostream>


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void wireframeMode(GLFWwindow *window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

// settings
const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 1200;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
bool toggleObject = true;

std::vector<glm::mat4> Transforms;
std::vector<glm::highp_fdualquat> dualQuaternions;
std::vector<glm::mat2x4> DQs;

float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame
float animationTime = 0.0f;


int main(void)
{
	GLFWwindow* window;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	/* Initialize the library */
	if (!glfwInit())
		return -1;
	 
	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Master 2018", NULL, NULL);
	if (!window)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);
	
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// tell GLFW to capture our mouse
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (glewInit() != GLEW_OK)
	{
		std::cout << "Error!!!" << std::endl;
	}

	std::cout << glGetString(GL_VERSION) << std::endl;

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_PROGRAM_POINT_SIZE);

	//Shader modelShader("res/shaders/vertex.shader", "res/shaders/fragment.shader");
	Shader* lampShader = new Shader( "res/shaders/lamp.vs", "res/shaders/lamp.fs" );
	Shader skeletonShader("res/shaders/skeleton.vs", "res/shaders/skeleton.fs");
	Shader modelShader("res/shaders/vertex.shader", "res/shaders/fragment.shader");

	//Model aModel("res/object/body/skinning_test_2.fbx");
	//Model aModel("res/object/body/skinning_test.fbx");
	//Model aModel("res/object/body/skinning_test_3.fbx");
	//Model aModel("res/object/body/groo.fbx");
	//Model aModel("res/object/body/Pointing.fbx");
	//Model aModel("res/object/body/VictoryMonster.fbx");
	//Model aModel("res/object/body/balei.fbx");
	//Model aModel("res/object/body/VictoryMonster.fbx");
	Model aModel("res/object/body/get_up.fbx", false);
	
	//===========================================================
	// LAMP
	//===========================================================
	const glm::vec3 lampPos( 1.2f, 1.0f, 2.0f );
	const glm::vec3 lampColor( 1.0f, 1.0f, 1.0f );

	Lamp lamp( lampPos, lampColor );

	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	const char* glsl_version = "#version 430";
	ImGui_ImplOpenGL3_Init(glsl_version);
	ImGui::StyleColorsDark();


	bool lbs = false;
	bool dqs = false;
	static float f = 0.0f;
	
	bool show_demo_window = false;
	ImVec4 clear_color = ImVec4(0.098f, 0.231f, 0.298f, 1.00f);
	float startFrame = glfwGetTime();

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// per-frame time logic
		// --------------------
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		animationTime = currentFrame - startFrame;

		processInput(window);
		
		wireframeMode(window);

		glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//wireframe mode for debugging
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		
		//activate model shader
		// render 3D model
		modelShader.use(); //3d model shader
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		modelShader.setMat4("projection", projection);
		glm::mat4 view = camera.GetViewMatrix();
		modelShader.setMat4("view", view);
		glm::mat4 model(1.0f);
		model = glm::scale(model, glm::vec3(0.005f, 0.005f, 0.005f));	// it's a bit too big for our scene, so scale it down
		modelShader.setMat4("model", model);
		aModel.BoneTransform(animationTime, Transforms, dualQuaternions);
		for (unsigned int i = 0; i < Transforms.size(); ++i)
		{
			const std::string name = "gBones[" + std::to_string(i) + "]";
			GLuint boneTransform = glGetUniformLocation(modelShader.ID, name.c_str());
			glUniformMatrix4fv(boneTransform, 1, GL_FALSE, glm::value_ptr(Transforms[i]));
		}

		DQs.resize(dualQuaternions.size());
		for (unsigned int i = 0; i < dualQuaternions.size(); ++i) {
			DQs[i] = glm::mat2x4_cast(dualQuaternions[i]);
			const std::string name = "dqs[" + std::to_string(i) + "]";
			modelShader.setMat2x4(name, DQs[i]);
		}

		//set uniforms for model shader
		modelShader.setVec3( "lightPos", lamp.getPosition() );
		modelShader.setVec3( "lightColor", lamp.getColor() );
		modelShader.setVec3( "viewPos", camera.Position);
		//defult LBS
		modelShader.setBool("lbsOn", lbs);
		modelShader.setBool("dqsOn", dqs);
		modelShader.setFloat("ratio", f);
		aModel.Draw(modelShader);

		//activate lamp shader
		//render light cube(lamp)
		//lamp.Position.x = 1.0f + sin(glfwGetTime()) * 2.0f;
		lampShader->use();
		lampShader->setMat4("projection", projection);
		lampShader->setMat4("view", view);
		glm::mat4 lamp_cube(1.0f);
		//lamp_cube = glm::rotate(lamp_cube, (float)glfwGetTime(), glm::vec3(0.0, 1.0, 0.0));
		lamp_cube = glm::translate(lamp_cube, lamp.getPosition() );
		lamp_cube = glm::scale(lamp_cube, glm::vec3(0.2f));	// it's a bit too big for our scene, so scale it down
		//set uniforms for lamp shader
		lampShader->setMat4("model", lamp_cube);
		lamp.Draw( lampShader );

		//activate skeleton shader
		Skeleton* skeleton = new Skeleton( aModel.skeleton_pose );

		skeletonShader.use();
		skeletonShader.setMat4("projection", projection);
		skeletonShader.setMat4("view", view);
		glm::mat4 skeletom_model(1.0f);
		skeletom_model = glm::scale(skeletom_model, glm::vec3(0.005f, 0.005f, 0.005f));	// it's a bit too big for our scene, so scale it down
		skeletonShader.setMat4("model", skeletom_model);
		skeleton->Draw(skeletonShader);

		if (show_demo_window)
			ImGui::ShowDemoWindow(&show_demo_window);

		{
			
			ImGui::Begin("Advanced Vertex Skinning");         

			ImGui::Checkbox("LBS", &lbs);
			ImGui::Checkbox("DQS", &dqs);

			ImGui::SliderFloat("Ratio on DQS", &f, 0.0f, 1.0f);     // Edit 1 float using a slider from 0.0f to 1.0f
			ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

			ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
	return 0;
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height) 
{
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window) 
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
}

void wireframeMode(GLFWwindow *window) 
{

	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) 
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		toggleObject = true;
		glfwSetCursorPosCallback(window, mouse_callback);

	}
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		toggleObject = false;
		glfwSetCursorPosCallback(window, mouse_callback);
	}
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
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

	if (toggleObject)
	{
		camera.ProcessMouseMovement(xoffset, yoffset);
	}

	
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}


