
#include "Utility.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF_IPF
#pragma optimize( "", off ) // CFG_ALLOW_OPTIMIZE_OFF_IPF
#endif

namespace Ipf
{

const bool FindLayers( TLayerList& list )
{
	list.Clear();

	// Get paths
	AIArtHandle **artStore	= NULL;
	long artCount			= 0;
	AIErr error				= 0;

	// Only concerned with group items.
	AIMatchingArtSpec spec = 
	{
		kGroupArt,
		0,
		0,
	};
	sAIMatchingArt->GetMatchingArt( &spec, 1, &artStore, &artCount );

	if( artCount > 0 )
	{	
		for( long i = 0; i < artCount && !error; i++ )
		{
			AIArtHandle groupHandle = (*artStore)[i];
			AIArtHandle groupParent = NULL;

			sAIArt->GetArtParent( groupHandle, &groupParent );

			// Only concerned with layers. All layers have no parent.
			if ( !groupParent )
				list.PushBack( groupHandle );
		}
		return true;
	}
	return false;
}


const UI::C_AdobeColour ConvertCustomColour( const AICustomColor& in )
{
	F32 r, g, b;
	switch(in.kind)
	{
	case kCustomFourColor:
		// CMYK
		r	= (1.0f - in.c.f.cyan)		* (1.0f - in.c.f.black);
		g	= (1.0f - in.c.f.magenta)	* (1.0f - in.c.f.black);
		b	= (1.0f - in.c.f.yellow)	* (1.0f - in.c.f.black);
		break;
	case kCustomThreeColor:
		// RGB
		r	= in.c.rgb.red;
		g	= in.c.rgb.green;
		b	= in.c.rgb.blue;
		break;
	default:
		r	= 1.0f;
		g	= 1.0f;
		b	= 0.0f;
		break;
	}

	return UI::C_AdobeColour( r, g, b );
}

const UI::C_AdobeColour ConvertColour( const AIColor& in )
{
	F32 r, g, b;
	switch(in.kind)
	{
	case kGrayColor:
		// Grey scale
		r	= 
		g	= 
		b	= 1.0f - in.c.g.gray;
		break;
	case kFourColor:
		// CMYK
		r	= (1.0f - in.c.f.cyan)		* (1.0f - in.c.f.black);
		g	= (1.0f - in.c.f.magenta)	* (1.0f - in.c.f.black);
		b	= (1.0f - in.c.f.yellow)	* (1.0f - in.c.f.black);
		break;
	case kThreeColor:
		// RGB
		r	= in.c.rgb.red;
		g	= in.c.rgb.green;
		b	= in.c.rgb.blue;
		break;
	case kCustomColor:
		{
			AICustomColor customColor;
			sAICustomColor->GetCustomColor( in.c.c.color, &customColor );
			return ConvertCustomColour( customColor );
		}
		break;
	default:
		// Not supported
		break;
	}

	return UI::C_AdobeColour( r, g, b );
}


const Util::C_String ConvertUnicodeString( const ai::UnicodeString& uniString )
{
	char pathName[256];
	uniString.as_Roman( pathName, 256 );

	return pathName;
}


const Util::C_String ConvertUnicodeString( AIArtHandle artHandle )
{
	ai::UnicodeString	name;
	ASBoolean			hasName;

	sAIArt->GetArtName( artHandle, name, &hasName );

	return ConvertUnicodeString( name );
}


const F32 BezierLength( const Maths::HVec3 b[4] )
{
	Maths::HVec3 p0 = (b[0] - b[1]);
	Maths::HVec3 p1 = (b[2] - b[1]);
	Maths::HVec3 p2;
	Maths::HVec3 p3 = (b[3] - b[2]);
	F32 l0 = p0.Length();
	F32 l1 = p1.Length();
	F32 l3 = p3.Length();
	if (l0 > 0.f) p0 /= l0;
	if (l1 > 0.f) p1 /= l1;
	if (l3 > 0.f) p3 /= l3;
	p2 = -p1;
	F32 a = Maths::Abs(p0 | p1) + Maths::Abs(p2 | p3);

	if ((a > 1.98f))// || ((l0+l1+l3) < ((4.f - a)*8.f))) 
	{
		return l0+l1+l3;
	} 

	Maths::HVec3 bl[4];
	Maths::HVec3 br[4];
	bl[0] = b[0];
	bl[1] = (b[0]+b[1])*.5f;
	Maths::HVec3 mid = (b[1]+b[2])*.5f;
	bl[2] = (bl[1]+mid)*.5f;
	br[3] = b[3];
	br[2] = (b[2]+b[3])*.5f;
	br[1] = (br[2]+mid)*.5f;
	br[0] = (br[1]+bl[2])*.5f;
	bl[3] = br[0];

	return BezierLength(bl) + BezierLength(br);
}


}; // namespace Ipf

#ifdef CFG_ALLOW_OPTIMIZE_OFF_IPF
#pragma optimize( "", on ) // CFG_ALLOW_OPTIMIZE_OFF_IPF
#endif

