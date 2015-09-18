/******************************************************************************
Class:
Implements:
Author:Rich Davison	<richard.davison4@newcastle.ac.uk> and YOU!
Description: We have a revised 'main' file this time around. The old main file
was sometimes quite hard to debug when classes failed to initialise, so I 
have made a dedicated 'Quit' function, which will destroy everything, and 
will halt and post a quit message if anything has gone wrong. The Window class
now also brings the console window into focus ini ts destructor, so that we
can immediately see which class has caused initialisation to end. 

You may like to take this further and have each class initialised in the 
main file to have a number of 'error strings' to output in this function in 
order to quickly track down what has gone wrong - it shouldn't be too hard to
implement this, so I'll leave it up to you!

Note that this time the Window is a single class, instantiated and destroyed
via static functions. Noone ever used the ability to have multiple renderers
and windows to good effect, and it made the code more complex, so I have 
removed these abilities in the code download for this module! The less things
that can go wrong, the more time you potentially have to have fun with physics!

-_-_-_-_-_-_-_,------,   
_-_-_-_-_-_-_-|   /\_/\   NYANYANYAN
-_-_-_-_-_-_-~|__( ^ .^) /
_-_-_-_-_-_-_-""  ""   

*//////////////////////////////////////////////////////////////////////////////
#include "../../nclgl/Window.h"
#include "MyGame.h"
#include <thread>

#pragma comment(lib, "nclgl.lib")

int Quit(bool pause = false, const string &reason = "") {
	PhysicsSystem::Destroy();
	Window::Destroy();
	Renderer::Destroy();

	if(pause) {
		std::cout << reason << std::endl;
		system("PAUSE");
	}

	return 0;
}

int main() {
	if(!Window::Initialise("Game Technologies", 1920,1080,true)) {
		return Quit(true, "Window failed to initialise!");
	}

	if(!Renderer::Initialise()) {
		return Quit(true, "Renderer failed to initialise!");
	}

	volatile bool cont = true;

	PhysicsSystem::Initialise();

	MyGame* game = new MyGame();

	Window::GetWindow().LockMouseToWindow(true);
	Window::GetWindow().ShowOSPointer(false);

	std::thread phys = std::thread(&GameClass::UpdatePhysics, game, &cont);

	while(Window::GetWindow().UpdateWindow() && !Window::GetKeyboard()->KeyDown(KEYBOARD_ESCAPE)){
		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_1)) {
			game->ShootSphere(((rand() % 4) + 1) * 20.0f);
		}
		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_P)) {
			Renderer::GetRenderer().ToggleEffect(1);
		}
		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_O)) {
			game->ToggleSlowMo();
		}
		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_G)) {
			game->ToggleGravity();
		}
		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_UP)) {
			game->IncCohesion();
		}
		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_DOWN)) {
			game->DecCohesion();
		}
		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_LEFT)) {
			game->IncAlign();
		}
		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_RIGHT)) {
			game->DecAlign();
		}
		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_8)) {
			game->IncSep();
		}
		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_9)) {
			game->DecSep();
		}


		float msec = Window::GetWindow().GetTimer()->GetTimedMS();	//How many milliseconds since last update?
		game->UpdateCore(msec);	//Update our 'sybsystem' logic (renderer and physics!)
		game->UpdateGame(msec);	//Update our game logic
	}
	cont = false;
	phys.join();
	
	delete game;	//Done with our game now...bye bye!
	return Quit();
}