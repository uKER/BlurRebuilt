#ifndef JPEGHELPER_JPEGENCODER_H
#define JPEGHELPER_JPEGENCODER_H

#include <btl/containers/vector.h>

struct jpeg_compress_struct;

namespace JpegHelper
{

class C_JpegEncoder
{
public:
	C_JpegEncoder();
	~C_JpegEncoder() { Exit(); }
	void SetQuality(S32 quality) { m_Quality = quality; }
	void SetComments(const void* data, S32 dataLen);
	bool EncodeFromBuffer(const U8* image, S32 pitch, S32 width, S32 height, S32 format);
	U8* JpegData();
	S32 JpegLength();
	void Exit();
	
private:
	Btl::Vector<U8> m_JpegData;
	static const S32 m_SectSize = 256;
	S32 m_Quality; // Jpeg quality 1-100
	Btl::Vector<U8> m_Comments;

	static void InitDestination(jpeg_compress_struct* cinfo);
	static unsigned char EmptyOutputBuffer(jpeg_compress_struct* cinfo);
	static void TermDestination(jpeg_compress_struct* cinfo);
};

} // End of namespace JpegHelper

#endif // JPEGHELPER_JPEGENCODER_H
