/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 */
#include <sstream>
#include <time.h>

#include <cBroadcastServer.h>
#include <cMouseEventHandler.h>
#include <cKeyboardHandler.h>
#include <cMessageHandler.h>

#include "cPNGEncoder.h"

#ifdef JPEG_ENCODING
#include "cTurboJpegEncoder.h"
#endif

#ifdef NVPIPE_ENCODING
#include "cNvPipeEncoder.h"
#endif

unsigned char webSocketKey;
//extern bool		keyChangedFlag;

broadcast_server::broadcast_server() {
	// Set logging settings
	m_server.set_error_channels(websocketpp::log::elevel::all);
	m_server.set_access_channels(
			websocketpp::log::alevel::all
					^ websocketpp::log::alevel::frame_payload);

	m_server.init_asio();

	// set up access channels to only log interesting things
	// enable these if problems occur
	//m_server.clear_access_channels(websocketpp::log::alevel::none);
	m_server.clear_access_channels(websocketpp::log::alevel::all);
	//m_server.set_access_channels(websocketpp::log::alevel::connect);
	//m_server.set_access_channels(websocketpp::log::alevel::disconnect);
	//m_server.set_access_channels(websocketpp::log::alevel::app);

	m_server.set_reuse_addr(true);
	m_server.set_open_handler(bind(&broadcast_server::on_open, this, ::_1));
	m_server.set_close_handler(bind(&broadcast_server::on_close, this, ::_1));
	m_server.set_message_handler(
			bind(&broadcast_server::on_message, this, ::_1, ::_2));

	needMoreFrames 	= false;
	saveFrame		= false;
	stop 			= true;

	mouseHandler = 0;
	keyboardHandler = 0;
	messageHandler = 0;

#ifdef	JPEG_ENCODING
	targetTime = TIME_RESPONSE;
	jpegQuality = TJPEG_QUALITY;
	stTimer1 = high_resolution_clock::now();
	stTimer2 = stTimer1;
	jpegEncoder = new cTurboJpegEncoder();
	jpegEncoder->setEncoderParams(jpegQuality);
	jpegEncoder->setImageParams(IMAGE_WIDTH*RESOLUTION_FACTOR, IMAGE_HEIGHT*RESOLUTION_FACTOR, 4);
	if (!(jpegEncoder->initEncoder())) {
		std::cout << "Sight@Frameserver. Warning: JPEG Encoder failed at initialization \n";
	}
	std::cout << "Sight@Frameserver: JPEG Encoder initialized\n";
#endif

#ifdef NVPIPE_ENCODING
	m_nvpipe = new cNvPipeEncoderWrapper ( );

	 if (!(m_nvpipe->initNvPipe(IMAGE_WIDTH, IMAGE_HEIGHT, MBPS, TARGET_FPS)))
	 //if (!(m_nvpipe->initAV(IMAGE_WIDTH, IMAGE_HEIGHT, MBPS, TARGET_FPS)))
	 {
		 std::cout << "Sight@Frameserver: Failed to create GPU encoder: " << NvPipe_GetError(NULL) << std::endl;
	 }
	 std::cout << "Sight@Frameserver: GPU Encoder & Wrapper initialized\n";
#endif

	pngEncoder = new cPNGEncoder ();
	pngEncoder->setImageParams(IMAGE_WIDTH, IMAGE_HEIGHT);
	if (!(pngEncoder->initEncoder()))
	{
		std::cout << "Warning: PNG Encoder failed at initialization \n";
	}
	std::cout << "Sight@Frameserver: PNG Encoder initialized\n";
}
//
//=======================================================================================
//
broadcast_server::~broadcast_server() {
	// TODO: stop the server
#ifdef JPEG_ENCODING
	delete jpegEncoder;
	jpegEncoder = 0;
#endif

#ifdef NVPIPE_ENCODING
	delete m_nvpipe;
	m_nvpipe = 0;
#endif

	delete pngEncoder;
	pngEncoder = 0;
}

