#include "stdafx.h"
#include "Display/Device/Device.h"
#include "Render/Shaders/Constants.h"
#include "Render/Shaders/Effects/Ocean/OceanConstants.h"
#include "Render/Effects/Ocean/Ocean.h"
#include "Render/State/RenderState.h"

#include "System/Misc/Random.h"

EXTERN_SHADER(vsOceanOld);
EXTERN_SHADER(psOceanOld);
EXTERN_SHADER(psOceanOldHDRU32);

#ifndef OBJ_SHADERS
#if defined(DEVICE_PS3GCM)
#include "Render/Shaders/Effects/Ocean/PS3/vsOcean.shader"
#include "Render/Shaders/Effects/Ocean/PS3/psOcean.shader"
#include "Render/Shaders/Effects/Ocean/PS3/psOceanHDRU32.shader"
#endif
#endif

#ifdef CFG_ALLOW_OPTIMIZE_OFF
//#pragma optimize("", off) // CFG_ALLOW_OPTIMIZE_OFF
#endif



namespace Ocean
{



static C_Waves g_Waves; // global wave structure



const F32	kTileSize		=	0.25f;
const U32	kTileCountX		=	24;
const U32	kTileCountY		=	12;
const F32	kWidth			=	kTileSize * kTileCountX;
const F32	kHeight			=	kTileSize * kTileCountY;



struct C_Vertex
{
	F32	m_X;
	F32	m_Y;
	F32	m_Z;
};



Debug::Bool		g_Wireframe("Graphics.Render.Ocean.Wireframe", false);
Debug::Float	g_ZoomOut("Graphics.Render.Ocean.ZoomOut", 1.05f);

Debug::Float	g_DampingAmount("Graphics.Render.Ocean.DampingAmount", 0.000005f, Debug::Range(0.0f, 0.0001f));
Debug::Float	g_FogMultiplier("Graphics.Render.Ocean.FogMultiplier", 1.00f, Debug::Range(0.0f, 8.0f));

Debug::Function	g_RegenerateWaves("Graphics.Render.Ocean.RegenerateWaves", RegenerateWaves, NULL);
Debug::Vector	g_WindDirection("Graphics.Render.Ocean.WindDirection", 1.0f, 0.0f, 0.0f);
Debug::Float	g_WindAngle("Graphics.Render.Ocean.WindAngle", 120.0f, Debug::Range(0.0f, 180.0f));
Debug::Float	g_Choppiness("Graphics.Render.Ocean.Choppiness", 0.00f, Debug::Range(0.0f, 1.0f));
Debug::Float	g_MedianAmplitude("Graphics.Render.Ocean.MedianAmplitude", 0.10f, Debug::Range(0.0f, 1.0f));
Debug::Float	g_MedianWaveLength("Graphics.Render.Ocean.MedianWaveLength", 32.0f, Debug::Range(10.0f, 50.0f));

Debug::Colour	g_DeepColour("Graphics.Render.Ocean.DeepColour", 0.250f, 0.500f, 0.550f, 1.0f, Debug::Gamma(2.2f));
Debug::Colour	g_WaveColour("Graphics.Render.Ocean.WaveColour", 0.250f, 0.500f, 0.650f, 1.0f, Debug::Gamma(2.2f));

Debug::Float	g_SpecularBlurFactor( "Graphics.Render.Ocean.SpecularBlurFactor", 1.0f, Debug::Range(0.0f, 8.0f) );
Debug::Float	g_SpecularReflection( "Graphics.Render.Ocean.SpecularReflection", 0.25f, Debug::Range(0.0f, 1.0f) );
Debug::Float	g_DullSpecularPower( "Graphics.Render.Ocean.DullSpecularPower", 32.0f, Debug::Range(0.0f, 1024.0f) );
Debug::Float	g_DullSpecularIntensity( "Graphics.Render.Ocean.DullSpecularIntensity", 0.08f, Debug::Range(0.0f, 4.0f) );
Debug::Float	g_SpecularPower( "Graphics.Render.Ocean.SpecularPower", 128.0f, Debug::Range(64.0f, 2048.0f) );
Debug::Float	g_SpecularIntensity( "Graphics.Render.Ocean.SpecularIntensity", 1.6f, Debug::Range(0.0f, 4.0f) );

Debug::Vector	g_ScrollingDirection0("Graphics.Render.Ocean.ScrollingDirection0", 0.0f, 1.0f, 0.0f);
Debug::Vector	g_ScrollingDirection1("Graphics.Render.Ocean.ScrollingDirection1", -1.0f, -1.0f, 0.0f);
Debug::Float	g_ScrollingSize0("Graphics.Render.Ocean.ScrollingSize0", 0.025f, Debug::Range(0.0f, 1.0f));
Debug::Float	g_ScrollingSize1("Graphics.Render.Ocean.ScrollingSize1", 0.025f, Debug::Range(0.0f, 1.0f));
Debug::Float	g_ScrollingSpeed("Graphics.Render.Ocean.ScrollingSpeed", 0.025f, Debug::Range(-1.0f, 1.0f));
Debug::Float	g_DiffuseScale("Graphics.Render.Ocean.DiffuseScale", 0.20f, Debug::Range(0.0f, 1.0f));
	
Debug::String	g_NormalMap0("Graphics.Render.Ocean.NormalMap0", "textures\\fx\\ocean\\n0_ocean.dds");
Debug::String	g_NormalMap1("Graphics.Render.Ocean.NormalMap1", "textures\\fx\\ocean\\n1_ocean.dds");
Debug::String	g_EnvironmentMap("Graphics.Render.Ocean.CubeMap", "textures\\fx\\ocean\\default_environment.dds");



void RegenerateWaves(void *pParam)
{
	Util::C_Random rnd;
	rnd.Seed(rand());

	const F32	kMedianWaveLength	=	g_MedianWaveLength.Value();
	const F32	kMinWaveLength		=	kMedianWaveLength * 0.5f;
	const F32	kMaxWaveLength		=	kMedianWaveLength * 2.0f;
	const F32	kMedianAmplitude	=	g_MedianAmplitude.Value();
	const F32	kRatio				=	kMedianAmplitude / kMedianWaveLength;
	const F32	kAngle				=	g_WindAngle.Value() * Maths::Pi / 180.0f;

	// generate wave lengths
	F32	waveLengths[8];
	waveLengths[0]		=	rnd.FRand(kMinWaveLength, kMedianWaveLength);
	waveLengths[1]		=	rnd.FRand(kMinWaveLength, kMedianWaveLength);
	waveLengths[2]		=	rnd.FRand(kMinWaveLength, kMedianWaveLength);
	waveLengths[3]		=	rnd.FRand(kMinWaveLength, kMedianWaveLength);
	waveLengths[4]		=	rnd.FRand(kMedianWaveLength, kMaxWaveLength);
	waveLengths[5]		=	rnd.FRand(kMedianWaveLength, kMaxWaveLength);
	waveLengths[6]		=	rnd.FRand(kMedianWaveLength, kMaxWaveLength);
	waveLengths[7]		=	rnd.FRand(kMedianWaveLength, kMaxWaveLength);

	for (U32 ii = 0; ii < 8; ++ii)
	{
		g_Waves.m_Heights[ii]		=	waveLengths[ii] * kRatio;
		g_Waves.m_Frequencies[ii]	=	Maths::Pi2 / waveLengths[ii];
		g_Waves.m_Speeds[ii]		=	sqrt( 9.8f * Maths::Pi2 / waveLengths[ii] );
		g_Waves.m_Offsets[ii]		=	rnd.FRand(0.0f, Maths::Pi2);
		g_Waves.m_Choppiness[ii]	=	g_Choppiness.Value() / (8.0f * Maths::Pi2 * kRatio);
	}

	// directions
	Maths::HVec2	windDir			=	Maths::HVec2( g_WindDirection.Value().vx, g_WindDirection.Value().vz ).Normalised();
	windDir.Normalise();
	for (U32 ii = 0; ii < 8; ++ii)
	{
		F32	angle	=	rnd.FRand(-kAngle / 2.0f, +kAngle / 2.0f);
		g_Waves.m_DirectionsX[ii]	=	windDir.X() * cosf(angle) - windDir.Y() * sinf(angle);
		g_Waves.m_DirectionsY[ii]	=	windDir.X() * sinf(angle) + windDir.Y() * cosf(angle);
	}
}



}; // end namespace Ocean



