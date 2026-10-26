#ifndef UI_RENDERER_H
#define UI_RENDERER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/Containers/containers.h>

#define	_CREATE_VS( _FILE, _ENUM, _REF )	\
	if ( ! Display::CreateVertexShaderFromFunc( _FILE, 0, &m_VertexShaders[ _ENUM ], REF_SHADER( _REF ) ) )	\
		return false;																						\

#define	_CREATE_PS( _FILE, _ENUM, _REF )	\
	if ( ! Display::CreatePixelShaderFromFunc( _FILE, 0, &m_PixelShaders[ _ENUM ], REF_SHADER( _REF ) ) )	\
		return false;																						\

#define	_SHADER_MAPPING( _VS, _PS )			( ( _VS << 16 ) | _PS )
#define	_GET_VS( _MAPPING )					( _MAPPING >> 16 )
#define	_GET_PS( _MAPPING )					( _MAPPING & 0xffff )


namespace UI
{

// forward decls
class C_FontInterfaceParam;
class C_UITriListRenderInfo;

/// vertex shaders
enum E_VertexShader
{
	E_VertexShader_MultiTexture1 = 0,					///< @a
	E_VertexShader_MultiTexture2,						///< @a
	E_VertexShader_MultiTexture3,						///< @a

	E_VertexShader_PolygonFlat,							///< @a
	E_VertexShader_PolygonFlatFinnedOuter,				///< @a
	E_VertexShader_PolygonStroke,						///< @a
	E_VertexShader_PolygonGradient,						///< @a
	E_VertexShader_PolygonMultiply,						///< @a
	E_VertexShader_PolygonTrail,						///< @a
	E_VertexShader_PolygonPaint,						///< @a

	E_VertexShader_PolygonFont,							///< @a
	E_VertexShader_PolygonFontOutlineFixed,				///< @a
	E_VertexShader_PolygonFontOutlineFade,				///< @a
	E_VertexShader_PolygonFontOutlineFixedFade,			///< @a
	E_VertexShader_PolygonFontTextured,					///< @a
	E_VertexShader_PolygonFontGradient,					///< @a
	E_VertexShader_PolygonFontFinned,					///< @a
	E_VertexShader_PolygonFontFinnedOuter,				///< @a

	// Amax shaders
	E_VertexShader_AM_AudioEq,							///< @a

	E_VertexShader_Count,
	E_VertexShader_Null,								///< @a
};

/// pixel shaders
enum E_PixelShader
{
	E_PixelShader_Default = 0,							///< @a
	
	E_PixelShader_OneColour,							///< @a
	E_PixelShader_MultiTexture1,						///< @a
	E_PixelShader_MultiTexture2,						///< @a
	E_PixelShader_MultiTexture3,						///< @a
	E_PixelShader_MultiTextureFullAlpha1,				///< @a
	E_PixelShader_MultiTextureFullAlpha2,				///< @a
	E_PixelShader_MultiTextureFullAlpha3,				///< @a

	E_PixelShader_IntensityAsAlpha,						///< @a
	E_PixelShader_IntensityAsAlphaTex,					///< @a
	E_PixelShader_IntensityAsAlphaTexNoAlpha,			///< @a
	E_PixelShader_IntensityAsAlphaTex2AdditiveNoAlpha,	///< @a
	E_PixelShader_IntensityAsAlphaInvert,				///< @a
	E_PixelShader_IntensityAsAlphaInvertNotOne,			///< @a
	E_PixelShader_IntensityCutout,						///< @a

	E_PixelShader_UserGammaCorrect,						///< @a
	E_PixelShader_UserGammaCorrect2_IAA,				///< @a
	E_PixelShader_GreyScale,							///< @a
	E_PixelShader_GreyScale2,							///< @a
	E_PixelShader_GreyScale2_IAA,						///< @a
	E_PixelShader_Movie,								///< @a
	E_PixelShader_MovieWithAlpha,						///< @a
	E_PixelShader_IntensityLerp,						///< @a
	
