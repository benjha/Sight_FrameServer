/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 */

#ifndef CMOUSEEVENTHANDLER_H_
#define CMOUSEEVENTHANDLER_H_

#define MOUSE_EVENT 5 // according to the RFB protocol

#include <iostream>
#include <sstream>

class cMouseHandler
{
public:
	enum
	{
		LEFT_BUTTON,
		MIDDLE_BUTTON,
		RIGHT_BUTTON,
		DOWN,
		UP,
	};

	cMouseHandler						(	) { x = 0, y = 0, button = -1, state = UP;	isRefresh = false;};
	~cMouseHandler 						(	) {	};

	void 	setCoords						( int x_, int y_				)	{ x = x_, y = y_; 					};

	void	setButton						( int buttonMask				)
	{
		switch (buttonMask)
		{
			case 0:
				state = UP;
				//std::cout << "State up" << std::endl;
				break;
			case 1:
				button 	= LEFT_BUTTON;
				state 	= DOWN;
				//std::cout << "Left button Down\n";
				break;
			case 2:
				button	= MIDDLE_BUTTON;
				state	= DOWN;
				//std::cout << "Middle button Down\n";
				break;
			case 4:
				button	= RIGHT_BUTTON;
				state	= DOWN;
				//std::cout << "Right button Down\n";
				break;
		}

	};

	void	refresh							( bool flag 					)
	{
		if (flag == false) // reset mouse state
		{
			 button = -1;
			 state = UP;
		}
		isRefresh = flag;
	};

	int 	getX							(	) 		{ return x; 			};
	int 	getY							(	)		{ return y; 			};
	int		getState						(	)		{ return state;			};
	int		getButton						(	)		{ return button;		};

	bool	refreshed						(	)		{ return isRefresh;	};

	void	parse							(std::stringstream *value)
	{
		int buttonMask, x, y;

		buttonMask = value->str().data()[1];
		x = (unsigned char)value->str().data()[2]*255 + (unsigned char)value->str().data()[3];
		y = (unsigned char)value->str().data()[4]*255 + (unsigned char)value->str().data()[5];

		setCoords 	( x, y		 );
		setButton 	( buttonMask );
		refresh		( true 		 );

	};


private:

	int 	x, y;
	int 	button;
	int		state;
	bool	isRefresh;

};




#endif /* CMOUSEEVENTHANDLER_H_ */