C_Ocean::C_Ocean(const F32 height) : Display::I_Renderer("Ocean"), m_Height(height), m_Plane( Maths::HVec4(0.0f, -1.0f, 0.0f, height) ), m_IsVisible(false), m_Time(0.0f)
{
	Display::C_VertexFormat vertexFormat;
	vertexFormat.AddElement( 0, Display::E_VertexElementType_F32_3, Display::E_VertexElementUsage_Position, 0 );
	m_VertexStride	=	vertexFormat.GetStride(0);
	m_pDecl			=	Display::Device()->CreateVertexDeclaration(vertexFormat);

	m_VertexCount	=	(Ocean::kTileCountX + 1) * (Ocean::kTileCountY + 1);
	//m_IndexCount	=	6 * Ocean::kTileCountX * Ocean::kTileCountY;
	m_IndexCount	=	Ocean::kTileCountY * (2 * Ocean::kTileCountX + 4);

	Ocean::RegenerateWaves(NULL);
}



C_Ocean::~C_Ocean()
{
	DestroyRenderer();
}



bool C_Ocean::OnCreate()
{
	RESOURCE_MEMORY_ID("Ocean");

	#ifndef OBJ_SHADERS
	if ( !Display::CreateVertexShaderFromFunc("Shaders/vsOcean", 0, &m_VertexShader, REF_SHADER(vsOcean)) )
		return false;
	if ( !Display::CreatePixelShaderFromFunc("Shaders/psOcean", 0, &m_PixelShader[0], REF_SHADER(psOcean)) )
		return false;
	if ( !Display::CreatePixelShaderFromFunc("Shaders/psOceanHDRU32", 0, &m_PixelShader[1], REF_SHADER(psOceanHDRU32)) )
		return false;
	#else
	if ( !Display::CreateVertexShaderFromFunc("Shaders/vsOcean", 0, &m_VertexShader, REF_SHADER(vsOceanOld)) )
		return false;
	if ( !Display::CreatePixelShaderFromFunc("Shaders/psOcean", 0, &m_PixelShader[0], REF_SHADER(psOceanOld)) )
		return false;
	if ( !Display::CreatePixelShaderFromFunc("Shaders/psOceanHDRU32", 0, &m_PixelShader[1], REF_SHADER(psOceanOldHDRU32)) )
		return false;
	#endif

	m_psLuminance[0] = m_PixelShader[0].GetParameter(CP_RENDER_POSTPROCESS_LUMINANCE, "g_vLuminance");
	m_psLuminance[1] = m_PixelShader[1].GetParameter(CP_RENDER_POSTPROCESS_LUMINANCE, "g_vLuminance");

#ifdef PLATFORM_PS3
	for (U32 hdrNo = 0; hdrNo < 2; ++hdrNo)
	{
		m_Settings_m_Specular[hdrNo]	= m_PixelShader[hdrNo].GetParameter(CP_RENDER_SETTINGS, "g_Settings.m_Specular");
		m_Settings_m_Reflect[hdrNo]		= m_PixelShader[hdrNo].GetParameter(CP_RENDER_SETTINGS + 1, "g_Settings.m_Reflect");
		m_Settings_m_Scales[hdrNo]		= m_PixelShader[hdrNo].GetParameter(CP_RENDER_SETTINGS + 2, "g_Settings.m_Scales");
		m_Settings_m_WaveColour[hdrNo]	= m_PixelShader[hdrNo].GetParameter(CP_RENDER_SETTINGS + 3, "g_Settings.m_WaveColour");
		m_Settings_m_DeepColour[hdrNo]	= m_PixelShader[hdrNo].GetParameter(CP_RENDER_SETTINGS + 4, "g_Settings.m_DeepColour");
		m_Settings_m_ObjSunDir[hdrNo]	= m_PixelShader[hdrNo].GetParameter(CP_RENDER_SETTINGS + 5, "g_Settings.m_ObjSunDir");
		m_Settings_m_SunCol[hdrNo]		= m_PixelShader[hdrNo].GetParameter(CP_RENDER_SETTINGS + 6, "g_Settings.m_SunCol");
		m_Settings_m_SkyCol[hdrNo]		= m_PixelShader[hdrNo].GetParameter(CP_RENDER_SETTINGS + 7, "g_Settings.m_SkyCol");
		m_Settings_m_FogColour[hdrNo]	= m_PixelShader[hdrNo].GetParameter(CP_RENDER_SETTINGS + 8, "g_Settings.m_FogColour");
		m_Settings_m_FogInfo[hdrNo]		= m_PixelShader[hdrNo].GetParameter(CP_RENDER_SETTINGS + 9, "g_Settings.m_FogInfo");
		m_Settings_m_Animation[hdrNo]	= m_PixelShader[hdrNo].GetParameter(CP_RENDER_SETTINGS + 10, "g_Settings.m_Animation");
	}
#endif

	Display::Device()->CreateVertexBuffer(m_VertexCount * m_VertexStride, 0, &m_VertexBuffer);
	Ocean::C_Vertex * pVertices = (Ocean::C_Vertex *) m_VertexBuffer.Lock();
		U32 vertex	=	0;
		for (U32 yTile = 0; yTile <= Ocean::kTileCountY; ++yTile)
			for (U32 xTile = 0; xTile <= Ocean::kTileCountX; ++xTile)
			{
				pVertices[vertex].m_X	=	xTile / (F32) Ocean::kTileCountX;
				pVertices[vertex].m_Y	=	yTile / (F32) Ocean::kTileCountY;
				pVertices[vertex].m_Z	=	0.0f;
				++vertex;
			}
	m_VertexBuffer.Unlock();

	Display::Device()->CreateIndexBuffer(m_IndexCount * sizeof(U16), 0, Display::E_Format_Index16, &m_IndexBuffer);
		U16 *pIndices = (U16 *) m_IndexBuffer.Lock();
		U16 baseVertex = 0;
		for (U32 yTile = 0; yTile < Ocean::kTileCountY; ++yTile)
		{
			for (U32 xTile = 0; xTile < Ocean::kTileCountX; ++xTile)
			{
				*pIndices++ = baseVertex;
				*pIndices++ = baseVertex + Ocean::kTileCountX + 1;

				++baseVertex;
			}

			*pIndices++ = baseVertex;
			*pIndices++ = baseVertex + Ocean::kTileCountX + 1;
			*pIndices++ = baseVertex + Ocean::kTileCountX + 1;
			*pIndices++ = baseVertex + 1;

			++baseVertex;
		}
	m_IndexBuffer.Unlock();

	if (!Display::CreateTexture2DFromFile( Util::C_String(Ocean::g_NormalMap0.Value()), &m_Normal0Map ))
		return false;
	if (!Display::CreateTexture2DFromFile( Util::C_String(Ocean::g_NormalMap1.Value()), &m_Normal1Map ))
		return false;
	if (!Display::CreateTextureCubeFromFile( Util::C_String(Ocean::g_EnvironmentMap.Value()), &m_EnvironmentMap ))
		return false;


	return true;
}



