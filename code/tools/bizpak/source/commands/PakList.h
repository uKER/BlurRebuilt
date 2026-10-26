#ifndef BIZPAK_PAKLIST_H
#define BIZPAK_PAKLIST_H

#ifdef _MSC_VER
#pragma once
#endif

struct C_Options;

namespace Commands
{
	E_ReturnCode PakList(const C_Options& options);
}

#endif
