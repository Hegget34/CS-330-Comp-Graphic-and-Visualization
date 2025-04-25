///////////////////////////////////////////////////////////////////////////////
// viewmanager.cpp
// ============
// manage the viewing of 3D objects within the viewport
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#include "ViewManager.h"

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>    

// declaration of the global variables and defines
namespace
{
	// Variables for window width and height
	const int WINDOW_WIDTH = 1000;
	const int WINDOW_HEIGHT = 800;
	const char* g_ViewName = "view";
	const char* g_ProjectionName = "projection";

	// camera object used for viewing and interacting with
	// the 3D scene
	Camera* g_pCamera = nullptr;

	// Global pointer to the view manager for access in callbacks
	ViewManager* g_ViewManager = nullptr;

	// these variables are used for mouse movement processing
	float gLastX = WINDOW_WIDTH / 2.0f;
	float gLastY = WINDOW_HEIGHT / 2.0f;
	bool gFirstMouse = true;

	// time between current frame and last frame
	float gDeltaTime = 0.0f;
	float gLastFrame = 0.0f;

	// the following variable is false when orthographic projection
	// is off and true when it is on
	bool bOrthographicProjection = false;

	// Define camera movement parameters
	const float CAMERA_SPEED = 5.0f;     // Default speed
	float currentCameraSpeed = CAMERA_SPEED;

	// Mouse sensitivity - reduced for smoother control
	const float MOUSE_SENSITIVITY = 0.05f;
}

/***********************************************************
 *  ViewManager()
 *
 *  The constructor for the class
 ***********************************************************/
ViewManager::ViewManager(
	ShaderManager* pShaderManager)
{
	// initialize the member variables
	m_pShaderManager = pShaderManager;
	m_pWindow = NULL;
	g_pCamera = new Camera();
	// default camera view parameters
	g_pCamera->Position = glm::vec3(0.0f, 3.0f, 8.0f);
	g_pCamera->Front = glm::vec3(0.0f, -0.3f, -1.0f);
	g_pCamera->Up = glm::vec3(0.0f, 1.0f, 0.0f);

	// Set the global pointer to this instance
	g_ViewManager = this;
}

/***********************************************************
 *  ~ViewManager()
 *
 *  The destructor for the class
 ***********************************************************/
ViewManager::~ViewManager()
{
	// free up allocated memory
	m_pShaderManager = NULL;
	m_pWindow = NULL;
	if (NULL != g_pCamera)
	{
		delete g_pCamera;
		g_pCamera = NULL;
	}
}

/***********************************************************
 *  SetCameraPreset()
 *
 *  This method sets the camera to predefined viewing positions
 ***********************************************************/
void ViewManager::SetCameraPreset(int presetNumber)
{
	if (g_pCamera != nullptr)
	{
		switch (presetNumber)
		{
		case 1: // Front view
			g_pCamera->Position = glm::vec3(0.0f, 0.0f, 8.0f);
			g_pCamera->Yaw = -90.0f;
			g_pCamera->Pitch = 0.0f;
			break;
		case 2: // Top-down view
			g_pCamera->Position = glm::vec3(0.0f, 8.0f, 0.0f);
			g_pCamera->Yaw = -90.0f;
			g_pCamera->Pitch = -89.0f;
			break;
		case 3: // Side view 
			g_pCamera->Position = glm::vec3(8.0f, 0.0f, 0.0f);
			g_pCamera->Yaw = 180.0f;
			g_pCamera->Pitch = 0.0f;
			break;
		case 4: // Angled view
			g_pCamera->Position = glm::vec3(5.0f, 3.0f, 5.0f);
			g_pCamera->Yaw = -135.0f;
			g_pCamera->Pitch = -30.0f;
			break;
		default:
			return;
		}
		g_pCamera->updateVectors();
	}
}

/***********************************************************
 *  CreateDisplayWindow()
 *
 *  This method is used to create the main display window.
 ***********************************************************/
