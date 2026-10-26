#include "StdAfx.h"
#include <jpeghelper/JpegDecoder.h>


extern "C"
{
#include <jpeg/jpeglib.h>
}
#include <setjmp.h>

#pragma warning (disable:4065) // switch statement contains 'default' but no 'case' labels

namespace JpegHelper
{

static jmp_buf JumpBuf1;
static void JpegError1(j_common_ptr)
{
	longjmp(JumpBuf1,1);
}

static inline S32 CalcAddr(S32 x, S32 y, S32 pitch)
{
	return y * pitch + (x << 2);
}

C_JpegDecoder::C_JpegDecoder()
{
	m_Data = 0;
	m_DataLen = 0;
}

bool C_JpegDecoder::DecodeIntoBuffer(U8* image, S32 pitch, S32 destWidth, S32 destHeight, S32 format)
{
	jpeg_decompress_struct cinfo;
	memset(&cinfo, 0, sizeof(cinfo));
	cinfo.client_data = this;

	if (setjmp(JumpBuf1))
	{
		// We come back here if we got an error in any of the Jpeg functions
		jpeg_destroy_decompress(&cinfo);
		return false;
	}

	jpeg_error_mgr jerr;
	memset(&jerr, 0, sizeof(jerr));
	cinfo.err = jpeg_std_error(&jerr);
	cinfo.err->error_exit = JpegError1;

	jpeg_create_decompress(&cinfo);
	
	jpeg_source_mgr sourceManager =
	{
		m_Data,
		m_DataLen,
		C_JpegDecoder::InitSource,
		C_JpegDecoder::FillInputBuffer,
		C_JpegDecoder::SkipInputData,
		0,
		C_JpegDecoder::TermSource
	};
	cinfo.src = &sourceManager;

	jpeg_set_marker_processor(&cinfo, JPEG_COM, C_JpegDecoder::ReadComments);
	jpeg_read_header(&cinfo, 1);

	cinfo.out_color_space = JCS_RGB; // Decompress as RGB
	jpeg_start_decompress(&cinfo);

	S32 rowLen = cinfo.image_width * 3;
	U8* row = new U8[rowLen];
	if (!row)
	{
		jpeg_destroy_decompress(&cinfo);
		return false;
	}
	memset(row, 0, rowLen);
	
	S32 width = cinfo.image_width;
	if (width > destWidth)
		width = destWidth;

	for (S32 y = 0; (unsigned)y < cinfo.image_height; y++)
	{
		jpeg_read_scanlines(&cinfo, &row, 1);
		if (y >= destHeight)
			continue;

		U8* ps = row;
		
		switch (format)
		{
			default:
				for (S32 x = 0; x < width; x++, ps += 3)
				{
					U8* pd = image + CalcAddr(x, y, pitch);
					
					pd[0] = 0xff;  // Alpha
					pd[1] = ps[0]; // Red
					pd[2] = ps[1]; // Green
					pd[3] = ps[2]; // Blue
				}
			break;
			
#ifdef PLATFORM_PC
			case D3DFMT_A8R8G8B8:
			case D3DFMT_X8R8G8B8:
				for (S32 x = 0; x < width; x++, ps += 3)
				{
					U8* pd = image + CalcAddr(x, y, pitch);
					*(U32 *)pd = 0xff000000 | (ps[0] << 16) | (ps[1] << 8) | ps[2];
				}
			break;
#endif
		}
	}

	delete[] row;

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	
	return true;	
}

const U8* C_JpegDecoder::Comments()
{
	if (m_Comments.Size() > 0)
		return &m_Comments[0];
	else
		return 0;
}

S32 C_JpegDecoder::CommentsSize()
{
	return m_Comments.Size();
}

void C_JpegDecoder::Exit()
{
}

// Jpeg handlers for inputting data
void C_JpegDecoder::InitSource(jpeg_decompress_struct*)
{
}

unsigned char C_JpegDecoder::FillInputBuffer(jpeg_decompress_struct*)
{
	return 0;
}

void C_JpegDecoder::SkipInputData(jpeg_decompress_struct* cinfo, long numBytes)
{
	cinfo->src->next_input_byte += numBytes;
	cinfo->src->bytes_in_buffer -= numBytes;
}

void C_JpegDecoder::TermSource(jpeg_decompress_struct* cinfo)
{
}

boolean C_JpegDecoder::ReadComments(jpeg_decompress_struct* cinfo)
{
	C_JpegDecoder* self = (C_JpegDecoder*)cinfo->client_data;
	if (!self)
		return false;
	
	S32 len = ReadByte(cinfo) << 8;
	len |= ReadByte(cinfo);
	if (len >= 2)
		len -= 2;
	
	self->m_Comments.Resize(len);
	for (S32 i = 0; i < len; i++)
	{
		self->m_Comments[i] = ReadByte(cinfo);
	}
	
	return true;
}

U8 C_JpegDecoder::ReadByte(jpeg_decompress_struct* cinfo)
{
	jpeg_source_mgr* datasrc = cinfo->src;
	if (datasrc->bytes_in_buffer == 0)
		(*datasrc->fill_input_buffer)(cinfo);

	datasrc->bytes_in_buffer--;
	
	U8 byte = *datasrc->next_input_byte++;
	return byte;
}

} // End of namespace JpegHelper

