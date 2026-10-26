#ifndef CS_GEN_BMP_IMAGE_HEADER_BUILDER_H
#define CS_GEN_BMP_IMAGE_HEADER_BUILDER_H

namespace TexAtlas
{
	class BMPImageHeaderBuilder
	{
	public:
		static Attk::C_Status Build(ImageHeader& header, const char* pFileName);
	};
}

#endif