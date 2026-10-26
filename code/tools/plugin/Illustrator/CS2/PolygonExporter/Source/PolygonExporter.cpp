
#include <string>

#include "AIPlugIn.h"
#include "ADMBasic.h"

#include "Defines.h"
#include "PolygonExporter.h"

#include "ui/Illustrator/Data/UIAdobeDataManager.h"
#include "ui/Illustrator/Data/UIAdobeData.h"
#include "ui/Illustrator/UIAdobeScene.h"
#include "ui/Illustrator/UIAdobePolygon.h"


#define MAX_SEGMENTS		8192

using namespace UI;

F32 BezierLength(const Maths::HVec3 b[4])
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

	return C_AdobeColour( r, g, b );
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

	return C_AdobeColour( r, g, b );
}

//-------------------------------------------------------------------
// Global instance of the data manager.
//-------------------------------------------------------------------
AIRealRect			g_MaxSceneBounds;

//-------------------------------------------------------------------
// Global instance of the data manager.
//-------------------------------------------------------------------
Ut::UtVector< Ut::DynamicVector< AIGradientHandle > >	g_UsedGradientsList;

//-------------------------------------------------------------------
// File export work is done by these 3 functions.
//-------------------------------------------------------------------
AIErr	CreatePolygonFile( const char * pathName );
void	BuildPolygonShapesList( C_AdobeData& sceneData );
void	BuildGradientDictionary( C_AdobeData& sceneData );
void	GetMaxSceneBounds();

AIErr CreatePolygonFile( const char * pathName ) 
{
	AIErr error = 0;
	C_AdobeData sceneData;

	g_UsedGradientsList.Clear();

	GetMaxSceneBounds();

	// Gather Shapes from the scene that we are interested
	// in and then write them out to the file.
	BuildPolygonShapesList( sceneData );

	BuildGradientDictionary( sceneData );

	// Store the scene bounds so they can be written out to the file.
//	g_AdobeDataManager.SetRealSceneBounds( Maths::HVec3( g_MaxSceneBounds.left, g_MaxSceneBounds.bottom, 0.0f ), Maths::HVec3( g_MaxSceneBounds.right, g_MaxSceneBounds.top, 0.0f ) );
	sceneData.m_RealSceneBounds = Maths::C_AxisBox( Maths::HVec3( g_MaxSceneBounds.left, g_MaxSceneBounds.bottom, 0.0f ), Maths::HVec3( g_MaxSceneBounds.right, g_MaxSceneBounds.top, 0.0f ) );

	// Write the contents of the scene.
	C_AdobeDataManager::Instance().WriteData( pathName, &sceneData );

	return error;
}

void GetMaxSceneBounds()
{
	AIErr	error = 0;

	// Get paths
	AIMatchingArtSpec spec;
	AIArtHandle **artStore = NULL;
	long artCount = 0;

	// Only concerned with group items.
	spec.type = kPathArt;
	spec.whichAttr = 0;
	spec.attr = 0;
	(void)sAIMatchingArt->GetMatchingArt( &spec, 1, &artStore, &artCount );

	if ( artCount > 0 )
	{
		AIArtHandle	artHandle = (*artStore)[0];
		sAIArt->GetArtBounds( artHandle, &g_MaxSceneBounds );
	}

	for( long i = 1; i < artCount && !error; i++ )
	{
		AIRealRect rect;
		AIArtHandle	artHandle = (*artStore)[i];
		
		sAIArt->GetArtBounds( artHandle, &rect );

		if ( rect.left < g_MaxSceneBounds.left )
			g_MaxSceneBounds.left = rect.left;

		if ( rect.top > g_MaxSceneBounds.top )
			g_MaxSceneBounds.top = rect.top;

		if ( rect.right > g_MaxSceneBounds.right )
			g_MaxSceneBounds.right = rect.right;

		if ( rect.bottom < g_MaxSceneBounds.bottom )
			g_MaxSceneBounds.bottom = rect.bottom;
	}
}


