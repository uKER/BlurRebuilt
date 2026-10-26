#include "stdafx.h"
#include "Display/Texture/GL/GLTexture2D.h"

namespace NewDisplay
{
namespace GL
{

C_Texture2D::C_Texture2D() : m_Flags(0)
{
}

C_Texture2D::~C_Texture2D()
{
	assert(!IsCreated() && "The gl 2d texture has not been destroyed");
}

void* C_Texture2D::Lock(const U32 level)
{
	return NULL;
}

void C_Texture2D::Unlock(const U32 level)
{
}

bool C_Texture2D::Upload(C_TextureParams* pParams)
{
	assert(pParams);

	switch (pParams->m_Format)
	{
	case E_Format_DXT1:
	case E_Format_DXT3:
	case E_Format_DXT5:
	case E_Format_A8R8G8B8:
		break;
	default:
#ifndef PLATFORM_PS3
		LOGERROR(g_Log, "Unsupported texture 2d format %i\n", pParams->m_Format);
#endif
		return false;
	}

	assert(m_glID != 0);
	glBindTexture(GL_TEXTURE_2D, m_glID);

	U32 width = pParams->m_Width;
	U32 height = pParams->m_Height;
	U32 level = 0;

	//assert(pParams->m_Offset < pParams->m_Size);
	U8* pData = &pParams->m_pData[pParams->m_Offset];

	while (1)
	{
		U32 dataSize = Size(pParams->m_Format, width, height);
		
		if (level >= pParams->m_Levels)
		{
			pData = new U8[dataSize];
			memset(pData, 0, dataSize);
		}

		const GLint glFormat = GLFormat(pParams->m_Format);

		switch (pParams->m_Format)
		{
		case E_Format_DXT1:
		case E_Format_DXT3:
		case E_Format_DXT5:
			glCompressedTexImage2D(GL_TEXTURE_2D, level, glFormat, width, height, 0, dataSize, pData);
			break;
		case E_Format_A8R8G8B8:
			glTexImage2D(GL_TEXTURE_2D, level, glFormat, width, height, 0, glFormat, GL_UNSIGNED_BYTE, pData);
			break;
		default:
			assert(0 && "Unsupported texture format");
			break;
		}

		if (level < pParams->m_Levels)
		{
			pData += dataSize;
		}
		else
		{
			SAFE_DELETE_ARY(pData);
		}

		if (width == 1 && height == 1)
		{
			break;
		}
		else
		{
			if (width > 1)
				width >>= 1;
			if (height > 1)
				height >>= 1;
		}

		level++;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, pParams->m_Levels > 1? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return true;
}

const U32 C_Texture2D::Size(const E_Format format, const U32 width, const U32 height)
{
	switch (format)
	{
	case E_Format_DXT1:
		return ((width + 3) / 4) * ((height + 3) / 4) * 8;
	case E_Format_DXT3:
	case E_Format_DXT5:
		return ((width + 3) / 4) * ((height + 3) / 4) * 16;
	case E_Format_A8R8G8B8:
		return width * height * 4;
	default:
		assert(0 && "Unsupported texture format");
		return 0;
	}
}

}; // namespace GL
}; // namespace NewDisplay
