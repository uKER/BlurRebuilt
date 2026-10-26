#include "StdAfx.h"
#include <jpeghelper/JpegEncoder.h>
#include <Display.h>


extern "C"
{
#include <jpeg/jpeglib.h>
}
#include <setjmp.h>

namespace JpegHelper
{

static jmp_buf JumpBuf2;
static void JpegError2(j_common_ptr)
{
	longjmp(JumpBuf2,1);
}

static inline S32 CalcAddr2(S32 x, S32 y, S32 pitch)
{
	return y * pitch + (x << 2);
}

C_JpegEncoder::C_JpegEncoder()
{
	m_Quality = 90;
}

void C_JpegEncoder::SetComments(const void* data, S32 dataLen)
{
	m_Comments.Resize(dataLen);

	U8* comments = &m_Comments[0];

	if (comments)
		memcpy(comments, data, dataLen);
}

#pragma warning (disable:4065) // switch statement contains 'default' but no 'case' labels

bool C_JpegEncoder::EncodeFromBuffer(const U8* image, S32 pitch, S32 width, S32 height, S32 format)
{
	if (!image)
		return false;

	unsigned char* row = new unsigned char[width * 3];
	if (!row)
		return false;

	memset(row, 0, width * 3);

	jpeg_compress_struct cinfo;
	memset(&cinfo, 0, sizeof(cinfo));
	cinfo.client_data = this;

	if (setjmp(JumpBuf2))
	{
		// We come back here if we got an error in any of the Jpeg functions
		jpeg_destroy_compress(&cinfo);
		delete[] row;
		return false;
	}

	jpeg_error_mgr jerr;
	memset(&jerr, 0, sizeof(jerr));
	cinfo.err = jpeg_std_error(&jerr);
	cinfo.err->error_exit = JpegError2;
	jpeg_create_compress(&cinfo);

	static jpeg_destination_mgr destManager =
	{
		0,
		0,
		C_JpegEncoder::InitDestination,
		C_JpegEncoder::EmptyOutputBuffer,
		C_JpegEncoder::TermDestination
	};
	cinfo.dest = &destManager;

	// Set info about the JPEG:
	cinfo.image_width = width;
	cinfo.image_height = height;
	cinfo.input_components = 3;
	cinfo.in_color_space = JCS_RGB;
	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, m_Quality, 1);
	jpeg_start_compress(&cinfo, 1);

	// Write comments field, if we have one:
	if (m_Comments.Size() > 0)
		jpeg_write_marker(&cinfo, JPEG_COM, &m_Comments[0], m_Comments.Size());

	S32 x = 0, y = 0;
	for (y = 0; y < height; y++)
	{
		U8* pd = row;

		switch (format)
		{
			default:
				for (x = 0; x < width; x++, pd += 3)
				{
					const U8* ps = image + CalcAddr2(x, y, pitch);
					pd[0] = ps[1]; // Red
					pd[1] = ps[2]; // Green
					pd[2] = ps[3]; // Blue
				}
			break;
#ifdef PLATFORM_PC
			case D3DFMT_A8R8G8B8:
			case D3DFMT_X8R8G8B8:
				for (x = 0; x < width; x++, pd += 3)
				{
					const U8* ps = image + CalcAddr2(x, y, pitch);
					U32 v = *(U32 *)ps;
					pd[0] = (U8)(v >> 16); // Red
					pd[1] = (U8)(v >> 8); // Green
					pd[2] = (U8) v; // Blue
				}
			break;
#endif
		}
		
		jpeg_write_scanlines(&cinfo, &row, 1);
	}

	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);

	delete[] row;
	return true;	
}

U8* C_JpegEncoder::JpegData()
{
	if (m_JpegData.Size() > 0)
		return &m_JpegData[0];

	return 0;
}

S32 C_JpegEncoder::JpegLength()
{
	return m_JpegData.Size();
}

void C_JpegEncoder::Exit()
{
	m_JpegData.Clear();
	m_Comments.Clear();
}

// Jpeg handlers for outputting data - save into Btl::Vector
void C_JpegEncoder::InitDestination(jpeg_compress_struct* cinfo)
{
	C_JpegEncoder* s = (C_JpegEncoder*)cinfo->client_data;
	if (!s)
		return;
	
	s->m_JpegData.Resize(s->m_SectSize);

	cinfo->dest->next_output_byte = &s->m_JpegData[0];
	cinfo->dest->free_in_buffer = s->m_JpegData.Size();
}

unsigned char C_JpegEncoder::EmptyOutputBuffer(jpeg_compress_struct* cinfo)
{
	C_JpegEncoder* s = (C_JpegEncoder*)cinfo->client_data;
	if (!s)
		return 0;
	
	S32 len = s->m_JpegData.Size();
	s->m_JpegData.Resize(len + s->m_SectSize);
	
	cinfo->dest->next_output_byte = &s->m_JpegData[len];
	cinfo->dest->free_in_buffer = s->m_SectSize;
	return 1;
}

void C_JpegEncoder::TermDestination(jpeg_compress_struct* cinfo)
{
	C_JpegEncoder* s = (C_JpegEncoder*)cinfo->client_data;
	if (!s)
		return;
	
	S32 len = s->m_JpegData.Size();
	S32 left = cinfo->dest->free_in_buffer;
	s->m_JpegData.Resize(len - left);
}

} // End of namespace JpegHelper

