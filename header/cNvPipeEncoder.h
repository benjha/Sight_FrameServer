/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 */
#ifndef CNVPIPEENCODER_H_
#define CNVPIPEENCODER_H_

#include "NvPipe.h"
extern "C" {
	#include "libavformat/avformat.h"
	#include "libavutil/mathematics.h"
}
/*
 *
 * h264 and hENVC GPU encoding in a MP4 wrapper
 *
 */
class cNvPipeEncoderWrapper
{
public:
	cNvPipeEncoderWrapper ( )
	{
		m_compressedImg = 0;
		m_encoder = 0;
		m_width = 0;
		m_height = 0;
		m_bitrateMbps = 0;
		m_targetFps = 0;
		m_frame = 1;
		//TODO: For now Broadway.js allows decoding H264 frames
		// libav mpeg4 wrapper stuff
		/*
		m_formatCtx = 0;
		m_outFormat = 0;
		m_videoStream = 0;
		m_codec = 0;
		m_codecCtx = 0;
		m_AVframe = 0;
		*/
	};

	~cNvPipeEncoderWrapper ()
	{
		if (m_encoder)
			NvPipe_Destroy(m_encoder);
		delete [] m_compressedImg;

		// TODO: For now Broadway.js allows decoding H264 frames
		/*
        avformat_free_context(m_formatCtx);
        if (m_codecCtx)
        	avcodec_free_context(&m_codecCtx);

        if (m_AVframe)
        	av_frame_free (&m_AVframe);
        	*/
	};

	bool reset ()
	{
		if (m_encoder)
		{
			NvPipe_Destroy(m_encoder);
			m_encoder = NvPipe_CreateEncoder (NVPIPE_RGBA32, NVPIPE_H264, NVPIPE_LOSSY, m_bitrateMbps * 1000 * 1000, m_targetFps);
			if (!m_encoder)
			{
				// TODO: design proper error messages
				return false;
			}
		}
		else
		{
			return false;
		}
		return true;
	}

	/*
	 * Uses NvPipe Encoder and libAV MP4 wrapper
	 */
	bool initNvPipe (unsigned int w, unsigned int h, unsigned int bitrateMbps=10, unsigned int targetFps=30 )
	{
		m_width = w;
		m_height = h;
		m_bitrateMbps = bitrateMbps;
		m_targetFps = targetFps;
		m_encoder = NvPipe_CreateEncoder (NVPIPE_RGBA32, NVPIPE_H264, NVPIPE_LOSSY, bitrateMbps * 1000 * 1000, targetFps);

		m_compressedImg = new unsigned char[w*h*4];
		if (!m_encoder)
		{
			return false;
		}
		// TODO: For now Broadway.js allows decoding H264 frames
		// Initalize libav to enable MP4 wrapper
		/*
		const char *filename = "file.mp4";
		av_register_all();
		m_formatCtx = avformat_alloc_context();
		if (!m_formatCtx)
		{
			return false;
		}
		// Set mpeg4 wrapper for output
		m_outFormat = av_guess_format(0, filename, 0);
	    if (!m_outFormat) {
	        return false;
	    }
	    // Set H264 encoder for output
	    m_outFormat->video_codec = AV_CODEC_ID_H264;
	    m_formatCtx->oformat = m_outFormat;
	    strcpy(m_formatCtx->filename, filename);

	    m_videoStream = avformat_new_stream(m_formatCtx, 0);
	    if (!m_videoStream)
	    {
	    	return false;
	    }
	    m_videoStream->id = 0;
	    m_videoStream->time_base = (AVRational){1, (int)targetFps};
	    m_videoStream->sample_aspect_ratio = (AVRational){1, 1};

	    //m_videoStream->avg_frame_rate;

	    AVCodecParameters *vidCodecParams = m_videoStream->codecpar;
	    vidCodecParams->codec_type = AVMEDIA_TYPE_VIDEO;
	    vidCodecParams->codec_id = AV_CODEC_ID_H264;
	    vidCodecParams->bit_rate = bitrateMbps * 1000 * 1000;
	    vidCodecParams->width = m_width;
	    vidCodecParams->height = m_height;
	    vidCodecParams->format = AV_PIX_FMT_NV12; // likely this?
	    vidCodecParams->sample_aspect_ratio = (AVRational){1, 1};

	    //vidCodecParams->field_order;
	    //  Codec-specific bitstream restrictions that the stream conforms to.
	    //vidCodecParams->profile;
	    //vidCodecParams->level;

	    av_dump_format (m_formatCtx, 0, filename,1);

	    if (!(m_formatCtx->oformat->flags & AVFMT_NOFILE))
	    	if (avio_open(&m_formatCtx->pb, filename, AVIO_FLAG_WRITE) < 0)
	    	{
	            return false;
	        }

	    avformat_write_header(m_formatCtx, 0);
	    */
		return true;
	};

