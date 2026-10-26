#ifndef TEXTURE_CONVERTER_H
#define TEXTURE_CONVERTER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Options.h"

namespace ActTools { class Texture; }

class C_TextureConverter
{
public:
	static bool Process(
		Tools::FileCopy::C_FileCopyList& uberCopyList,
		const C_Options& options);

	static void WriteTexture(ActTools::Texture *pTexture, const C_Options& options, Util::E_Platform platform, Sys::I_File& file);
	static void ColourNormalize(ActTools::Texture *pTexture);
};

#endif
