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

#include "cTimer.h"
#include "cStats.h"

//#define STATS
//#define REMOTE
#define REMOTE_GPU_ENCODING
//#define NO_COMPRESSION
//#define EVEREST

#define SERVER_PORT     9002


#ifdef REMOTE_GPU_ENCODING
        #define NVPIPE_ENCODING
        #define MBPS                            32 // Works for FULLHD
		//#define MBPS                            256
        #define TARGET_FPS                      30
        //#define TWOK
        //#define FOURK
        #define FULLHD
#endif

#ifdef NO_COMPRESSION
	#define FULLHD
#endif

#ifdef REMOTE
        #define JPEG_ENCODING
        //#define CHANGE_RESOLUTION
        #define RESOLUTION_FACTOR       1.0f
        #define FULLHD
#endif

#ifdef EVEREST
        #define IMAGE_WIDTH             1920*2
        #define IMAGE_HEIGHT    1080
#endif

#ifdef FULLHD
        #define IMAGE_WIDTH             1920
        #define IMAGE_HEIGHT    1088
#endif

#ifdef TWOK
        #define IMAGE_WIDTH     2560
        #define IMAGE_HEIGHT    1440
#endif

#ifdef FOURK
        #define IMAGE_WIDTH     3840
        #define IMAGE_HEIGHT    2160
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

#ifdef NVPIPE_ENCODING
	class cNvPipeEncoderWrapper;
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

#ifdef NVPIPE_ENCODING
    void	sendFrame					( void *gpuFrameBufferPtr  );
#endif
    void 	setFrame 					( float 				*img							);
    void 	setMouseHandler				( cMouseHandler			*mouseH							);
    void 	setKeyboardHandler			( cKeyboardHandler		*keyboardH						);
    void 	setMessageHandler			( cMessageHandler		*messageH						);

    bool	sendMoreFrames				(	) 				{return needMoreFrames; };
    bool 	saveFrame					( 	)				{return m_saveFrame; };
    void	save						( unsigned char *img);
    void	printStats					( );

private:

    void	parse 					( int type, std::stringstream *value 	);
    void	scale 					( unsigned char *in, unsigned char *out, float factor );
    void	sendJPEGFrame 			( unsigned char *rgb ); // img must be RGB 8 bits per channel
    void	sendNvPipeFrame 		( unsigned char *rgba ); // img must be RGBA 8 bits per channel
    void	sendNvPipeFrame 		(void *rgbaDevice ); //
    bool				needMoreFrames, stop, m_saveFrame;
    typedef	std::set<connection_hdl,std::owner_less<connection_hdl>> con_list;
    server 				m_server;
    con_list 			m_connections;
    std::stringstream 	string;

    cMouseHandler		*mouseHandler;
    cKeyboardHandler	*keyboardHandler;
    cMessageHandler		*messageHandler;

    // PNG Encoder
	cPNGEncoder								*pngEncoder;
	cTimer									m_netStatsTimer, m_statsTimer, m_sendTimer, m_encStatsTimer;
	AverageStats							m_netStats; // reports round-trip latency encode -> send (server) -> receive (client) -> decoding (client) -> send Next_frame msg (client) -> receive Next_frame msg (server)
	AverageStats							m_encStats; // reports encoder latency
	AverageStats							m_sendStats; // reports send latency



#ifdef NVPIPE_ENCODING
	cNvPipeEncoderWrapper					*m_nvpipe;
	bool									m_clientClosed;
#endif

#ifdef JPEG_ENCODING
    // Adjust quality of the JPEG according to the
    // image transport throughput
    void adjustJpegQuality			( 	);
	// JPEG m_encoder
	cTurboJpegEncoder						*jpegEncoder;
	// JPEG encoding quality
	unsigned int							jpegQuality;
	// Specify the desired time response for image transport in milliseconds
	float 									targetTime;
	//streaming timers used to calculate image transport throughput
	high_resolution_clock::time_point		stTimer1, stTimer2;
	// image transport duration in microiseconds
	std::chrono::microseconds				stDuration;
#endif
#ifdef SAVE_IMG
    void	scale				( unsigned char *in, unsigned char *out, float factor );
#endif
};