void C_Ocean::OnDestroy()
{
	Display::Device()->Destroy( &m_Normal0Map );
	Display::Device()->Destroy( &m_Normal1Map );
	Display::Device()->Destroy( &m_EnvironmentMap );

	Display::Device()->Destroy( &m_VertexBuffer );
	Display::Device()->Destroy( &m_IndexBuffer );

	Display::Device()->Destroy( &m_VertexShader );
	Display::Device()->Destroy( &m_PixelShader[1] );
	Display::Device()->Destroy( &m_PixelShader[0] );
}



void C_Ocean::Update(const Maths::C_TimeS &timeStep)
{
	m_Time += timeStep;
}



void C_Ocean::Render()
{
	PERF_EVENT("Ocean::Render");

	CalculateProjectiveQuad();
	
	if (m_IsVisible)
	{
		const U32 hdrNo = Render::C_RenderState::Instance().ShaderHDR() ? 1 : 0;
		
		if (Ocean::g_Wireframe)
		{
			Display::Device()->SetFillMode(Display::E_FillMode_Wireframe);
		}

		Display::Device()->SetBlendMode(Display::E_BlendMode_Solid);
		Display::Device()->SetCullMode(Display::E_CullMode_None);

		Display::Device()->SetVertexShader(&m_VertexShader);
		Display::Device()->SetPixelShader(&m_PixelShader[hdrNo]);

		UploadConstants();
		
		if (Render::C_RenderState::Instance().ShaderHDR())
		{
			//Display::Device()->SetColourWriteEnable(Display::E_ColourWriteEnable_All);
			Display::Device()->SetTexture(15, Render::C_RenderState::Instance().HDR()->GetAdaptedLuminanceTexture());
			Display::Device()->SetTextureFilter(15, Display::E_TextureFilter_Point);
			Display::Device()->SetTextureAddress(15, Display::E_TextureAddress_Clamp);
		}

		Display::Device()->SetTextureFilterMagMinMip(0, Display::E_TextureFilter_Linear, Display::E_TextureFilter_Linear, Display::E_TextureFilter_Linear);
		Display::Device()->SetTextureFilterMagMinMip(1, Display::E_TextureFilter_Linear, Display::E_TextureFilter_Linear, Display::E_TextureFilter_Linear);
		Display::Device()->SetTextureFilterMagMinMip(2, Display::E_TextureFilter_Linear, Display::E_TextureFilter_Linear, Display::E_TextureFilter_Linear);

		Display::Device()->SetTextureAddressUVW(0, Display::E_TextureAddress_Wrap, Display::E_TextureAddress_Wrap, Display::E_TextureAddress_Wrap);
		Display::Device()->SetTextureAddressUVW(1, Display::E_TextureAddress_Wrap, Display::E_TextureAddress_Wrap, Display::E_TextureAddress_Wrap);
		Display::Device()->SetTextureAddressUVW(2, Display::E_TextureAddress_Clamp, Display::E_TextureAddress_Clamp, Display::E_TextureAddress_Clamp);

		Display::Device()->SetTextureGamma(SP_OCEAN_REFLECTIONMAP, Display::E_Bool_True);

		UploadBoundaries(m_ProjectiveCoords[0], m_ProjectiveCoords[1], m_ProjectiveCoords[2], m_ProjectiveCoords[3]);

		Display::Device()->SetVertexDeclaration(m_pDecl);
		Display::Device()->SetVertexBuffer( 0, &m_VertexBuffer, 0, m_VertexStride );
		Display::Device()->SetIndexBuffer( &m_IndexBuffer );

		Display::Device()->DrawIndexedPrimitive( Display::E_PrimitiveType_TriangleStrip, 0, 0, m_VertexCount, 0, m_IndexCount );
		if (4 < m_TessellatedEdges)
		{
			UploadBoundaries(m_ProjectiveCoords[0], m_ProjectiveCoords[3], m_ProjectiveCoords[3], m_ProjectiveCoords[4]);

			Display::Device()->DrawIndexedPrimitive( Display::E_PrimitiveType_TriangleStrip, 0, 0, m_VertexCount, 0, m_IndexCount );
		}

		if (Ocean::g_Wireframe)
		{
			Display::Device()->SetFillMode(Display::E_FillMode_Solid);
		}

		Display::Device()->SetTextureGamma(SP_OCEAN_REFLECTIONMAP, Display::E_Bool_False);
	}
}



