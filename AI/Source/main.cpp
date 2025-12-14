#include "Application.h"

int main( void )
{
	// Get the instance for Application class
	Application &app = Application::GetInstance();
	// Load the scene based on user's selection
	if (app.LoadScene() == true)	// If the user selected a valid scene
	{
		// Initialise the application
		app.Init();
		// Run the application
		app.Run();
		// Exit the application
		app.Exit();
		// Return 0 since the application was ran successfully
		return 0;
	}

	// Return 1 since the application was not ran successfully
	return 1;
}