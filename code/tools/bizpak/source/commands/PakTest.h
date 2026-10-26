#ifndef BIZPAK_PAKTEST_H
#define BIZPAK_PAKTEST_H

#ifdef _MSC_VER
#pragma once
#endif

struct C_Options;

namespace Commands
{
	E_ReturnCode PakTest(const C_Options& options);
}

#endif
