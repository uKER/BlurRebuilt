#include "stdafx.h"

#include <data/ConversionOptions.h>

#include "CreateLodImposters.h"
#include <AftkUt/AftkUt.h>

#include <converter_shared/helpers/AftkHelpers.h>
#include <converter_shared/lux_interface/LuxInterface.h>

#include <tools_shared/utility/misc.h>

namespace CreateLodImposters
{
static const char* g_baseScene = "imposters_base.a3d";
static const char* g_litBaseScene = "imposters_base_lit.a3d";
static const char* g_lightingModel = "imposter_lightmodel.xml";

bool IsAncestorOf(const Aftk::C_SceneNode *pNode, const Aftk::C_SceneNode *pRoot)
{
	while (pNode)
	{
		if (pNode == pRoot)
			return true;

		pNode = pNode->GetParentPtr();
	}
	return false;
}

Aftk::C_SceneNode* CloneSceneNodeAndChildren(const Aftk::C_SceneNode* pNode, Aftk::C_Scene& newScene)
{
	if (!pNode || pNode->GetScenePtr() == &newScene)
		return 0;

	Aftk::C_SceneNode *pClonedRoot = (Aftk::C_SceneNode*)pNode->Clone(&newScene);
	if (pClonedRoot)
	{
		const Aftk::C_SceneNode *pChild = pNode->GetChildPtr();
		while(pChild)
		{
			Aftk::C_SceneNode* pClonedChild = CloneSceneNodeAndChildren(pChild, newScene);
			if (pClonedChild)
				pClonedRoot->AddChildPtr(pClonedChild);
		}
		return pClonedRoot;
	}

	return 0;
}


U32 GetBaseMeshes(const Aftk::C_Scene& scene, Btl::Vector<const Aftk::C_SceneMeshNode*>* pResults)
{
	U32 baseMeshCount = 0;

	for (Aftk::C_SceneNodeConstIterator it(&scene); it; ++it)
	{
		if (it->HasType(Aftk::E_ObjType_SceneMesh))
		{
			bool skipTree = false;
			AftkUt::C_NodeFlagParser actorParser(*it, NULL, false);
			std::string lodType;
			if (actorParser.GetParameterValue("AutoLodGeneration", lodType) && lodType != "None" && lodType != "Inherit")
			{
				baseMeshCount++;
				if (pResults)
					pResults->PushBack(static_cast<const Aftk::C_SceneMeshNode*>((Aftk::C_SceneNode*)it));
			}
			else
			{
				//check instances for overrides
				for (U32 i=0; i<it->GetNumInstancesOfThisNode(); ++i)
				{
					Aftk::C_SceneNode *pSceneInstanceNode = it->GetInstanceOfThisNode(i);
					AftkUt::C_NodeFlagParser instParser(*pSceneInstanceNode , NULL, false);
					std::string instLodType;
					if (instParser.GetParameterValue("AutoLodGeneration", instLodType) && instLodType != "None" && instLodType != "Inherit")			
					{
						baseMeshCount++;
						if (pResults)
							pResults->PushBack(static_cast<const Aftk::C_SceneMeshNode*>((Aftk::C_SceneNode*)it));

						break;
					}
				}
			}

			if (skipTree)
			{
				//increment iterator to skip subtree ...
				const Aftk::C_SceneNode *pNode = it;
				Aftk::C_SceneNodeIterator it2 = it;
				it2++;
				while (IsAncestorOf(it2, pNode))
				{
					it = it2;
					++it2;
				}
			}
		}
	}

	return baseMeshCount;
}


bool ShouldGenerateImposters(const Aftk::C_Scene& scene)
{
	U32 baseMeshCount = GetBaseMeshes(scene, NULL);
	return baseMeshCount > 0;
}

bool GenerateLitMeshScene(const Aftk::C_Scene& scene, const C_DataConversionOptions& conversionOptions)
{
	const Util::C_String baseMeshPath = conversionOptions.GetImposterTempDirectory() + g_baseScene;
	const Util::C_String litBaseMeshPath = conversionOptions.GetImposterTempDirectory() + g_litBaseScene;
	const Util::C_String lightingModelPath = conversionOptions.GetImposterTempDirectory() + g_lightingModel;

	Btl::Vector<const Aftk::C_SceneMeshNode*> baseMeshes;
	GetBaseMeshes(scene, &baseMeshes);

	if (!baseMeshes.Size())
		return false;

	Aftk::C_Scene newScene;
	//Create a dummy root node
	{
		Aftk::C_SceneNode* pRootNode = new Aftk::C_SceneNode(&newScene);
		if (!pRootNode)
			return false;

		pRootNode->SetName("AftkRoot_ImposterMeshes");
		newScene.SetRootSceneNodePtr(pRootNode);
		pRootNode->SetWorldMatrix(scene.GetRootSceneNodePtr()->GetWorldMatrix());
	}

	//Copy scene nodes.  Space meshes out for lighting
	Aftk::Matrix44 m;
	UtMaths::SetIdentity(&m);

	for (S32 i=0; i<baseMeshes.Size(); ++i)
	{
		Aftk::C_SceneNode *pCloned = CloneSceneNodeAndChildren(baseMeshes[i], newScene);
		if (!pCloned)
			continue;

		//attach uid of original node so the imposter renderer can identify it
		AftkUt::C_NodeFlagParser parser(*pCloned);
		parser.SetParameterValue("BaseMeshUID",baseMeshes[i]->GetUniqueId());
		parser.SetParameterValue("UseVertexLighting", true);
		parser.SetParameterValue("UseSHLighting", false);
		parser.SetParameterValue("UseLightmapLighting", false);

		if (i>0)
			m[3][2] += Maths::Sqrt(pCloned->GetWorldBounds().GetObjectRadiusSquared());

		pCloned->SetLocalMatrix(m);
		newScene.GetRootSceneNodePtr()->AddChildPtr(pCloned);
		m[3][2] += Maths::Sqrt(pCloned->GetWorldBounds().GetObjectRadiusSquared());
	}

	if (!AftkHelpers::WriteAftkScene(newScene, baseMeshPath, NULL))
		return false;

	//call the lighting tool.
	{
		bool success = true;
		const C_LightingOptions& lightingOptions = conversionOptions.GetLightingOptions();
		Util::C_String defaultLightingModelFilename = conversionOptions.GetToolConfig().GetLuxBaseConfigFilename();
		Util::C_String specificLightingModelFilename = conversionOptions.GetLightingModelOptions().GetLuxSpecificConfigFilename();
		Util::C_String lightingModelFilename = conversionOptions.GetLightingModelFilename();

		LuxInterface::C_LightingModel lightingModel;
		success = lightingModel.ReadXml( defaultLightingModelFilename );
		if(!success) return false;

		if(specificLightingModelFilename.Length() > 0)
		{
			success = lightingModel.ReadXml( specificLightingModelFilename );
			if(!success) return false;
		}

		Maths::HVec3 sunDirection = AftkHelpers::CalculateSunDirection( lightingOptions.GetSunHeading(), lightingOptions.GetSunElevation() );
		S32 bouncePasses = lightingOptions.GetBouncePasses();
		F32 bounceScale = lightingOptions.GetBounceScale();
		F32 skyIntensity = lightingOptions.GetSkyIntensity();

		//setup lighting (from lightaftk.cpp)
		{
			const U32 noSunSamples = 8;
			const U32 noSkySamples = 256;
			F32 sunPenumbra = conversionOptions.GetToolConfig().GetSunPenumbra();

			if (lightingOptions.GetAddSkyLight())
			{
				if(bouncePasses > 1)
				{
					LuxInterface::C_LightBounceGather* pLightBounceGather = new LuxInterface::C_LightBounceGather();
					lightingModel.m_LightList.PushBack( pLightBounceGather );

					//pLightBounceGather->m_Colour = Maths::C_RGBA_F32(1.0f, 1.0f, 1.0f, 1.0f);
					pLightBounceGather->m_Colour = lightingOptions.GetSkyColour();
					pLightBounceGather->m_Intensity = skyIntensity;
					pLightBounceGather->m_Samples = noSkySamples;
					pLightBounceGather->m_BounceScale = bounceScale;
					pLightBounceGather->m_BouncePasses = bouncePasses;
				}
				else
				{
					LuxInterface::C_LightHemisphere* pLightHemisphere = new LuxInterface::C_LightHemisphere();
					lightingModel.m_LightList.PushBack( pLightHemisphere );

					//pLightHemisphere->m_Colour = Maths::C_RGBA_F32(1.0f, 1.0f, 1.0f, 1.0f);
					pLightHemisphere->m_Colour = lightingOptions.GetSkyColour();
					pLightHemisphere->m_Intensity = skyIntensity;
					pLightHemisphere->m_Samples = noSkySamples;
				}
			}

			if(lightingOptions.GetAddSunLight())
			{
				F32 sunIntensity = 1.0f;

				LuxInterface::C_LightDirectional* pLightDirectional = new LuxInterface::C_LightDirectional();
				lightingModel.m_LightList.PushBack( pLightDirectional );

				pLightDirectional->m_Colour = lightingOptions.GetSunColour();
				
				//Maths::C_RGBA_F32(1.0f, 1.0f, 1.0f, 1.0f);
				pLightDirectional->m_Intensity = sunIntensity;
				pLightDirectional->m_Lambert = LuxInterface::E_Off;
				pLightDirectional->m_Direction = sunDirection;
				pLightDirectional->m_Penumbra = sunPenumbra;
				pLightDirectional->m_Samples = noSunSamples;
			}
		}

		//now set up lighting passes
		{

			LuxInterface::C_Pass newPass;
			newPass.m_Name = "Main";
			newPass.m_SetList.PushBack( LuxInterface::C_Set( LuxInterface::E_SetAttribute_Light, "All" ) );

			lightingModel.m_PassList.PushBack(newPass);
		}

		if (!lightingModel.WriteXml(lightingModelPath))
			return false;
	}

	//run lux
	{
		Util::C_String luxApp = conversionOptions.GetToolConfig().GetLuxFilenameWin32();
		Util::C_String luxAppServer = Tools::FormatToolPathFor64bit(conversionOptions.GetToolConfig().GetLuxFilenameServer());

		Util::C_String luxCommand = "";

		if (conversionOptions.GetLightingOptions().GetUseLightingScreensavers())
			luxCommand = luxCommand + "-w " + luxApp + " ";

		luxCommand = luxCommand + 
			"\"" + baseMeshPath + "\" \"" + litBaseMeshPath + "\" \"" + lightingModelPath + "\" \"" + conversionOptions.GetImposterTempDirectory() + "\"";

		S32 r = Sys::ShellExec::ExecuteCommandAndWait(
			luxAppServer, luxCommand, "", Sys::ShellExec::E_LaunchState_Minimise,
			&Tools::g_pMyApplication->GetLogger());

		if (r != 0)
			return false;
	}

	return true;
}

bool Process(Aftk::C_Scene& scene, const C_DataConversionOptions& conversionOptions)
{
	//whilst we can simply write out the entire scene, call the imposter creation tool, and read in the results,
	//it may be better in the long term to write out a minimal scene, and merge the results back in (less memory fragmentation?)
	//or even statically/dynamically link in the imposter generation lib (pending 64-bit requirements)

	if (!ShouldGenerateImposters(scene) || conversionOptions.GetToolConfig().GetImposterFilenameWin32().Length()==0)
	{
		LOGDEBUG(Tools::g_log, "Skipping sprite generation");
		return true;
	}

	if (!AftkHelpers::WriteAftkScene(scene, conversionOptions.GetImposterSetupFilename(), NULL))
	{
		LOGWARN(Tools::g_log, "Error processing imposter lods");
		return true;
	}

	bool lit = false;
	if (conversionOptions.GetLightingOptions().GetLightScene())
	{
		//create a minimal lit scene comprising of our base meshes
		if (GenerateLitMeshScene(scene, conversionOptions))
			lit = true;
	}


	//call the imposter renderer
	const Util::C_String& imposterExe = conversionOptions.GetToolConfig().GetImposterFilenameWin32();
	Util::C_String imposterExeParams;
	imposterExeParams.Printf("\"%s\" --atlas", conversionOptions.GetImposterSetupFilename().Chars());
	if (lit)
		imposterExeParams += " --base " + conversionOptions.GetImposterTempDirectory() + g_litBaseScene;

	int result = Sys::ShellExec::ExecuteCommandAndWait(
		imposterExe, imposterExeParams, "", Sys::ShellExec::E_LaunchState_Minimise,
		&Tools::g_pMyApplication->GetLogger());

    if(result != 0)
    {
		LOGWARN(Tools::g_log, "Automatic sprite generation failed");
        return true;	//do not abort export
    }

	//read scene back in...
	return AftkHelpers::ReadAftkScene(scene, Util::C_String(conversionOptions.GetImposterSetupFilename()), NULL);
}

}