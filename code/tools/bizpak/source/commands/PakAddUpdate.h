#ifndef BIZPAK_PAKADDUPDATE_H
#define BIZPAK_PAKADDUPDATE_H

#ifdef _MSC_VER
#pragma once
#endif

struct C_Options;

namespace Commands
{
	E_ReturnCode PakUpdate(const C_Options& options);
}

#endif
