#ifndef CS_GEN_IMAGE_HEADER_H
#define CS_GEN_IMAGE_HEADER_H


namespace TexAtlas
{

class ImageHeader
{
public:
	ImageHeader();
	ImageHeader(const ImageHeader& rhs);
	~ImageHeader();

	const ImageHeader& operator=(const ImageHeader& rhs);

	U32 Width() const;
	U32 Height() const;

	///
	///@Reads image header information from the given file.
	Attk::C_Status Read(const char* pFileName);

private:
	U32 m_width;
	U32 m_height;

	friend class TGAImageHeaderBuilder;
	friend class BMPImageHeaderBuilder;
};

}

#endif