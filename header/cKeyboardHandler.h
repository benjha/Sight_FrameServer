/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 */

#ifndef CKEYBOARDHANDLER_H_
#define CKEYBOARDHANDLER_H_

#define KEY_EVENT   4  // according to the RFB protocol

class cKeyboardHandler {
public:
	enum {
		UP, DOWN
	};

	cKeyboardHandler() {
		key = -1, state = UP;
	}
	;
	~cKeyboardHandler() {
	}
	;

	void setState(int state_) {
		state = state_;
	}
	;

	void setKey(int keyMask) {
		key = keyMask;
	};

	void refresh(bool flag)
	{
		if (flag == false) // reset keyboard state
		{
			key = -1, state = UP;
		}
		isRefresh = flag;
	}
	;

	int getState() {
		return state;
	}
	;
	int getKey() {
		return key;
	}
	;
	bool refreshed() {
		return isRefresh;
	}
	;

	void parse(std::stringstream *value) {
		int keyMask = (unsigned char) value->str().data()[4] * 255
				+ (unsigned char) value->str().data()[5] * 255
				+ (unsigned char) value->str().data()[6] * 255
				+ (unsigned char) value->str().data()[7];

		setState(value->str().data()[1]);
		setKey(keyMask);
		refresh(true);

		//if (getState() == cKeyboardHandler::DOWN)
		//	std::cout << "cKeyboardHandler Key: " << getKey() << std::endl;
	}
	;

private:
	int key;
	int state;
	bool isRefresh;
};

#endif /* CKEYBOARDHANDLER_H_ */
