#include "stdafx.h"

#include <converter_shared/helpers/AftkHelpers.h>

#include <data/DataTypes.h>
#include <data/ConversionOptions.h>
#include <data/SceneDescription.h>
#include <data/RenderData.h>
#include <data/PVSData.h>


#include <process/BuildRenderData.h>
#include <converter_shared/RenderDataConverter.h>
#include <converter_shared/RenderDataConversion/PolyInform.h>


#include <Polyflags.h>

//#include <shader/Constants.h>

/// the scene converter's implementation of RenderDataConversion::I_PolyInform
class RenderPolyInform : public RenderDataConversion::I_PolyInform
{
public:
	/// constructor
	RenderPolyInform(const Polyflags::C_Config *pConfig)
	{
		m_pConfig = pConfig;
	}

	/// return false if the polygon is not supposed to be rendered
	bool IsVisible(const Aftk::C_Polygon *pPoly) const
	{
		return Polyflags::C_Polyflags( *m_pConfig, pPoly ).GetValue( E_Polyflag_Invisible )==0;
	}

	/// Should never be called in model
	bool IsPortalCulled(const Aftk::C_Polygon *pPoly) const
	{
		assert(0);	// Should never get called in model converter.
		return false;
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
	const Polyflags::C_Config *m_pConfig;
};

C_ProcessBuildRenderData::C_ProcessBuildRenderData()
{
}

const Util::C_String C_ProcessBuildRenderData::GetName() const
{
	return Util::C_String("C_ProcessBuildRenderData");
}


void BuildRenderAftkNodeList( const ModelData::C_AftkNodeList& modelAftkNodeList, RenderingData::C_AftkSceneNodeList& renderAftkNodeList )
{
	for(ModelData::C_AftkNodeList::ConstIterator iAftkNode = modelAftkNodeList.Begin(); iAftkNode!=modelAftkNodeList.End(); ++iAftkNode )
	{
		const Aftk::C_SceneNode* pSceneNode = *iAftkNode;
		if(!pSceneNode)
			continue;

		if( AftkHelpers::GetCollisionType( pSceneNode ) == AftkHelpers::E_CollisionType_None )
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

	C_DataRenderData* pOutputRenderData = (C_DataRenderData*)GetOutput( E_GraphData_RenderData );
	assert(pOutputRenderData);

	const C_DataPvs* pInputPVSData = (C_DataPvs*)GetInput( E_GraphData_PvsData );

	RenderingData::C_Converter *pConverter = pOutputRenderData->m_pConverter;

	RenderPolyInform polyInform(&pInputConversionOptions->GetPolyflagConfig());
	pConverter->SetPolyInform( &polyInform );

	S32 lodsToGenerate = pInputConversionOptions->GetLodConfig().GetLodsToGenerate();
	F32 reductionFactor = pInputConversionOptions->GetLodConfig().GetReductionFactor();
	S32 minVertices = pInputConversionOptions->GetLodConfig().GetMinVertices();
	if( lodsToGenerate > 0)
	{
		pConverter->SetLodGenerationOptions(lodsToGenerate, reductionFactor, minVertices, false);
	}

	RenderingData::C_Converter::SetEffectVariation("");
	RenderingData::C_Converter::SetDefaultEffect(pInputConversionOptions->GetModelOptions().GetDefaultEffect());

	pConverter->SetMissingTexture( pInputConversionOptions->GetToolConfig().GetMissingTextureFilename() );

	pConverter->EnableSkinning(true, 32, 4, "_skin");

	if (!pInputConversionOptions->LodFade())
		pConverter->SetLODFadeDistance(0.f);

	pConverter->SetStreamed(false);

	//Set PVS data to the converter
	if (pInputPVSData)
		pConverter->SetPVSData(&pInputPVSData->GetPVSData());
	else
		pConverter->SetPVSData(NULL);

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
			BuildRenderAftkNodeList( pModelElement->GetAftkNodeList(), renderAftkNodeList );

			LOGDEBUG( Tools::g_log, "AddElement:%s", pModelElement->GetName().Chars() );
			ModelData::DumpAfktNodeList( renderAftkNodeList, ":" );

			bool bSharedTextures = pInputConversionOptions->GetModelOptions().GetUseSharedTextures();
			pConverter->AddElement( pModelElement->GetMatrix(), renderAftkNodeList, pModelElement->GetRootAftkNode(), bSharedTextures );
		}

		LOGDEBUG( Tools::g_log, "EndModel:%s", pModel->GetName().Chars() );
		pConverter->EndModel();
	}
		
	pOutputRenderData->m_pConverter->Process();

	return true;
}

