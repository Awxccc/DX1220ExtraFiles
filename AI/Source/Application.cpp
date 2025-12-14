#include "Application.h"

//Include GLEW
#include <GL/glew.h>

//Include GLFW
#include <GLFW/glfw3.h>

//Include the standard C++ headers
#include <stdio.h>
#include <stdlib.h>

#include "SceneTicTacToe.h"
#include "SceneMovement_Week01.h"
#include "SceneMovement_Week02.h"
#include "SceneMovement_Week03.h"
#include "SceneMovement_Week04.h"
#include "SceneMovement_Week05.h"
#include "SceneKnight.h"
#include "SceneMaze.h"
#include "SceneQueen.h"
#include "SceneTurn.h"
#include "SceneSandbox.h"

GLFWwindow* m_window;
const unsigned char FPS = 60; // FPS of this game
const unsigned int frameTime = 1000 / FPS; // time for each frame
int m_width, m_height;

//Define an error callback
static void error_callback(int error, const char* description)
{
	fputs(description, stderr);
	_fgetchar();
}

//Define the key input callback
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}


void resize_callback(GLFWwindow* window, int w, int h)
{
	m_width = w;
	m_height = h;
	glViewport(0, 0, w, h);
}

bool Application::IsKeyPressed(unsigned short key)
{
    return ((GetAsyncKeyState(key) & 0x8001) != 0);
}
bool Application::IsMousePressed(unsigned short key) //0 - Left, 1 - Right, 2 - Middle
{
	return glfwGetMouseButton(m_window, key) != 0;
}
void Application::GetCursorPos(double *xpos, double *ypos)
{
	glfwGetCursorPos(m_window, xpos, ypos);
}
int Application::GetWindowWidth()
{
	return m_width;
}
int Application::GetWindowHeight()
{
	return m_height;
}

Application::Application()
	: m_scene{}, m_timer{}
{
}

Application::~Application()
{
}

/**
 *	Load scene based on user selection
 */
bool Application::LoadScene()
{
	bool bContinue = true;	// Variable to control whether the do-loop is continued
	int choice; // Variable to store user's menu choice

	do {
		// Display the menu options
		std::cout << "--- Main Menu : Choose the scene to run ---" << std::endl;
		std::cout << "1.  Week 1A. SceneTicTacToe" << std::endl;
		std::cout << "2.  Week 1B. SceneMovement_Week01" << std::endl;
		std::cout << "3.  Week 2.  SceneMovement_Week02" << std::endl;
		std::cout << "4.  Week 3.  SceneMovement_Week03" << std::endl;
		std::cout << "5.  Week 4.  SceneMovement_Week04" << std::endl;
		std::cout << "6.  Week 5.  SceneMovement_Week05" << std::endl;
		std::cout << "7.  Week 6A. SceneKnight" << std::endl;
		std::cout << "8.  Week 6B. SceneQueen" << std::endl;
		std::cout << "9.  Week 7.  SceneMaze" << std::endl;
		std::cout << "10. Week 8.  SceneTurn" << std::endl;
		std::cout << "11. Week 9.  SceneGraph_Week09" << std::endl;
		std::cout << "12. Week 12. SceneGraph_Week12" << std::endl;
		std::cout << "13. Week 13. SceneHex" << std::endl;
		std::cout << "14. Week 14. SceneReversi" << std::endl;
		std::cout << "15. Week 16. SceneFlappyBird" << std::endl;
		std::cout << "16. Assignment 1" << std::endl;
		std::cout << "0. Exit" << std::endl;
		std::cout << "Enter your choice: ";

		// Get user input
		std::cin >> choice;

		// Process the user's choice
		switch (choice) {
		case 1:
			std::cout << "You selected SceneTicTacToe.\n";
			m_scene = new SceneTicTacToe();
			bContinue = false;
			break;
		case 2:
			std::cout << "You selected SceneMovement_Week01.\n";
			m_scene = new SceneMovement_Week01();
			bContinue = false;
			break;
		case 3:
			std::cout << "You selected SceneMovement_Week02.\n";
			m_scene = new SceneMovement_Week02();
			bContinue = false;
			break;
		case 4:
			std::cout << "You selected SceneMovement_Week03.\n";
			m_scene = new SceneMovement_Week03();
			bContinue = false;
			break;
		case 5:
			std::cout << "You selected SceneMovement_Week04.\n";
			m_scene = new SceneMovement_Week04();
			bContinue = false;
			break;
		case 6:
			std::cout << "You selected SceneMovement_Week05.\n";
			m_scene = new SceneMovement_Week05();
			bContinue = false;
			break;
		case 7:
			std::cout << "You selected SceneKnight.\n";
			m_scene = new SceneKnight();
			bContinue = false;
			break;
		case 8:
			std::cout << "You selected SceneQueen.\n";
			m_scene = new SceneQueen();
			bContinue = false;
			break;
		case 9:
			std::cout << "You selected SceneMaze.\n";
			m_scene = new SceneMaze();
			bContinue = false;
			break;
		case 10:
			std::cout << "You selected SceneTurn.\n";
			m_scene = new SceneTurn();
			bContinue = false;
			break;
		case 11:
			std::cout << "You selected SceneGraph_Week09.\n";
			bContinue = false;
			break;
		case 12:
			std::cout << "You selected SceneGraph_Week12.\n";
			bContinue = false;
			break;
		case 13:
			std::cout << "You selected SceneHex.\n";
			bContinue = false;
			break;
		case 14:
			std::cout << "You selected SceneReversi.\n";
			bContinue = false;
			break;
		case 15:
			std::cout << "You selected SceneFlappyBird.\n";
			bContinue = false;
			break;
		case 16:
			std::cout << "You selected SceneAssignment1.\n";
			m_scene = new SceneSandbox();
			bContinue = false;
			break;
		case 0:
			std::cout << "You selected quitting this application.\n";
			return false;
			break;
		default:
			std::cout << "Invalid choice. Please try again.\n";
			// Clear error flags and ignore remaining input in the buffer
			std::cin.clear();
			std::cin.ignore(10000, '\n');
			break;
		}
	} while (bContinue == true); // Loop until bContinue != true

	return true;
}