void C_Ocean::Reset()
{
	m_Time = 0.0f;
}



void C_Ocean::UploadConstants()
{
	const U32 hdrNo = Render::C_RenderState::Instance().ShaderHDR() ? 1 : 0;

	const Display::C_ViewSettings3D& viewSettings3D = Display::Device()->GetViewSettings3D();

	// vertex shader
	MathsStorage::Float4 minMaxZ;
	minMaxZ.x = viewSettings3D.GetNearClip();
	minMaxZ.y = viewSettings3D.GetFarClip();
	
	MathsStorage::Float4 damping;
	damping.x = Ocean::g_DampingAmount.Value();

	Maths::HMat44 worldToProjMatrix	= Maths::Load<Maths::HMat44>( &viewSettings3D.GetViewProjMatrix() );
	Maths::HMat44 projToWorldMatrix	= worldToProjMatrix.Inverse();

	Display::C_Matrix projToWorldMatrix44;
	Maths::Store(projToWorldMatrix, &projToWorldMatrix44);

	Display::Device()->SetVertexShaderConstantF(CV_DISPLAY_VIEW_POSITION, (F32*)&viewSettings3D.m_ViewPosition, 1);

	Display::Device()->SetVertexShaderConstantF(CV_OCEAN_PROJWORLD_MATRIX, reinterpret_cast<F32 *>(&projToWorldMatrix44), 4);
	Display::Device()->SetVertexShaderConstantF(CV_OCEAN_TIME, Display::C_VSConstant(m_Time), 1);
	Display::Device()->SetVertexShaderConstantF(CV_OCEAN_WORLDTOUV, Display::C_VSConstant(Ocean::g_ScrollingSize0, Ocean::g_ScrollingSize1), 1);
	Display::Device()->SetVertexShaderConstantF(CV_OCEAN_MINMAXZ, reinterpret_cast<F32 *>(&minMaxZ), 1);
	Display::Device()->SetVertexShaderConstantF(CV_OCEAN_WAVES, reinterpret_cast<F32 *>(&Ocean::g_Waves), 14);
	Display::Device()->SetVertexShaderConstantF(CV_OCEAN_DAMPING, reinterpret_cast<F32 *>(&damping), 1);

	// pixel shader
#ifndef PLATFORM_PS3
	Ocean::C_PSConstants psConstants;

	psConstants.m_Specular.x	=	Ocean::g_SpecularIntensity;
	psConstants.m_Specular.y	=	Ocean::g_SpecularPower;
	psConstants.m_Specular.z	=	Ocean::g_DullSpecularIntensity;
	psConstants.m_Specular.w	=	Ocean::g_DullSpecularPower;

	psConstants.m_Reflect.x		=	Ocean::g_SpecularReflection;
	psConstants.m_Reflect.y		=	Ocean::g_SpecularBlurFactor;

	psConstants.m_Scales.x		=	Ocean::g_DiffuseScale;

	Maths::Store(Maths::HVec4(m_SunDir), &psConstants.m_ObjSunDir);
	Maths::Store(m_SunColour, &psConstants.m_SunCol);
	Maths::Store(m_SkyColour, &psConstants.m_SkyCol);
	Maths::Store(m_FogColour, &psConstants.m_FogColour);
	psConstants.m_FogInfo.x		=	m_FogInfo.X() * Ocean::g_FogMultiplier;
	psConstants.m_FogInfo.y		=	m_FogInfo.Y();
	psConstants.m_FogInfo.z		=	m_FogInfo.Z();

	psConstants.m_WaveColour.x	=	Ocean::g_WaveColour.Value().r;
	psConstants.m_WaveColour.y	=	Ocean::g_WaveColour.Value().g;
	psConstants.m_WaveColour.z	=	Ocean::g_WaveColour.Value().b;
	psConstants.m_WaveColour.w	=	Ocean::g_WaveColour.Value().a;

	psConstants.m_DeepColour.x	=	Ocean::g_DeepColour.Value().r;
	psConstants.m_DeepColour.y	=	Ocean::g_DeepColour.Value().g;
	psConstants.m_DeepColour.z	=	Ocean::g_DeepColour.Value().b;
	psConstants.m_DeepColour.w	=	Ocean::g_DeepColour.Value().a;

	psConstants.m_Animation.x	=	m_Time * Ocean::g_ScrollingSpeed * Ocean::g_ScrollingDirection0.Value().vx;
	psConstants.m_Animation.y	=	m_Time * Ocean::g_ScrollingSpeed * Ocean::g_ScrollingDirection0.Value().vy;
	psConstants.m_Animation.z	=	m_Time * Ocean::g_ScrollingSpeed * Ocean::g_ScrollingDirection1.Value().vx;
	psConstants.m_Animation.w	=	m_Time * Ocean::g_ScrollingSpeed * Ocean::g_ScrollingDirection1.Value().vy;

	Display::Device()->SetPixelShaderConstantF(CP_RENDER_SETTINGS, reinterpret_cast<F32 *>(&psConstants), 11);

	if (Render::C_RenderState::Instance().ShaderHDR())
	{
		Display::C_Float4 psLuminance = Render::C_RenderState::Instance().HDR()->GetShaderParameters();
		Display::Device()->SetPixelShaderParameterF(m_psLuminance[hdrNo], (F32 *) &psLuminance);
	}
	else
	{
		Display::C_Float4 psGain = Render::C_RenderState::Instance().GetGainShaderParameters();
		Display::Device()->SetPixelShaderParameterF(m_psLuminance[hdrNo], (F32 *) &psGain);
	}
#else
	PatchPixelShader();
#endif

	// textures
	if(m_Normal0Map.IsCreated())
		Display::Device()->SetTexture(SP_OCEAN_NORMAL0MAP, &m_Normal0Map);
	if(m_Normal1Map.IsCreated())
		Display::Device()->SetTexture(SP_OCEAN_NORMAL1MAP, &m_Normal1Map);
	if(m_EnvironmentMap.IsCreated())
		Display::Device()->SetTexture(SP_OCEAN_REFLECTIONMAP, &m_EnvironmentMap);
}



