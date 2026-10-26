#include "stdafx.h"

#include <core/containers/genesis_containers/Sort.h>
#include <AftkUt/AftkUt.h>

#include <converter_shared/ProcessGraph.h>
#include <converter_shared/helpers/AftkProgress.h>
#include <converter_shared/helpers/AftkHelpers.h>

#include <data/DataTypes.h>
#include <data/ConversionOptions.h>
#include <data/AftkSceneData.h>
#include <data/SceneDescription.h>

#include <process/BuildSceneDescription.h>

#include <Polyflags.h>
#include <RouteNo.h>

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

	const Aftk::C_SceneNode* pRootNode = pInputLitAftk->GetAftkScene().GetRootSceneNodePtr();
	
	//CreateDefaultModel( modelData );

	// add routes
	const U32 layerCount = pInputLitAftk->GetAftkScene().GetNumLayers();
	for( U32 layerNo=0; layerNo<layerCount; layerNo++)
	{
		const Aftk::C_LayerNode* pLayerNode = pInputLitAftk->GetAftkScene().GetLayerPtr(layerNo);
		S32 routeNo = GetRouteNo(pInputConversionOptions, pLayerNode->GetName().c_str());		
		if( routeNo >= 0 )
		{
			pOutputSceneDescription->AddRoute( routeNo );
		}
	}

	if(pRootNode)
	{
		success = BuildSceneDescription( pOutputSceneDescription, &modelData, NULL, NULL, pRootNode, pInputConversionOptions );
		if(!success)
			return false;
	}

	modelData.CalculateBounds();

	bool dumpAftkSceneNodeList = true;
	modelData.Dump(dumpAftkSceneNodeList);

	return success;
}

const Util::C_String C_ProcessBuildSceneDescription::GetName() const 
{
	return Util::C_String("C_ProcessBuildSceneDescription");
}

