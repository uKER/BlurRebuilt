#include "stdafx.h"

#include <converter_shared/helpers/AftkHelpers.h>

#include <data/DataTypes.h>
#include <data/ConversionOptions.h>
#include <data/SceneDescription.h>
#include <data/RenderData.h>


#include <process/BuildRenderData.h>
#include <converter_shared/RenderDataConverter.h>
#include <converter_shared/RenderDataConversion/PolyInform.h>

#include <Polyflags.h>

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
		return Polyflags::C_Polyflags( *m_pConfig, pPoly ).GetValue( E_Polyflag_Invisible )!=0;
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

	RenderingData::C_Converter *pConverter = pOutputRenderData->m_pConverter;

	RenderPolyInform *p = new RenderPolyInform(&pInputConversionOptions->GetPolyflagConfig());
	pConverter->SetPolyInform(p);
	//RenderingData::C_Converter::SetEffectVariation("");
	RenderingData::C_Converter::SetEffectVariation(pInputConversionOptions->GetLightingModelOptions().GetEffectVariation());
	RenderingData::C_Converter::SetDefaultEffect("default.fx");

	RenderingData::C_AftkSceneNodeList renderAftkNodeList;
	BuildRenderAftkNodeList( pInputSceneDescription->GetStaticNodes(), renderAftkNodeList );

	pConverter->AddStaticNodes( renderAftkNodeList );

	if(0)
		pConverter->TestProcess();
	else
		pConverter->Process();
	
	delete p;

	return true;
}

