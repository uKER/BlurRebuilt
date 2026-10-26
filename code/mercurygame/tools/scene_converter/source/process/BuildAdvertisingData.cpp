#include "stdafx.h"

#include <AftkUt/AftkUt.h>

#include <data/DataTypes.h>
#include <data/ConversionOptions.h>
#include <data/AftkSceneData.h>
#include <data/SceneDescription.h>
#include <data/AdvertisingData.h>

#include <process/BuildAdvertisingData.h>

#include <RouteNo.h>

const char* g_sAdNameConvention = "advert";

const Aftk::C_SceneMeshRemappableNode* GetMeshRemappableNode(const Aftk::C_SceneNode* pSceneNode)
{
	if (pSceneNode->HasType(Aftk::E_ObjType_SceneMeshRemappable))
	{
		return (const Aftk::C_SceneMeshRemappableNode*)pSceneNode;
	}
	else if (pSceneNode->HasType(Aftk::E_ObjType_SceneInstance))
	{
		const Aftk::C_SceneInstanceNode* pInst = 
			(const Aftk::C_SceneInstanceNode*)pSceneNode;

		if (pInst->GetSourceNodePtr())
			return GetMeshRemappableNode(pInst->GetSourceNodePtr());
	}

	return 0;
}

bool C_ProcessBuildAdvertisingData::IsAdPlacementNode(const Aftk::C_SceneNode *pAftkSceneNode)
{
	const Aftk::C_SceneMeshRemappableNode* pRemappable = GetMeshRemappableNode(pAftkSceneNode);
	if (pRemappable)
	{
		bool hasAdvertPlug=false, hasAdvertShader=false;
		//look for a plug named "advert"...
		for (U32 i=0; i<pAftkSceneNode->GetNumDynamicPlugs(); i++)
		{
			const Aftk::I_Plug& dynPlug = *pAftkSceneNode->GetDynamicPlugPtr(i);
			if (dynPlug.GetName() == g_sAdNameConvention)
				hasAdvertPlug = true;
		}

		//look for the Advert.fx shader
		for (S16 i=0; i<pRemappable->GetNumPolygonMaterialMappings(); ++i)
		{
			const Aftk::C_MaterialResource *pResource = pRemappable->GetPolygonMaterialMappingPtr(i);
			if (pResource)
			{
				const Aftk::C_MaterialNode *pMaterial = &pResource->GetContent();
				for (U32 j=0; j<pMaterial->GetNumShaderNodes(); ++j)
				{
					const Aftk::C_ShaderNode *pShader = pMaterial->GetShaderNodePtr(j);
					std::string shader;
					const Aftk::I_Plug *pPlug = pShader->GetPlugPtr("shader");
					if (pPlug && pPlug->GetValue(shader) && shader.find("Advert.fx") != std::string::npos)
						hasAdvertShader = true;
				}
			}
		}

		//check if quad?

		if (hasAdvertPlug != hasAdvertShader)
		{
			//if (hasAdvertShader)
			//	LOGWARN(Tools::g_log, "Advertising shader assigned to a non-advert mesh [%s]", pAftkSceneNode->GetPath().c_str());
			if (hasAdvertPlug)
				LOGWARN(Tools::g_log, "Advert mesh has no advert shader assigned [%s]", pAftkSceneNode->GetPath().c_str());
			return false;
		}
		else if (hasAdvertPlug)
		{
			//check valid placement index?
			S32 placementIndex = GetAdPlacementIndex(pAftkSceneNode);

			if (placementIndex >= 0)
				return true;
			else
				LOGWARN(Tools::g_log, "Invalid exposed colour value found on advert shader [%s]", pAftkSceneNode->GetPath().c_str());
		}	
	}

	return false;
}

C_ProcessBuildAdvertisingData::C_ProcessBuildAdvertisingData()
{
}

const Util::C_String C_ProcessBuildAdvertisingData::GetName() const
{
	return Util::C_String("C_ProcessBuildAdvertisingData");
}