	/*
	 * For now Broadway.js allows decoding H264 frames
	 * TODO: Uses libAV encoder and libAV MP4 wrapper
	 *
	 */
	/*
	bool initAV (unsigned int w, unsigned int h, unsigned int bitrateMbps=10, unsigned int targetFps=30 )
	{
		m_width = w;
		m_height = h;
		m_bitrateMbps = bitrateMbps;
		m_targetFps = targetFps;

		AVIOContext*        output_io_context = NULL;

		AVCodec*            codec = NULL;
		AVFrame*            frame = NULL;

		av_register_all();
		avcodec_register_all();

		char output_filename[] = "filename.mp4";


		avio_open(&output_io_context, output_filename,
				AVIO_FLAG_WRITE);

		if (!output_io_context)
		{
			return false;
		}

		m_formatCtx = avformat_alloc_context();
		if (!m_formatCtx)
		{
			return false;
		}
		m_formatCtx->pb = output_io_context;
		m_formatCtx->oformat = av_guess_format("mp4", 0, 0);
		strcpy(m_formatCtx->filename, output_filename);

		codec = avcodec_find_encoder(AV_CODEC_ID_H264);
		m_codecCtx = avcodec_alloc_context3(codec);
		m_codecCtx->bit_rate = m_bitrateMbps * 1000 * 1000;
		m_codecCtx->width = m_width;
		m_codecCtx->height = m_height;
		m_codecCtx->time_base = (AVRational){1, (int)targetFps};
		m_codecCtx->framerate = (AVRational){(int)targetFps, 1};
		m_codecCtx->gop_size = 10; // emit one intra frame every 10 frames
		m_codecCtx->max_b_frames = 1;
		m_codecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
		m_codecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

		if (avcodec_open2(m_codecCtx, codec, 0)<0)
		{
			return false;
		}


		m_videoStream = avformat_new_stream(m_formatCtx, NULL);
		m_videoStream->id = 0;
		m_videoStream->time_base = (AVRational){1, (int)targetFps};
		m_videoStream->sample_aspect_ratio = (AVRational){1, 1};
		avcodec_parameters_from_context(m_videoStream->codecpar,m_codecCtx);
		m_videoStream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
		m_videoStream->codecpar->codec_id = AV_CODEC_ID_H264;
		m_videoStream->codecpar->sample_aspect_ratio = (AVRational){1, 1};

	    m_AVframe = av_frame_alloc();
	    if (!m_AVframe)
	    {
	    	return false;
	    }
	    m_AVframe->width = m_width;
	    m_AVframe->height = m_height;
	    m_AVframe->format = m_codecCtx->pix_fmt;
	    m_AVframe->key_frame = 1;   // first frame
	    m_AVframe->sample_aspect_ratio = (AVRational){1, 1};
	    m_AVframe->quality = 1;

		av_dump_format (m_formatCtx, 0, output_filename,1);

	    avformat_write_header(m_formatCtx, 0);


	    return true;
	}
	*/

	bool encodeAndWrapNvPipe (void *rgba)
	{
    	//std::cout << "encodeAndWrap \n";

		if (m_width <= 0 || m_height <= 0 )
		{
			std::cerr << "cNvPipeEncode@encodeAndWrap: Invalid image size or image parameters not set\n";
			return false;
		}

		// first frame produce SPS (0 0 0 1 103) -> 29 characters
		// PPS (0 0 0 1 104)  - 8 characters
		// I frame (0 0 0 1 41) - ?
		// Next frames are P frames (0 0 0 1 97)
		//
		m_compressedSize = NvPipe_Encode(m_encoder, rgba, m_width * 4, m_compressedImg, m_width*m_height*4, m_width, m_height);
		if (m_compressedSize == 0 )
		{
	        std::cerr << "cNvPipeEncode@encodeAndWrap: Encoding error: " << NvPipe_GetError(m_encoder) << std::endl;
		}

		/*
		if (m_frame==1)
		{
			for (int i=0;i<m_compressedSize/1000; i++)
			{
				std::cout << m_compressedImg[i] << " ";
			}
		}
		*/
		m_frame++;
		// TODO: For now Broadway.js allows decoding H264 frames
		//std::cout << "cNvPipeEncoderWrapper: NvPipe compressed size " << size << std::endl;
		/*
		if (m_frame<100)
		{
			std::cout << "Frame " << m_frame << " saved\n";
			AVPacket pkt = {0};
			av_init_packet(&pkt);
			pkt.pts = av_rescale_q(m_frame++, m_videoStream->time_base, m_videoStream->time_base);
			pkt.dts = pkt.pts;
			pkt.stream_index = m_videoStream->index;
			pkt.data = m_compressedImg;
			pkt.size = m_compressedSize;

			// how to get NVVE_PIC_TYPE_IFRAME
			//if (m_frame == 1)
			pkt.flags = AV_PKT_FLAG_KEY;

			av_write_frame(m_formatCtx, &pkt);

			av_free_packet(&pkt);
		}
        if (m_frame==50)
        {
        	av_write_trailer(m_formatCtx);
        	avio_close(m_formatCtx->pb); // close the output file
        	std::cout << "avio_close \n";
        	m_frame++;
        }
		*/
		return true;
	};

	/*
	 * TODO: For now Broadway.js allows decoding H264 frames
	 */
	/*
	bool encodeAndWraplibAV (unsigned char *rgba)
	{
		m_AVframe->data[0] = rgba;
		avcodec_send_frame(m_codecCtx,m_AVframe);
	}
	*/

	unsigned char *getImg 		(	)
	{
		return  m_compressedImg;
	}

	int	getSize 			(	) 	{ return m_compressedSize; }


private:
	int					m_width, m_height;
	int					m_bitrateMbps, m_targetFps;
	uint64_t 			m_compressedSize, m_frame;
	unsigned char 		*m_compressedImg;
	NvPipe				*m_encoder;

	/* TODO: For now Broadway.js allows decoding H264 frames
	AVFormatContext 	*m_formatCtx;
	AVOutputFormat		*m_outFormat;
	AVStream			*m_videoStream;
	AVCodec				*m_codec;
	AVCodecContext		*m_codecCtx;
	AVFrame				*m_AVframe;
	*/



};


#endif /* CNVPIPEENCODER_H_ */