C_AdobePolygon* BuildAdobePolygon
(
	AIArtHandle& artHandle, 
	const char* name,
	E_AdobeArtType type, 
	E_AdobeShapeMode mode = E_AdobeShapeMode_None, 
	S32 compoundId = -1 
)
{
	C_AdobePolygon* pNewData = new C_AdobePolygon( type, mode, compoundId );
	pNewData->Setup( name );

	Maths::HVec3 b[4] = 
	{
		Maths::HVec3( 0,0,0 ),
		Maths::HVec3( 1,1,0 ),
		Maths::HVec3( 2,0,0 ),
		Maths::HVec3( 3,1,0 ),
	};
	F32 len = BezierLength( b );

	// Create the data for the polygon.
	AIPathSegment	segments[MAX_SEGMENTS];
	AIPathStyle		style;
	short			count = 0;
	AIBoolean		closed = false;

	sAIPath->GetPathClosed( artHandle, &closed);
	sAIPath->GetPathSegmentCount( artHandle, &count);
	sAIPath->GetPathSegments( artHandle, 0, count, segments);

	sAIPathStyle->GetPathStyle( artHandle, &style );

	// Set blend
	const AIReal opacity = sAIBlendStyle->GetOpacity( artHandle );
	const AIBlendingMode bm = sAIBlendStyle->GetBlendingMode( artHandle );
	if( opacity != 1.0f || bm != kAINormalBlendingMode)
	{
		C_AdobeBlendStyle::E_BlendMode blendMode;
		switch( bm )
		{
		// Supported modes
		case kAINormalBlendingMode :
			blendMode = C_AdobeBlendStyle::E_BlendMode_Normal;
			break;
		case kAIMultiplyBlendingMode :
			blendMode = C_AdobeBlendStyle::E_BlendMode_Multiply;
			break;
		// Not currently supported
		case kAIScreenBlendingMode :
		case kAIOverlayBlendingMode :
		case kAISoftLightBlendingMode :
		case kAIHardLightBlendingMode :
		case kAIColorDodgeBlendingMode :
		case kAIColorBurnBlendingMode :
		case kAIDarkenBlendingMode :
		case kAILightenBlendingMode :
		case kAIDifferenceBlendingMode :
		case kAIExclusionBlendingMode :
		case kAIHueBlendingMode :
		case kAISaturationBlendingMode :
		case kAIColorBlendingMode :
		case kAILuminosityBlendingMode :
		default:
			// Just export as 'normal'
			blendMode = C_AdobeBlendStyle::E_BlendMode_Normal;
			break;
		};

		pNewData->GettShapeBlendStyle() = C_AdobeBlendStyle( blendMode, opacity );
	}	

	// Bounding box
	F32 width = g_MaxSceneBounds.right - g_MaxSceneBounds.left;
	F32 height = g_MaxSceneBounds.top - g_MaxSceneBounds.bottom;
	bool hasControlPoints = false;

	F32 mul;
	if( width > height )
		mul = 1.0f / width * ILLUSTRATOR_SCALE;
	else
		mul = 1.0f / height * ILLUSTRATOR_SCALE;

	// Add each of the points to the polygon.
	AIPathSegment *p = segments;
	for ( short i = 0; i < count; i++, p++ )
	{
		// Knock the scene bounds off to sort out the origin.
		F32 pX = p->p.h - g_MaxSceneBounds.left;
		F32	pY = p->p.v - g_MaxSceneBounds.bottom;
		pY = -pY + height;
		pX *= mul;
		pY *= mul;
		Maths::HVec2 pos = Maths::HVec2( pX, pY );

		F32 inX = p->in.h - g_MaxSceneBounds.left;
		F32 inY = p->in.v - g_MaxSceneBounds.bottom;
		inY = -inY + height;
		inX *= mul;
		inY *= mul;
		Maths::HVec2 in = Maths::HVec2( inX, inY );

		F32 outX = p->out.h - g_MaxSceneBounds.left;
		F32 outY = p->out.v - g_MaxSceneBounds.bottom;
		outY = -outY + height;
		outX *= mul;
		outY *= mul;
		Maths::HVec2 out = Maths::HVec2( outX, outY );
		
		bool corner	= false;
		if(p->corner)
			corner = true;		

		// If the in / out = point then its a stright line.
		if ( pos.X() != in.X() || pos.Y() != in.Y() ||
			 pos.X() != out.X() || pos.Y() != out.Y() )
			hasControlPoints = true;

		pNewData->AddPoint( pos, in, out, corner );
	}
	// Flag that the shape has control points.
	pNewData->HasControlPoints( hasControlPoints );

	// AIBoolean is gay and wont let you cast.
	if ( closed > 0 )
		pNewData->IsClosed( true );
	else
		pNewData->IsClosed( false );

	// Shape has stroke style?
	if(style.strokePaint)
		pNewData->GetShapeStrokeStyle() = C_AdobeStrokeStyle( ConvertColour( style.stroke.color ), style.stroke.width );

	// Shape has fill style
	if(style.fillPaint)
	{
		C_AdobeColour colour;

		C_AdobeFillStyle::E_FillStyle_Kind kind = C_AdobeFillStyle::E_FillStyle_Kind_None;
		switch( style.fill.color.kind )
		{
		case kCustomColor:
		case kThreeColor:
		case kFourColor:
		case kGrayColor:
			kind = C_AdobeFillStyle::E_FillStyle_Kind_Solid;
			break;
		case kGradient:
			kind = C_AdobeFillStyle::E_FillStyle_Kind_Gradient;
			break;
		}

		if( kind == C_AdobeFillStyle::E_FillStyle_Kind_Solid )
		{
			pNewData->GetShapeFillStyle() = C_AdobeFillStyle( ConvertColour( style.fill.color ) );
		}
		else
		if( kind == C_AdobeFillStyle::E_FillStyle_Kind_Gradient )
		{
			// Check if this already exists
			S32 index = -1; 
			for(S32 i = 0; i < g_UsedGradientsList.Size(); i++)
				if( style.fill.color.c.b.gradient == g_UsedGradientsList[i] )
				{
					index = i;
					break;
				}

			if(index == - 1)
			{
				// Push dictionary index
				index = g_UsedGradientsList.Size();
				g_UsedGradientsList.PushBack( style.fill.color.c.b.gradient );
			}

			// Get the rotation angle from the gradient matrix
			const Maths::C_MatrixRotateRPH rph(Maths::HMat33( 
				Maths::HVec3( style.fill.color.c.b.matrix.a,	style.fill.color.c.b.matrix.b,	0.0f ),
				Maths::HVec3( style.fill.color.c.b.matrix.c,	style.fill.color.c.b.matrix.d,	0.0f ),
				Maths::HVec3( 0.0f,								0.0f,							1.0f )).GetRotationRPH() );
			F32 angle = (Maths::HF32)rph.Roll();

			// Build our gradient object
			C_AdobeGradientStyle gradient;

			gradient.m_DictionaryIndex	= index;
			gradient.m_Angle			= style.fill.color.c.b.gradientAngle + Maths::RadToDeg( angle );
			gradient.m_Px				= style.fill.color.c.b.gradientOrigin.h;
			gradient.m_Py				= style.fill.color.c.b.gradientOrigin.v;
			gradient.m_Length			= style.fill.color.c.b.gradientLength;

			// Asign to the shape's fill style
			pNewData->GetShapeFillStyle() = C_AdobeFillStyle( gradient );
		}
	}
	return pNewData;
}	


