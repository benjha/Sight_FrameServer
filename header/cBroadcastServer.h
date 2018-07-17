/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 */
#pragma once

#define _WEBSOCKETPP_CPP11_STL_
#define ASIO_STANDALONE


// WebSockets
#include <set>
#include <memory>
#include <iostream>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <functional>

#define REMOTE
//#define	FULLHD
//#define EVEREST


#define	SERVER_PORT 	9002

#ifdef FULLHD
	#define	IMAGE_WIDTH		1920
	#define IMAGE_HEIGHT	1080
#endif

#ifdef REMOTE
	#define JPEG_ENCODING
	#define CHANGE_RESOLUTION
	#define	RESOLUTION_FACTOR	1.0f
	#define	IMAGE_WIDTH			1920
	#define IMAGE_HEIGHT		1080
#endif

#ifdef EVEREST
	#define	IMAGE_WIDTH		1920*2
	#define IMAGE_HEIGHT	1080
#endif


typedef websocketpp::server<websocketpp::config::asio> server;

using websocketpp::connection_hdl;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

class cMouseHandler;
class cKeyboardHandler;
class cMessageHandler;

#ifdef JPEG_ENCODING
	#define TIME_RESPONSE		30
	#include <chrono>
	using namespace std::chrono;
	class cTurboJpegEncoder;
#endif

class cPNGEncoder;

class broadcast_server {
public:
			broadcast_server		(	);
			~broadcast_server		(	);

    void 	on_open						( connection_hdl 		hdl								);
    void 	on_close					( connection_hdl 		hdl								);
    void 	on_message					( connection_hdl 		hdl, server::message_ptr msg	);
    void 	stop_listening				(	);
    void 	run							( uint16_t 				port							);
    void 	sendFrame 					( float 				*img							);
    void 	sendFrame 					( unsigned char			*img							);
    void 	setFrame 					( float 				*img							);
    void 	setMouseHandler				( cMouseHandler			*mouseH							);
    void 	setKeyboardHandler			( cKeyboardHandler		*keyboardH						);
    void 	setMessageHandler			( cMessageHandler		*messageH						);

    bool	sendMoreFrames				(	) 				{return needMoreFrames; };

private:

    void	parse 					( int type, std::stringstream *value 	);
    void	scale 					( unsigned char *in, unsigned char *out, float factor );
    bool				needMoreFrames, stop, saveFrame;
    typedef	std::set<connection_hdl,std::owner_less<connection_hdl>> con_list;
    server 				m_server;
    con_list 			m_connections;
    std::stringstream 	string;

    cMouseHandler		*mouseHandler;
    cKeyboardHandler	*keyboardHandler;
    cMessageHandler		*messageHandler;

    // PNG Encoder
	cPNGEncoder								*pngEncoder;


#ifdef JPEG_ENCODING
    // Adjust quality of the JPEG according to the
    // image transport throughput
    void adjustJpegQuality			( 	);
	// JPEG encoder
	cTurboJpegEncoder						*jpegEncoder;
	// JPEG encoding quality
	unsigned int							jpegQuality;
	// Specify the desired time response for image transport in milliseconds
	float 									targetTime;
	//streaming timers used to calculate image transport throughput
	high_resolution_clock::time_point		stTimer1, stTimer2;
	// image transport duration in milliseconds
	std::chrono::microseconds				stDuration;
#endif
#ifdef SAVE_IMG
    void	scale				( unsigned char *in, unsigned char *out, float factor );
#endif
};