#ifdef PLATFORM_PS3
void C_Ocean::PatchPixelShader()
{
	const U32 hdrNo = Render::C_RenderState::Instance().ShaderHDR() ? 1 : 0;

	const Display::C_ViewSettings3D& viewSettings3D = Display::Device()->GetViewSettings3D();

	Ocean::C_PSConstants psConstants;

	psConstants.m_Specular.x	=	Ocean::g_SpecularIntensity.Value();
	psConstants.m_Specular.y	=	Ocean::g_SpecularPower.Value();
	psConstants.m_Specular.z	=	Ocean::g_DullSpecularIntensity.Value();
	psConstants.m_Specular.w	=	Ocean::g_DullSpecularPower.Value();

	psConstants.m_Reflect.x		=	Ocean::g_SpecularReflection.Value();
	psConstants.m_Reflect.y		=	Ocean::g_SpecularBlurFactor.Value();

	psConstants.m_Scales.x		=	Ocean::g_DiffuseScale.Value();

	Maths::Store(Maths::HVec4(m_SunDir), &psConstants.m_ObjSunDir);
	Maths::Store(m_SunColour, &psConstants.m_SunCol);
	Maths::Store(m_SkyColour, &psConstants.m_SkyCol);
	Maths::Store(m_FogColour, &psConstants.m_FogColour);

	psConstants.m_FogInfo.x		=	m_FogInfo.X() * Ocean::g_FogMultiplier.Value();
	psConstants.m_FogInfo.y		=	m_FogInfo.Y();
	psConstants.m_FogInfo.z		=	m_FogInfo.Z();

	psConstants.m_WaveColour.x	=	Ocean::g_WaveColour.Value().r;
	psConstants.m_WaveColour.y	=	Ocean::g_WaveColour.Value().g;
	psConstants.m_WaveColour.z	=	Ocean::g_WaveColour.Value().b;
	psConstants.m_WaveColour.w	=	Ocean::g_WaveColour.Value().a;

	psConstants.m_DeepColour.x	=	Ocean::g_DeepColour.Value().r;
	psConstants.m_DeepColour.y	=	Ocean::g_DeepColour.Value().g;
	psConstants.m_DeepColour.z	=	Ocean::g_DeepColour.Value().b;
	psConstants.m_DeepColour.w	=	Ocean::g_DeepColour.Value().a;

	psConstants.m_Animation.x	=	m_Time * Ocean::g_ScrollingSpeed * Ocean::g_ScrollingDirection0.Value().vx;
	psConstants.m_Animation.y	=	m_Time * Ocean::g_ScrollingSpeed * Ocean::g_ScrollingDirection0.Value().vy;
	psConstants.m_Animation.z	=	m_Time * Ocean::g_ScrollingSpeed * Ocean::g_ScrollingDirection1.Value().vx;
	psConstants.m_Animation.w	=	m_Time * Ocean::g_ScrollingSpeed * Ocean::g_ScrollingDirection1.Value().vy;

	Display::Device()->SetPixelShaderParameterF(m_Settings_m_Specular[hdrNo],	reinterpret_cast<F32 *>(&psConstants.m_Specular));
	Display::Device()->SetPixelShaderParameterF(m_Settings_m_Reflect[hdrNo],	reinterpret_cast<F32 *>(&psConstants.m_Reflect));
	Display::Device()->SetPixelShaderParameterF(m_Settings_m_Scales[hdrNo],		reinterpret_cast<F32 *>(&psConstants.m_Scales));
	Display::Device()->SetPixelShaderParameterF(m_Settings_m_WaveColour[hdrNo],	reinterpret_cast<F32 *>(&psConstants.m_WaveColour));
	Display::Device()->SetPixelShaderParameterF(m_Settings_m_DeepColour[hdrNo],	reinterpret_cast<F32 *>(&psConstants.m_DeepColour));
	Display::Device()->SetPixelShaderParameterF(m_Settings_m_ObjSunDir[hdrNo],	reinterpret_cast<F32 *>(&psConstants.m_ObjSunDir));
	Display::Device()->SetPixelShaderParameterF(m_Settings_m_SunCol[hdrNo],		reinterpret_cast<F32 *>(&psConstants.m_SunCol));
	Display::Device()->SetPixelShaderParameterF(m_Settings_m_SkyCol[hdrNo],		reinterpret_cast<F32 *>(&psConstants.m_SkyCol));
	Display::Device()->SetPixelShaderParameterF(m_Settings_m_FogColour[hdrNo],	reinterpret_cast<F32 *>(&psConstants.m_FogColour));
	Display::Device()->SetPixelShaderParameterF(m_Settings_m_FogInfo[hdrNo],	reinterpret_cast<F32 *>(&psConstants.m_FogInfo));
	Display::Device()->SetPixelShaderParameterF(m_Settings_m_Animation[hdrNo],	reinterpret_cast<F32 *>(&psConstants.m_Animation));

	if (Render::C_RenderState::Instance().ShaderHDR())
	{
		Display::C_Float4 psLuminance = Render::C_RenderState::Instance().HDR()->GetShaderParameters();
		Display::Device()->SetPixelShaderParameterF(m_psLuminance[hdrNo], (F32*)&psLuminance);
	}
	else
	{
		Display::C_Float4 psGain = Render::C_RenderState::Instance().GetGainShaderParameters();
		Display::Device()->SetPixelShaderParameterF(m_psLuminance[hdrNo], (F32 *) &psGain);
	}
}
#endif



