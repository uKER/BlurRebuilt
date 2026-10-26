#pragma pack(push, 4)
#include "AIPlugIn.h"
#include "ADMBasic.h"
#include "IAIUnicodeString.h"
#include "AIFileFormat.h"
#pragma pack(pop)

#include "Defines.h"
#include "Utility.h"
#include "PolygonExporter.h"
#include "MessageHandler.h"
#include "ShapeBuilder.h"

#include "..\Resource.h"

#include "ui/Illustrator/UIAdobeScene.h"
#include "ui/Illustrator/UIAdobePolygon.h"
#include "ui/Illustrator/UIAdobeGroup.h"
#include "ui/Illustrator/Ipf/IpfData.h"
#include "ui/Illustrator/Ipf/IpfWriter.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF_IPF
#pragma optimize( "", off ) // CFG_ALLOW_OPTIMIZE_OFF_IPF
#endif

using namespace UI;
using namespace Ipf;

//-------------------------------------------------------------------
// Global instance of the data manager.
//-------------------------------------------------------------------
Ut::UtVector< Ut::DynamicVector< AIGradientHandle > >	gUsedGradientsList;

//-------------------------------------------------------------------
// File export work is done by these 3 functions.
//-------------------------------------------------------------------
void BuildPolygonShapesList( C_IpfData& sceneData );
void BuildGradientDictionary( C_IpfData& sceneData );


AIErr CreatePolygonFile( const char* pFileName, const F32 fCurveStep, const bool bFixedSpaceCurve, const bool bDisableFinning ) 
{
	char message[512];

	C_IpfData data;
	C_IpfWriter ipfWriter;

	C_ShapeBuilder::Instance().BuildOptions( fCurveStep, bFixedSpaceCurve, bDisableFinning );
	gUsedGradientsList.Clear();

	// Gather Shapes from the scene that we are interested
	// in and then write them out to the file.
	BuildPolygonShapesList( data );
	BuildGradientDictionary( data );

	// Check that the user isnt trying to export an empty scene.
	if ( !data.HasDataToWrite() )
	{
		C_MessageHandler::Instance().ShowMessage( C_MessageHandler::E_MessageType_Warning, "Failed to export scene! Layers or Gradient list is empty!" );
		return 0;
	}

	// Write the contents of the scene.
	//E_AdobeResult result = C_IpfDataManager::Instance().WriteData( pFileName, &sceneData );

	E_Result result  = ipfWriter.ExportScene( pFileName, &data );

	switch( result )
	{
		case E_Result_Success :
		{
			// Inform user that the save was successfull.
			sprintf_s( message, 512, "%s %s %d %s %d %s %d %s %d",		"Scene Exported Successfull! \n",
																		"\nTotal Layers : ", data.GetNumberOfLayers(), 
																		"\nTotal Groups : ", data.GetNumberOfGroups(), 
																		"\nTotal Shapes : ", data.GetNumberOfShapes(), 
																		"\nTotal Gradients : ", data.GetNumberOfGradients() );
			C_MessageHandler::Instance().ShowMessage( C_MessageHandler::E_MessageType_Info, message );
			break;
		}
		case E_Result_ReadOnly :
		{
			// Inform user that the save was successfull.
			sprintf_s( message, 512, "Scene could not be exported! \n\nThe file \"%s\" is read only.", pFileName );
			C_MessageHandler::Instance().ShowMessage( C_MessageHandler::E_MessageType_Warning, message );
			break;
		}
		case E_Result_Failed :
		{
			C_MessageHandler::Instance().ShowMessage( C_MessageHandler::E_MessageType_Error, "Scene could not be exported!" );
			break;
		}
	}

	return 0;
}


void CalculatePathBounds( const AIArtHandle pPath, Maths::C_AxisBox& sBounds )
{
	AIPathStyle	style;
	sAIPathStyle->GetPathStyle( pPath, &style );

	AIRealRect rect;
	sAIArt->GetArtBounds( pPath, &rect );

	// Knock off the outline size as we only care about the fill dimensions
	F32 halfOutline = 0.0f;//( style.strokePaint ) ? style.stroke.width * 0.5f : 0.0f;

	sBounds.Min( Maths::HVec3( rect.left + halfOutline, rect.bottom + halfOutline, 0.0f ) );
	sBounds.Max( Maths::HVec3( rect.right - halfOutline, rect.top - halfOutline, 0.0f ) );
}


