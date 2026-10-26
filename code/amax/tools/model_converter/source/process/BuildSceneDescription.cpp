#include "stdafx.h"

#include <core/containers/genesis_containers/Sort.h>

#include <converter_shared/ProcessGraph.h>
#include <converter_shared/helpers/AftkProgress.h>
#include <converter_shared/helpers/AftkHelpers.h>

#include <data/DataTypes.h>
#include <data/ConversionOptions.h>
#include <data/AftkSceneData.h>
#include <data/SceneDescription.h>

#include <process/BuildSceneDescription.h>

#include <Polyflags.h>

C_ProcessBuildSceneDescription::C_ProcessBuildSceneDescription()
{
}

C_ProcessBuildSceneDescription::~C_ProcessBuildSceneDescription()
{
}


bool C_ProcessBuildSceneDescription::Process( Progress::I_ProgressBar* pProgress )
{
	const C_DataConversionOptions* pInputConversionOptions = (C_DataConversionOptions*)GetInput( E_GraphData_ConversionOptions );
	assert(pInputConversionOptions);

	const C_DataAftkScene* pInputLitAftk = (C_DataAftkScene*)GetInput( E_GraphData_LitAftk );
	assert(pInputLitAftk);

	C_DataSceneDescription* pOutputSceneDescription = (C_DataSceneDescription*)GetOutput( E_GraphData_SceneDescription );
	assert(pOutputSceneDescription);

	ModelData::C_ModelData& modelData = pOutputSceneDescription->GetModelData();

	bool success = true;
	//success = CreateDefaultModel( modelData ) && success;
	//if(!success)
	//	return false;

	//success = CreateDefaultElement( GetDefaultModel() ) && success;
	//if(!success)
	//	return false;

	const Aftk::C_SceneNode* pRootNode = pInputLitAftk->GetAftkScene().GetRootSceneNodePtr();
	
	if(pRootNode)
	{
		success = BuildSceneDescription( &modelData, NULL, NULL, pRootNode, pInputConversionOptions ) && success;
		if(!success)
			return false;
	}

	bool dumpAftkSceneNodeList = true;
	modelData.Dump(dumpAftkSceneNodeList);
	
	return success;
}

const Util::C_String C_ProcessBuildSceneDescription::GetName() const 
{
	return Util::C_String("C_ProcessBuildSceneDescription");
}

bool C_ProcessBuildSceneDescription::CreateDefaultModel( ModelData::C_ModelData& modelData )
{
	Maths::HMat43 idMatrix;
	idMatrix.SetIdentity();
	ModelData::C_Model* pNewModel = new ModelData::C_Model( "DefaultModel", idMatrix, NULL );

	modelData.AddRootNode(pNewModel);

	m_pDefaultModel = pNewModel;
	return true;
}

ModelData::C_Model* C_ProcessBuildSceneDescription::GetDefaultModel()
{
	return m_pDefaultModel;
}

ModelData::C_ModelElement* C_ProcessBuildSceneDescription::CreateDefaultElement( ModelData::C_Model* pModel )
{
	Maths::HMat43 idMatrix;
	idMatrix.SetIdentity();

	ModelData::C_ModelElement* pNewModelElement = new ModelData::C_ModelElement( "DefaultElement", idMatrix, NULL );

	pModel->AddRootNode(pNewModelElement);

	return pNewModelElement;
}

bool C_ProcessBuildSceneDescription::BuildSceneDescription( ModelData::C_ModelData* pModelData, 
														    ModelData::C_Model* pCurrModel, 
															ModelData::C_ModelElement* pCurrElement, 
															const Aftk::C_SceneNode* pAftkNode, 
															const C_DataConversionOptions* pOptions )
{
	Objectflags::C_Objectflags objectFlags( pOptions->GetObjectflagConfig(), pAftkNode );
	
	ModelData::C_Model* pNewModel = NULL;

	if( objectFlags.GetValue( E_Objectflag_Model ) )
	{
		pNewModel = new ModelData::C_Model( pAftkNode->GetName().c_str(), AftkHelpers::GetHMat43( pAftkNode ), pAftkNode );
	}
	else
	if(pCurrModel == NULL)
	{
		pNewModel = new ModelData::C_Model( "DefaultModel", AftkHelpers::GetHMat43( pAftkNode ), pAftkNode );
	}

	if( pNewModel )
	{
		if( pCurrModel )
		{
			//LOGDEBUG( Tools::g_log, "%s->AddChildModel(%s)", pCurrModel->GetName().Chars(), pNewModel->GetName().Chars());
			pCurrModel->AddChildNode( pNewModel );
		}
		else
		{
			//LOGDEBUG( Tools::g_log, "AddRootModel(%s)", pNewModel->GetName().Chars());
			pModelData->AddRootNode( pNewModel );
		}
	
		pCurrModel = pNewModel;
		pCurrElement = NULL;
	}

	ModelData::C_ModelElement* pNewElement = NULL;

	if( objectFlags.GetValue( E_Objectflag_Element ) || objectFlags.GetValue( E_Objectflag_Model ) )
	{
		pNewElement = new ModelData::C_ModelElement( pAftkNode->GetName().c_str(), AftkHelpers::GetHMat43( pAftkNode ), pAftkNode );
	}
	else
	if( pCurrElement == NULL )
	{
		pNewElement = new ModelData::C_ModelElement( "DefaultElement", AftkHelpers::GetHMat43( pAftkNode ), pAftkNode );
	}

	if( pNewElement )
	{
		if( pCurrElement )
		{
			//LOGDEBUG( Tools::g_log, "%s->AddChildElement(%s)", pCurrElement->GetName().Chars(), pNewElement->GetName().Chars());
			pCurrElement->AddChildNode( pNewElement );
		}
		else
		{
			//LOGDEBUG( Tools::g_log, "%s->AddRootElement(%s)", pCurrModel->GetName().Chars(), pNewElement->GetName().Chars());
			pCurrModel->AddRootNode( pNewElement );
		}

		pCurrElement = pNewElement;
	}

	if( AftkHelpers::IsConstraint( pAftkNode ) )
	{
		pModelData->AddConstraint( pAftkNode );
	}
	else
	{
		pCurrElement->AddAftkNode( pAftkNode );			
	}

	// Process all child nodes of pAftkNode - dont need to handle multiple root Aftk nodes, MacLeod says "There can only be one"
	const U32 aftkNodeChildCount = pAftkNode->GetNumChildren();
	for(U32 aftkNodeChildNo=0; aftkNodeChildNo<aftkNodeChildCount; aftkNodeChildNo++)
	{
		const Aftk::C_SceneNode* pChildNode = pAftkNode->GetChildPtr(aftkNodeChildNo);
		bool success = BuildSceneDescription(pModelData, pCurrModel, pCurrElement, pChildNode, pOptions);

		if(!success)
			return false;
	}

	return true;
}

