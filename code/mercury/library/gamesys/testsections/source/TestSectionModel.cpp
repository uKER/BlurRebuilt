#include "stdafx.h"
#include "TestSections/TestSectionModel.h"

#include "FX/FX.h"
#include "Render/State/RenderState.h"
#include "Render/Utils/Helper.h"
#include "Render/View/ViewManager.h"
#include "RenderingData/Archive.h"
#include "System/Serialise/C_FileSystem.h"
#include "System/Serialise/C_FileBlock.h"

namespace Framework
{

Debug::Float	g_debugTestSectionModelSunHeading	("TestSection.Model.SunHeading",	0.0f,	Debug::Range(0.0f, 360.0f));
Debug::Float	g_debugTestSectionModelSunElevation	("TestSection.Model.SunElevation",	45.0f,	Debug::Range(0.0f, 90.0f));
Debug::Colour	g_debugTestSectionModelSunColour	("TestSection.Model.SunColour",		1.0f, 1.0f, 1.0f, 1.0f);
Debug::Colour	g_debugTestSectionModelSkyColour	("TestSection.Model.SkyColour",		0.5f, 0.5f, 0.5f, 1.0f);
Debug::Colour	g_debugTestSectionModelGroundColour	("TestSection.Model.GroundColour",	0.1f, 0.1f, 0.1f, 1.0f);

C_TestSectionModel::C_TestSectionModel(const Util::C_String& name) : C_TestSectionHDR(name)
{
	FX::Initialise();
}

C_TestSectionModel::~C_TestSectionModel()
{
}

bool C_TestSectionModel::OnCreate()
{
	Sys::I_File* pFile = Sys::C_FileSystem::Instance().Open("Models/Model.dat", Sys::E_FileAccess_Read);

	if (!pFile)
	{
		assert(!"Could not open the model file.");
		return false;
	}
	else
	{
		Sys::C_FileBlock rootBlock(pFile);
		 
		// keep reading child blocks
		while (rootBlock.GetPosition() < rootBlock.Length())
		{
			Sys::C_FileBlock block(&rootBlock);
 
			if (block.HeaderId() == "Scene")
			{
				Arch::BinaryInputArchive ar(block);
				ar >> m_Scene;

				for (int i = 0; i < m_Scene.GetNumResourceBlocks(); ++i)
				{
					ar >> m_Scene.GetResourceBlock(i);
				}
			}
		}
	}

	delete pFile;

	m_Scene.UpdateResourcePointers();

	if (!Display::CreateTextureCubeFromFile("Textures/Environment.dds", &m_TextureCube))
		Display::CreateTextureCubeFromFile("Dontship/Textures/Environment.dds", &m_TextureCube);;

	return true;
}

bool C_TestSectionModel::OnCreateVolatile()
{
	return true;
}

void C_TestSectionModel::OnDestroy()
{
	Display::Device()->Destroy(&m_TextureCube);
}

void C_TestSectionModel::OnDestroyVolatile()
{
}

void C_TestSectionModel::OnUpdate()
{
}

void C_TestSectionModel::OnRenderHdr3DViewWithAA(const S32 viewIndex)
{
	// set parameters
	FX::Parameter *pCubemap = FX::GetSharedParameter("g_tEnvironment");
	const void *pAddress = m_TextureCube.GetGPUTexture();
	FX::SetParameter(pCubemap, &pAddress);
	FX::SetSamplerState(pCubemap,
						Display::E_TextureAddress_Clamp, Display::E_TextureAddress_Clamp, Display::E_TextureAddress_Clamp,
						Display::E_TextureFilter_Linear, Display::E_TextureFilter_Linear, Display::E_TextureFilter_Linear,
						1, 0, 0.0f);
	
	const F32 fCosHeading = cosf(g_debugTestSectionModelSunHeading * Maths::Pi / 180.0f);
	const F32 fSinHeading = sinf(g_debugTestSectionModelSunHeading * Maths::Pi / 180.0f);
	const F32 fCosElevation = cosf(g_debugTestSectionModelSunElevation * Maths::Pi / 180.0f);
	const F32 fSinElevation = sinf(g_debugTestSectionModelSunElevation * Maths::Pi / 180.0f);
	const MathsStorage::Float4 vSunDirection(fSinHeading * fCosElevation, fSinElevation, fCosHeading * fCosElevation, 0.0f);
	FX::SetSharedParameterByName("g_vSunDirection", &vSunDirection);
	FX::SetSharedParameterByName("g_vSunColour", &g_debugTestSectionModelSunColour.Value());
	FX::SetSharedParameterByName("g_vSkyColour", &g_debugTestSectionModelSkyColour.Value());
	FX::SetSharedParameterByName("g_vGroundColour", &g_debugTestSectionModelGroundColour.Value());

	// set hdr parameters
	if (Render::C_RenderState::Instance().ShaderHDR())
	{
		FX::Parameter*	pVectorParameter	= FX::GetSharedParameter("g_vLuminance");
		FX::Parameter*	pTextureParameter	= FX::GetSharedParameter("g_tLuminance");
		
		const Display::C_Float4	pVector		= Render::C_RenderState::Instance().HDR()->GetShaderParameters();
		const void*				pTexture	= Render::C_RenderState::Instance().HDR()->GetAdaptedLuminanceTexture()->GetGPUTexture();
		
		FX::SetParameter(pVectorParameter, &pVector);
		FX::SetParameter(pTextureParameter, &pTexture);
		FX::SetSamplerState(pTextureParameter,
							Display::E_TextureAddress_Clamp, Display::E_TextureAddress_Clamp, Display::E_TextureAddress_Clamp,
							Display::E_TextureFilter_Point, Display::E_TextureFilter_Point, Display::E_TextureFilter_Point,
							1, 0, 0.0f);
	}
	
	// render the scene
	m_Scene.Render();
	m_SkyDome.Render();
}

void C_TestSectionModel::OnRenderHdr3DViewWithoutAA(const S32 viewIndex)
{
}

}; // namespace Framework