GLFWwindow* ViewManager::CreateDisplayWindow(const char* windowTitle)
{
	GLFWwindow* window = nullptr;

	// try to create the displayed OpenGL window
	window = glfwCreateWindow(
		WINDOW_WIDTH,
		WINDOW_HEIGHT,
		windowTitle,
		NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return NULL;
	}
	glfwMakeContextCurrent(window);

	// tell GLFW to capture all mouse events
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// this callback is used to receive mouse moving events
	glfwSetCursorPosCallback(window, &ViewManager::Mouse_Position_Callback);

	// set up scroll callback for speed adjustment
	glfwSetScrollCallback(window, [](GLFWwindow* window, double xoffset, double yoffset) {
		// Adjust camera speed with scroll
		// Use smaller increments for finer control
		float speedMultiplier = (currentCameraSpeed < 5.0f) ? 0.2f : 0.5f;
		currentCameraSpeed += (float)yoffset * speedMultiplier;

		if (currentCameraSpeed < 0.5f) currentCameraSpeed = 0.5f;
		if (currentCameraSpeed > 15.0f) currentCameraSpeed = 15.0f;
		std::cout << "Camera Speed: " << currentCameraSpeed << "x" << std::endl;
		});

	// setup key callback for projection toggle and camera presets
	glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
		// Toggle orthographic/perspective view with O/P keys
		if (key == GLFW_KEY_O && action == GLFW_PRESS && !bOrthographicProjection) {
			bOrthographicProjection = true;
			std::cout << "Switched to Orthographic View" << std::endl;
		}
		else if (key == GLFW_KEY_P && action == GLFW_PRESS && bOrthographicProjection) {
			bOrthographicProjection = false;
			std::cout << "Switched to Perspective View" << std::endl;
		}
		// Camera preset positions
		else if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
			g_ViewManager->SetCameraPreset(1);
			std::cout << "Camera Preset 1: Front View" << std::endl;
		}
		else if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
			g_ViewManager->SetCameraPreset(2);
			std::cout << "Camera Preset 2: Top View" << std::endl;
		}
		else if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
			g_ViewManager->SetCameraPreset(3);
			std::cout << "Camera Preset 3: Side View" << std::endl;
		}
		else if (key == GLFW_KEY_4 && action == GLFW_PRESS) {
			g_ViewManager->SetCameraPreset(4);
			std::cout << "Camera Preset 4: Angled View" << std::endl;
		}
		});

	// enable blending for supporting tranparent rendering
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	m_pWindow = window;

	return(window);
}

/***********************************************************
 *  Mouse_Position_Callback()
 *
 *  This method is automatically called from GLFW whenever
 *  the mouse is moved within the active GLFW display window.
 ***********************************************************/
void ViewManager::Mouse_Position_Callback(GLFWwindow* window, double xMousePos, double yMousePos)
{
	if (gFirstMouse)
	{
		gLastX = (float)xMousePos;
		gLastY = (float)yMousePos;
		gFirstMouse = false;
	}

	float xoffset = (float)xMousePos - gLastX;
	float yoffset = gLastY - (float)yMousePos; // reversed since y-coordinates go from bottom to top
	gLastX = (float)xMousePos;
	gLastY = (float)yMousePos;

	// Apply reduced sensitivity for smoother camera control
	xoffset *= MOUSE_SENSITIVITY;
	yoffset *= MOUSE_SENSITIVITY;

	// Update camera orientation
	if (g_pCamera != nullptr)
	{
		// Update the camera Front vector based on input
		glm::vec3 front;
		g_pCamera->Yaw += xoffset;
		g_pCamera->Pitch += yoffset;

		// Constrain pitch
		if (g_pCamera->Pitch > 89.0f)
			g_pCamera->Pitch = 89.0f;
		if (g_pCamera->Pitch < -89.0f)
			g_pCamera->Pitch = -89.0f;

		// Calculate new front vector
		front.x = cos(glm::radians(g_pCamera->Yaw)) * cos(glm::radians(g_pCamera->Pitch));
		front.y = sin(glm::radians(g_pCamera->Pitch));
		front.z = sin(glm::radians(g_pCamera->Yaw)) * cos(glm::radians(g_pCamera->Pitch));
		g_pCamera->Front = glm::normalize(front);

		// Re-calculate Right and Up vectors
		g_pCamera->Right = glm::normalize(glm::cross(g_pCamera->Front, glm::vec3(0.0f, 1.0f, 0.0f)));
		g_pCamera->Up = glm::normalize(glm::cross(g_pCamera->Right, g_pCamera->Front));
	}
}

