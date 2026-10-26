#pragma once

#ifdef _CONVERTER
#include "compress.h"
typedef U32 CCompression;
#else
#include "system\resource\compression.h"
#endif

template <typename T>
class ZLibCompressStreamObject
{
public:
	const static int VERSION = 0;

	ZLibCompressStreamObject(const T& object, CCompression *pCompress, U8* pOut, U32 bufferSize) 
		:	m_object(const_cast<T&>(object)), 
			m_compress(*pCompress),
			m_pOut(pOut),
			m_outputBufferSize(bufferSize)
	{}

	const T& GetObject() const {return m_object;}
	T& GetObject() {return m_object;}
	U8* GetOutputPointer() {return m_pOut;}
	U32 GetOutputBufferSize() { return m_outputBufferSize; }

	CCompression& GetCompress() { return m_compress; }

private:
	T& m_object;
	CCompression& m_compress;
	U8* m_pOut;
	U32 m_outputBufferSize;
};

template <typename T>
ZLibCompressStreamObject<T> ZLibCompress(T& object, CCompression *pCompress = 0, U8* pOut = 0, U32 bufferSize=0)
{
	return ZLibCompressStreamObject<T>(object, pCompress, pOut, bufferSize);
}

template<typename Stream, typename T>
Stream& operator << (Stream& out, const ZLibCompressStreamObject<T>& data)
{
	// create memory buffer and stream the object
	Ut::BinaryStreamBuffer stream;
	OutputStream<BinaryOutputStreamBuffer<Stream::EndianType> > buffer(stream);
	buffer << data.GetObject();

	// compress the buffer
	CZLib zlib;
	U8 *pCompressedData = 0;
	DWORD dwCompressedSize = 0;

	zlib.Compress((U8*) buffer.Data(), buffer.Size(), &pCompressedData, dwCompressedSize);

	// write the data
	out << ZLibCompressStreamObject<T>::VERSION;
	out << (U32) buffer.Size();
	out << (U32) dwCompressedSize;

	out.WriteData(pCompressedData, dwCompressedSize);

	// clean up
	delete [] pCompressedData;

	return out;
}

template<typename Stream, typename T>
Stream& operator >> (Stream& in, ZLibCompressStreamObject<T>& data)
{
	int version; in >> version; 
	assert(version == 0);

	U32 originalSize; in >> originalSize;
	U32 compressedSize; in >> compressedSize;

	assert((data.GetOutputBufferSize() == 0) || (originalSize <= data.GetOutputBufferSize()));

	//  read in data
	Stream::PointerType compressedData = in.Read(0, compressedSize);

	if (data.GetOutputPointer())
	{
		// just decompress directly into the space provided
		bool success = data.GetCompress().Decompress(&*compressedData, compressedSize, data.GetOutputPointer(), originalSize);

		if (!success)
			assert(0 && "Decompression failed.");
	}
	else
	{
		// decompress and stream
		BinaryStreamBuffer buffer;
		buffer.Initialise(originalSize);

		if (buffer.Capacity() > 0)
		{
			PixEventSimple pix(0xffff0000, "Decompress");
			bool success = data.GetCompress().Decompress(&*compressedData, compressedSize, (U8*) buffer.Data(), originalSize);

			if (!success)
				assert(0 && "Decompression failed.");
		}

		// stream the object
		Ut::InputStream<BinaryInputStreamBuffer<Stream::EndianType> > stream((U8*) buffer.Data(), MTK_GFX | MTK_GFX_CITY);
		stream >> data.GetObject();
	}

	return in;
}