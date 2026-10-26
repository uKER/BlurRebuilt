#include "stdafx.h"

#include "ui/Game/Marketplace/Marketplace.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

namespace UI
{

// -------------
// C_Marketplace
// -------------
C_Marketplace C_Marketplace::m_sInstance;

}; // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif
