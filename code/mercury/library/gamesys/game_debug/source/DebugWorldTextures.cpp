#include "GameDebugPch.h"
#include <game_debug/DebugWorldTextures.h>
#include <RenderingData/DebugTextures.h>
#include "DebugSystems\DebugHUDTextureViewer.h"

namespace GameDebug
{

Debug::Bool		g_debugOverrideTextures			("Graphics.Debug.WorldTextures.OverrideTextures",		false);
Debug::Bool		g_debugShowOnlySelectedTexture	("Graphics.Debug.TextureViewer.Options.ShowOnlySelectedTexture",false);

Debug::Bool		g_debugOverrideAlbedo			("Graphics.Debug.WorldTextures.OverrideAlbedo",			false);
Debug::Colour	g_debugOverrideAlbedoColour		("Graphics.Debug.WorldTextures.AlbedoColour",				1.0f, 1.0f, 1.0f, 1.0f);
Debug::String	g_debugOverrideAlbedoFileName	("Graphics.Debug.WorldTextures.AlbedoFileName",			"");

Debug::Bool		g_debugOverrideNormal			("Graphics.Debug.WorldTextures.OverrideTangentNormal",	false);
Debug::Vector	g_debugOverrideNormalVector		("Graphics.Debug.WorldTextures.TangentNormalVector",		0.0f, 0.0f, 1.0f);
Debug::String	g_debugOverrideNormalFileName	("Graphics.Debug.WorldTextures.TangentNormalFileName",	"");

Debug::Bool		g_debugOverrideSpecular			("Graphics.Debug.WorldTextures.OverrideSpecular",		false);
Debug::Float	g_debugOverrideSpecularR		("Graphics.Debug.WorldTextures.SpecularR",				0.0f,	Debug::Range(0.0f, 255.0f));
Debug::Float	g_debugOverrideSpecularG		("Graphics.Debug.WorldTextures.SpecularG",				0.0f,	Debug::Range(0.0f, 255.0f));
Debug::Float	g_debugOverrideSpecularB		("Graphics.Debug.WorldTextures.SpecularB",				0.0f,	Debug::Range(0.0f, 255.0f));
Debug::String	g_debugOverrideSpecularFileName	("Graphics.Debug.WorldTextures.SpecularFileName",		"");

Debug::Bool		g_debugOverrideEmissive			("Graphics.Debug.WorldTextures.OverrideEmissive",		false);
Debug::Colour	g_debugOverrideEmissiveColour	("Graphics.Debug.WorldTextures.EmissiveColour",			0.0f, 0.0f, 0.0f, 1.0f);
Debug::String	g_debugOverrideEmissiveFileName	("Graphics.Debug.WorldTextures.EmissiveFileName",		"");

C_DebugWorldTextures::C_DebugWorldTextures() : Display::I_Renderer("DebugWorldTextures")
{
	m_AlbedoIndex	= 0;
	m_NormalIndex	= 0;
	m_SpecularIndex	= 0;
	m_EmissiveIndex	= 0;


	m_AlbedoGamma = false;
	m_NormalGamma = false;
	m_SpecularGamma = false;
	m_EmissiveGamma = false;
}

C_DebugWorldTextures::~C_DebugWorldTextures()
{
	DestroyRenderer();
}

void C_DebugWorldTextures::SetAlbedoGamma(const bool srgb)
{
	m_AlbedoGamma = srgb;
}


void C_DebugWorldTextures::SetNormalGamma(const bool srgb)
{
	m_NormalGamma = srgb;

}

void C_DebugWorldTextures::SetSpecularGamma(const bool srgb)
{
	m_SpecularGamma = srgb;
}

void C_DebugWorldTextures::SetEmissiveGamma(const bool srgb)
{
	m_EmissiveGamma = srgb;
}

bool C_DebugWorldTextures::OnCreate()
{
	return true;
}

bool C_DebugWorldTextures::OnCreateVolatile()
{
	RESOURCE_MEMORY_ID("DebugWorldTextures");

	for (S32 i = 0; i < c_DebugTextureBufferSize; ++i)
	{
		#if PLATFORM_PC
		const U32 flags = Display::E_Usage_Managed;
		#else
		const U32 flags = Display::E_Usage_Video;
		#endif
		if (!Display::Device()->CreateTexture2D(1, 1, 1, flags, Display::E_Format_A8R8G8B8, &m_AlbedoSample[i]))
			return false;
		if (!Display::Device()->CreateTexture2D(1, 1, 1, flags, Display::E_Format_A8R8G8B8, &m_NormalSample[i]))
			return false;
		if (!Display::Device()->CreateTexture2D(1, 1, 1, flags, Display::E_Format_A8R8G8B8, &m_SpecularSample[i]))
			return false;
		if (!Display::Device()->CreateTexture2D(1, 1, 1, flags, Display::E_Format_A8R8G8B8, &m_EmissiveSample[i]))
			return false;
		
		#if TRACK_DISPLAY2_RESOURCES
		Display2::Device()->SetTextureName(&m_AlbedoSample[i], "_DebugAlbedo");
		Display2::Device()->SetTextureName(&m_NormalSample[i], "_DebugNormal");
		Display2::Device()->SetTextureName(&m_SpecularSample[i], "_DebugSpecular");
		Display2::Device()->SetTextureName(&m_EmissiveSample[i], "_DebugEmissive");
		#endif
	}

	SetSampleGamma();

	return true;
}

void C_DebugWorldTextures::SetSampleGamma()
{
}

void C_DebugWorldTextures::OnDestroy()
{
}

void C_DebugWorldTextures::OnDestroyVolatile()
{
	for (S32 i = 0; i < c_DebugTextureBufferSize; ++i)
	{
		Display::Device()->Destroy(&m_AlbedoSample[i]);
		Display::Device()->Destroy(&m_NormalSample[i]);
		Display::Device()->Destroy(&m_SpecularSample[i]);
		Display::Device()->Destroy(&m_EmissiveSample[i]);
	}
}

void C_DebugWorldTextures::Update()
{
	if (!g_debugOverrideTextures)
	{
		RenderingData::Target::g_DebugTextures.bEnable = false;
		RenderingData::Target::g_DebugTextures.m_pTexture0 = 0;
		RenderingData::Target::g_DebugTextures.m_pTexture1 = 0;
		RenderingData::Target::g_DebugTextures.m_pTexture2 = 0;
		RenderingData::Target::g_DebugTextures.m_pTexture3 = 0;
	}
	else
	{
		RenderingData::Target::g_DebugTextures.bEnable = true;
		RenderingData::Target::g_DebugTextures.m_pTexture0 = GetAlbedoTexture();
		RenderingData::Target::g_DebugTextures.m_pTexture1 = GetNormalTexture();
		RenderingData::Target::g_DebugTextures.m_pTexture2 = GetSpecularTexture();
		RenderingData::Target::g_DebugTextures.m_pTexture3 = GetEmissiveTexture();
	}

	if (g_debugShowOnlySelectedTexture)
	{
		RenderingData::Target::g_DebugTextures.m_pSelectedTexture = DebugSystems::C_DebugHUDTextureViewer::GetStreamDebugTexture();
	}
	else
	{
		RenderingData::Target::g_DebugTextures.m_pSelectedTexture = 0;
	}
}

Display::C_Texture2D* C_DebugWorldTextures::GetAlbedoTexture()
{
	if (!g_debugOverrideAlbedo)
		return 0;

	// first try to override with a file
	const Util::C_String fileName = g_debugOverrideAlbedoFileName.Value();
	if (fileName != "" && fileName != m_AlbedoName)
	{
		m_AlbedoName = fileName;
		if (UpdateTexture(m_AlbedoName, &m_AlbedoTexture, m_AlbedoGamma))
			return &m_AlbedoTexture;
	}
	if (m_AlbedoTexture.IsCreated())
		return &m_AlbedoTexture;

	// if failed, override with a 1x1 texture
	m_AlbedoIndex = (m_AlbedoIndex + 1) % c_DebugTextureBufferSize;
	
	const U32 colour = Maths::RGBA(g_debugOverrideAlbedoColour.Value().r, g_debugOverrideAlbedoColour.Value().g, g_debugOverrideAlbedoColour.Value().b, g_debugOverrideAlbedoColour.Value().a);
	if (UpdateSample(colour, &m_AlbedoSample[m_AlbedoIndex]))
		return &m_AlbedoSample[m_AlbedoIndex];
	else
		return 0;
}

Display::C_Texture2D* C_DebugWorldTextures::GetNormalTexture()
{
	if (!g_debugOverrideNormal)
		return 0;

	// first try to override with a file
	const Util::C_String fileName = g_debugOverrideNormalFileName.Value();
	if (fileName != "" && fileName != m_NormalName)
	{
		m_NormalName = fileName;
		if (UpdateTexture(m_NormalName, &m_NormalTexture, m_NormalGamma))
			return &m_NormalTexture;
	}
	if (m_NormalTexture.IsCreated())
		return &m_NormalTexture;

	// if failed, override with a 1x1 texture
	m_NormalIndex = (m_NormalIndex + 1) % c_DebugTextureBufferSize;

	const F32 fNX = g_debugOverrideNormalVector.Value().vx;
	const F32 fNY = g_debugOverrideNormalVector.Value().vy;
	const F32 fNZ = g_debugOverrideNormalVector.Value().vz;
	const F32 fLength = sqrtf(fNX * fNX + fNY * fNY * fNZ * fNZ);

	U32 colour = Maths::RGBA((fNX / fLength) * 0.5f + 0.5f, (fNY / fLength) * 0.5f + 0.5f, (fNZ / fLength * 0.5f + 0.5f), 1.0f);
	if (fLength < 0.00001f)
		colour = Maths::RGBA(0.5f, 0.5f, 1.0f);

	if (UpdateSample(colour, &m_NormalSample[m_NormalIndex]))
		return &m_NormalSample[m_NormalIndex];
	else
		return 0;
}

Display::C_Texture2D* C_DebugWorldTextures::GetSpecularTexture()
{
	if (!g_debugOverrideSpecular)
		return 0;

	// first try to override with a file
	const Util::C_String fileName = g_debugOverrideSpecularFileName.Value();
	if (fileName != "" && fileName != m_SpecularName)
	{
		m_SpecularName = fileName;
		if (UpdateTexture(m_SpecularName, &m_SpecularTexture, m_SpecularGamma))
			return &m_SpecularTexture;
	}
	if (m_SpecularTexture.IsCreated())
		return &m_SpecularTexture;

	// if failed, override with a 1x1 texture
	m_SpecularIndex = (m_SpecularIndex + 1) % c_DebugTextureBufferSize;
	
	const U32 colour = Maths::RGBA(g_debugOverrideSpecularR / 255.0f, g_debugOverrideSpecularG / 255.0f, g_debugOverrideSpecularB / 255.0f, 1.0f);
	if (UpdateSample(colour, &m_SpecularSample[m_SpecularIndex]))
		return &m_SpecularSample[m_SpecularIndex];
	else
		return 0;
}

Display::C_Texture2D* C_DebugWorldTextures::GetEmissiveTexture()
{
	if (!g_debugOverrideEmissive)
		return 0;

	// first try to override with a file
	const Util::C_String fileName = g_debugOverrideEmissiveFileName.Value();
	if (fileName != "" && fileName != m_EmissiveName)
	{
		m_EmissiveName = fileName;
		if (UpdateTexture(m_EmissiveName, &m_EmissiveTexture, m_EmissiveGamma))
			return &m_EmissiveTexture;
	}
	if (m_EmissiveTexture.IsCreated())
		return &m_EmissiveTexture;

	// if failed, override with a 1x1 texture
	m_EmissiveIndex = (m_EmissiveIndex + 1) % c_DebugTextureBufferSize;
	
	const U32 colour = Maths::RGBA(g_debugOverrideEmissiveColour.Value().r, g_debugOverrideEmissiveColour.Value().g, g_debugOverrideEmissiveColour.Value().b, g_debugOverrideEmissiveColour.Value().a);
	if (UpdateSample(colour, &m_EmissiveSample[m_EmissiveIndex]))
		return &m_EmissiveSample[m_EmissiveIndex];
	else
		return 0;
}

bool C_DebugWorldTextures::UpdateSample(const U32 colour, Display::C_Texture2D* pTexture)
{
	const Display::C_LockedRect rect = pTexture->Lock(0);
	if (rect.pBits)
	{
		*((U32*)rect.pBits) = colour;
	}
	pTexture->Unlock(0);
	
	return (rect.pBits != 0);
}

bool C_DebugWorldTextures::UpdateTexture(const Util::C_String& fileName, Display::C_Texture2D* pTexture, const bool srgb)
{
	#if PLATFORM_PS3
	Display::Device()->BlockUntilIdle();
	#endif

	Display::Device()->Destroy(pTexture);

	if (!Sys::GetDefaultFileSystem()->FileExists(fileName))
		return false;
	if (!Display::CreateTexture2DFromFile(fileName, pTexture))
		return false;


	return true;
}

}; // namespace Game
