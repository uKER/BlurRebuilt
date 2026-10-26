#ifndef RESOURCEUTILSDECLARE_5ED51136_CFD5_4019_88CA_D71B0B79D057_H
#define RESOURCEUTILSDECLARE_5ED51136_CFD5_4019_88CA_D71B0B79D057_H

#ifdef _MSC_VER
#pragma once
#endif


namespace ResourceUtils
{

struct C_XtCycleResourceDef;
struct C_GameModel;
struct C_GameTexture;
struct C_XtSkeletonResourceDef;

class I_ResourceBag;

enum E_GameFileFlags
{
	E_GameFileFlag_DownloadableContent		= 0x0001,
};

typedef U32 C_ResourceBagTag; ///< Identifies a resource within a resource bag.

} // namespace ResourceUtils


#endif // RESOURCEUTILSDECLARE_5ED51136_CFD5_4019_88CA_D71B0B79D057_H