/***********************************************************
 *  ProcessKeyboardEvents()
 *
 *  This method is called to process any keyboard events
 *  that may be waiting in the event queue.
 ***********************************************************/
void ViewManager::ProcessKeyboardEvents()
{
	// close the window if the escape key has been pressed
	if (glfwGetKey(m_pWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(m_pWindow, true);
	}

	// Process camera movement
	if (g_pCamera != nullptr && m_pWindow != nullptr)
	{
		// Calculate delta time
		float currentFrame = (float)glfwGetTime();
		gDeltaTime = currentFrame - gLastFrame;
		gLastFrame = currentFrame;

		// Calculate velocity
		float velocity = currentCameraSpeed * gDeltaTime;

		// Forward movement - W key
		if (glfwGetKey(m_pWindow, GLFW_KEY_W) == GLFW_PRESS)
		{
			g_pCamera->Position += g_pCamera->Front * velocity;
		}

		// Backward movement - S key
		if (glfwGetKey(m_pWindow, GLFW_KEY_S) == GLFW_PRESS)
		{
			g_pCamera->Position -= g_pCamera->Front * velocity;
		}

		// Left movement - A key
		if (glfwGetKey(m_pWindow, GLFW_KEY_A) == GLFW_PRESS)
		{
			g_pCamera->Position -= g_pCamera->Right * velocity;
		}

		// Right movement - D key
		if (glfwGetKey(m_pWindow, GLFW_KEY_D) == GLFW_PRESS)
		{
			g_pCamera->Position += g_pCamera->Right * velocity;
		}

		// Up movement - Q key (absolute world up)
		if (glfwGetKey(m_pWindow, GLFW_KEY_Q) == GLFW_PRESS)
		{
			g_pCamera->Position += glm::vec3(0.0f, 1.0f, 0.0f) * velocity;
		}

		// Down movement - E key (absolute world down)
		if (glfwGetKey(m_pWindow, GLFW_KEY_E) == GLFW_PRESS)
		{
			g_pCamera->Position -= glm::vec3(0.0f, 1.0f, 0.0f) * velocity;
		}
	}
}

/***********************************************************
 *  PrepareSceneView()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene
 *  rendering
 ***********************************************************/
void ViewManager::PrepareSceneView()
{
	glm::mat4 view;
	glm::mat4 projection;

	// process any keyboard events that may be waiting in the 
	// event queue
	ProcessKeyboardEvents();

	// get the current view matrix from the camera
	view = g_pCamera->GetViewMatrix();

	// define the current projection matrix
	if (bOrthographicProjection)
	{
		// Orthographic projection
		float aspectRatio = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;
		float orthoSize = 5.0f;
		projection = glm::ortho(
			-orthoSize * aspectRatio, orthoSize * aspectRatio,  // left, right
			-orthoSize, orthoSize,                              // bottom, top
			0.1f, 100.0f);                                      // near, far
	}
	else
	{
		// Perspective projection
		projection = glm::perspective(
			glm::radians(45.0f),                            // FOV
			(float)WINDOW_WIDTH / (float)WINDOW_HEIGHT,     // aspect ratio
			0.1f,                                           // near plane
			100.0f);                                        // far plane
	}

	// if the shader manager object is valid
	if (NULL != m_pShaderManager)
	{
		// set the view matrix into the shader for proper rendering
		m_pShaderManager->setMat4Value(g_ViewName, view);
		// set the view matrix into the shader for proper rendering
		m_pShaderManager->setMat4Value(g_ProjectionName, projection);
		// set the view position of the camera into the shader for proper rendering
		m_pShaderManager->setVec3Value("viewPosition", g_pCamera->Position);
	}
}