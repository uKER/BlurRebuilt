#include "Stdafx.h"

#include "ui/Illustrator/UIAdobePolygon.h"
#include "ui/Illustrator/UIAdobePoint.h"


#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize( "", off ) // CFG_ALLOW_OPTIMIZE_OFF_UI
#endif

namespace UI
{

C_AdobePolygon::C_AdobePolygon()
: C_AdobeElement( Ipf::E_ElementType_Polygon )
{}


}// namespace UI