	E_PixelShader_PolygonFlat,							///< @a
	E_PixelShader_PolygonStrokeAlpha,					///< @a
	E_PixelShader_PolygonStrokeIntensityAsAlpha,		///< @a
	E_PixelShader_PolygonMultiply,						///< @a
	E_PixelShader_PolygonGradient,						///< @a
	E_PixelShader_PolygonGradientParam,					///< @a
	E_PixelShader_PolygonGradientRadial,				///< @a
	E_PixelShader_PolygonTrail,							///< @a
	E_PixelShader_PolygonPaint,							///< @a

	E_PixelShader_PolygonFont,							///< @a
	E_PixelShader_PolygonFontTextured,					///< @a

	// Nightwing shaders
	E_PixelShader_NW_FightCountdown,					///< @a

	// Amax shaders
	E_PixelShader_AM_DamageBar,							///< @a
	E_PixelShader_AM_Perks,								///< @a
	E_PixelShader_AM_Loading,							///< @a
	
	E_PixelShader_Count,
};

/// effects - map to a vertex and pixel shader
enum E_Effect
{
	E_Effect_Default = 0,								///< @a

	E_Effect_OneColour,									///< @a
	E_Effect_MultiTexture,								///< @a
	E_Effect_MultiTextureFullAlpha,						///< @a

	E_Effect_IntensityAsAlpha,							///< @a
	E_Effect_IntensityAsAlphaTex,						///< @a
	E_Effect_IntensityAsAlphaTexNoAlpha,				///< @a
	E_Effect_IntensityAsAlphaTex2AdditiveNoAlpha,		///< @a
	E_Effect_IntensityAsAlphaInvert,					///< @a
	E_Effect_IntensityAsAlphaInvertNotOne,				///< @a
	E_Effect_IntensityCutout,							///< @a

	E_Effect_UserGammaCorrect,							///< @a
	E_Effect_UserGammaCorrect2_IAA,						///< @a
	E_Effect_GreyScale,									///< @a
	E_Effect_GreyScale2_IAA,							///< @a
	E_Effect_Movie,										///< @a
	E_Effect_MovieWithAlpha,							///< @a
	E_Effect_IntensityLerp,								///< @a
	
	E_Effect_PolygonFlat,								///< @a
	E_Effect_PolygonFlatFinnedOuter,					///< @a
	E_Effect_PolygonStrokeAlpha,						///< @a
	E_Effect_PolygonStrokeIntensityAsAlpha,				///< @a
	E_Effect_PolygonMultiply,							///< @a
	E_Effect_PolygonGradient,							///< @a
	E_Effect_PolygonGradient_Param,						///< @a
	E_Effect_PolygonGradient_Radial,					///< @a
	E_Effect_PolygonTrail,								///< @a
	E_Effect_PolygonPaint,								///< @a

	E_Effect_PolygonFont,								///< @a
	E_Effect_PolygonFontOutlineFixed,					///< @a
	E_Effect_PolygonFontOutlineFade,					///< @a
	E_Effect_PolygonFontOutlineFixedFade,				///< @a
	E_Effect_PolygonFontTextured,						///< @a
	E_Effect_PolygonFontGradient,						///< @a
	E_Effect_PolygonFontGradient_Param,					///< @a
	E_Effect_PolygonFontGradient_Radial,				///< @a
	E_Effect_PolygonFontFinned,							///< @a
	E_Effect_PolygonFontFinnedOuter,					///< @a

	// Nightwing effects
	E_Effect_NW_FightCountdown,							///< @a

	// Amax effects
	E_Effect_AM_DamageBar,								///< @a
	E_Effect_AM_Perks,									///< @a
	E_Effect_AM_AudioEqAlpha,							///< @a
	E_Effect_AM_AudioEqIntensityAsAlpha,				///< @a
	E_Effect_AM_Loading,								///< @a

