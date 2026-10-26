#include "stdafx.h"

#include <converter_shared/helpers/AftkHelpers.h>

#include <data/DataTypes.h>
#include <data/ConversionOptions.h>
#include <data/SceneDescription.h>
#include <data/RenderData.h>
#include <data/PVSData.h>
#include <data/CubemapPointsData.h>
#include <data/PortalData.h>

#include <process/BuildRenderData.h>
#include <converter_shared/RenderDataConverter.h>
#include <converter_shared/RenderDataConversion/PolyInform.h>
#include <converter_shared/renderdataconversion/NodeInform.h>

#include <Polyflags.h>
#include <RouteNo.h>

//#include <shader/Constants.h>

// the scene converter's implementation of RenderDataConversion::I_PolyInform
class RenderPolyInform : public RenderDataConversion::I_PolyInform
{
public:
	// constructor
	RenderPolyInform(const Polyflags::C_Config *pConfig, const bool renderCollideMesh)
		: m_RenderCollideMesh( renderCollideMesh )
		, m_pConfig( pConfig )
	{
	}

	// return false if the polygon is not supposed to be rendered
	bool IsVisible(const Aftk::C_Polygon *pPoly) const
	{
		Polyflags::C_Polyflags polyflags( *m_pConfig, pPoly );

		if(m_RenderCollideMesh)
		{
			const bool hasCollisionFlags = (polyflags.GetValue(E_Polyflag_CollideFlags) != 0);
			if( hasCollisionFlags )
			{
				return true;
			}
		}

		// default case
		const bool invisible = (polyflags.GetValue( E_Polyflag_Invisible ) != 0);
		return !invisible;
	}


	/// Return true if the polygon should be classified and culled using the portal system.
	bool IsPortalCulled(const Aftk::C_Polygon* pPoly) const
	{
		Polyflags::C_Polyflags polyflags(*m_pConfig, pPoly);
		const bool bNoCull = (polyflags.GetValue(E_Polyflag_ForceNoPortalCulling) != 0);
		return !bNoCull;
	}

	/// Return true if the polygon should be rendered with cullmode none.
	bool DoNotBackfaceCull(const Aftk::C_Polygon *pPoly) const
	{
		Polyflags::C_Polyflags polyflags( *m_pConfig, pPoly );

		const bool doNotBackfaceCull = (polyflags.GetValue(E_Polyflag_NoBackfaceCulling) != 0);
		return doNotBackfaceCull;
	}

	/// Return custom primitive flags
	int GetCustomFlags(const Aftk::C_Polygon *pPoly) const
	{
		return Polyflags::C_Polyflags( *m_pConfig, pPoly ).GetValue( E_Polyflag_CustomPrimitiveFlags );
	}

private:
	const bool m_RenderCollideMesh;
	const Polyflags::C_Config *m_pConfig;
};

// the scene converter's implementation of RenderDataConversion::I_NodeInform
class RenderNodeInform : public RenderDataConversion::I_NodeInform
{
public:
	// constructor
	RenderNodeInform(const C_DataConversionOptions* pDataConversionOptions )
	{
		m_pDataConversionOptions = pDataConversionOptions;
	}

	// returns the route number of the node 
	S32 GetRouteNo(const Aftk::C_SceneNode *pSceneNode) const
	{
		return ::GetRouteNo( m_pDataConversionOptions, pSceneNode );
	}

private:
	const C_DataConversionOptions* m_pDataConversionOptions;
};

C_ProcessBuildRenderData::C_ProcessBuildRenderData()
{
}

const Util::C_String C_ProcessBuildRenderData::GetName() const
{
	return Util::C_String("C_ProcessBuildRenderData");
}

void BuildRenderAftkNodeList( const ModelData::C_AftkNodeList& modelAftkNodeList, RenderingData::C_AftkSceneNodeList& renderAftkNodeList, const bool renderCollideMesh )
{
	for(ModelData::C_AftkNodeList::ConstIterator iAftkNode = modelAftkNodeList.Begin(); iAftkNode!=modelAftkNodeList.End(); ++iAftkNode )
	{
		const Aftk::C_SceneNode* pSceneNode = *iAftkNode;
		if(!pSceneNode)
			continue;

		const bool isCollisionNode = (AftkHelpers::GetCollisionType( pSceneNode ) != AftkHelpers::E_CollisionType_None);

		if(!isCollisionNode || renderCollideMesh)		
		{
			renderAftkNodeList.PushBack( pSceneNode );
		}
	}
}