bool C_ProcessBuildSceneDescription::BuildSceneDescription(	C_DataSceneDescription* pSceneDescription, 
															ModelData::C_ModelData* pModelData,
															ModelData::C_Model* pCurrModel, 
															ModelData::C_ModelElement* pCurrElement, 
															const Aftk::C_SceneNode* pAftkNode, 
															const C_DataConversionOptions* pOptions )
{
	//LOGDEBUG( Tools::g_log, "Processing Aftk Node '%s'", pAftkNode->GetName().c_str() );

	Objectflags::C_Objectflags objectFlags( pOptions->GetObjectflagConfig(), pAftkNode );

	if( objectFlags.GetValue( E_Objectflag_DontExport ) )
	{
		// skip this node
	}
	else
	if( AftkUt::Query::IsBaseInstance(pAftkNode) )
	{
		if ( AftkUt::Query::IsBaseCrowdInstance(pAftkNode) )
			pSceneDescription->AddCrowdBaseInstanceNode(pAftkNode);
		else
		{
			// skip this node
		}
	}
	else
	if( AftkUt::Query::IsCrowdSkeletonNode(pAftkNode) )
	{
		pSceneDescription->AddCrowdSkeletonNode(pAftkNode);
	}
	else
	if( pAftkNode->GetType() == Aftk::E_ObjType_SceneJoint)
	{
		// skip this node
	}
	else
	if (AftkHelpers::IsSceneNodeLight( AftkHelpers::InstanceGetSourceNode(pAftkNode)))
	{
		pSceneDescription->AddLightNode( pAftkNode );
	}
	else
	if( objectFlags.GetValue( E_Objectflag_NavMesh ) )
	{
		pSceneDescription->AddNavMeshNode( pAftkNode );
	}
	else
	if( objectFlags.GetValue( E_Objectflag_PortalRooms ) )
	{
		pSceneDescription->AddPortalRoomsNode( pAftkNode );
	}
	else
	if( objectFlags.GetValue( E_Objectflag_PvsZones ) )
	{
		pSceneDescription->AddPvsZonesNode( pAftkNode );
	}
	else
	{
		//if( objectFlags.GetValue( E_Objectflag_Model ) )
		//{
		//	ModelData::C_Model* pNewModel = new ModelData::C_Model( pAftkNode->GetName().c_str(), AftkHelpers::GetHMat43( pAftkNode, true, true ), pAftkNode );

		//	if(pCurrModel)
		//	{
		//		pCurrModel->AddChildNode( pNewModel );
		//		//LOGDEBUG( Tools::g_log, "%s->AddChild(%s)", pCurrPlatform->GetName().Chars(), pNewPlatformNode->GetName().Chars());
		//	}
		//	else
		//	{
		//		pSceneDescription->GetModelData().AddRootNode( pNewModel );
		//		//LOGDEBUG( Tools::g_log, "AddRootNode(%s)", pNewPlatformNode->GetName().Chars());
		//	}

		//	pCurrModel = pNewModel;

		//	ModelData::C_ModelElement* pNewElement = new ModelData::C_ModelElement( pAftkNode->GetName().c_str(), AftkHelpers::GetHMat43( pAftkNode, true, true ), pAftkNode );
		//	pCurrModel->AddRootNode( pNewElement );
		//	pCurrElement = pNewElement;
		//}

		//if( objectFlags.GetValue( E_Objectflag_Element ) )
		//{
		//	ModelData::C_ModelElement* pNewElement = new ModelData::C_ModelElement( pAftkNode->GetName().c_str(), AftkHelpers::GetHMat43( pAftkNode, true, true ), pAftkNode );

		//	if(pCurrElement)
		//	{
		//		pCurrElement->AddChildNode( pNewElement );
		//		//LOGDEBUG( Tools::g_log, "%s->AddChild(%s)", pCurrPlatform->GetName().Chars(), pNewPlatformNode->GetName().Chars());
		//	}
		//	else
		//	{
		//		if(pCurrModel)
		//		{
		//			pCurrModel->AddRootNode( pNewElement );
		//		}
		//		else
		//		{
		//			assert(0);
		//		}
		//		//LOGDEBUG( Tools::g_log, "AddRootNode(%s)", pNewPlatformNode->GetName().Chars());
		//	}

		//	pCurrElement = pNewElement;
		//}

		//if( AftkHelpers::IsConstraint( pAftkNode ) )
		//{
		//	pModelData->AddConstraint( pAftkNode );
		//}
		//else
		//{
		//	if(pCurrElement)
		//	{
		//		//S32 routeNo = GetRouteNo(pOptions, pAftkNode);
		//		//if(routeNo > 0)
		//		//{
		//		//	LOGWARN( Tools::g_log, "Element node should be in route 0. Node '%s' in Element '%s'", pAftkNode->GetName().c_str(), pCurrElement->GetName().Chars() );
		//		//}

		//		//LOGDEBUG( Tools::g_log, "Add Aftk Node '%s' to platform '%s'", pAftkNode->GetName().c_str(), pCurrPlatform->GetName().Chars() );
		//		pCurrElement->AddAftkNode( pAftkNode );
		//	}
		//	else
		//	{
		//		//LOGDEBUG( Tools::g_log, "Add Aftk Node '%s' to static mesh", pAftkNode->GetName().c_str());

		//		//S32 routeNo = GetRouteNo(pOptions, pAftkNode);
		//		pSceneDescription->AddStaticMeshNode( pAftkNode );
		//	}
		//}

		if( AftkUt::Query::IsCrowdInstance(pAftkNode) )
		{
			pSceneDescription->AddCrowdInstanceNode( pAftkNode );
		}

		ModelData::C_Model* pNewModel = NULL;

		Maths::HMat43 nodeHMat43 = AftkHelpers::GetHMat43( pAftkNode, true, true );
		//Maths::HMat43 nodeHMat43 = AftkHelpers::GetHMat43( pAftkNode );

		if( objectFlags.GetValue( E_Objectflag_Model ) )
		{
			pNewModel = new ModelData::C_Model( pAftkNode->GetName().c_str(), nodeHMat43, pAftkNode );
		}
		else
		if( (pCurrModel == NULL) && objectFlags.GetValue( E_Objectflag_Element ))
		{
			// create a default model if we are trying to create an element
			pNewModel = new ModelData::C_Model( "DefaultModel", nodeHMat43, pAftkNode );
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

		// only add elements if we have a model
		if( pCurrModel )
		{
			ModelData::C_ModelElement* pNewElement = NULL;

			if( objectFlags.GetValue( E_Objectflag_Element ) || objectFlags.GetValue( E_Objectflag_Model ) )
			{
				pNewElement = new ModelData::C_ModelElement( pAftkNode->GetName().c_str(), nodeHMat43, pAftkNode );
			}
			else
			if( pCurrElement == NULL )
			{
				pNewElement = new ModelData::C_ModelElement( "DefaultElement", nodeHMat43, pAftkNode );
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
		}

		if( AftkHelpers::IsConstraint( pAftkNode ) )
		{
			pModelData->AddConstraint( pAftkNode );
		}
		else
		{
			if(pCurrElement)
			{
				//S32 routeNo = GetRouteNo(pOptions, pAftkNode);
				//if(routeNo > 0)
				//{
				//	LOGWARN( Tools::g_log, "Element node should be in route 0. Node '%s' in Element '%s'", pAftkNode->GetName().c_str(), pCurrElement->GetName().Chars() );
				//}

				//LOGDEBUG( Tools::g_log, "Add Aftk Node '%s' to element '%s'", pAftkNode->GetName().c_str(), pCurrElement->GetName().Chars() );
				pCurrElement->AddAftkNode( pAftkNode );
			}
			else
			{

				S32 routeNo = GetRouteNo(pOptions, pAftkNode);
				if( routeNo == -1 )
				{
					//LOGDEBUG( Tools::g_log, "Add Aftk Node '%s' to static mesh", pAftkNode->GetName().c_str());
					pSceneDescription->AddStaticMeshNode( pAftkNode );
				}
				else
				{
					//LOGDEBUG( Tools::g_log, "Add Aftk Node '%s' to route %d", pAftkNode->GetName().c_str(), routeNo);
					pSceneDescription->AddRouteNode( pAftkNode, routeNo );
				}
			}

			//pCurrElement->AddAftkNode( pAftkNode );			
		}
	}

	// Process all child nodes of pAftkNode - dont need to handle multiple root Aftk nodes, MacLeod says "There can only be one"
	const U32 aftkNodeChildCount = pAftkNode->GetNumChildren();
	for(U32 aftkNodeChildNo=0; aftkNodeChildNo<aftkNodeChildCount; aftkNodeChildNo++)
	{
		const Aftk::C_SceneNode* pChildNode = pAftkNode->GetChildPtr(aftkNodeChildNo);
		bool success = BuildSceneDescription(pSceneDescription, pModelData, pCurrModel, pCurrElement, pChildNode, pOptions);

		if(!success)
			return false;
	}

	return true;
}

