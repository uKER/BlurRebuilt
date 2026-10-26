#ifndef TEXTURE_CONVERTER_H
#define TEXTURE_CONVERTER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Options.h"

namespace Attk { class C_Image;}

class C_TextureConverter
{
public:
	static bool Process(
		Tools::FileCopy::C_FileCopyList& uberCopyList,
		const C_Options& options);

	static void WriteTexture(Attk::C_Image* pImage, const C_Options& options, Tools::E_Platform platform, Sys::I_File& file);
	static void ColourNormalize(Attk::C_Image* pImage);
};

#endif
