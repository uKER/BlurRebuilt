#ifndef BIZPAK_PAKCREATE_H
#define BIZPAK_PAKCREATE_H

#ifdef _MSC_VER
#pragma once
#endif

struct C_Options;

namespace Commands
{
	E_ReturnCode PakCreate(const C_Options& options);
}

#endif