void TraverseGroupList( C_AdobeData& sceneData, AIArtHandle& groupHandle )
{
	char			parentName[128];
	char			artName[128];
	ASBoolean		hasArtName;

	AIArtHandle		groupChild;
	int				compoundId = 0;
	short			artType;

	// Get the group name and the first child.
	sAIArt->GetArtName( groupHandle, parentName, sizeof(parentName), &hasArtName );
	sAIArt->GetArtFirstChild( groupHandle, &groupChild );

	// While children are still present.
	while ( groupChild )
	{
		sAIArt->GetArtName( groupChild, artName, sizeof(artName), &hasArtName );
		sAIArt->GetArtType( groupChild, &artType );

		// Traverse the group hierarchy
		if ( artType == kGroupArt )
		{
			sceneData.AddGroup( artName );
			TraverseGroupList( sceneData, groupChild );
			sceneData.CloseGroup();
		}
		// Compound paths are evil versions of compound shapes.
		// Need extra information so that they can go through
		// the hole resolver.
		else
		if ( artType == kCompoundPathArt )
		{
			AIArtHandle pathChild;
			sAIArt->GetArtFirstChild( groupChild, &pathChild );

			while( pathChild )
			{
				sAIArt->GetArtName( pathChild, artName, sizeof(artName), &hasArtName );

				// Add to work list
				C_AdobePolygon* pPolygon = 
							BuildAdobePolygon( pathChild, artName, E_AdobeArtType_CompoundPath, E_AdobeShapeMode_None, compoundId );

				sceneData.AddData( pPolygon );

				sAIArt->GetArtSibling( pathChild, &pathChild );
			}
			compoundId++; // increment compound shape Id.
		}
		else
		if ( artType == kPluginArt )
		{
			// Get paths
			AIMatchingArtSpec	spec;
			AIArtHandle**		artStore = NULL;
			long				artCount = 0;
			AIErr				error = 0;

			// Only concerned with group items.
			spec.type = kGroupArt;
			spec.whichAttr = 0;
			spec.attr = 0;
			(void)sAIMatchingArt->GetMatchingArt( &spec, 1, &artStore, &artCount );

			AIArtHandle	compoundGroup = NULL;
			AIArtHandle	compoundParent = NULL;
			AIBoolean	isCompoundShape = 0;

			// Need to find the compound shape handle.
			for( long i = 0; i < artCount && !error; i++ )
			{
				compoundGroup = (*artStore)[i];
				sAIArt->GetArtParent( compoundGroup, &compoundParent );
				if ( compoundParent == groupChild )
				{
					sAIPathFinder->IsCompoundShape( compoundParent, &isCompoundShape );
					break;
				}
			}
						
			// Pull out each path that make up the compound shape.
			AIArtHandle		pathChild = NULL;

			sAIArt->GetArtFirstChild( compoundGroup, &pathChild );

			while( pathChild )
			{
				sAIPathFinder->IsCompoundShape( pathChild, &isCompoundShape );

				// Only supported compound shapes made up of 2 paths only!!
				if ( !isCompoundShape )
				{
					AIShapeMode shape_mode = sAIPathFinder->GetShapeMode(pathChild);
					sAIArt->GetArtName( pathChild, artName, sizeof(artName), &hasArtName );

					// Add to work list
					C_AdobePolygon* pPolygon = 
						BuildAdobePolygon( pathChild, artName, E_AdobeArtType_CompoundShape, E_AdobeShapeMode_None, compoundId );

					sceneData.AddData( pPolygon );
				}
				sAIArt->GetArtSibling( pathChild, &pathChild );		
			}
			compoundId++; // increment compound shape Id.
		}
		else
		if ( artType == kPathArt )
		{
			// Add to work list
			C_AdobePolygon* pPolygon = 
				BuildAdobePolygon( groupChild, artName, E_AdobeArtType_Path );

			sceneData.AddData( pPolygon );
		}
		// Get next child.
		sAIArt->GetArtSibling( groupChild, &groupChild );
	}
}