void Application::Init()
{
	//Set the error callback
	glfwSetErrorCallback(error_callback);

	//Initialize GLFW
	if (!glfwInit())
	{
		exit(EXIT_FAILURE);
	}

	//Set the GLFW window creation hints - these are optional
	glfwWindowHint(GLFW_SAMPLES, 4); //Request 4x antialiasing
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); //Request a specific OpenGL version
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); //Request a specific OpenGL version
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //We don't want the old OpenGL 


	//Create a window and create its OpenGL context
	
	m_width = 1000;
	m_height = 600;
	m_window = glfwCreateWindow(m_width, m_height, "AI for Games", NULL, NULL);
	/*
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	m_width = mode->width;
	m_height = mode->height;
	m_window = glfwCreateWindow(m_width, m_height, "AI for Games", monitor, NULL);*/

	//If the window couldn't be created
	if (!m_window)
	{
		fprintf( stderr, "Failed to open GLFW window.\n" );
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	//This function makes the context of the specified window current on the calling thread. 
	glfwMakeContextCurrent(m_window);

	//Sets the key callback
	//glfwSetKeyCallback(m_window, key_callback);
	glfwSetWindowSizeCallback(m_window, resize_callback);

	glewExperimental = true; // Needed for core profile
	//Initialize GLEW
	GLenum err = glewInit();

	//If GLEW hasn't initialized
	if (err != GLEW_OK) 
	{
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		//return -1;
	}
}

void Application::Run()
{
	if (m_scene == NULL)
		return;

	//Main Loop
	m_scene->Init();

	m_timer.startTimer();    // Start timer to calculate how long it takes to render this frame
	while (!glfwWindowShouldClose(m_window) && !IsKeyPressed(VK_ESCAPE))
	{
		m_scene->Update(m_timer.getElapsedTime());
		m_scene->Render();
		//Swap buffers
		glfwSwapBuffers(m_window);
		//Get and organize events, like keyboard and mouse input, window resizing, etc...
		glfwPollEvents();
        m_timer.waitUntil(frameTime);       // Frame rate limiter. Limits each frame to a specified time in ms.   

	} //Check if the ESC key had been pressed or if the window had been closed
	m_scene->Exit();
	delete m_scene;
}

void Application::Exit()
{
	//Close OpenGL window and terminate GLFW
	glfwDestroyWindow(m_window);
	//Finalize and clean up GLFW
	glfwTerminate();
}

void Application::Iterate()
{
	m_scene->Update(0);
	m_scene->Render();
	glfwSwapBuffers(m_window);
	glfwPollEvents();
}
