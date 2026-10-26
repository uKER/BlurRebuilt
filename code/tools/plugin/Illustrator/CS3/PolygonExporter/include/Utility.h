
#ifndef IPF_UTILITY_H
#define IPF_UTILITY_H

#include "Defines.h"

#include <btl/Containers/vector.h>

#include "ui/Illustrator/UIAdobeColour.h"

namespace Ipf
{

/*---------------------------------------------------------------------------------------
	Type defs
---------------------------------------------------------------------------------------*/
typedef Btl::Vector< AIArtHandle > TLayerList;


/*---------------------------------------------------------------------------------------
	Helper functions
---------------------------------------------------------------------------------------*/

							/// Returns all of the layers in the document.
const bool					FindLayers( TLayerList& list );

							/// Converts AI colour into adobe colour.
const UI::C_AdobeColour		ConvertCustomColour( const AICustomColor& in );

							/// Converts AI colour into adobe colour.
const UI::C_AdobeColour		ConvertColour( const AIColor& in );

							/// Converts unicode strings to C_Strings
const Util::C_String		ConvertUnicodeString( const ai::UnicodeString& uniString );

							/// Converts unicode strings to C_Strings
const Util::C_String		ConvertUnicodeString( AIArtHandle artHandle );

							/// Returns the bezier length between 2 control points.
const F32					BezierLength( const Maths::HVec3 b[4] );


}; // Ipf

#endif // ADOBE_UTILITY_H