C_AdobeGradient ConvertGradient( AIGradientHandle gradientHandle )
{
	// Type
	short type;
	sAIGradient->GetGradientType( gradientHandle, &type );
	S16 isLinear(type == kLinearGradient);

#if 0
	// Name
	unsigned char p_name[256];
	sAIGradient->GetGradientName( gradientHandle, p_name );

	for(unsigned char c = 1; c <= p_name[0]; c++)
		if(p_name[c] == ' ' || p_name[c] == '~')
			p_name[c] = '_';
#ifdef _DEBUG
	unsigned char c_name[256];
	for(unsigned char c = 1; c <= p_name[0]; c++)
	{
		c_name[c-1] = p_name[c];
		if(c_name[c-1] == ' ' || c_name[c-1] == '~')
			c_name[c-1] = '_';
	}
	c_name[p_name[0]] = NULL;
#endif
#endif

	// Stops
	short count;
	sAIGradient->GetGradientStopCount( gradientHandle, &count );
	assert(count);

	AIGradientStop stop;
	C_AdobeGradient::C_Stop* pStops = new C_AdobeGradient::C_Stop[ count ];
	assert(pStops);

#ifdef _DEBUG
	memset(pStops, 0xbe, sizeof(C_AdobeGradient::C_Stop) * count );
#endif

	for(short c = 0; c < count; c++)
	{
		sAIGradient->GetNthGradientStop( gradientHandle, c, &stop );

		pStops[ c ].m_Mid		= stop.midPoint/100.0f;
		pStops[ c ].m_Ramp		= stop.rampPoint/100.0f;
		pStops[ c ].m_Colour	= ConvertColour( stop.color );
	}

	// Make out return object
	C_AdobeGradient ret( isLinear, (U16)count, pStops );

	// Clean up
	delete [] pStops;

	return ret;
}

void BuildGradientDictionary( C_AdobeData& sceneData )
{
	for ( S32 gradient = 0; gradient < g_UsedGradientsList.Size(); gradient++ )
	{
		sceneData.AddGradient( new C_AdobeGradient( ConvertGradient( g_UsedGradientsList[ gradient ] ) ) );
	}
}

