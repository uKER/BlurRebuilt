#include "stdafx.h"
#include "UI/Graphics/Renderer.h"
#include "UI/Shaders/Constants.h"
#include "UI/Game/Gadgets/UIStaticText.h"
#include "UI/UIDefines.h"

/*
Setting vertex shader constants:
	Use SetVertexShaderConstantF passing in the associated cv register index e.g. Display::Device()->SetVertexShaderConstantF( CV_POLYGON_TRANSFORM, (F32*)&polygonMatrix, 4 );

Setting pixel shader constants:
	Use SetPixelShaderParameterF passing in the assocated pixel shader parameter e.g Display::Device()->SetPixelShaderParameterF( ps.m_hPolygonGradientColours, m_PolygonColours[ 0 ] );
	Use SetPixelShaderParameterArrayF for arrays of pixel shader parameters
*/

// vertex shaders
EXTERN_SHADER(vsMultiTexture1)
EXTERN_SHADER(vsMultiTexture2)
EXTERN_SHADER(vsMultiTexture3)

EXTERN_SHADER(vsPolygonFlat)
EXTERN_SHADER(vsPolygonFlat_FinnedOuter)
EXTERN_SHADER(vsPolygonStroke)
EXTERN_SHADER(vsPolygonGradient)
EXTERN_SHADER(vsPolygonMultiply)
EXTERN_SHADER(vsPolygonTrail)
EXTERN_SHADER(vsPolygonPaint)

EXTERN_SHADER(vsPolygonFont)
EXTERN_SHADER(vsPolygonFont_OutlineFixed)
EXTERN_SHADER(vsPolygonFont_OutlineFade)
EXTERN_SHADER(vsPolygonFont_OutlineFixedFade)
EXTERN_SHADER(vsPolygonFont_Textured)
EXTERN_SHADER(vsPolygonFont_Gradient)
EXTERN_SHADER(vsPolygonFont_Finned)
EXTERN_SHADER(vsPolygonFont_FinnedOuter)

// pixel shaders
EXTERN_SHADER(psDefault)
EXTERN_SHADER(psOneColour)

EXTERN_SHADER(psMultiTexture1)
EXTERN_SHADER(psMultiTexture2)
EXTERN_SHADER(psMultiTexture3)
EXTERN_SHADER(psMultiTextureFullAlpha1)
EXTERN_SHADER(psMultiTextureFullAlpha2)
EXTERN_SHADER(psMultiTextureFullAlpha3)

EXTERN_SHADER(psIntensityAsAlpha)
EXTERN_SHADER(psIntensityAsAlphaTex)
EXTERN_SHADER(psIntensityAsAlphaTexNoAlpha)
EXTERN_SHADER(psIntensityAsAlphaTex2AdditiveNoAlpha)
EXTERN_SHADER(psIntensityAsAlphaInvert)
EXTERN_SHADER(psIntensityAsAlphaInvertNotOne)
EXTERN_SHADER(psIntensityCutout)

EXTERN_SHADER(psUserGammaCorrect)
EXTERN_SHADER(psUserGammaCorrect2_IAA)
EXTERN_SHADER(psGreyScale1)
EXTERN_SHADER(psGreyScale2)
EXTERN_SHADER(psGreyScale2_IAA)
EXTERN_SHADER(psMovie)
EXTERN_SHADER(psMovieWithAlpha)
EXTERN_SHADER(psIntensityLerp)

EXTERN_SHADER(psPolygonFlat)
EXTERN_SHADER(psPolygonStrokeAlpha)
EXTERN_SHADER(psPolygonStrokeIntensityAsAlpha)
EXTERN_SHADER(psPolygonMultiply)
EXTERN_SHADER(psPolygonTrail)
EXTERN_SHADER(psPolygonPaint)
EXTERN_SHADER(psPolygonGradient)
EXTERN_SHADER(psPolygonGradient_Param)
EXTERN_SHADER(psPolygonGradient_Radial)

EXTERN_SHADER(psPolygonFont)
EXTERN_SHADER(psPolygonFont_Textured)

// nightwing shaders
EXTERN_SHADER(ps_NW_FightCountdown)

// amax shaders
EXTERN_SHADER(vsPolygonStroke_AM_AudioEq)
EXTERN_SHADER(psIntensityLerp_AM_DamageBar)
EXTERN_SHADER(psIntensityLerp_AM_Perks)
EXTERN_SHADER(psAMLoading)

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", off)
#endif

#if !defined(CFG_GOLD) && defined(PLATFORM_PC)
Debug::Bool		g_debugUIFakeEndVertices( "Render.UI.FakeEndVertices", false );
#endif

Debug::Float	g_UiDebugShaderOutlineMult( "UI.Debug.ShaderOutlineMult", -1.0f, Debug::Range( -1.0f, 2.0f ), Debug::Step( 0.001f ) );

#ifndef CFG_GOLD
Debug::Bool		g_UiRenderWireframe( "UI.Render.Wireframe", false );
Debug::Int		g_UiRenderMaxIndexCount( "UI.Render.MaxIndexCount", -1, Debug::Range( -1, 256 ) );
Debug::Int		g_UiRenderMaxTriCount( "UI.Render.MaxTriangleCount", -1, Debug::Range( -1, 1024 ) );
#endif

// Annotate effects that can be used in sorcery / Lua
XT_TYPE( UI::E_Effect, "UIEffect" )
	XT_OPTION( UI::E_Effect_MultiTexture,							"MultiTexture" )
	XT_OPTION( UI::E_Effect_MultiTextureFullAlpha,					"MultiTextureFullAlpha" )

	XT_OPTION( UI::E_Effect_IntensityAsAlpha,						"IntensityAsAlpha" )
	XT_OPTION( UI::E_Effect_IntensityAsAlphaTex,					"IntensityAsAlphaTex" )
	XT_OPTION( UI::E_Effect_IntensityAsAlphaTexNoAlpha,				"IntensityAsAlphaTexNoAlpha" )
	XT_OPTION( UI::E_Effect_IntensityAsAlphaTex2AdditiveNoAlpha,	"IntensityAsAlphaTex2AdditiveNoAlpha" )

	XT_OPTION( UI::E_Effect_UserGammaCorrect,						"UserGammaCorrect" )
	XT_OPTION( UI::E_Effect_UserGammaCorrect2_IAA,					"UserGammaCorrect2_IAA" )
	XT_OPTION( UI::E_Effect_GreyScale,								"GreyScale" )
	XT_OPTION( UI::E_Effect_GreyScale2_IAA,							"GreyScale2_IAA" )
	XT_OPTION( UI::E_Effect_Movie,									"Movie" )
	XT_OPTION( UI::E_Effect_MovieWithAlpha,							"MovieWithAlpha" )
	XT_OPTION( UI::E_Effect_IntensityLerp,							"IntensityLerp" )
	XT_OPTION( UI::E_Effect_OneColour,								"OneColour" )

	XT_OPTION( UI::E_Effect_PolygonFlat,							"PolygonFlat" )
	XT_OPTION( UI::E_Effect_PolygonFlatFinnedOuter,					"PolygonFlatFinnedOuter" )
	XT_OPTION( UI::E_Effect_PolygonStrokeAlpha,						"PolygonStrokeAlpha" )
	XT_OPTION( UI::E_Effect_PolygonStrokeIntensityAsAlpha,			"PolygonStrokeIntensityAsAlpha" )
	XT_OPTION( UI::E_Effect_PolygonGradient,						"PolygonGradient" )
	XT_OPTION( UI::E_Effect_PolygonGradient_Param,					"PolygonGradientParam" )
	XT_OPTION( UI::E_Effect_PolygonGradient_Radial,					"PolygonGradientRadial" )
	XT_OPTION( UI::E_Effect_PolygonTrail,							"PolygonTrail" )

	XT_OPTION( UI::E_Effect_PolygonFont,							"PolygonFont" )
	XT_OPTION( UI::E_Effect_PolygonFontOutlineFixed,				"PolygonFontOutlineFixed" )
	XT_OPTION( UI::E_Effect_PolygonFontOutlineFade,					"PolygonFontOutlineFade" )
	XT_OPTION( UI::E_Effect_PolygonFontOutlineFixedFade,			"PolygonFontOutlineFixedFade" )
	XT_OPTION( UI::E_Effect_PolygonFontTextured,					"PolygonFontTextured" )
	XT_OPTION( UI::E_Effect_PolygonFontGradient,					"PolygonFontGradient" )
	XT_OPTION( UI::E_Effect_PolygonFontGradient_Param,				"PolygonFontGradientParam" )
	XT_OPTION( UI::E_Effect_PolygonFontGradient_Radial,				"PolygonFontGradientRadial" )
	XT_OPTION( UI::E_Effect_PolygonFontFinned,						"PolygonFontFinned" )
	XT_OPTION( UI::E_Effect_PolygonFontFinnedOuter,					"PolygonFontFinnedOuter" )

	XT_OPTION( UI::E_Effect_AM_AudioEqAlpha,						"AudioEqAlpha" )
	XT_OPTION( UI::E_Effect_AM_AudioEqIntensityAsAlpha,				"AudioEqIntensityAsAlpha" )
	XT_OPTION( UI::E_Effect_AM_Loading,								"AmaxLoading" )