S32 C_ProcessBuildAdvertisingData::GetAdPlacementIndex(const Aftk::C_SceneNode *pNode)
{
	S32 index = -1;

	const Aftk::C_SceneMeshRemappableNode* pRemappable = GetMeshRemappableNode(pNode);
	if (pRemappable)
	{
		//look for exposed parameter value on the 'Colour' shader parameter
		for (S16 i=0; i<pRemappable->GetNumPolygonMaterialMappings(); ++i)
		{
			const Aftk::C_MaterialResource *pResource = pRemappable->GetPolygonMaterialMappingPtr(i);
			if (pResource)
			{
				const Aftk::C_MaterialNode *pMaterial = &pResource->GetContent();
				for (U32 j=0; j<pMaterial->GetNumShaderNodes(); ++j)
				{
					const Aftk::C_ShaderNode *pShader = pMaterial->GetShaderNodePtr(j);
					std::string exposedName;
					const Aftk::I_Plug *pPlug = pShader->GetPlugPtr("Colour_ExposedValue");
					if (pPlug && pPlug->GetValue(exposedName))
					{
						if (_strnicmp(exposedName.c_str(), g_sAdNameConvention, 6) == 0)
						{
							const char *pOffset = exposedName.c_str() + 6;
							index = atoi(pOffset);
						}					
					}					
				}
			}
		}
	}

	return index;
}

bool C_ProcessBuildAdvertisingData::Process( Progress::I_ProgressBar* pProgress )
{
	Progress::UpdateSimple progress( pProgress, GetName() );

	C_DataAftkScene* pInputPreprocessedAftk = (C_DataAftkScene*)GetInput( E_GraphData_LitAftk );
	BIZ_ASSERT(pInputPreprocessedAftk);
	if (!pInputPreprocessedAftk->GetAftkScene().GetRootSceneNodePtr())
		return true;

	const C_DataConversionOptions* pInputConversionOptions = (const C_DataConversionOptions*)GetInput( E_GraphData_ConversionOptions );
	BIZ_ASSERT(pInputPreprocessedAftk);

	C_DataAdvertising* pAdvertisingData = (C_DataAdvertising*)GetOutput( E_GraphData_AdvertisingData );

	for (Aftk::C_SceneNodeConstIterator it(&pInputPreprocessedAftk->GetAftkScene()); it; ++it)
	{
		if (IsAdPlacementNode(it))
		{
			//Get quad extents
			Aftk::C_SceneMeshNode *pMeshNode = (Aftk::C_SceneMeshNode*)(&(*it));

			bool isValid = pMeshNode->GetNumVertexPositions()==4 && pMeshNode->GetNumVertexNormals()>0;
			/*
			if (!isValid)
				//should already be triangulated by this point, but we'll check anyway
				isValid = pMeshNode->GetNumPolygons()==1 && pMeshNode->GetPolygon(0).GetNumPolygonVertices()==4;

			if (!isValid)
				isValid = pMeshNode->GetNumPolygons()==2 && 
						  pMeshNode->GetPolygon(0).GetNumPolygonVertices()==3 &&
						  pMeshNode->GetPolygon(0).GetNumPolygonVertices()==3;
			*/
			if (!isValid)
			{
				LOGERROR(Tools::g_log, "Advertising node '%s' is not a quad", it->GetPath().c_str());
				continue;
			}
			
			Advertising::AdvertResource::AdvertQuad quad;

			//ensure order is consistent with the advertising system's expectations (lowerleft, upperleft, upperright, lowerright)
			Aftk::Vector3F32 worldPositions[4] =
			{
				Aftk::ApplyMatrix(it->GetWorldMatrix(), (const Aftk::Vector3F32&)pMeshNode->GetVertexPosition(0)),
				Aftk::ApplyMatrix(it->GetWorldMatrix(), (const Aftk::Vector3F32&)pMeshNode->GetVertexPosition(1)),
				Aftk::ApplyMatrix(it->GetWorldMatrix(), (const Aftk::Vector3F32&)pMeshNode->GetVertexPosition(2)),
				Aftk::ApplyMatrix(it->GetWorldMatrix(), (const Aftk::Vector3F32&)pMeshNode->GetVertexPosition(3))
			};

			quad.v[0] = (MathsStorage::Float3&) worldPositions[0];
			quad.v[1] = (MathsStorage::Float3&) worldPositions[2];
			quad.v[2] = (MathsStorage::Float3&) worldPositions[3];
			quad.v[3] = (MathsStorage::Float3&) worldPositions[1];

			quad.placementIndex = (U16) GetAdPlacementIndex(it);

			U16 routeMask = 0;
			S32 routeNo = GetRouteNo(pInputConversionOptions, it);
			if (routeNo == -1)
				routeMask = 0xffff;
			else
				routeMask = (U16)(1<<routeNo);

			quad.routeMask = (U16) routeMask;
			pAdvertisingData->AddAdvertQuad(quad);
		}
	}

	pAdvertisingData->SortAdvertQuads();

	return true;
}