void broadcast_server::on_open(connection_hdl hdl)
{
	std::cout << "Sight@Frameserver: Web browser opened.\n";
	m_connections.insert(hdl);

#ifdef NVPIPE_ENCODING
	// Need to reset GPU encoder for new connection
	// so new SPS and PPS H264 frames are sent to the client
	if (m_clientClosed)
	{
		if (!m_nvpipe->reset())
		{
			std::cout << "Sight@Frameserver: GPU encoder reset failed \n";
		}
		m_clientClosed = false;
	}
#endif

}
//
//=======================================================================================
//
void broadcast_server::on_close(connection_hdl hdl)
{
	// TODO: these should be in messageHandler
	needMoreFrames = false;
	stop = true;
	// END TODO
	std::cout << "Sight@Frameserver: Web browser closed\n";

	m_connections.erase(hdl);
#ifdef NVPIPE_ENCODING
	m_clientClosed = true;
#endif
}
/*
 * on_message is the entry point to access data sent by the HTML Viewer
 *
 */
void broadcast_server::on_message(connection_hdl hdl, server::message_ptr msg) {
	// TODO: Process Interaction msgs
	std::stringstream val;
	int type;
	//int buttonMask;
	//int xPosition;
	//int yPosition;

	for (auto it : m_connections) {
		try {
			val << msg->get_payload();

			if (!stop) {
				type = val.str().data()[0];
				// check what kind of message received.
				parse(type, &val);
			}
			// TODO: put the next code in  MESSAGE_EVENT/messageHandler
			// These strings come from HTML viewer
			if (val.str().compare("NXTFR") == 0
					|| val.str().compare("STVIS") == 0) {
#ifdef	JPEG_ENCODING
				stTimer2 = high_resolution_clock::now();
//				adjustJpegQuality();
#endif
				needMoreFrames = true;
				stop = false;
				//std::cout << "NXTFR" << std::endl;
			}
			if (val.str().compare("SAVE ") == 0)
			{
				saveFrame = true;
			}
			if (val.str().compare("END  ") == 0)
			{
				stop = true;
				needMoreFrames = false;
			}
			// END TODO
		} catch (const websocketpp::lib::error_code& e) {
			std::cout << "Sight@Frameserver: SEND failed because: " << e << "(" << e.message()
					<< ")" << std::endl;
		}
	}

}
//
//=======================================================================================
//
void broadcast_server::stop_listening( )
{
	try
	{
		m_server.stop_listening( );
	} catch (const std::exception & e) {
		std::cout << e.what() << std::endl;
	} catch (websocketpp::lib::error_code e) {
		std::cout << e.message() << std::endl;
	} catch (...) {
		std::cout << "other exception" << std::endl;
	}
/*
	for (auto it : m_connections)
	{
		try
		{
			m_server.close(it, websocketpp::close::status::normal, "");
		}
		catch(websocketpp::lib::error_code ec)
		{
			std::cout<<"lib::error_code "<<ec<<std::endl;
		}
		m_connections.erase(it);
	}
	*/
}
//
//=======================================================================================
//
void broadcast_server::parse(int type, std::stringstream *value) {
	switch (type) {
	case MOUSE_EVENT:
		if (mouseHandler) {
			mouseHandler->parse(value);
		} else {
			std::cout << "Sight@Frameserver: Is Mouse handler initialized? \n";
		}
		break;
	case KEY_EVENT:
		if (keyboardHandler) {
			keyboardHandler->parse(value);
		} else {
			std::cout << "Sight@Frameserver: Is keyboard handler initialized? \n";
		}
		break;
	case MESSAGE_EVENT:
		if (messageHandler) {
			messageHandler->parse(value);
			//needMoreFrames	= messageHandler->getFrameFlag 	(	);
			//stop			= messageHandler->stopStreaming (	);
		} else {
			std::cout << "Sight@Frameserver: Is Message handler initialized? \n";
		}
		break;

	}
}
//
//=======================================================================================
//
void broadcast_server::run(uint16_t port)
{
	m_server.listen(port);
	m_server.start_accept();
	// Start the ASIO io_service run loop
	try {
		m_server.run();
	} catch (const std::exception & e) {
		std::cout << e.what() << std::endl;
	} catch (websocketpp::lib::error_code e) {
		std::cout << e.message() << std::endl;
	} catch (...) {
		std::cout << "other exception" << std::endl;
	}
}
//
//=======================================================================================
//
void broadcast_server::sendFrame(float *img) {
	//setFrame (img);
	con_list::iterator it;
	for (it = m_connections.begin(); it != m_connections.end(); it++) {
		try {
			// when img is unsigned char
			//m_server.send(it, m_img, (size_t)width*height*3 , websocketpp::frame::opcode::BINARY);
			// when img is float
			m_server.send(*it, img,
					(size_t) IMAGE_WIDTH * IMAGE_HEIGHT * 3 * sizeof(float),
					websocketpp::frame::opcode::BINARY);

			needMoreFrames = false;
		} catch (const websocketpp::lib::error_code& e) {
			std::cout << "Sight@Frameserver: SEND failed because: " << e << "(" << e.message()
					<< ")" << std::endl;
		}
	}
}
//
//=======================================================================================
//
#ifdef JPEG_ENCODING
void broadcast_server::sendJPEGFrame (unsigned char *rgb)
{
#ifdef TIME_METRICS
	high_resolution_clock::time_point t1 = high_resolution_clock::now();
#endif

#ifdef CHANGE_RESOLUTION
	if (!jpegEncoder->encode(halfImg))
#else
	if (!jpegEncoder->encode(rgb))
#endif
	{
			std::cout << "Sight@Frameserver: Encoding error \n";
	}
	//std::cout << "Sight@Frameserver: jpegEncoder compressed size " << jpegEncoder->getJpegSize() << std::endl;
#ifdef TIME_METRICS
	high_resolution_clock::time_point t2 = high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
	std::cout << "JPEG compression: " << duration/1000 << std::endl;
#endif
	stTimer1 = high_resolution_clock::now();
	for (auto it : m_connections) {
		try {

			m_server.send(it, jpegEncoder->compressedImg,
					(size_t) jpegEncoder->getJpegSize(),
					websocketpp::frame::opcode::BINARY);

			/*
			m_server.send(it, std::string (reinterpret_cast<char*>(jpegEncoder->compressedImg)),
					(size_t) jpegEncoder->getJpegSize(),
					websocketpp::frame::opcode::BINARY);
					*/
			needMoreFrames = false;
		} catch (const websocketpp::lib::error_code& e) {
			std::cout << "Sight@Frameserver: SEND failed because: " << e << "(" << e.message()
					<< ")" << std::endl;
		}
	}
}
#endif
//
//=======================================================================================
//
void broadcast_server::sendFrame(unsigned char *img)
{
#ifdef CHANGE_RESOLUTION
	static unsigned int half_w = IMAGE_WIDTH*RESOLUTION_FACTOR;
	static unsigned int half_h = IMAGE_HEIGHT*RESOLUTION_FACTOR;
	unsigned char *halfImg = new unsigned char [half_w * half_h * 3 ];
	scale ( img, halfImg, RESOLUTION_FACTOR );
#endif

	if (saveFrame)
	{
		char date[128];
		time_t rawtime;
		struct tm * timeinfo;
		time(&rawtime);
		timeinfo = localtime (&rawtime);
		strftime (date,sizeof(date),"%Y-%m-%d_%OH_%OM_%OS",timeinfo);
		std::stringstream filename;
		filename << "Sight_" << date << ".png";
		pngEncoder->savePNG(filename.str(), img);
		std::cout << "Sight@Frameserver: " << filename.str().data() << " saved!\n";
		saveFrame = false;
	}

#ifdef JPEG_ENCODING
	sendJPEGFrame	(img);
#endif
#ifdef NVPIPE_ENCODING
	sendNvPipeFrame (img);
#endif
#ifdef FULLHD
	con_list::iterator it;
	for (it = m_connections.begin(); it != m_connections.end(); it++)
	{
		try
		{
#ifdef CHANGE_RESOLUTION
			m_server.send(*it, halfImg, (size_t)((IMAGE_WIDTH/2)*(IMAGE_HEIGHT/2)*3) , websocketpp::frame::opcode::BINARY);
#else
			// when img is unsigned char
			m_server.send(*it, img, (size_t)IMAGE_WIDTH*IMAGE_HEIGHT*3 , websocketpp::frame::opcode::BINARY);
			//m_server.send(it, "END  ", websocketpp::frame::opcode::text);
#endif
			needMoreFrames = false;
		}
		catch (const websocketpp::lib::error_code& e)
		{
			std::cout << "SEND failed because: " << e
			<< "(" << e.message() << ")" << std::endl;
		}
	}
#endif

#ifdef CHANGE_RESOLUTION
	delete [] halfImg;
#endif
}