bool C_ProcessBuildRenderData::Process( Progress::I_ProgressBar* pProgress )
{
	Progress::UpdateSimple progress( pProgress, GetName() );

	const C_DataConversionOptions* pInputConversionOptions = (const C_DataConversionOptions*)GetInput( E_GraphData_ConversionOptions );
	assert(pInputConversionOptions);

	const C_DataSceneDescription* pInputSceneDescription = (const C_DataSceneDescription*)GetInput( E_GraphData_SceneDescription );
	assert(pInputSceneDescription);

	const C_DataCubemapPoints* pInputCubemapPoints = (const C_DataCubemapPoints*)GetInput( E_GraphData_CubemapPoints );
	assert(pInputCubemapPoints);

	const C_DataPvs* pInputPVSData = (C_DataPvs*)GetInput( E_GraphData_PvsData );
	C_DataPortalData* pInputPortalData = (C_DataPortalData*)GetInput( E_GraphData_PortalRooms );

	C_DataRenderData* pOutputRenderData = (C_DataRenderData*)GetOutput( E_GraphData_RenderData );
	assert(pOutputRenderData);

	pOutputRenderData->m_EffectVariation = pInputConversionOptions->GetLightingModelOptions().GetEffectVariation();
	// we don't use the default effect variation
	//if(pInputConversionOptions->GetLightingOptions().GetLightScene())
	//{
	//	pOutputRenderData->m_EffectVariation = pInputConversionOptions->GetLightingModelOptions().GetEffectVariation();
	//}
	//{
	//	pOutputRenderData->m_EffectVariation = "";
	//}

	const bool renderCollideMesh = pInputConversionOptions->RenderCollideMesh();

	RenderingData::C_Converter *pConverter = pOutputRenderData->m_pConverter;

	RenderPolyInform polyInform(&pInputConversionOptions->GetPolyflagConfig(), renderCollideMesh);
	RenderNodeInform nodeInform(pInputConversionOptions);
	
	pConverter->SetPolyInform(&polyInform);
	pConverter->SetNodeInform(&nodeInform);
	pConverter->SetCubemapInform(pInputCubemapPoints);
	//RenderingData::C_Converter::SetEffectVariation("");
	RenderingData::C_Converter::SetEffectVariation(pInputConversionOptions->GetLightingModelOptions().GetEffectVariation());
	// we don't use the default effect variation
	//if(pInputConversionOptions->GetLightingOptions().GetLightScene())
	//{
	//	RenderingData::C_Converter::SetEffectVariation(pInputConversionOptions->GetLightingModelOptions().GetEffectVariation());
	//}
	RenderingData::C_Converter::SetDefaultEffect("default.fx");

	pConverter->SetMissingTexture( pInputConversionOptions->GetToolConfig().GetMissingTextureFilename() );

	pConverter->EnableSkinning(true, 32, 4, "_skin", pInputConversionOptions->GetToolConfig().CrowdRotateY180());

	for (S32 i = 0, count = pInputSceneDescription->GetNumCrowdSkeletonNodes(); i < count; ++i)
		pConverter->AddSkinningSkeletonNode(pInputSceneDescription->GetCrowdSkeletonNode(i));

	// Set the Portal data to the convertor.
	if(pInputPortalData)
	{
		if(!pInputPortalData->GetBuilder().GetRoomCount())	// No rooms.
			pConverter->SetPortalData(NULL);
		else
			pConverter->SetPortalData(&pInputPortalData->GetBuilder());
	}

	//Set PVS data to the converter
	if (pInputPVSData)
		pConverter->SetPVSData(&pInputPVSData->GetPVSData());
	else
		pConverter->SetPVSData(NULL);

	pConverter->SetLodGenerationOptions(0, 0.f, 8, true);

	RenderingData::C_AftkSceneNodeList renderAftkNodeList;
	BuildRenderAftkNodeList( pInputSceneDescription->GetStaticNodes(), renderAftkNodeList, renderCollideMesh );

	// add route nodes into this list too, they are handled by the render data converter
	for(S32 routeNo=0; routeNo<pInputSceneDescription->GetRouteCount(); routeNo++)
	{
		BuildRenderAftkNodeList( pInputSceneDescription->GetRouteNodes( routeNo ), renderAftkNodeList, renderCollideMesh );
	}

	pConverter->AddStaticNodes( renderAftkNodeList );

	// add Dynamic nodes

	const ModelData::C_ModelData& modelData = pInputSceneDescription->GetModelData();

	for( ModelData::C_ModelConstIterator iModel( &modelData ); iModel; ++iModel )
	{
		const ModelData::C_Model* pModel = iModel;

		LOGDEBUG( Tools::g_log, "BeginModel:%s", pModel->GetName().Chars() );
		pConverter->BeginModel();

		for( ModelData::C_ModelElementConstIterator iModelElement( pModel ); (const ModelData::C_ModelElement*)iModelElement; ++iModelElement )
		{
			const ModelData::C_ModelElement* pModelElement = iModelElement;
			
			RenderingData::C_AftkSceneNodeList renderAftkNodeList;
			BuildRenderAftkNodeList( pModelElement->GetAftkNodeList(), renderAftkNodeList, renderCollideMesh );

			LOGDEBUG( Tools::g_log, "AddElement:%s", pModelElement->GetName().Chars() );
			ModelData::DumpAfktNodeList( renderAftkNodeList, ":" );

			pConverter->AddElement( pModelElement->GetMatrix(), renderAftkNodeList, pModelElement->GetRootAftkNode(), true );
		}

		LOGDEBUG( Tools::g_log, "EndModel:%s", pModel->GetName().Chars() );
		pConverter->EndModel();
	}
		
	//if(0)
	//	pConverter->TestProcess();
	//else
		pConverter->Process();
	
	return true;
}