XT_END_TYPE

XT_TYPE( UI::E_TextureAddressMode, "UITextureAddressMode" )
	XT_OPTION( UI::E_TextureAddressMode_Clamp, "Clamp") 
	XT_OPTION( UI::E_TextureAddressMode_Wrap, "Wrap" )
	XT_OPTION( UI::E_TextureAddressMode_Mirror, "Mirror" )
	XT_OPTION( UI::E_TextureAddressMode_Clamp0 ,"Clamp0" )
XT_END_TYPE

namespace UI
{

bool C_PixelShader::CreateParams()
{
	m_hLightPos							= GetParameter( CP_LIGHTPOS,							"g_LightPos" );
	m_hLightDir							= GetParameter( CP_LIGHTDIR,							"g_LightDir" );
	m_hLightColourFrom					= GetParameter( CP_LIGHTCOLFROM,						"g_LightColourFrom" );
	m_hLightColourTo					= GetParameter( CP_LIGHTCOLTO,							"g_LightColourTo" );
	m_hLightFallOff						= GetParameter( CP_LIGHTFALLOFF,						"g_LightFallOff" );

	m_hColourFrom						= GetParameter( CP_COLFROM,								"g_ColFrom" );
	m_hColourTo							= GetParameter( CP_COLTO,								"g_ColTo" );

	m_hPolygonGradientColours			= GetParameterArray( CP_POLYGON_GRADIENT_COLOURS,		"g_PolygonGradientColours", 4 );
	m_hPolygonGradientColourParam		= GetParameter( CP_POLYGON_GRADIENT_COLOURS_PARAM,		"g_PolygonGradientColourParam" );

	m_hSettings							= GetParameter( CP_SETTINGS,							"g_Settings" );

	return true;
}

void C_Renderer::SetLight( const C_Light& in )
{
	m_Light = in;
}

C_Renderer::C_Renderer() 
	: m_eCurrentVertexShader( E_VertexShader_Null )
{
#if 0 //def PLATFORM_PC	// TODO - check PC - seems to work - maybe these can be removed now?
	Display::C_VertexFormat vertexFormat;
	vertexFormat.AddElement(0, Display::E_VertexElementType_F32_3, Display::E_VertexElementUsage_Position, 0);
	vertexFormat.AddElement(0, Display::E_VertexElementType_Colour, Display::E_VertexElementUsage_Colour, 0);
	vertexFormat.AddElement(0, Display::E_VertexElementType_F32_2, Display::E_VertexElementUsage_TexCoord, 0);
	vertexFormat.AddElement(0, Display::E_VertexElementType_F32_2, Display::E_VertexElementUsage_TexCoord, 1);
	vertexFormat.AddElement(0, Display::E_VertexElementType_F32_2, Display::E_VertexElementUsage_TexCoord, 2);
	
	Display::C_VertexDeclaration* pVertexDeclaration = Display::Device()->CreateVertexDeclaration(vertexFormat);
	U32 uVertexStride = vertexFormat.GetStride( 0 );

	for(U32 i=0; i<E_Shader_Count; i++)
	{
		m_pVertexDeclarations[i] = pVertexDeclaration;
		m_uVertexStrides[ i ] = uVertexStride;
	}
#else // PLATFORM_PC
	Display::C_VertexFormat vertexFormats[ E_VertexShader_Count ];

	// E_VertexShader_MultiTexture1
	vertexFormats[ E_VertexShader_MultiTexture1 ].AddElement( 0, Display::E_VertexElementType_F32_3, Display::E_VertexElementUsage_Position, 0 );
	vertexFormats[ E_VertexShader_MultiTexture1 ].AddElement( 0, Display::E_VertexElementType_Colour, Display::E_VertexElementUsage_Colour, 0 );
	vertexFormats[ E_VertexShader_MultiTexture1 ].AddElement( 0, Display::E_VertexElementType_F32_2, Display::E_VertexElementUsage_TexCoord, 0 );
	m_pVertexDeclarations[ E_VertexShader_MultiTexture1 ] = Display::Device()->CreateVertexDeclaration( vertexFormats[ E_VertexShader_MultiTexture1 ] );
	m_uVertexStrides[ E_VertexShader_MultiTexture1 ] = vertexFormats[ E_VertexShader_MultiTexture1 ].GetStride( 0 );

	// E_VertexShader_MultiTexture2
	vertexFormats[ E_VertexShader_MultiTexture2 ].AddElement( 0, Display::E_VertexElementType_F32_3, Display::E_VertexElementUsage_Position, 0 );
	vertexFormats[ E_VertexShader_MultiTexture2 ].AddElement( 0, Display::E_VertexElementType_Colour, Display::E_VertexElementUsage_Colour, 0 );
	vertexFormats[ E_VertexShader_MultiTexture2 ].AddElement( 0, Display::E_VertexElementType_F32_2, Display::E_VertexElementUsage_TexCoord, 0 );
	vertexFormats[ E_VertexShader_MultiTexture2 ].AddElement( 0, Display::E_VertexElementType_F32_2, Display::E_VertexElementUsage_TexCoord, 1 );
	m_pVertexDeclarations[ E_VertexShader_MultiTexture2 ] = Display::Device()->CreateVertexDeclaration( vertexFormats[ E_VertexShader_MultiTexture2 ] );
	m_uVertexStrides[ E_VertexShader_MultiTexture2 ] = vertexFormats[ E_VertexShader_MultiTexture2 ].GetStride( 0 );

	// E_VertexShader_MultiTexture1
	vertexFormats[ E_VertexShader_MultiTexture3 ].AddElement( 0, Display::E_VertexElementType_F32_3, Display::E_VertexElementUsage_Position, 0 );
	vertexFormats[ E_VertexShader_MultiTexture3 ].AddElement( 0, Display::E_VertexElementType_Colour, Display::E_VertexElementUsage_Colour, 0 );
	vertexFormats[ E_VertexShader_MultiTexture3 ].AddElement( 0, Display::E_VertexElementType_F32_2, Display::E_VertexElementUsage_TexCoord, 0 );
	vertexFormats[ E_VertexShader_MultiTexture3 ].AddElement( 0, Display::E_VertexElementType_F32_2, Display::E_VertexElementUsage_TexCoord, 1 );
	vertexFormats[ E_VertexShader_MultiTexture3 ].AddElement( 0, Display::E_VertexElementType_F32_2, Display::E_VertexElementUsage_TexCoord, 2 );
	m_pVertexDeclarations[ E_VertexShader_MultiTexture3 ] = Display::Device()->CreateVertexDeclaration( vertexFormats[ E_VertexShader_MultiTexture3 ] );
	m_uVertexStrides[ E_VertexShader_MultiTexture3 ] = vertexFormats[ E_VertexShader_MultiTexture3 ].GetStride( 0 );

	// E_VertexShader_PolygonFlat
	vertexFormats[ E_VertexShader_PolygonFlat ].AddElement( 0, Display::E_VertexElementType_S16_2, Display::E_VertexElementUsage_Position, 0 );
	vertexFormats[ E_VertexShader_PolygonFlat ].AddElement( 0, Display::E_VertexElementType_U8_4, Display::E_VertexElementUsage_Colour, 0 );
	m_pVertexDeclarations[ E_VertexShader_PolygonFlat ] = Display::Device()->CreateVertexDeclaration( vertexFormats[ E_VertexShader_PolygonFlat ] );
	m_uVertexStrides[ E_VertexShader_PolygonFlat ] = vertexFormats[ E_VertexShader_PolygonFlat ].GetStride( 0 );
	// E_VertexShader_PolygonFlatFinnedOuter
	m_pVertexDeclarations[ E_VertexShader_PolygonFlatFinnedOuter ] = m_pVertexDeclarations[ E_VertexShader_PolygonFlat ];
	m_uVertexStrides[ E_VertexShader_PolygonFlatFinnedOuter ] = m_uVertexStrides[ E_VertexShader_PolygonFlat ];

	// E_VertexShader_PolygonStroke
	vertexFormats[ E_VertexShader_PolygonStroke ].AddElement( 0, Display::E_VertexElementType_S16_2, Display::E_VertexElementUsage_Position, 0 );
	vertexFormats[ E_VertexShader_PolygonStroke ].AddElement( 0, Display::E_VertexElementType_S16_2, Display::E_VertexElementUsage_TexCoord, 0 );
	m_pVertexDeclarations[ E_VertexShader_PolygonStroke ] = Display::Device()->CreateVertexDeclaration( vertexFormats[ E_VertexShader_PolygonStroke ] );
	m_uVertexStrides[ E_VertexShader_PolygonStroke ] = vertexFormats[ E_VertexShader_PolygonStroke ].GetStride( 0 );

	// E_VertexShader_PolygonGradient
	m_pVertexDeclarations[ E_VertexShader_PolygonGradient ] = m_pVertexDeclarations[ E_VertexShader_PolygonFlat ];
	m_uVertexStrides[ E_VertexShader_PolygonGradient ] = m_uVertexStrides[ E_VertexShader_PolygonFlat ];
	// E_VertexShader_PolygonMultiply
	m_pVertexDeclarations[ E_VertexShader_PolygonMultiply ] = m_pVertexDeclarations[ E_VertexShader_PolygonFlat ];
	m_uVertexStrides[ E_VertexShader_PolygonMultiply ] = m_uVertexStrides[ E_VertexShader_PolygonFlat ];

	// E_VertexShader_PolygonTrail
	m_pVertexDeclarations[ E_VertexShader_PolygonTrail ] = m_pVertexDeclarations[ E_VertexShader_PolygonStroke ];
	m_uVertexStrides[ E_VertexShader_PolygonTrail ] = m_uVertexStrides[ E_VertexShader_PolygonStroke ];

	// E_VertexShader_PolygonPaint
	vertexFormats[ E_VertexShader_PolygonPaint ].AddElement( 0, Display::E_VertexElementType_F32_4, Display::E_VertexElementUsage_Position, 0 );
	vertexFormats[ E_VertexShader_PolygonPaint ].AddElement( 0, Display::E_VertexElementType_Colour, Display::E_VertexElementUsage_Colour, 0 );
	vertexFormats[ E_VertexShader_PolygonPaint ].AddElement( 0, Display::E_VertexElementType_F32_2, Display::E_VertexElementUsage_TexCoord, 0 );
	m_pVertexDeclarations[ E_VertexShader_PolygonPaint ] = Display::Device()->CreateVertexDeclaration( vertexFormats[ E_VertexShader_PolygonPaint ] );
	m_uVertexStrides[ E_VertexShader_PolygonPaint ] = vertexFormats[ E_VertexShader_PolygonPaint ].GetStride( 0 );

	// E_VertexShader_PolygonFont
	vertexFormats[ E_VertexShader_PolygonFont ].AddElement( 0, Display::E_VertexElementType_S16_2, Display::E_VertexElementUsage_Position, 0 );
	vertexFormats[ E_VertexShader_PolygonFont ].AddElement( 0, Display::E_VertexElementType_U8_4, Display::E_VertexElementUsage_Colour, 0 );
	m_pVertexDeclarations[ E_VertexShader_PolygonFont ] = Display::Device()->CreateVertexDeclaration( vertexFormats[ E_VertexShader_PolygonFont ] );
	m_uVertexStrides[ E_VertexShader_PolygonFont ] = vertexFormats[ E_VertexShader_PolygonFont ].GetStride( 0 );

	// E_VertexShader_PolygonFontOutlineFixed
	m_pVertexDeclarations[ E_VertexShader_PolygonFontOutlineFixed ] = Display::Device()->CreateVertexDeclaration( vertexFormats[ E_VertexShader_PolygonFont ] );
	m_uVertexStrides[ E_VertexShader_PolygonFontOutlineFixed ] = vertexFormats[ E_VertexShader_PolygonFont ].GetStride( 0 );
	// E_VertexShader_PolygonFontOutlineFade
	m_pVertexDeclarations[ E_VertexShader_PolygonFontOutlineFade ] = Display::Device()->CreateVertexDeclaration( vertexFormats[ E_VertexShader_PolygonFont ] );
	m_uVertexStrides[ E_VertexShader_PolygonFontOutlineFade ] = vertexFormats[ E_VertexShader_PolygonFont ].GetStride( 0 );
	// E_VertexShader_PolygonFontOutlineFixedFade
	m_pVertexDeclarations[ E_VertexShader_PolygonFontOutlineFixedFade ] = Display::Device()->CreateVertexDeclaration( vertexFormats[ E_VertexShader_PolygonFont ] );
	m_uVertexStrides[ E_VertexShader_PolygonFontOutlineFixedFade ] = vertexFormats[ E_VertexShader_PolygonFont ].GetStride( 0 );
	// E_VertexShader_PolygonFontTextured
	m_pVertexDeclarations[ E_VertexShader_PolygonFontTextured ] = Display::Device()->CreateVertexDeclaration( vertexFormats[ E_VertexShader_PolygonFont ] );
	m_uVertexStrides[ E_VertexShader_PolygonFontTextured ] = vertexFormats[ E_VertexShader_PolygonFont ].GetStride( 0 );
	// E_VertexShader_PolygonFontGradient
	m_pVertexDeclarations[ E_VertexShader_PolygonFontGradient ] = Display::Device()->CreateVertexDeclaration( vertexFormats[ E_VertexShader_PolygonFont ] );
	m_uVertexStrides[ E_VertexShader_PolygonFontGradient ] = vertexFormats[ E_VertexShader_PolygonFont ].GetStride( 0 );
	// E_VertexShader_PolygonFontFinned
	m_pVertexDeclarations[ E_VertexShader_PolygonFontFinned ] = Display::Device()->CreateVertexDeclaration( vertexFormats[ E_VertexShader_PolygonFont ] );
	m_uVertexStrides[ E_VertexShader_PolygonFontFinned ] = vertexFormats[ E_VertexShader_PolygonFont ].GetStride( 0 );
	// E_VertexShader_PolygonFontFinnedOuter
	m_pVertexDeclarations[ E_VertexShader_PolygonFontFinnedOuter ] = Display::Device()->CreateVertexDeclaration( vertexFormats[ E_VertexShader_PolygonFont ] );
	m_uVertexStrides[ E_VertexShader_PolygonFontFinnedOuter ] = vertexFormats[ E_VertexShader_PolygonFont ].GetStride( 0 );

	// Amax shaders
	m_pVertexDeclarations[ E_VertexShader_AM_AudioEq ] = m_pVertexDeclarations[ E_VertexShader_PolygonStroke ];
	m_uVertexStrides[ E_VertexShader_AM_AudioEq ] = m_uVertexStrides[ E_VertexShader_PolygonStroke ];

#endif // PLATFORM_PC

	// clear shader usage flags
	for( S32 i = 0; i < m_ShaderFlags.Size(); i++ )
		m_ShaderFlags[ i ] = (E_ShaderUsageFlags)0;

	// shader flags
	m_ShaderFlags[ E_Effect_IntensityCutout ]				= E_ShaderUsageFlags_Settings;
	m_ShaderFlags[ E_Effect_UserGammaCorrect ]				= E_ShaderUsageFlags_Settings;
	m_ShaderFlags[ E_Effect_UserGammaCorrect2_IAA ]			= E_ShaderUsageFlags_Settings;

	m_ShaderFlags[ E_Effect_PolygonFlat	]					= E_ShaderUsageFlags_Polygon | E_ShaderUsageFlags_PolygonColour | E_ShaderUsageFlags_NotTextured;
	m_ShaderFlags[ E_Effect_PolygonFlatFinnedOuter ]		= m_ShaderFlags[ E_Effect_PolygonFlat ];
	m_ShaderFlags[ E_Effect_PolygonStrokeAlpha ]			= E_ShaderUsageFlags_Polygon | E_ShaderUsageFlags_PolygonColour;
	m_ShaderFlags[ E_Effect_PolygonStrokeIntensityAsAlpha ]	= E_ShaderUsageFlags_Polygon | E_ShaderUsageFlags_PolygonColour;
	m_ShaderFlags[ E_Effect_PolygonMultiply ]				= E_ShaderUsageFlags_Polygon | E_ShaderUsageFlags_PolygonColour | E_ShaderUsageFlags_GradientBounds | E_ShaderUsageFlags_MultiplyBounds;
	m_ShaderFlags[ E_Effect_PolygonGradient ]				= E_ShaderUsageFlags_Polygon | E_ShaderUsageFlags_PolygonColour | E_ShaderUsageFlags_GradientBounds;
	m_ShaderFlags[ E_Effect_PolygonGradient_Param ]			= m_ShaderFlags[ E_Effect_PolygonGradient ];
	m_ShaderFlags[ E_Effect_PolygonGradient_Radial ]		= m_ShaderFlags[ E_Effect_PolygonGradient ];
	m_ShaderFlags[ E_Effect_PolygonTrail ]					= E_ShaderUsageFlags_Polygon | E_ShaderUsageFlags_PolygonColour;
	m_ShaderFlags[ E_Effect_PolygonPaint ]					= E_ShaderUsageFlags_Polygon;

	m_ShaderFlags[ E_Effect_PolygonFont ]					= E_ShaderUsageFlags_Polygon | E_ShaderUsageFlags_PolygonColour | E_ShaderUsageFlags_PolygonFont | E_ShaderUsageFlags_NotTextured;
	m_ShaderFlags[ E_Effect_PolygonFontOutlineFixed ]		= m_ShaderFlags[ E_Effect_PolygonFont ];
	m_ShaderFlags[ E_Effect_PolygonFontOutlineFade ]		= m_ShaderFlags[ E_Effect_PolygonFont ];
	m_ShaderFlags[ E_Effect_PolygonFontOutlineFixedFade ]	= m_ShaderFlags[ E_Effect_PolygonFont ];
	m_ShaderFlags[ E_Effect_PolygonFontTextured ]			= E_ShaderUsageFlags_Polygon | E_ShaderUsageFlags_PolygonColour | E_ShaderUsageFlags_PolygonFont;
	m_ShaderFlags[ E_Effect_PolygonFontGradient ]			= E_ShaderUsageFlags_Polygon | E_ShaderUsageFlags_PolygonColour | E_ShaderUsageFlags_PolygonFont | E_ShaderUsageFlags_GradientBounds;
	m_ShaderFlags[ E_Effect_PolygonFontGradient_Param ]		= m_ShaderFlags[ E_Effect_PolygonFontGradient ];
	m_ShaderFlags[ E_Effect_PolygonFontGradient_Radial ]	= m_ShaderFlags[ E_Effect_PolygonFontGradient ];
	m_ShaderFlags[ E_Effect_PolygonFontFinned ]				= m_ShaderFlags[ E_Effect_PolygonFont ];
	m_ShaderFlags[ E_Effect_PolygonFontFinnedOuter ]		= m_ShaderFlags[ E_Effect_PolygonFont ];

	// Amax effects
	m_ShaderFlags[ E_Effect_AM_Perks ]						= E_ShaderUsageFlags_Settings;
	m_ShaderFlags[ E_Effect_AM_Loading ]					= E_ShaderUsageFlags_Settings;
	m_ShaderFlags[ E_Effect_AM_AudioEqAlpha ]				= m_ShaderFlags[ E_Effect_PolygonStrokeAlpha ];
	m_ShaderFlags[ E_Effect_AM_AudioEqIntensityAsAlpha ]	= m_ShaderFlags[ E_Effect_PolygonStrokeIntensityAsAlpha ];

	// clear shader mapping
	for ( S32 i = 0; i < E_Effect_Count; i++ )
		m_ShaderMappings[ i ] = 0;

	// shader mappings
	m_ShaderMappings[ E_Effect_Default ]								= _SHADER_MAPPING( E_VertexShader_MultiTexture1, E_PixelShader_Default );

	m_ShaderMappings[ E_Effect_OneColour ]								= _SHADER_MAPPING( E_VertexShader_MultiTexture1, E_PixelShader_OneColour );
	m_ShaderMappings[ E_Effect_MultiTexture ]							= _SHADER_MAPPING( E_VertexShader_MultiTexture1, E_PixelShader_MultiTexture1 );
	m_ShaderMappings[ E_Effect_MultiTextureFullAlpha ]					= _SHADER_MAPPING( E_VertexShader_MultiTexture1, E_PixelShader_MultiTextureFullAlpha1 );

	m_ShaderMappings[ E_Effect_IntensityAsAlpha ]						= _SHADER_MAPPING( E_VertexShader_MultiTexture1, E_PixelShader_IntensityAsAlpha );
	m_ShaderMappings[ E_Effect_IntensityAsAlphaInvert ]					= _SHADER_MAPPING( E_VertexShader_MultiTexture1, E_PixelShader_IntensityAsAlphaInvert );
	m_ShaderMappings[ E_Effect_IntensityAsAlphaInvertNotOne ]			= _SHADER_MAPPING( E_VertexShader_MultiTexture1, E_PixelShader_IntensityAsAlphaInvertNotOne );
	m_ShaderMappings[ E_Effect_IntensityCutout ]						= _SHADER_MAPPING( E_VertexShader_MultiTexture2, E_PixelShader_IntensityCutout );
	m_ShaderMappings[ E_Effect_IntensityAsAlphaTex ]					= _SHADER_MAPPING( E_VertexShader_MultiTexture2, E_PixelShader_IntensityAsAlphaTex );
	m_ShaderMappings[ E_Effect_IntensityAsAlphaTexNoAlpha ]				= _SHADER_MAPPING( E_VertexShader_MultiTexture2, E_PixelShader_IntensityAsAlphaTexNoAlpha );
	m_ShaderMappings[ E_Effect_IntensityAsAlphaTex2AdditiveNoAlpha ]	= _SHADER_MAPPING( E_VertexShader_MultiTexture3, E_PixelShader_IntensityAsAlphaTex2AdditiveNoAlpha );

	m_ShaderMappings[ E_Effect_UserGammaCorrect ]						= _SHADER_MAPPING( E_VertexShader_MultiTexture1, E_PixelShader_UserGammaCorrect );
	m_ShaderMappings[ E_Effect_UserGammaCorrect2_IAA ]					= _SHADER_MAPPING( E_VertexShader_MultiTexture2, E_PixelShader_UserGammaCorrect2_IAA );
	m_ShaderMappings[ E_Effect_GreyScale ]								= _SHADER_MAPPING( E_VertexShader_MultiTexture1, E_PixelShader_GreyScale );
	m_ShaderMappings[ E_Effect_GreyScale2_IAA ]							= _SHADER_MAPPING( E_VertexShader_MultiTexture2, E_PixelShader_GreyScale2_IAA );
	m_ShaderMappings[ E_Effect_Movie ]									= _SHADER_MAPPING( E_VertexShader_MultiTexture1, E_PixelShader_Movie );
	m_ShaderMappings[ E_Effect_MovieWithAlpha ]							= _SHADER_MAPPING( E_VertexShader_MultiTexture2, E_PixelShader_MovieWithAlpha );
	m_ShaderMappings[ E_Effect_IntensityLerp ]							= _SHADER_MAPPING( E_VertexShader_MultiTexture1, E_PixelShader_IntensityLerp );

	m_ShaderMappings[ E_Effect_PolygonFlat ]							= _SHADER_MAPPING( E_VertexShader_PolygonFlat, E_PixelShader_PolygonFlat );
	m_ShaderMappings[ E_Effect_PolygonFlatFinnedOuter ]					= _SHADER_MAPPING( E_VertexShader_PolygonFlatFinnedOuter, E_PixelShader_PolygonFlat );
	m_ShaderMappings[ E_Effect_PolygonStrokeAlpha ]						= _SHADER_MAPPING( E_VertexShader_PolygonStroke, E_PixelShader_PolygonStrokeAlpha );
	m_ShaderMappings[ E_Effect_PolygonStrokeIntensityAsAlpha ]			= _SHADER_MAPPING( E_VertexShader_PolygonStroke, E_PixelShader_PolygonStrokeIntensityAsAlpha );
	m_ShaderMappings[ E_Effect_PolygonMultiply ]						= _SHADER_MAPPING( E_VertexShader_PolygonMultiply, E_PixelShader_PolygonMultiply );
	m_ShaderMappings[ E_Effect_PolygonGradient ]						= _SHADER_MAPPING( E_VertexShader_PolygonGradient, E_PixelShader_PolygonGradient );
	m_ShaderMappings[ E_Effect_PolygonGradient_Param ]					= _SHADER_MAPPING( E_VertexShader_PolygonGradient, E_PixelShader_PolygonGradientParam );
	m_ShaderMappings[ E_Effect_PolygonGradient_Radial ]					= _SHADER_MAPPING( E_VertexShader_PolygonGradient, E_PixelShader_PolygonGradientRadial );
	m_ShaderMappings[ E_Effect_PolygonTrail ]							= _SHADER_MAPPING( E_VertexShader_PolygonTrail, E_PixelShader_PolygonTrail );
	m_ShaderMappings[ E_Effect_PolygonPaint ]							= _SHADER_MAPPING( E_VertexShader_PolygonPaint, E_PixelShader_PolygonPaint );

	m_ShaderMappings[ E_Effect_PolygonFont ]							= _SHADER_MAPPING( E_VertexShader_PolygonFont, E_PixelShader_PolygonFont );
	m_ShaderMappings[ E_Effect_PolygonFontOutlineFixed ]				= _SHADER_MAPPING( E_VertexShader_PolygonFontOutlineFixed, E_PixelShader_PolygonFont );
	m_ShaderMappings[ E_Effect_PolygonFontOutlineFade ]					= _SHADER_MAPPING( E_VertexShader_PolygonFontOutlineFade, E_PixelShader_PolygonFont );
	m_ShaderMappings[ E_Effect_PolygonFontOutlineFixedFade ]			= _SHADER_MAPPING( E_VertexShader_PolygonFontOutlineFixedFade, E_PixelShader_PolygonFont );
	m_ShaderMappings[ E_Effect_PolygonFontTextured ]					= _SHADER_MAPPING( E_VertexShader_PolygonFontTextured, E_PixelShader_PolygonFontTextured );
	m_ShaderMappings[ E_Effect_PolygonFontGradient ]					= _SHADER_MAPPING( E_VertexShader_PolygonFontGradient, E_PixelShader_PolygonGradient );
	m_ShaderMappings[ E_Effect_PolygonFontGradient_Param ]				= _SHADER_MAPPING( E_VertexShader_PolygonFontGradient, E_PixelShader_PolygonGradientParam );
	m_ShaderMappings[ E_Effect_PolygonFontGradient_Radial ]				= _SHADER_MAPPING( E_VertexShader_PolygonFontGradient, E_PixelShader_PolygonGradientRadial );
	m_ShaderMappings[ E_Effect_PolygonFontFinned ]						= _SHADER_MAPPING( E_VertexShader_PolygonFontFinned, E_PixelShader_PolygonFont );
	m_ShaderMappings[ E_Effect_PolygonFontFinnedOuter ]					= _SHADER_MAPPING( E_VertexShader_PolygonFontFinnedOuter, E_PixelShader_PolygonFont );

	// Nightwing effects
	m_ShaderMappings[ E_Effect_NW_FightCountdown ]						= _SHADER_MAPPING( E_VertexShader_MultiTexture1, E_PixelShader_NW_FightCountdown );

	// Amax effects
	m_ShaderMappings[ E_Effect_AM_Perks ]								= _SHADER_MAPPING( E_VertexShader_MultiTexture1, E_PixelShader_AM_Perks );
	m_ShaderMappings[ E_Effect_AM_Loading ]								= _SHADER_MAPPING( E_VertexShader_MultiTexture1, E_PixelShader_AM_Loading );
	m_ShaderMappings[ E_Effect_AM_DamageBar ]							= _SHADER_MAPPING( E_VertexShader_MultiTexture2, E_PixelShader_AM_DamageBar );
	m_ShaderMappings[ E_Effect_AM_AudioEqAlpha ]						= _SHADER_MAPPING( E_VertexShader_AM_AudioEq, E_PixelShader_PolygonStrokeAlpha );
	m_ShaderMappings[ E_Effect_AM_AudioEqIntensityAsAlpha ]				= _SHADER_MAPPING( E_VertexShader_AM_AudioEq, E_PixelShader_PolygonStrokeIntensityAsAlpha );

	m_CurrentVertexStride = -1;
}

C_Renderer::~C_Renderer()
{
	//
}

bool C_Renderer::Create()
{
	RESOURCE_MEMORY_ID("UI");

	// vertex shaders
	_CREATE_VS( "Shaders/vsMultiTexture1", E_VertexShader_MultiTexture1, vsMultiTexture1 );
	_CREATE_VS( "Shaders/vsMultiTexture2", E_VertexShader_MultiTexture2, vsMultiTexture2 );
	_CREATE_VS( "Shaders/vsMultiTexture3", E_VertexShader_MultiTexture3, vsMultiTexture3 );

	_CREATE_VS( "Shaders/vsPolygonFlat", E_VertexShader_PolygonFlat, vsPolygonFlat );
	_CREATE_VS( "Shaders/vsPolygonFlat_FinnedOuter", E_VertexShader_PolygonFlatFinnedOuter, vsPolygonFlat_FinnedOuter );
	_CREATE_VS( "Shaders/vsPolygonStroke", E_VertexShader_PolygonStroke, vsPolygonStroke );
	_CREATE_VS( "Shaders/vsPolygonGradient", E_VertexShader_PolygonGradient, vsPolygonGradient );
	_CREATE_VS( "Shaders/vsPolygonMultiply", E_VertexShader_PolygonMultiply, vsPolygonMultiply );
	_CREATE_VS( "Shaders/vsPolygonTrail", E_VertexShader_PolygonTrail, vsPolygonTrail );
	_CREATE_VS( "Shaders/vsPolygonPaint", E_VertexShader_PolygonPaint, vsPolygonPaint );

	_CREATE_VS( "Shaders/vsPolygonFont", E_VertexShader_PolygonFont, vsPolygonFont );
	_CREATE_VS( "Shaders/vsPolygonFont_OutlineFixed", E_VertexShader_PolygonFontOutlineFixed, vsPolygonFont_OutlineFixed );
	_CREATE_VS( "Shaders/vsPolygonFont_OutlineFade", E_VertexShader_PolygonFontOutlineFade, vsPolygonFont_OutlineFade );
	_CREATE_VS( "Shaders/vsPolygonFont_OutlineFixedFade", E_VertexShader_PolygonFontOutlineFixedFade, vsPolygonFont_OutlineFixedFade );
	_CREATE_VS( "Shaders/vsPolygonFont_Textured", E_VertexShader_PolygonFontTextured, vsPolygonFont_Textured );
	_CREATE_VS( "Shaders/vsPolygonFont_Gradient", E_VertexShader_PolygonFontGradient, vsPolygonFont_Gradient );
	_CREATE_VS( "Shaders/vsPolygonFont_Finned", E_VertexShader_PolygonFontFinned, vsPolygonFont_Finned );
	_CREATE_VS( "Shaders/vsPolygonFont_FinnedOuter", E_VertexShader_PolygonFontFinnedOuter, vsPolygonFont_FinnedOuter );

	// pixel shaders
	_CREATE_PS( "Shaders/psDefault", E_PixelShader_Default, psDefault );

	_CREATE_PS( "Shaders/psOneColour", E_PixelShader_OneColour, psOneColour );
	_CREATE_PS( "Shaders/psMultiTexture1", E_PixelShader_MultiTexture1, psMultiTexture1 );
	_CREATE_PS( "Shaders/psMultiTexture2", E_PixelShader_MultiTexture2, psMultiTexture2 );
	_CREATE_PS( "Shaders/psMultiTexture3", E_PixelShader_MultiTexture3, psMultiTexture3 );

	_CREATE_PS( "Shaders/psMultiTextureFullAlpha1", E_PixelShader_MultiTextureFullAlpha1, psMultiTextureFullAlpha1 );
	_CREATE_PS( "Shaders/psMultiTextureFullAlpha2", E_PixelShader_MultiTextureFullAlpha2, psMultiTextureFullAlpha2 );
	_CREATE_PS( "Shaders/psMultiTextureFullAlpha3", E_PixelShader_MultiTextureFullAlpha3, psMultiTextureFullAlpha3 );

	_CREATE_PS( "Shaders/psIntensityAsAlpha", E_PixelShader_IntensityAsAlpha, psIntensityAsAlpha );
	_CREATE_PS( "Shaders/psIntensityAsAlphaTex", E_PixelShader_IntensityAsAlphaTex, psIntensityAsAlphaTex );
	_CREATE_PS( "Shaders/psIntensityAsAlphaTexNoAlpha", E_PixelShader_IntensityAsAlphaTexNoAlpha, psIntensityAsAlphaTexNoAlpha );
	_CREATE_PS( "Shaders/psIntensityAsAlphaTex2AdditiveNoAlpha", E_PixelShader_IntensityAsAlphaTex2AdditiveNoAlpha, psIntensityAsAlphaTex2AdditiveNoAlpha );
	_CREATE_PS( "Shaders/psIntensityAsAlphaInvert", E_PixelShader_IntensityAsAlphaInvert, psIntensityAsAlphaInvert );
	_CREATE_PS( "Shaders/psIntensityAsAlphaInvertNotOne", E_PixelShader_IntensityAsAlphaInvertNotOne, psIntensityAsAlphaInvertNotOne );
	_CREATE_PS( "Shaders/psIntensityCutout", E_PixelShader_IntensityCutout, psIntensityCutout );

	_CREATE_PS( "Shaders/psGreyScale", E_PixelShader_GreyScale, psGreyScale1 );
	_CREATE_PS( "Shaders/psUserGammaCorrect", E_PixelShader_UserGammaCorrect, psUserGammaCorrect );
	_CREATE_PS( "Shaders/psUserGammaCorrect2_IAA", E_PixelShader_UserGammaCorrect2_IAA, psUserGammaCorrect2_IAA );
	_CREATE_PS( "Shaders/psGreyScale2", E_PixelShader_GreyScale2, psGreyScale2 );
	_CREATE_PS( "Shaders/psGreyScale2_IAA", E_PixelShader_GreyScale2_IAA, psGreyScale2_IAA );
	_CREATE_PS( "Shaders/psMovie", E_PixelShader_Movie, psMovie );
	_CREATE_PS( "Shaders/psMovieWithAlpha", E_PixelShader_MovieWithAlpha, psMovieWithAlpha );
	_CREATE_PS( "Shaders/psIntensityLerp", E_PixelShader_IntensityLerp, psIntensityLerp );

	_CREATE_PS( "Shaders/psPolygonFlat", E_PixelShader_PolygonFlat, psPolygonFlat );
	_CREATE_PS( "Shaders/psPolygonStrokeAlpha", E_PixelShader_PolygonStrokeAlpha, psPolygonStrokeAlpha );
	_CREATE_PS( "Shaders/psPolygonStrokeIntensityAsAlpha", E_PixelShader_PolygonStrokeIntensityAsAlpha, psPolygonStrokeIntensityAsAlpha );
	_CREATE_PS( "Shaders/psPolygonMultiply", E_PixelShader_PolygonMultiply, psPolygonMultiply );
	_CREATE_PS( "Shaders/psPolygonTrail", E_PixelShader_PolygonTrail, psPolygonTrail );
	_CREATE_PS( "Shaders/psPolygonPaint", E_PixelShader_PolygonPaint, psPolygonPaint );

	_CREATE_PS( "Shaders/psPolygonGradient", E_PixelShader_PolygonGradient, psPolygonGradient );
	_CREATE_PS( "Shaders/psPolygonGradient_Param", E_PixelShader_PolygonGradientParam, psPolygonGradient_Param );
	_CREATE_PS( "Shaders/psPolygonGradient_Radial", E_PixelShader_PolygonGradientRadial, psPolygonGradient_Radial );

	_CREATE_PS( "Shaders/psPolygonFont", E_PixelShader_PolygonFont, psPolygonFont );
	_CREATE_PS( "Shaders/psPolygonFont_Textured", E_PixelShader_PolygonFontTextured, psPolygonFont_Textured );

	// Nightwing shaders
	_CREATE_PS( "Shaders/ps_NW_FightCountdown", E_PixelShader_NW_FightCountdown, ps_NW_FightCountdown );

	// Amax shaders
	_CREATE_VS( "Shaders/vsPolygonStroke_AM_AudioEq", E_VertexShader_AM_AudioEq, vsPolygonStroke_AM_AudioEq );
	_CREATE_PS( "Shaders/psIntensityLerp_AM_DamageBar", E_PixelShader_AM_DamageBar, psIntensityLerp_AM_DamageBar );
	_CREATE_PS( "Shaders/psIntensityLerp_AM_Perks", E_PixelShader_AM_Perks, psIntensityLerp_AM_Perks );
	_CREATE_PS( "Shaders/psAMLoading", E_PixelShader_AM_Loading, psAMLoading );

	for(U32 i=0; i<E_PixelShader_Count; i++)
		m_PixelShaders[i].CreateParams();

	return true;
}

void C_Renderer::Destroy()
{
	for ( U32 i = 0; i < E_VertexShader_Count; i++ )
		Display::Device()->Destroy( &m_VertexShaders[ i ] );

	for ( U32 i = 0; i < E_PixelShader_Count; i++ )
		Display::Device()->Destroy( &m_PixelShaders[ i ] );
}

void C_Renderer::Bind()
{
	Display::Device()->SetTexture(0, NULL);
	Display::Device()->SetTextureFilterMag(0, Display::E_TextureFilter_Linear);
	Display::Device()->SetTextureFilterMin(0, Display::E_TextureFilter_Anisotropic);
	Display::Device()->SetTextureFilterMip(0, Display::E_TextureFilter_Linear);
	Display::Device()->SetTextureAddressU(0, Display::E_TextureAddress_Wrap);
	Display::Device()->SetTextureAddressV(0, Display::E_TextureAddress_Wrap);
	Display::Device()->SetTextureLodBias(0, 0);

	Display::Device()->SetTexture(1, NULL);
	Display::Device()->SetTextureFilterMag(1, Display::E_TextureFilter_Linear);
	Display::Device()->SetTextureFilterMin(1, Display::E_TextureFilter_Anisotropic);
	Display::Device()->SetTextureFilterMip(1, Display::E_TextureFilter_Linear);
	Display::Device()->SetTextureAddressU(1, Display::E_TextureAddress_Wrap);
	Display::Device()->SetTextureAddressV(1, Display::E_TextureAddress_Wrap);
	Display::Device()->SetTextureLodBias(1, 0);

	Display::Device()->SetTexture(2, NULL);
	Display::Device()->SetTextureFilterMag(2, Display::E_TextureFilter_Linear);
	Display::Device()->SetTextureFilterMin(2, Display::E_TextureFilter_Anisotropic);
	Display::Device()->SetTextureFilterMip(2, Display::E_TextureFilter_Linear);
	Display::Device()->SetTextureAddressU(2, Display::E_TextureAddress_Wrap);
	Display::Device()->SetTextureAddressV(2, Display::E_TextureAddress_Wrap);
	Display::Device()->SetTextureLodBias(2, 0);
}

void C_Renderer::Unbind()
{
	m_eCurrentVertexShader = E_VertexShader_Null;
}

const bool C_Renderer::BeginVerticesLocked() const
{
	return Display::Device()->BeginVerticesLocked();
}

void* C_Renderer::BeginVertices( const Display::E_PrimitiveType ePrimitiveType, const U32 uVertexCount, const U32 uVertexStride )
{
	assert( m_eCurrentVertexShader < E_VertexShader_Count );
	return Display::Device()->BeginVertices( ePrimitiveType, uVertexCount, uVertexStride, m_pVertexDeclarations[ m_eCurrentVertexShader ], false ); // this must not fail
}

void C_Renderer::EndVertices( void* pVertex )
{
	#if PLATFORM_PS3
	if(!Display::Device()->GetRenderTarget().GetDepthStencilSurface() || !Display::Device()->GetRenderTarget().GetDepthStencilSurface()->IsCreated())
		Display::Device()->SetStencilEnable(Display::E_Bool_False); // No depth buffer plus stenciling enabled crashes the rsx...we need a cleaner fix than this!
	#endif

	#if !defined(CFG_GOLD) && defined(PLATFORM_PC)
	if ( g_debugUIFakeEndVertices )
		Display::Device()->FakeEndVertices( pVertex );
	else
	#endif
		Display::Device()->EndVertices( pVertex );
}

const U32 C_Renderer::SetEffect( const E_Effect eEffect, const U32 uSamplerCount, const E_TextureAddressMode addressMode )
{
	assert( eEffect < E_Effect_Count );
	
	// get the shader enums
	E_VertexShader eVertexShader = (E_VertexShader)_GET_VS( m_ShaderMappings[ eEffect ] );
	E_PixelShader ePixelShader = (E_PixelShader)_GET_PS( m_ShaderMappings[ eEffect ] );

	// check for shader override
	switch ( eEffect )
	{
		case E_Effect_MultiTexture:
			if ( uSamplerCount > 2 )		ePixelShader = E_PixelShader_MultiTexture3;
			else if ( uSamplerCount > 1 )	ePixelShader = E_PixelShader_MultiTexture2;
			else							ePixelShader = E_PixelShader_MultiTexture1;

			if ( uSamplerCount > 2 )		{ eVertexShader = E_VertexShader_MultiTexture3; } 
			else if ( uSamplerCount > 1 )	{ eVertexShader = E_VertexShader_MultiTexture2; }
			else							{ eVertexShader = E_VertexShader_MultiTexture1; }

			break;

		case E_Effect_MultiTextureFullAlpha:
			if ( uSamplerCount > 2 )		{ ePixelShader = E_PixelShader_MultiTextureFullAlpha3; }
			else if ( uSamplerCount > 1 )	{ ePixelShader = E_PixelShader_MultiTextureFullAlpha2; }
			else							{ ePixelShader = E_PixelShader_MultiTextureFullAlpha1; }

			if ( uSamplerCount > 2 )		eVertexShader = E_VertexShader_MultiTexture3;
			else if ( uSamplerCount > 1 )	eVertexShader = E_VertexShader_MultiTexture2;
			else							eVertexShader = E_VertexShader_MultiTexture1;

			break;

		case E_Effect_GreyScale:
			if ( uSamplerCount > 1 )		{ ePixelShader = E_PixelShader_GreyScale2; }
			else							{ ePixelShader = E_PixelShader_GreyScale; }

			if ( uSamplerCount > 1 )		eVertexShader = E_VertexShader_MultiTexture2;
			else							eVertexShader = E_VertexShader_MultiTexture1;
			break;

		default:
			break;
	}

	const Display::C_VertexShader& vs = m_VertexShaders[ eVertexShader ];
	const C_PixelShader& ps = m_PixelShaders[ ePixelShader ];

	// texture addressing
	switch( addressMode )
	{
		case E_TextureAddressMode_Clamp0:
			Display::Device()->SetTextureAddressU( 0, Display::E_TextureAddress_Clamp );
			Display::Device()->SetTextureAddressV( 0, Display::E_TextureAddress_Clamp );
			Display::Device()->SetTextureAddressU( 1, Display::E_TextureAddress_Wrap );
			Display::Device()->SetTextureAddressV( 1, Display::E_TextureAddress_Wrap );
			break;
		case E_TextureAddressMode_Wrap:
			Display::Device()->SetTextureAddressU( 0, Display::E_TextureAddress_Wrap );
			Display::Device()->SetTextureAddressV( 0, Display::E_TextureAddress_Wrap );
			Display::Device()->SetTextureAddressU( 1, Display::E_TextureAddress_Wrap );
			Display::Device()->SetTextureAddressV( 1, Display::E_TextureAddress_Wrap );
			break;
		case E_TextureAddressMode_Clamp:
			Display::Device()->SetTextureAddressU( 0, Display::E_TextureAddress_Clamp );
			Display::Device()->SetTextureAddressV( 0, Display::E_TextureAddress_Clamp );
			Display::Device()->SetTextureAddressU( 1, Display::E_TextureAddress_Clamp );
			Display::Device()->SetTextureAddressV( 1, Display::E_TextureAddress_Clamp );
			break;
		case E_TextureAddressMode_Mirror:
			Display::Device()->SetTextureAddressU( 0, Display::E_TextureAddress_Mirror );
			Display::Device()->SetTextureAddressV( 0, Display::E_TextureAddress_Mirror );
			Display::Device()->SetTextureAddressU( 1, Display::E_TextureAddress_Mirror );
			Display::Device()->SetTextureAddressV( 1, Display::E_TextureAddress_Mirror );
			break;
		default:
			LOGWARN( g_Log, DBSTR "Invalid texture address mode" );
			break;
	}

	// common polygon settings based on the shader flags
	if ( m_ShaderFlags[ eEffect ] & E_ShaderUsageFlags_Polygon )
	{
#ifdef PLATFORM_PC
		//m_PolygonMatrix.Transpose();	// this fixes PC rendering for now - but meh
#endif
		Display::C_Matrix polygonMatrix;
		Maths::Store(m_PolygonMatrix, &polygonMatrix);
		Display::Device()->SetVertexShaderConstantF( CV_POLYGON_TRANSFORM, (F32*)&polygonMatrix, 4 );
		//Display::Device()->SetVertexShaderConstantF( CV_POLYGON_TRANSFORM, (F32*)&m_PolygonMatrix, 4 );
	}
	if ( m_ShaderFlags[ eEffect ] & E_ShaderUsageFlags_PolygonColour )
	{
		Display::Device()->SetVertexShaderConstantF( CV_POLYGON_COLOUR, m_PolygonColours[ 0 ], 1 );
	}
	if ( m_ShaderFlags[ eEffect ] & E_ShaderUsageFlags_PolygonFont )
	{
		Display::Device()->SetVertexShaderConstantF( CV_POLYGONFONT_OUTLINE_SETTINGS, (F32*)&m_PolygonFontSettings, 1 );
	}
	if ( m_ShaderFlags[ eEffect ] & E_ShaderUsageFlags_GradientBounds )
	{
		Display::C_Matrix polygonMatrix;
		Maths::Store(m_PolygonColourMatrix, &polygonMatrix);

		Display::Device()->SetVertexShaderConstantF( CV_POLYGON_COLOUR_TRANSFORM, (F32*)&polygonMatrix, 4 );
		Display::Device()->SetPixelShaderParameterArrayF( ps.m_hPolygonGradientColours, m_PolygonColours[ 0 ] );
	}

	// specific vertex shader params
	switch ( eVertexShader )
	{
		case E_VertexShader_PolygonStroke:
		{
			Display::Device()->SetVertexShaderConstantF( CV_POLYGON_TEXTURE_VECTOR, m_TextureVector, 1 );
			break;
		}
		case E_VertexShader_AM_AudioEq:
		{
			Display::Device()->SetVertexShaderConstantF( CV_POLYGON_TEXTURE_VECTOR, m_TextureVector, 1 );
			Display::Device()->SetVertexShaderConstantF( CV_UI_AUDIO_EQ, m_AudioEqValues, 2 );
			Display::Device()->SetVertexShaderConstantF( CV_POLYGON_WAVE, m_TrailSettings, 1 );
			break;
		}
		case E_VertexShader_PolygonMultiply:
		{
			Display::Device()->SetVertexShaderConstantF( CV_POLYGON_MULTIPLY_TRANSFORM, (F32*)&m_PolygonMultiMatrix, 4 );
			break;
		} 
		case E_VertexShader_PolygonTrail:
		{
			Display::Device()->SetVertexShaderConstantF( CV_POLYGON_WAVE, m_TrailSettings, 1 );
			Display::Device()->SetVertexShaderConstantF( CV_POLYGON_TEXTURE_VECTOR, m_TextureVector, 1 );
			break;
		}
		default:
			break;
	}

	// specific pixel shader params
	if ( m_ShaderFlags[ eEffect ] & E_ShaderUsageFlags_Settings )
	{
		Display::Device()->SetPixelShaderParameterF( ps.m_hSettings, m_Settings );
	}

	switch ( ePixelShader )
	{
		case E_PixelShader_PolygonGradientParam:
		case E_PixelShader_PolygonGradientRadial:
		case E_PixelShader_PolygonMultiply:
			Display::Device()->SetPixelShaderParameterF( ps.m_hPolygonGradientColourParam, m_ColourParam );
			break;
		
		case E_PixelShader_IntensityLerp:
		case E_PixelShader_AM_DamageBar:
		case E_PixelShader_AM_Perks:
			Display::Device()->SetPixelShaderParameterF( ps.m_hColourFrom,	m_PolygonColours[ 0 ] );
			Display::Device()->SetPixelShaderParameterF( ps.m_hColourTo,	m_PolygonColours[ 1 ] );
			break;

		default:
			break;
	}

	Display::Device()->SetPixelShaderParameterF( ps.m_hLightPos,		(F32*)&m_Light.m_Pos );
	Display::Device()->SetPixelShaderParameterF( ps.m_hLightDir,		(F32*)&m_Light.m_Dir );
	Display::Device()->SetPixelShaderParameterF( ps.m_hLightColourFrom,	(F32*)&m_Light.m_ColourFrom );
	Display::Device()->SetPixelShaderParameterF( ps.m_hLightColourTo,	(F32*)&m_Light.m_ColourTo );
	Display::Device()->SetPixelShaderParameterF( ps.m_hLightFallOff,	(F32*)&m_Light.m_FallOff );

	// set the shaders
	Display::Device()->SetVertexShader( &vs );
	Display::Device()->SetPixelShader( &ps );

	#ifdef PLATFORM_PS3
	Display::Device()->SetPixelShaderParameters( &ps ); // Make sure all calls to SetPixelShaderParameterF are done before this!
	#endif
	
	m_eCurrentVertexShader = eVertexShader;
	// For exposure
	m_CurrentVertexStride = GetVertexStride( eVertexShader );

	// Get number of samplers for this vertex shader
	U32 uNumSamplers = 1;
	switch( eVertexShader )
	{
	case E_VertexShader_MultiTexture2:
		uNumSamplers = 2;
		break;
	case E_VertexShader_MultiTexture3:
		uNumSamplers = 3;
		break;
	}

	return uNumSamplers;
}

void C_Renderer::SetFontEffectSettings(const C_FontInterfaceParam* pParam)
{
	assert(pParam);

	const F32 d = 15.0f;
	m_FontEffectSettings[0] = F32(pParam->m_Data.m_CentreI) / d;
	m_FontEffectSettings[1] = F32(pParam->m_Data.m_PipingI) / d;
	m_FontEffectSettings[2] = F32(pParam->m_Data.m_OutlineI) / d;
	m_FontEffectSettings[3] = 0.f;
	
	m_FontEffectSettings[4] = F32(pParam->m_Data.m_CentreA) / d;
	m_FontEffectSettings[5] = F32(pParam->m_Data.m_PipingA) / d;
	m_FontEffectSettings[6] = F32(pParam->m_Data.m_OutlineA) / d;
	m_FontEffectSettings[7] = 0.f;
	
	m_FontEffectSettings[8] = F32(pParam->m_Data.m_CentreTex);
	m_FontEffectSettings[9] = F32(pParam->m_Data.m_PipingTex);
	m_FontEffectSettings[10] = F32(pParam->m_Data.m_OutlineTex);
	m_FontEffectSettings[11] = 0.f;
}

void C_Renderer::SetPolygonFlatSettings( const Maths::HMat43& mat, const F32* pColour )
{
	for ( S32 i = 0; i < 4; i++ )
		m_PolygonColours[ 0 ][ i ] = pColour[ i ];

	m_PolygonMatrix = mat;
}

void C_Renderer::SetPolygonTrailSettings( const F32 fAngle, const F32 fIntensity, const F32 fFrequency )
{
	m_TrailSettings[0] = fAngle;
	m_TrailSettings[1] = fIntensity;
	m_TrailSettings[2] = fFrequency;
	m_TrailSettings[3] = 0.0f;
}


void C_Renderer::SetPolygonGradientSettings( const Maths::HMat43& mat, const Maths::HMat43& col_mat, const F32* pColour, const F32* pParam )
{
	memcpy( m_PolygonColours, pColour, sizeof(F32) * m_NumGradientColours * 4);

	m_PolygonMatrix = mat;
	m_PolygonColourMatrix = col_mat;

	if(pParam)
	{
		memcpy(m_ColourParam, pParam, sizeof(F32) * m_NumColourParams );
	}
}

void C_Renderer::SetLerpColourParam( const U32 param )
{
	C_UIColourARGB16 _f( (U16) (param >> 16) );
	C_UIColourARGB16 _t( (U16) (param & 0xffff) );

	C_UIColour f( _f.A(), _f.R(), _f.G(), _f.B() );
	C_UIColour t( _t.A(), _t.R(), _t.G(), _t.B() );

	m_PolygonColours[ 0 ][ 0 ] = (F32)f.R()/255.0f;
	m_PolygonColours[ 0 ][ 1 ] = (F32)f.G()/255.0f;
	m_PolygonColours[ 0 ][ 2 ] = (F32)f.B()/255.0f;
	m_PolygonColours[ 0 ][ 3 ] = (F32)f.A()/255.0f;

	m_PolygonColours[ 1 ][ 0 ] = (F32)t.R()/255.0f;
	m_PolygonColours[ 1 ][ 1 ] = (F32)t.G()/255.0f;
	m_PolygonColours[ 1 ][ 2 ] = (F32)t.B()/255.0f;
	m_PolygonColours[ 1 ][ 3 ] = (F32)t.A()/255.0f;
}

void C_Renderer::SetTextureVectorSettings( const F32 pu, const F32 pv, const F32 su, const F32 sv )
{
	m_TextureVector[0] = pu; 
	m_TextureVector[1] = pv; 
	m_TextureVector[2] = su; 
	m_TextureVector[3] = sv; 
}

void C_Renderer::SetSettings( const F32 f0, const F32 f1, const F32 f2, const F32 f3 )
{
	m_Settings[ 0 ] = f0;
	m_Settings[ 1 ] = f1;
	m_Settings[ 2 ] = f2;
	m_Settings[ 3 ] = f3;
}

void C_Renderer::SetAudioEqValues( const F32 f0, const F32 f1, const F32 f2, const F32 f3, const F32 f4, const F32 f5, const F32 f6, const F32 f7 )
{
	m_AudioEqValues[ 0 ] = f0;
	m_AudioEqValues[ 1 ] = f1;
	m_AudioEqValues[ 2 ] = f2;
	m_AudioEqValues[ 3 ] = f3;
	m_AudioEqValues[ 4 ] = f4;
	m_AudioEqValues[ 5 ] = f5;
	m_AudioEqValues[ 6 ] = f6;
	m_AudioEqValues[ 7 ] = f7;
}

void C_Renderer::SetAudioEqSettings( const F32 f0, const F32 f1, const F32 f2, const F32 f3 )
{
	m_TrailSettings[ 0 ] = f0;
	m_TrailSettings[ 1 ] = f1;
	m_TrailSettings[ 2 ] = f2;
	m_TrailSettings[ 3 ] = f3;
}

void C_Renderer::RenderTriList( const Display::C_VertexBuffer& vertexBuffer,
								const Display::C_IndexBuffer& indexBuffer,
								const U32 uIndexCount,
								const U32 uVertexCount )
{
	if ( uVertexCount == 0 )
		return;

	assert( m_eCurrentVertexShader < E_VertexShader_Count );

#ifndef CFG_GOLD
	if ( g_UiRenderWireframe )
		Display::Device()->SetFillMode( Display::E_FillMode_Wireframe );
#endif

	Display::Device()->SetVertexBuffer( 0, &vertexBuffer, 0, m_uVertexStrides[ m_eCurrentVertexShader ] );
	Display::Device()->SetVertexDeclaration( m_pVertexDeclarations[ m_eCurrentVertexShader ] );

	if ( uIndexCount >= 3 )
	{
		Display::Device()->SetIndexBuffer( &indexBuffer );
#ifdef CFG_GOLD
		Display::Device()->DrawIndexedPrimitive( Display::E_PrimitiveType_TriangleList, 0, 0, uVertexCount, 0, uIndexCount );
#else
		U32 indexCount = (g_UiRenderMaxIndexCount < 1 || uIndexCount < (U32)g_UiRenderMaxIndexCount*3) ? uIndexCount : (U32)g_UiRenderMaxIndexCount*3;
		Display::Device()->DrawIndexedPrimitive( Display::E_PrimitiveType_TriangleList, 0, 0, uVertexCount, 0, indexCount );
#endif
	}
	else
	if( uIndexCount == 0 )
	{
		Display::Device()->DrawPrimitive( Display::E_PrimitiveType_TriangleList, 0, uVertexCount );
	}

#ifndef CFG_GOLD
	if ( g_UiRenderWireframe )
		Display::Device()->SetFillMode( Display::E_FillMode_Solid );
#endif
}


void C_Renderer::RenderTriStrip( const Display::C_VertexBuffer& vertexBuffer, const U32 uVertexCount, const U32 uVertexStart )
{
	if ( uVertexCount < 3 )
		return;

	assert( m_eCurrentVertexShader < E_VertexShader_Count );

#ifndef CFG_GOLD
	if ( g_UiRenderWireframe )
		Display::Device()->SetFillMode( Display::E_FillMode_Wireframe );
#endif

	Display::Device()->SetVertexBuffer( 0, &vertexBuffer, 0, m_uVertexStrides[ m_eCurrentVertexShader ] );
	Display::Device()->SetVertexDeclaration( m_pVertexDeclarations[ m_eCurrentVertexShader ] );

#ifdef CFG_GOLD
	U32 vertexStart = (uVertexStart < 0 ) ? 0 : uVertexStart;
	Display::Device()->DrawPrimitive( Display::E_PrimitiveType_TriangleStrip, vertexStart, uVertexCount );
#else
	U32 vertexCount = (uVertexCount < (U32)g_UiRenderMaxTriCount || g_UiRenderMaxTriCount < 3) ? uVertexCount : (U32)g_UiRenderMaxTriCount;
	U32 vertexStart = (uVertexStart < 0 ) ? 0 : uVertexStart;

	Display::Device()->DrawPrimitive( Display::E_PrimitiveType_TriangleStrip, vertexStart, vertexCount );
#endif

#ifndef CFG_GOLD
	if ( g_UiRenderWireframe )
		Display::Device()->SetFillMode( Display::E_FillMode_Solid );
#endif
}

}; // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", on) 
#endif