#ifdef NVPIPE_ENCODING
void broadcast_server::sendNvPipeFrame (unsigned char *rgba)
{
	if (!m_nvpipe->encodeAndWrapNvPipe(rgba))
	{
		std::cout << "Sight@Frameserver: Encoding error \n";
	}
	//std::cout << "Sight@Frameserver: NvPipe compressed size " << m_nvpipe->getSize() << std::endl;
	for (auto it : m_connections)
	{
		try
		{
			m_server.send(it, m_nvpipe->getImg(),
					(size_t) m_nvpipe->getSize(),
					websocketpp::frame::opcode::BINARY);
			needMoreFrames = false;
		}
		catch (const websocketpp::lib::error_code& e)
		{
				std::cout << "Sight@Frameserver: SEND failed because: " << e << "(" << e.message()
						<< ")" << std::endl;
		}
	}
}
//
//=======================================================================================
//
void broadcast_server::sendNvPipeFrame (void *rgbaDevice)
{
	if (!m_nvpipe->encodeAndWrapNvPipe(rgbaDevice))
	{
		std::cout << "Sight@Frameserver: Encoding error \n";
	}
	//std::cout << "Sight@Frameserver: NvPipe compressed size " << m_nvpipe->getSize() << std::endl;
	for (auto it : m_connections)
	{
		try
		{
			m_server.send(it, m_nvpipe->getImg(),
					(size_t) m_nvpipe->getSize(),
					websocketpp::frame::opcode::BINARY);
			needMoreFrames = false;
		}
		catch (const websocketpp::lib::error_code& e)
		{
				std::cout << "Sight@Frameserver: SEND failed because: " << e << "(" << e.message()
						<< ")" << std::endl;
		}
	}
}
//
//=======================================================================================
//
void broadcast_server::sendFrame(unsigned char *img, void *gpuFrameBufferPtr)
{
#ifdef CHANGE_RESOLUTION
	static unsigned int half_w = IMAGE_WIDTH*RESOLUTION_FACTOR;
	static unsigned int half_h = IMAGE_HEIGHT*RESOLUTION_FACTOR;
	unsigned char *halfImg = new unsigned char [half_w * half_h * 3 ];
	scale ( img, halfImg, RESOLUTION_FACTOR );
#endif

	if (saveFrame)
	{
		char date[128];
		time_t rawtime;
		struct tm * timeinfo;
		time(&rawtime);
		timeinfo = localtime (&rawtime);
		strftime (date,sizeof(date),"%Y-%m-%d_%OH_%OM_%OS",timeinfo);
		std::stringstream filename;
		filename << "Sight_" << date << ".png";
		pngEncoder->savePNG(filename.str(), img);
		std::cout << "Sight@Frameserver: " << filename.str().data() << " saved!\n";
		saveFrame = false;
	}
	if (gpuFrameBufferPtr)
	{
		sendNvPipeFrame (gpuFrameBufferPtr);
	}
	else
	{
		std::cout << "Optix Frame Buffer not shared with CUDA for encoding\n";
		return;
	}

#ifdef CHANGE_RESOLUTION
	delete [] halfImg;
#endif
}
#endif
//
//=======================================================================================
//
void broadcast_server::setMouseHandler(cMouseHandler *mouseH) {
	mouseHandler = mouseH;
}
//
//=======================================================================================
//
void broadcast_server::setKeyboardHandler(cKeyboardHandler *keyboardH) {
	keyboardHandler = keyboardH;
}
//
//=======================================================================================
//
void broadcast_server::setMessageHandler(cMessageHandler *messageH) {
	messageHandler = messageH;
}
//
//=======================================================================================
//
#ifdef JPEG_ENCODING
void broadcast_server::adjustJpegQuality() {
	stDuration = std::chrono::duration_cast < std::chrono::microseconds
			> (stTimer2 - stTimer1);
	//std::cout << "Image Size: " << jpegEncoder->getJpegSize()/1000.0f << " (KB) Image transport duration: " << stDuration.count()/1000.0f << " (ms) \n";
	//std::cout << "Rate: " << (jpegEncoder->getJpegSize()/1000.0f) / (stDuration.count()/1000000.0f) << " (KBps) \n";

	if ((stDuration.count() / 1000.0f) > targetTime) {
		jpegQuality -= 1;
		if (jpegQuality < 30)
			jpegQuality = 30;
		jpegEncoder->setEncoderParams(jpegQuality);
//		std::cout << std::endl << "jpegQuality: " << jpegQuality << std::endl;
	} else {
		jpegQuality += 1;
		if (jpegQuality > 100)
			jpegQuality = 100;
		jpegEncoder->setEncoderParams(jpegQuality);
//		std::cout << std::endl << "jpegQuality: " << jpegQuality << std::endl;
	}
}
#endif
//
//=======================================================================================
//
#ifdef CHANGE_RESOLUTION
void broadcast_server::scale ( unsigned char *in, unsigned char *out, float factor )
{
	unsigned int half_w = IMAGE_WIDTH*factor;
	unsigned int half_h = IMAGE_HEIGHT*factor;

/*
#pragma omp parallel sections num_threads(2) // starts a new team
	{
#pragma omp section
		for (unsigned int j=0; j<half_h ; j++)
		{
			for (unsigned int i=0;i< half_w/2; i++)
			{
				unsigned int halfIdx	= (half_w * 3) * j + i*3;
				unsigned int idx		= ( j / factor ) * (IMAGE_WIDTH *3) + (i / factor ) * 3;

				out[halfIdx  ] = in[idx  ];
				out[halfIdx+1] = in[idx+1];
				out[halfIdx+2] = in[idx+2];
			}
		}

#pragma omp section
		for (unsigned int j=0; j<half_h ; j++)
		{
			for (unsigned int i=half_h/2;i< half_w; i++)
			{
				unsigned int halfIdx	= (half_w * 3) * j + i*3;
				unsigned int idx		= ( j / factor ) * (IMAGE_WIDTH *3) + (i / factor ) * 3;

				out[halfIdx  ] = in[idx  ];
				out[halfIdx+1] = in[idx+1];
				out[halfIdx+2] = in[idx+2];
			}
		}
	}
	*/
/*
#pragma omp parallel for num_threads (2)
	for (unsigned int i=0; i<half_w * half_h; i++)
	{
		unsigned int x = i%half_w;
		unsigned int y = i/half_w;

		unsigned int halfIdx	= (half_w * 3) * y + x*3;
		unsigned int idx 		= ( y / factor ) * (IMAGE_WIDTH *3) + (x / factor ) * 3;

		out[halfIdx  ] = in[idx  ];
		out[halfIdx+1] = in[idx+1];
		out[halfIdx+2] = in[idx+2];

	}
*/

//#pragma omp parallel for collapse (2)
	for (unsigned int j=0; j<half_h ; j++)
	{
		for (unsigned int i=0;i< half_w; i++)
		{
			unsigned int halfIdx	= (half_w * 3) * j + i*3;
            unsigned int idx		= ( j / factor ) * (IMAGE_WIDTH *3) + (i / factor ) * 3;

            out[halfIdx  ] = in[idx  ];
			out[halfIdx+1] = in[idx+1];
			out[halfIdx+2] = in[idx+2];
		}
	}

}
#endif
//
//=======================================================================================
//