	E_Effect_Count,
	E_Effect_Null,										///< @a
};

enum E_TextureAddressMode
{
	E_TextureAddressMode_Clamp = 0,						/// Clamp all
	E_TextureAddressMode_Wrap,							/// Wrap all
	E_TextureAddressMode_Mirror,						/// Mirror all
	E_TextureAddressMode_Clamp0,						/// Clamp 0 : Wrap N
};

/// flags for setting register values shared between shaders
enum E_ShaderUsageFlags
{
	E_ShaderUsageFlags_NotTextured		= ( 1<<0 ),		///< @a
	E_ShaderUsageFlags_Polygon			= ( 1<<1 ),		///< @a
	E_ShaderUsageFlags_GradientBounds	= ( 1<<2 ),		///< @a
	E_ShaderUsageFlags_MultiplyBounds	= ( 1<<3 ),		///< @a
	E_ShaderUsageFlags_PolygonColour	= ( 1<<4 ),		///< @a
	E_ShaderUsageFlags_PolygonFont		= ( 1<<5 ),		///< @a
	E_ShaderUsageFlags_Settings			= ( 1<<6 ),		///< @a
};

struct C_PixelShader : public Display::C_PixelShader
{
	bool CreateParams();

	Display::C_PixelShaderParameter			m_hLightPos;
	Display::C_PixelShaderParameter			m_hLightDir;
	Display::C_PixelShaderParameter			m_hLightColourFrom;
	Display::C_PixelShaderParameter			m_hLightColourTo;
	Display::C_PixelShaderParameter			m_hLightFallOff;
	
	Display::C_PixelShaderParameter			m_hColourFrom;
	Display::C_PixelShaderParameter			m_hColourTo;

	Display::C_PixelShaderParameterArray	m_hPolygonGradientColours;
	Display::C_PixelShaderParameter			m_hPolygonGradientColourParam;

	Display::C_PixelShaderParameter			m_hSettings;
};

// ----------
// C_Renderer
// ----------
/// the UI's renderer
class C_Renderer
{
public:
	struct C_Light
	{
		Maths::HVec4 m_Pos;
		Maths::HVec4 m_Dir;
		Maths::HVec4 m_ColourFrom;
		Maths::HVec4 m_ColourTo;
		Maths::HVec4 m_FallOff;

		void Clear( )
		{
			m_Pos			= Maths::HVec4( 0.0f, 0.0f, 0.0f, 0.0f );
			m_Dir			= Maths::HVec4( 0.0f, 0.0f, 1.0f, 0.0f );
			m_ColourFrom	= Maths::HVec4( 0.0f, 0.0f, 0.0f, 1.0f );
			m_ColourTo		= Maths::HVec4( 0.0f, 0.0f, 0.0f, 1.0f );
			m_FallOff		= Maths::HVec4( 1000.0f, 1000.0f, 1000.0f, 1000.0f );
		}
	};

public:
	C_Renderer();			///< setup vertex decls and shader flags
	~C_Renderer();			///< @a

	bool		Create();	///< create shaders
	void		Destroy();	///< destroy shaders

	void		Bind();		///< call before rendering - binds sampler states
	void		Unbind();	///< call after rendering

	const bool	BeginVerticesLocked() const;																						///< @a
	void*		BeginVertices( const Display::E_PrimitiveType ePrimitiveType, const U32 uVertexCount, const U32 uVertexStride );	///< @a
	void		EndVertices( void* pVertex );																						///< @a

	const U32	SetEffect( const E_Effect eEffect, const U32 uSamplerCount, const E_TextureAddressMode addressMode );	///< set shaders and any register values

	void		SetFontEffectSettings( const C_FontInterfaceParam* pParam );																///< @a
	void		SetLerpColourParam( const U32 param );																							///< @a


	void		SetPolygonFlatSettings( const Maths::HMat43& mat, const F32* pColour );	
	void		SetPolygonTrailSettings( const F32 fAngle, const F32 fIntensity, const F32 fFrequency );

