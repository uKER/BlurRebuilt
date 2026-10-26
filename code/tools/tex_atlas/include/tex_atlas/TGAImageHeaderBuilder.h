#ifndef CS_GEN_TGA_IMAGE_HEADER_BUILDER_H
#define CS_GEN_TGA_IMAGE_HEADER_BUILDER_H

namespace TexAtlas
{
	class TGAImageHeaderBuilder
	{
	public:
		static Attk::C_Status Build(ImageHeader& header, const char* pFileName);
	};
}

#endif