void CalculateGroupBounds( const AIArtHandle pGroup, Maths::C_AxisBox& sBounds )
{
	// Only use exported layers / groups
	if ( ConvertUnicodeString( pGroup ).Search( EXCLUDE_LAYER_NAME ) == 1 )
		return;

	AIArtHandle pChild;
	sAIArt->GetArtFirstChild( pGroup, &pChild );

	// While children are still present.
	short artType;
	while ( pChild )
	{
		sAIArt->GetArtType( pChild, &artType );
		
		if ( artType == kGroupArt )
		{
			CalculateGroupBounds( pChild, sBounds );
		}
		else
		if ( artType == kPathArt )
		{
			Maths::C_AxisBox pathBounds;
			CalculatePathBounds( pChild, pathBounds );

			// Update box so it fits the contents of the group
			sBounds.UpdateExtents( pathBounds );
		}
		else
		if ( artType == kCompoundPathArt )
		{
			CalculateGroupBounds( pChild, sBounds );
		}
		// Get next child.
		sAIArt->GetArtSibling( pChild, &pChild );
	}
}


void TraverseGroupList( C_IpfData& sceneData, AIArtHandle& groupHandle, const bool bLayer )
{
	ai::UnicodeString	parentName;
	ai::UnicodeString	artName;
	ASBoolean			hasArtName;
	
	AIArtHandle		groupChild;
	int				compoundId = 0;
	short			artType;

	// Get the group name and the first child.
	sAIArt->GetArtName( groupHandle, parentName, &hasArtName );
	sAIArt->GetArtFirstChild( groupHandle, &groupChild );

	Maths::C_AxisBox groupBounds;
	CalculateGroupBounds( groupHandle, groupBounds );

	// While children are still present.
	while ( groupChild )
	{
		sAIArt->GetArtName( groupChild, artName, &hasArtName );
		sAIArt->GetArtType( groupChild, &artType );

		if ( artType == kGroupArt )
		{
			// Traverse the group hierarchy
			sceneData.AddGroup( ConvertUnicodeString( artName ) );
			TraverseGroupList( sceneData, groupChild, false );
			sceneData.CloseGroup();
		}
		else
		if ( artType == kPathArt )
		{
			Maths::C_AxisBox pathBounds;
			CalculatePathBounds( groupChild, pathBounds );

			C_AdobePolygon* pPolygon = new C_AdobePolygon();	
			C_ShapeBuilder::Instance().BuildPath( groupChild, pPolygon, ConvertUnicodeString( artName ), ( bLayer ) ? pathBounds : groupBounds );

			sceneData.AddData( pPolygon );
		}
		else
		if ( artType == kCompoundPathArt )
		{
			Maths::C_AxisBox pathBounds;
			CalculateGroupBounds( groupChild, pathBounds );

			C_AdobePolygon* pPolygon = new C_AdobePolygon();

			if ( !C_ShapeBuilder::Instance().BuildCompoundPath( groupChild, pPolygon, ConvertUnicodeString( artName ), ( bLayer ) ? pathBounds : groupBounds ) )
			{
				char buffer[128];
				sprintf_s( buffer, 128, "Failed to create compound path! \n\nLayer : %s \nObject : %s", sceneData.GetCurrentLayer()->GetName().Chars(), ConvertUnicodeString(artName).Chars() );

				C_MessageHandler::Instance().ShowMessage( C_MessageHandler::E_MessageType_Warning, buffer );

				SAFE_DELETE_PTR( pPolygon );
			}
			else
				sceneData.AddData( pPolygon );
		}
		else
		{
			char buffer[128];
			sprintf_s( buffer, 128, "Shape mode not supported. Please expand the following shape! \n\nLayer : %s \nObject : %s", sceneData.GetCurrentLayer()->GetName().Chars(), ConvertUnicodeString(artName).Chars() );

			C_MessageHandler::Instance().ShowMessage( C_MessageHandler::E_MessageType_Warning, buffer );
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

void BuildGradientDictionary( C_IpfData& sceneData )
{
	for ( S32 gradient = 0; gradient < gUsedGradientsList.Size(); gradient++ )
	{
		sceneData.AddGradient( new C_AdobeGradient( ConvertGradient( gUsedGradientsList[ gradient ] ) ) );
	}
}

void BuildPolygonShapesList( C_IpfData& sceneData )
{
	AIErr	error = 0;
	S32		layerId	= 0;

	TLayerList layers;
	if ( !FindLayers( layers ) )
		return;

	// Calculate the entire scene bounds
	Maths::C_AxisBox totalBounds;
	for ( S32 i = 0; i < layers.Size(); i++ )
	{
		CalculateGroupBounds( layers[i], totalBounds );
	}
	C_ShapeBuilder::Instance().SetSceneBounds( totalBounds );

	// Cycle each layer calculating the max scene bounds.
	for ( S32 i = 0; i < layers.Size(); i++ )
	{
		AIArtHandle pLayer = layers[i];

		if ( ConvertUnicodeString( pLayer ).Search( EXCLUDE_LAYER_NAME ) == -1 )
		{
			// Make a new layer so that groups and shapes can be added.
			sceneData.AddLayer( ConvertUnicodeString( pLayer ) );

			TraverseGroupList( sceneData, pLayer, true );
		}
	}
}


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
										  &affd, kFileFormatExport,
										  &gGlobals->fileFormatSelected );

	return error;
}


AIErr goFileFormat( AIFileFormatMessage *message ) 
{
	AIErr error = kNoErr;

	if ( message->option & kFileFormatExport ) 
	{
		ADMItemRef editBox = sADMDialog->GetItem( gGlobals->nonModalDialog, IDC_EDIT1 );
		ADMItemRef checkBox1 = sADMDialog->GetItem( gGlobals->nonModalDialog, IDC_CHECK1 );
		ADMItemRef checkBox2 = sADMDialog->GetItem( gGlobals->nonModalDialog, IDC_CHECK2 );

		F32 curveStep = ( editBox ) ? sADMItem->GetFloatValue( editBox ) : DEFAULT_CURVE_STEP;
		bool fixedSpaceSpline = ( checkBox1 ) ? ( ( sADMItem->GetBooleanValue( checkBox1 ) ) ? true : false ) : false;
		bool disableFinning = ( checkBox2 ) ? ( ( sADMItem->GetBooleanValue( checkBox2 ) ) ? true : false ) : false;

		error = CreatePolygonFile( ConvertUnicodeString( message->GetFilePath().GetFullPath() ), curveStep, fixedSpaceSpline, disableFinning );
	}

	return error;
}


/*
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

		pNewData->GetShapeBlendStyle() = C_AdobeBlendStyle( blendMode, opacity );
	}	

	// Bounding box
	F32 width = gSceneBounds.Width();
	F32 height = gSceneBounds.Height();
	bool hasControlPoints = false;

	
	F32 mul;
	if( width > height )
		mul = 1.0f / width * ILLUSTRATOR_SCALE;
	else
		mul = 1.0f / height * ILLUSTRATOR_SCALE;
	
	//F32 mulX = 1.0f / width * ILLUSTRATOR_SCALE;
	//F32	mulY = 1.0f / height * ILLUSTRATOR_SCALE;

	// Add each of the points to the polygon.
	AIPathSegment *p = segments;
	for ( short i = 0; i < count; i++, p++ )
	{
		// Knock the scene bounds off to sort out the origin.
		F32 pX = p->p.h - gSceneBounds.BoundingBox().Min().X();
		F32	pY = p->p.v - gSceneBounds.BoundingBox().Min().Y();
		pY = -pY + height;
		pX *= mul;
		pY *= mul;
		Maths::HVec2 pos = Maths::HVec2( pX, pY );

		F32 inX = p->in.h - gSceneBounds.BoundingBox().Min().X();
		F32 inY = p->in.v - gSceneBounds.BoundingBox().Min().Y();
		inY = -inY + height;
		inX *= mul;
		inY *= mul;
		Maths::HVec2 in = Maths::HVec2( inX, inY );

		F32 outX = p->out.h - gSceneBounds.BoundingBox().Min().X();
		F32 outY = p->out.v - gSceneBounds.BoundingBox().Min().Y();
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
			for(S32 i = 0; i < gUsedGradientsList.Size(); i++)
				if( style.fill.color.c.b.gradient == gUsedGradientsList[i] )
				{
					index = i;
					break;
				}

			if(index == - 1)
			{
				// Push dictionary index
				index = gUsedGradientsList.Size();
				gUsedGradientsList.PushBack( style.fill.color.c.b.gradient );
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
*/

#ifdef CFG_ALLOW_OPTIMIZE_OFF_IPF
#pragma optimize( "", on ) // CFG_ALLOW_OPTIMIZE_OFF_IPF
#endif