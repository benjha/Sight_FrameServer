/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 */
#ifndef CDUMMYRENDERER_H_
#define CDUMMYRENDERER_H_

#include <stdlib.h>
/*
 * Implements a dummy rendering to populate with random colors the frame buffer to be send
 * to the HTML viewer
 *
 */

// UI/Event Handlers to be used within cDummyRenderer class
#include "../header/cMouseEventHandler.h"
#include "../header/cKeyboardHandler.h"

class cDummyRenderer
{
public:
	cDummyRenderer (int w, int h)
	{
		m_w = w;
		m_h = h;
		framebuffer = new unsigned char[m_w*m_h*3];
		m_mouseHandler = 0;
		m_keyboardHandler = 0;
		m_renderMode = 1;
	};
	~cDummyRenderer ( )
	{
		delete [] framebuffer;
	};
	void setMouseHandler (cMouseHandler *mh)
	{
		m_mouseHandler = mh;
	};
	void setKeyboardHandler (cKeyboardHandler *kbh)
	{
		m_keyboardHandler = kbh;
	};
	unsigned char *getFramebuffer ( )
	{
		return framebuffer;
	};
	void render ()
	{
		unsigned int i;
		if (m_renderMode == 1)
		{
			for (i = 0; i < m_w * m_h * 3; i += 3)
			{
				//std::cout << (int)pixels[i] << ", " << (int)pixels[i+1] << ", " << (int)pixels[i+2] << "\t\n";
				framebuffer[i] = rand() % 255;
				framebuffer[i + 1] = rand() % 255;
				framebuffer[i + 2] = rand() % 255;
			}
		}
		else if (m_renderMode == 2)
		{
			for (i = 0; i < m_w * m_h * 3; i += 3)
			{
				//std::cout << (int)pixels[i] << ", " << (int)pixels[i+1] << ", " << (int)pixels[i+2] << "\t\n";
				framebuffer[i] = 255;
				framebuffer[i + 1] = 0;
				framebuffer[i + 2] = 0;
			}
		}

	};
	void checkKeyboard ()
	{
		if (!m_keyboardHandler)
		{
			std::cout << "Sight@cDummyRenderer: Is Keyboard handler initialized? \n";
			return;
		}
		if (m_keyboardHandler->refreshed()) // if new data available
		{

			if (m_keyboardHandler->getState()==cKeyboardHandler::DOWN)
			{
				int key = m_keyboardHandler->getKey();
				switch (key)
				{
				case '1':
					m_renderMode = 1;
					break;
				case '2':
					m_renderMode = 2;
					break;
				}
				std::cout << "Sight@cDummyRenderer: Render mode " << m_renderMode << std::endl;
			}

			m_keyboardHandler->refresh(false); // data was consumed
		}
	}
	void checkMouse ()
	{
		if (!m_mouseHandler)
		{
			std::cout << "Sight@cDummyRenderer: Is mouse handler initialized? \n";
			return;
		}
		if ( m_mouseHandler->refreshed() )
		{
			std::cout << "Sight@cDummyRenderer: Mouse X: " << m_mouseHandler->getX() << " Y: " << m_mouseHandler->getY () << " \n";

			switch (m_mouseHandler->getButton())
			{
			case cMouseHandler::LEFT_BUTTON:
				if (m_mouseHandler->getState()==cMouseHandler::DOWN)
				{
					std::cout << "Sight@cDummyRenderer: Mouse status LEFT_BUTTON DOWN\n";
				}
				break;
			case cMouseHandler::RIGHT_BUTTON:
				if (m_mouseHandler->getState()==cMouseHandler::DOWN)
				{
					std::cout << "Sight@cDummyRenderer: Mouse status RIGHT_BUTTON DOWN\n";
				}
				break;
			case cMouseHandler::MIDDLE_BUTTON:
				if (m_mouseHandler->getState()==cMouseHandler::DOWN)
				{
					std::cout << "Sight@cDummyRenderer: Mouse status MIDDLE_BUTTON DOWN\n";

				}
				break;
			}

			m_mouseHandler->refresh(false);
		}

	}
private:

	int 			m_renderMode;
	unsigned int	m_w, m_h;
	unsigned char	*framebuffer;
	// UI/Event Handlers to be used within cDummyRenderer class
	cMouseHandler	*m_mouseHandler;
	cKeyboardHandler *m_keyboardHandler;


};


#endif /* CDUMMYRENDERER_H_ */
