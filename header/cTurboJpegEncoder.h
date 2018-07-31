/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 */
#pragma once

#include <iostream>
#include <turbojpeg.h>

using namespace std;

#define	TJPEG_QUALITY		80
#define TJPEG_COLOR_COMPONENTS	3


class cTurboJpegEncoder
{
public:
				cTurboJpegEncoder 			( )
				{
					jpegSize 		= 0;
					compressedImg 	= 0;
					width			= 0;
					height			= 0;
					compressor		= 0;
					samplingFactor	= TJSAMP_444;
					colorSpace		= TJPF_RGB;
					quality			= TJPEG_QUALITY;
				};

				~cTurboJpegEncoder			(	)
				{
					tjDestroy	(compressor);
					tjFree		(compressedImg);
				}

		void	setEncoderParams		( int quality_ = TJPEG_QUALITY, int restart_interval  = 8, int interleaved = 0)
		{
			quality = quality_;

		};

		void	setImageParams			( int width_, int height_, int color_components = TJPEG_COLOR_COMPONENTS,
										  int color_space 		= TJPF_RGB,
										  int sampling_factor 	= TJSAMP_444 )
		{
			width = width_;
			height = height_;

			colorSpace 		= color_space;
			samplingFactor	= sampling_factor;

		};

		bool	initEncoder ( )
		{
			compressor = tjInitCompress ();
			if (!compressor)
			{
				cout << tjGetErrorStr ();
				return false;
			}
			return true;
		};

		bool encode 				( unsigned char* img )
		{
			//cout << "jpegSize: " << jpegSize << endl;
			if (width <= 0 || height <= 0 )
			{
				std::cerr << "Invalid image size or image parameters not set\n";
				return false;
			}

			if ( tjCompress2 (compressor, img, width, 0, height, colorSpace, &compressedImg, &jpegSize,  samplingFactor, quality, TJFLAG_FASTDCT) != 0 )
			{
				cout << tjGetErrorStr ();
				return false;
			}


			return true;
		};

		unsigned char *getImg 		(	)
		{
			return  compressedImg;
		}

		int	getJpegSize 			(	) 	{ return jpegSize; }

		// Stores the compressed image.
		unsigned char			*compressedImg;


private:

		// Width and height of the compressed image
		int						width, height;
		// Next vars. store compressor configuration
		int						quality;
		// Next vars. store image parameters
		int 					colorSpace;
		int						samplingFactor;
		// Stores image size after compression
		long unsigned int		jpegSize;
		// The jpeg compressor handler
		tjhandle				compressor;

};

