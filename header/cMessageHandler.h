/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 */
#ifndef CMESSAGE_H_
#define CMESSAGE_H_

#define MESSAGE_EVENT			0 // from websockets client

class cMessageHandler
{
public:
				cMessageHandler			(	) { };
				~cMessageHandler 		(	) { };

	void		parse 					( std::stringstream *value )
	{
		std::cout << value->str().data()[0] << std::endl;
		std::cout << value->str().data()[1] << std::endl;
	};

private:
};



#endif /* CMESSAGE_H_ */
