/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 */

#ifndef CPNGENCODER_H_
#define CPNGENCODER_H_

#include <string>
#include <png.h>

class cPNGEncoder
{
public:
			cPNGEncoder ()
			{
				m_width 	= 0;
				m_height 	= 0;

			};

			~cPNGEncoder ()
			{

			};

			bool initEncoder ()
			{


				return true;

			};

			void setImageParams (int  width, int height )
			{
				m_width = width;
				m_height = height;
			}

			bool encode ( std::string filename, unsigned char *img)
			{
				// TODO:
				return true;
			}


			bool savePNG (std::string filename, unsigned char *img)
			{

				FILE *fp = fopen(filename.data(), "wb");
				if (!fp)
					return false;

				m_writeStruct = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);

				if (!m_writeStruct)
					return false;

				m_writeStructInfo = png_create_info_struct(m_writeStruct);
				if (!m_writeStructInfo)
					return false;

				if (setjmp(png_jmpbuf(m_writeStruct)))
					return false;



				png_init_io(m_writeStruct, fp);


				if (setjmp(png_jmpbuf(m_writeStruct)))
					return false;
				/* Set the library compression level.  Currently, valid values range from
				 * 0 - 9, corresponding directly to the zlib compression levels 0 - 9
				 * (0 - no compression, 9 - "maximal" compression).  Note that tests have
				 * shown that zlib compression levels 3-6 usually perform as well as level 9
				 * for PNG images, and do considerably fewer caclulations.  In the future,
				 * these values may not correspond directly to the zlib compression levels.
				 */
				 png_set_compression_level		(m_writeStruct, 3);
				 png_set_compression_strategy	(m_writeStruct, 0);
				 png_set_filter					(m_writeStruct, 0, PNG_FILTER_NONE);

				 png_set_IHDR(m_writeStruct, m_writeStructInfo, m_width, m_height,
							 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
							 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

				png_write_info(m_writeStruct, m_writeStructInfo);


				if (setjmp(png_jmpbuf(m_writeStruct)))
					return false;

				png_bytepp  rows  = (png_bytep*) malloc(sizeof(png_bytep) * m_height);

				for (int i = 0; i < m_height; ++i)
				{
					rows[i] = (png_bytep)(img + (m_height - 1 - i) * (m_width * 3));
				}


				png_write_image(m_writeStruct, rows);


				/* end write */
				if (setjmp(png_jmpbuf(m_writeStruct)))
					return false;

				png_write_end(m_writeStruct, 0);

				png_destroy_write_struct(&m_writeStruct, &m_writeStructInfo);


				free (rows);
				fclose(fp);

				return true;
			}


private:

			// Width and height of the compressed image
			int				m_width, m_height;
			png_structp		m_writeStruct;
			png_infop 		m_writeStructInfo;
};



#endif /* CPNGENCODER_H_ */