	void		SetPolygonFontSettings( const Maths::HVec4& v )										{ m_PolygonFontSettings = v; }			///< @a
	void		SetPolygonMultiplySettings( const Maths::HMat43& multi_mat )						{ m_PolygonMultiMatrix = multi_mat; }	///< @a

	void		SetTextureVectorSettings( const F32 pu, const F32 pv, const F32 su, const F32 sv );															///< @a
	void		SetPolygonGradientSettings( const Maths::HMat43& mat, const Maths::HMat43& col_mat, const F32* pColour, const F32* effect_param = NULL );	///< @a

	void		SetSettings( const F32 f0 = 0.0f, const F32 f1 = 0.0f, const F32 f2 = 0.0f, const F32 f3 = 0.0f );		///< @a

	void		SetAudioEqValues( const F32 f0 = 0.0f, const F32 f1 = 0.0f, const F32 f2 = 0.0f, const F32 f3 = 0.0f, const F32 f4 = 0.0f, const F32 f5 = 0.0f, const F32 f6 = 0.0f, const F32 f7 = 0.0f );		///< @a
	void		SetAudioEqSettings( const F32 f0 = 0.0f, const F32 f1 = 0.0f, const F32 f2 = 0.0f, const F32 f3 = 0.0f );

	void		RenderTriList( const Display::C_VertexBuffer& vertexBuffer, const Display::C_IndexBuffer& indexBuffer, const U32 uIndexCount, const U32 uVertexCount );		///< @a
	void		RenderTriStrip( const Display::C_VertexBuffer& vertexBuffer, const U32 uVertexCount, const U32 uVertexStart = 0 );												///< @a

	U32			ShaderUsage( const E_Effect eEffect ) const											{ return m_ShaderFlags[ eEffect ]; }			///< @a
	void		SetLight( const C_Light& in );

	Display::C_VertexDeclaration*	GetVertexDeclaration( const E_VertexShader eShader ) const		{ return m_pVertexDeclarations[ eShader ]; }	///< @a
	const U32						GetVertexStride( const E_VertexShader eShader ) const			{ return m_uVertexStrides[ eShader ]; }			///< @a
	const U32						GetVertexStride() const											{ return m_CurrentVertexStride; }
	Display::C_VertexDeclaration*	GetVertexDeclaration() const									{ return m_pVertexDeclarations[ m_eCurrentVertexShader ]; }

public:
	static const U32						m_NumGradientColours = 4;
	static const U32						m_NumColourParams = 4;

private:
	// font effects
	F32										m_FontEffectSettings[ 12 ];

	Maths::HMat44							m_PolygonMatrix;
	Maths::HMat44							m_PolygonColourMatrix;
	Maths::HMat44							m_PolygonMultiMatrix;

	F32										m_PolygonColours[ m_NumGradientColours ][ 4 ];
	F32										m_ColourParam[ m_NumColourParams ];

	// polygon fonts
	Maths::HVec4							m_PolygonFontSettings;
	C_Light									m_Light;

	// polygon stroke
	F32										m_TextureVector[4];

	// polygon trail
	F32										m_TrailSettings[4];

	F32										m_Settings[ 4 ];		// general use settings

	F32										m_AudioEqValues[ 8 ];

	// shaders
	E_VertexShader							m_eCurrentVertexShader;

	Display::C_VertexDeclaration*			m_pVertexDeclarations[ E_VertexShader_Count ];
	U32										m_uVertexStrides[ E_VertexShader_Count ];

	Display::C_VertexShader					m_VertexShaders[ E_VertexShader_Count ];
	C_PixelShader							m_PixelShaders[ E_PixelShader_Count ];

	// shader mapping - effect => vertex/pixel shader
	U32										m_ShaderMappings[ E_Effect_Count ];
	U32										m_CurrentVertexStride;

	// shader flags
	Btl::Vector< U32, Btl::VectorParameters_Array< E_Effect_Count > >	m_ShaderFlags;
};

}; // namespace UI

#endif // UI_RENDERER_H
