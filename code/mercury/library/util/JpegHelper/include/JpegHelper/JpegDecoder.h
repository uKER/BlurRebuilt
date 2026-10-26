#ifndef JPEGHELPER_JPEGDECODER_H
#define JPEGHELPER_JPEGDECODER_H

#include <btl/containers/vector.h>
#include <Display.h>

struct jpeg_decompress_struct;

namespace JpegHelper
{

class C_JpegDecoder
{
public:
	C_JpegDecoder();
	~C_JpegDecoder() { Exit(); }
	void SetData(U8* data, S32 len) { m_Data = data; m_DataLen = len; }
	bool DecodeIntoBuffer(U8* image, S32 pitch, S32 destWidth, S32 destHeight, S32 format);
	const U8* Comments();
	S32 CommentsSize();
	void Exit();

private:
	U8* m_Data;
	S32 m_DataLen;
	Btl::Vector<U8> m_Comments;

	static void InitSource(jpeg_decompress_struct*);
	static unsigned char FillInputBuffer(jpeg_decompress_struct*);
	static void SkipInputData(jpeg_decompress_struct* cinfo, long numBytes);
	static void TermSource(jpeg_decompress_struct* cinfo);
	static unsigned char ReadComments(jpeg_decompress_struct* cinfo);
	static U8 ReadByte(jpeg_decompress_struct* cinfo);
};

} // End of namespace JpegHelper

#endif // JPEGHELPER_JPEGDECODER_H
