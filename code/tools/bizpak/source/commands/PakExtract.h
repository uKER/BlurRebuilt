#ifndef BIZPAK_PAKEXTRACT_H
#define BIZPAK_PAKEXTRACT_H

#ifdef _MSC_VER
#pragma once
#endif

struct C_Options;

namespace Commands
{
	E_ReturnCode PakExtract(const C_Options& options);
}

#endif
