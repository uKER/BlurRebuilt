#ifndef TOOLS_SHARED_ASSET_META_H
#define TOOLS_SHARED_ASSET_META_H

#pragma once

#include <system/serialise/filepath.h>

namespace Tools
{

	bool MetaMismatch(
		const Sys::C_FilePath& outputFilePath,
		const Sys::C_FilePath& sourceFilePath,
		S64 version);

	bool MetaMismatch(
		const Sys::C_FilePath& outputFilePath,
		const Sys::C_FilePath& sourceFilePath,
		const Sys::C_FilePath& toolFile);
}

#endif
