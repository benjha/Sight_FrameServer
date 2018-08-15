/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 */

#define ASIO_STANDALONE

#include <thread>
#include <string.h>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <csignal>


#include "../header/cBroadcastServer.h"
#include "../header/cMouseEventHandler.h"
#include "../header/cKeyboardHandler.h"
#include "../header/cMessageHandler.h"
#include "../header/cDummyRenderer.h"

/*
 * websocket server
 */
broadcast_server *wsserver = 0;
/*
 * Handlers tracks events from the HTML viewer widgets and are global objects that can be shared by different classes.
 * They are populated in the broadcast_server class which receives data via websockets from the HTML viewer. Every control
 * in the HTML client should be registered in a handler. A handler could track one or more than one UI widget.
 * To access the handler's data, e.g. in a rendering class to change light position, the rendering class should
 * include a pointer to that handler.
 *
 */
cMouseHandler 		*mouseHandler = 0;
cKeyboardHandler 	*keyboardHandler = 0;
cMessageHandler		*msgHandler = 0;
cDummyRenderer		*dummyRenderer = 0;

volatile std::sig_atomic_t stop=0;

void display()
{
	dummyRenderer->render();

	if (wsserver->sendMoreFrames())
	{
		wsserver->sendFrame(dummyRenderer->getFramebuffer());
	}
	if (wsserver->saveFrame())
	{
		wsserver->save(dummyRenderer->getFramebuffer());
	}
	//std::cout << "Display\n";
}
//
//=======================================================================================
//
void idle()
{
	dummyRenderer->checkKeyboard();
	dummyRenderer->checkMouse();

}
//
//=======================================================================================
//
void loops()
{
	while (!stop)
	{
		display();
		idle();
	}
}
//
//=======================================================================================
//
void webSocketServer()
{
	std::cout << "Sight: launching server at port " << SERVER_PORT << std::endl;
	std::cout << "Sight: Press CTRL+C to exit...\n" << std::endl;

	wsserver->run(SERVER_PORT);

	std::cerr << "Sight: Exiting thread!" << std::endl;
}
//
//=======================================================================================
//
void setHandlers()
{
	mouseHandler	= new cMouseHandler();
	keyboardHandler = new cKeyboardHandler();
	msgHandler 		= new cMessageHandler();
	wsserver 		= new broadcast_server();
	//dummyRenderer	= new cDummyRenderer (IMAGE_WIDTH, IMAGE_HEIGHT, 3); // use 3 -> RGB components for JPEG Encoding
	dummyRenderer	= new cDummyRenderer (IMAGE_WIDTH, IMAGE_HEIGHT, 4); // use 4 -> RGBA components for GPU Encoding

	// share the handlers between classes that will use them
	// wsserver writes the state of the handlers
	wsserver->setMouseHandler(mouseHandler);
	wsserver->setKeyboardHandler(keyboardHandler);
	wsserver->setMessageHandler(msgHandler);

	// dummyrenderer reads the state of the handlers
	dummyRenderer->setMouseHandler(mouseHandler);
	dummyRenderer->setKeyboardHandler(keyboardHandler);
}
//
//=======================================================================================
//
void signalHandler (int signum)
{
	std::cout << "Sight: Stopped listening.\n";
	stop = signum;

}
#include <mutex>

//
//=======================================================================================
//
int main(int argc, char** argv)
{
	signal(SIGINT, signalHandler);

	srand (time(NULL));

	setHandlers();
	std::thread wsserverThread(webSocketServer);

	loops();

	wsserver->stop_listening();
	wsserverThread.join();

	delete dummyRenderer;
	delete mouseHandler;
	delete keyboardHandler;
	delete msgHandler;
	delete wsserver;

	return 0;
}

