#pragma once

namespace ResourceBundler
{
	namespace Utility
	{
		const char* GetTextureFormatString(Display::ResourceBundler::E_TextureFormat format);

		Display::ResourceBundler::E_TextureFormat GetTextureFormatType(const char* format);

		U32 GetBytesPerBlock(Display::ResourceBundler::E_TextureFormat format);

		U32 GetBlockWidth(Display::ResourceBundler::E_TextureFormat format);

		U32 GetPitch(U32 width, Display::ResourceBundler::E_TextureFormat format);

		U32 GetSlicePitch(U32 width, U32 height, Display::ResourceBundler::E_TextureFormat format);

		bool RequiresPow2Size(Display::ResourceBundler::E_TextureFormat format);

		bool IsPow2Size(U32 width, U32 height, U32 depth);

		S32 MakePow2(S32 val);

		Display::ResourceBundler::E_TextureFormat GetEquivalentTextureFormat(Attk::E_PixelFormat format);
	}
}