void BuildPolygonShapesList( C_AdobeData& sceneData )
{
	AIErr	error = 0;
	S32		layerId	= 0;

	// Get paths
	AIMatchingArtSpec spec;
	AIArtHandle **artStore = NULL;
	long artCount = 0;

	// Only concerned with group items.
	spec.type = kGroupArt;
	spec.whichAttr = 0;
	spec.attr = 0;
	(void)sAIMatchingArt->GetMatchingArt( &spec, 1, &artStore, &artCount );

	if( artCount > 0 )
	{	
		for( long i = 0; i < artCount && !error; i++ )
		{
			AIArtHandle		groupHandle = (*artStore)[i];
			char			art_name[128];
			ASBoolean		hasArtName;

			sAIArt->GetArtName( groupHandle, art_name, sizeof(art_name), &hasArtName );

			// Get the arts parent to see if it is a group associated with a compound shape.
			// Find the parent of the groupd to see if it is a compound shape!
			AIArtHandle		groupParent;
			AIBoolean		isCompoundShape = 0;

			sAIArt->GetArtParent( groupHandle, &groupParent );

			// Only concerned with layers. All layers have no parent.
			if ( !groupParent )
			{
				// Make a new layer so that groups and shapes can be added.
				sceneData.AddLayer( art_name );

				TraverseGroupList( sceneData, groupHandle );
			}
		}
	}
}

typedef struct
{
	AIReal r;
	AIReal g;
	AIReal b;
} rgb_t;

typedef struct
{
	AIReal		mid;
	AIReal		ramp;
	rgb_t		colour;
} stops_t;
/*
const bool WriteOutGradient( AIGradientHandle gradientHandle, FILE* fp )
{
	// Type
	short type;
	sAIGradient->GetGradientType( gradientHandle, &type );
	short isLinear(type == kLinearGradient);

	// Name
	unsigned char p_name[256];
	sAIGradient->GetGradientName( gradientHandle, p_name );

	for(unsigned char c = 1; c <= p_name[0]; c++)
		if(p_name[c] == ' ' || p_name[c] == '~')
			p_name[c] = '_';

#ifdef _DEBUG
	unsigned char c_name[256];
	for(unsigned char c = 1; c <= p_name[0]; c++)
	{
		c_name[c-1] = p_name[c];
		if(c_name[c-1] == ' ' || c_name[c-1] == '~')
			c_name[c-1] = '_';
	}
	c_name[p_name[0]] = NULL;
#endif

	// Stops
	short count;
	sAIGradient->GetGradientStopCount( gradientHandle, &count );
	if(count == 0)
		return false;

	AIGradientStop stop;
	stops_t* pStops = new stops_t[ count ];
	if(!pStops)
		return false;

#ifdef _DEBUG
	memset(pStops, 0xbe, sizeof(stops_t) * count );
#endif

	for(short c = 0; c < count; c++)
	{
		sAIGradient->GetNthGradientStop( gradientHandle, c, &stop );

		pStops[ c ].mid			= stop.midPoint/100.0f;
		pStops[ c ].ramp		= stop.rampPoint/100.0f;

		ConvertColour(stop.color, pStops[ c ].colour);
	}

	fwrite(&isLinear, sizeof(isLinear), 1, fp);
	fwrite(&count, sizeof(count), 1, fp);
#if 0 // Write out name
	fwrite(p_name, sizeof(unsigned char) * (p_name[0] + 1), 1, fp);
#endif
	fwrite(pStops, sizeof(stops_t) * count, 1, fp);

	delete [] pStops;

	return true;
}
*/


//-------------------------------------------------------------------
// Wrapper functions that are called from PluginHandlerCallback
//-------------------------------------------------------------------
AIErr addFileFormat( SPInterfaceMessage *message )
{
	AIErr error;
	PlatformAddFileFormatData affd;
	std::auto_ptr<char> title( new char[100] ); 

	strcpy_s( title.get(), 100, "Illustrator Polygon File" );
	SUctopstr( title.get() );
	affd.title = reinterpret_cast<unsigned char *>( title.get() );
	affd.titleOrder = 0;
	affd.extension = "ipf";

	error = sAIFileFormat->AddFileFormat( message->d.self, "Selected text format",
		&affd, kFileFormatWrite,
		&g->fileFormatSelected );

	return error;
}


AIErr goFileFormat( AIFileFormatMessage *message ) 
{
	AIErr error = kNoErr;
	char pathName[300];

	strcpy_s( pathName, sizeof( pathName ), message->file.path );

	if ( message->option & kFileFormatWrite ) 
	{
		// Get whatever art is supposed to be saved.  You might walk the tree writing out 
		// information on each object as you come to it, or as in this case
		// just get a bunch of art and work in batch mode.  It depends on what your doing.					
		error = CreatePolygonFile( pathName );
	}

	return error;
}