void C_Ocean::UploadBoundaries(const Maths::HVec4 &bottomLeft, const Maths::HVec4 &topLeft, const Maths::HVec4 &topRight, const Maths::HVec4 &bottomRight)
{
	Ocean::C_PositionConstants posConstants;

	Maths::Store<Maths::HVec4, MathsStorage::Float4>(bottomLeft, &posConstants.m_BottomLeft);
	Maths::Store<Maths::HVec4, MathsStorage::Float4>(topLeft, &posConstants.m_TopLeft);
	Maths::Store<Maths::HVec4, MathsStorage::Float4>(topRight, &posConstants.m_TopRight);
	Maths::Store<Maths::HVec4, MathsStorage::Float4>(bottomRight, &posConstants.m_BottomRight);

	Display::Device()->SetVertexShaderConstantF(CV_OCEAN_POSITION, reinterpret_cast<F32 *>(&posConstants), 4);
}



void C_Ocean::CalculateProjectiveQuad()
{
	const Display::C_ViewSettings3D& viewSettings3D = Display::Device()->GetViewSettings3D();

	const Display::C_Matrix &viewProj = viewSettings3D.GetViewProjMatrix();
	const Maths::HMat44 &view = Maths::Load<Maths::HMat44>( &viewSettings3D.GetViewMatrix() );
	const Maths::HMat44 &proj = Maths::Load<Maths::HMat44>( &viewSettings3D.GetProjMatrix() );

	const F32 nearClip = viewSettings3D.GetNearClip();
	const F32 farClip = viewSettings3D.GetFarClip();
	const F32 perspDist = viewSettings3D.GetPerspDist();

	const Maths::C_Plane viewPlane = m_Plane * Maths::HMat43(view);

	const Maths::C_Rect viewExtents3D = viewSettings3D.GetViewExtents();

	Maths::HVec2	clipMin( viewExtents3D.Min() );
	Maths::HVec2	clipMax( viewExtents3D.Max() );
	Maths::HVec2	nearClipMin	=	clipMin * nearClip / perspDist;
	Maths::HVec2	nearClipMax	=	clipMax * nearClip / perspDist;
	Maths::HVec2	farClipMin	=	clipMin * farClip / perspDist;
	Maths::HVec2	farClipMax	=	clipMax * farClip / perspDist;
	
	Maths::C_Vertex	viewVertices[8];
	F32	tIntersect[8];
	U32	startInterp[8]	=	{ 0, 4, 1, 5, 2, 7, 3, 4 };
	U32 endInterp[8]	=	{ 4, 5, 5, 6, 6, 6, 7, 7 };

	viewVertices[0]	=	Maths::C_Vertex(0.0f, 0.0f, 0.0f);
	viewVertices[1]	=	Maths::C_Vertex(0.0f, 0.0f, 0.0f);
	viewVertices[2]	=	Maths::C_Vertex(0.0f, 0.0f, 0.0f);
	viewVertices[3]	=	Maths::C_Vertex(0.0f, 0.0f, 0.0f);
	viewVertices[4]	=	Maths::C_Vertex( farClipMin.X() * Ocean::g_ZoomOut, farClipMin.Y() * Ocean::g_ZoomOut, farClip );
	viewVertices[5]	=	Maths::C_Vertex( farClipMin.X() * Ocean::g_ZoomOut, farClipMax.Y() * Ocean::g_ZoomOut, farClip );
	viewVertices[6]	=	Maths::C_Vertex( farClipMax.X() * Ocean::g_ZoomOut, farClipMax.Y() * Ocean::g_ZoomOut, farClip );
	viewVertices[7]	=	Maths::C_Vertex( farClipMax.X() * Ocean::g_ZoomOut, farClipMin.Y() * Ocean::g_ZoomOut, farClip );

	tIntersect[0]	=	viewPlane.Intersect( Maths::C_Line(viewVertices[0], viewVertices[4]) );
	tIntersect[1]	=	viewPlane.Intersect( Maths::C_Line(viewVertices[4], viewVertices[5]) );
	tIntersect[2]	=	viewPlane.Intersect( Maths::C_Line(viewVertices[1], viewVertices[5]) );
	tIntersect[3]	=	viewPlane.Intersect( Maths::C_Line(viewVertices[5], viewVertices[6]) );
	tIntersect[4]	=	viewPlane.Intersect( Maths::C_Line(viewVertices[2], viewVertices[6]) );
	tIntersect[5]	=	viewPlane.Intersect( Maths::C_Line(viewVertices[7], viewVertices[6]) );
	tIntersect[6]	=	viewPlane.Intersect( Maths::C_Line(viewVertices[3], viewVertices[7]) );
	tIntersect[7]	=	viewPlane.Intersect( Maths::C_Line(viewVertices[4], viewVertices[7]) );

	Maths::HVec3	viewCoords[5];

	U32 vertex = 0;
	for (U32 ii = 0; ii < 8; ++ii)
	{
		F32	t	=	tIntersect[ii];
		if (0.0f <= t && t <= 1.0f)
		{
			Maths::C_Vertex	vStart	=	viewVertices[startInterp[ii]];
			Maths::C_Vertex	vEnd	=	viewVertices[endInterp[ii]];
			viewCoords[vertex++]	=	vStart.Lerp(vEnd, t);
		}
	}
	if (vertex < 4)
	{
		viewCoords[3]	=	viewCoords[0];
	}
	m_TessellatedEdges	=	vertex;

	for (U32 ii = 0; ii < 5; ++ii)
	{
		m_ProjectiveCoords[ii]	=	viewCoords[ii] * proj;
	}

	m_IsVisible = true;
}
