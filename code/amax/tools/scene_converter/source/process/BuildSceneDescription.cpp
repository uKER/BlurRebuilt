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
	
	CreateDefaultModel( modelData );

	if(pRootNode)
	{
		success = BuildSceneDescription( pOutputSceneDescription, &modelData, GetDefaultModel(), NULL, pRootNode, pInputConversionOptions );
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

//bool C_ProcessBuildSceneDescription::CreateGroups( SceneDesc::C_SceneDesc& sceneDesc )
//{
//	Btl::Vector< Util::C_String > sceneDescGroupNames;
//	GetSceneDescGroupNames( sceneDescGroupNames );
//
//	assert( sceneDescGroupNames.Size() == E_SceneDescGroup_Count );
//
//	sceneDesc.SetGroupCount( E_SceneDescGroup_Count );
//	for(S32 groupNo=0; groupNo<E_SceneDescGroup_Count; groupNo++)
//	{
//		sceneDesc.GetGroup( groupNo ).SetName( sceneDescGroupNames[groupNo] );
//	}
//	return true;
//}
//
//S32 GetRouteNo(const C_DataConversionOptions* pOptions, const Util::C_String& layerName)
//{
//	const Util::C_String matchString = pOptions->GetToolConfig().GetRouteLayerNameBase();
//	const S32 matchStringLength = matchString.Length();
//	if(matchStringLength == 0)
//	{
//		return 0;
//	}
//
//	const S32 matchStringFirstDigit = matchString.Search('$');
//	const S32 matchStringLastDigit = matchStringLength;
//	const S32 noDigits = matchStringLastDigit - matchStringFirstDigit;
//
//	if(matchStringFirstDigit == -1)
//		return 0;
//
//	const Util::C_String matchStringPrefix = matchString(0, matchStringFirstDigit);
//
//	const S32 routePosStart = layerName.ISearch( matchStringPrefix );
//	if(routePosStart < 0)
//	{
//		return 0;
//	}
//
//	S32 routeDigitsStart = routePosStart + matchStringFirstDigit;
//	S32 routeDigitsEnd = routePosStart + matchStringLastDigit;
//	if( routeDigitsEnd > layerName.Length() )
//	{
//		LOGWARN( Tools::g_log, "Malformed Route layer name '%s'", layerName.Chars());
//		return 0;
//	}
//
//	Util::C_String routeDigits = layerName( routeDigitsStart, routeDigitsEnd );
//
//	for(S32 i=0; i<routeDigits.Length(); i++)
//	{
//		char ch = routeDigits[i];
//		if(( ch < '0' ) || ( ch > '9'))
//		{
//			LOGWARN( Tools::g_log, "Malformed Route layer name '%s'", layerName.Chars());
//			return 0;
//		}
//	}
//
//	return atoi( routeDigits.Chars() );
//}
//
//S32 GetRouteNo( const C_DataConversionOptions* pOptions, const Aftk::C_SceneNode* pAftkNode )
//{
//	const Util::C_String layerName = AftkHelpers::GetLayerName( pAftkNode );
//	return GetRouteNo( pOptions, layerName );
//}

//// find a child / root node with name 'name' or create one if it does not exist
//SceneDesc::C_GroupNode* CreateNamedChildNode( SceneDesc::C_Group& group, SceneDesc::C_GroupNode* pParent, const Util::C_String& name )
//{
//	// find route node
//	SceneDesc::C_GroupNode* pNewNode = NULL;
//
//	if( pParent == NULL )
//	{
//		pNewNode = group.GetRootNode();
//	}
//	else
//	{
//		pNewNode = pParent->GetFirstChild();
//	}
//
//	while(pNewNode != NULL)
//	{
//		if(pNewNode->GetName() == name)
//		{
//			return pNewNode;
//		}
//
//		pNewNode = pNewNode->GetNextSibling();
//	}
//
//	if(pNewNode == NULL)
//	{
//		pNewNode = new SceneDesc::C_GroupNode();
//		pNewNode->SetName( name );
//		if( !pParent )
//		{
//			group.AddRootNode( pNewNode );
//		}
//		else
//		{
//			pParent->AddChildNode( pNewNode );
//		}
//	}
//
//	return pNewNode;
//}
//
//Util::C_String GetRouteName( const S32 routeNo )
//{
//	Util::C_String routeName;
//	routeName.Printf( "Route_%02d", routeNo );
//	return routeName;
//}
//
//bool C_ProcessBuildSceneDescription::CreateRouteNodes(	SceneDesc::C_SceneDesc& sceneDesc, 
//														const Aftk::C_Scene* pAftkScene, 
//														const C_DataConversionOptions* pOptions )
//{
//	// Iterate through layer names
//
//	Btl::Vector< Util::C_String > routeNameList;
//
//	routeNameList.PushBack( GetRouteName(0) );
//
//	U32 layerCount = pAftkScene->GetNumLayers();
//	for(U32 layerNo=0; layerNo<layerCount; layerNo++)
//	{
//		const Aftk::C_LayerNode* pLayerNode = pAftkScene->GetLayerPtr(layerNo);
//		if(pLayerNode)
//		{
//			const Util::C_String layerName = pLayerNode->GetName().c_str();
//			const S32 routeNo = GetRouteNo( pOptions, layerName );
//			const Util::C_String routeName = GetRouteName( routeNo );
//			routeNameList.PushBack( routeName );
//		}
//	}
//
//	// cant find a Ut / btl equivalent of these without defining mad stuff
//
//	const S32 routeNameCount = routeNameList.Size();
//	Util::ShellSortArray<Btl::Vector< Util::C_String >, Util::C_String>(routeNameList, routeNameCount);
//
//	SceneDesc::C_Group& group = sceneDesc.GetGroup( E_SceneDescGroup_StaticLevel );
//
//	for(S32 routeNameNo=0; routeNameNo<routeNameCount; routeNameNo++)
//	{
//		CreateNamedChildNode( group, NULL, routeNameList[routeNameNo] );
//	}
//
//	return true;
//}

//E_SceneDescGroup C_ProcessBuildSceneDescription::ClassifyAftkSceneNode( const C_DataConversionOptions* pOptions, const Aftk::C_SceneNode* pAftkNode )
//{
//	Objectflags::C_Objectflags objectFlags( pOptions->GetObjectflagConfig(), pAftkNode );
//	
//	if( objectFlags.GetValue( E_Objectflag_NavMesh ) )
//	{
//		return E_SceneDescGroup_NavMesh;
//	}
//	else
//	if( objectFlags.GetValue( E_Objectflag_PortalRooms ) )
//	{
//		return E_SceneDescGroup_PortalRooms;
//	}
//	else
//	if( objectFlags.GetValue( E_Objectflag_PvsZones ) )
//	{
//		return E_SceneDescGroup_PvsZones;
//	}
//	else
//	if( objectFlags.GetValue( E_Objectflag_Element ) )
//	{
//		// note: we are just identifying wheteher this is a top level element node.
//		// nodes below in the hierarchy are still part of this element node
//		// but will be returned as E_SceneDescGroup_StaticLevel.
//		return E_SceneDescGroup_Elements;
//	}
//
//	return E_SceneDescGroup_StaticLevel;
//}

bool C_ProcessBuildSceneDescription::BuildSceneDescription(	C_DataSceneDescription* pSceneDescription, 
															ModelData::C_ModelData* pModelData,
															ModelData::C_Model* pCurrModel, 
															ModelData::C_ModelElement* pCurrElement, 
															const Aftk::C_SceneNode* pAftkNode, 
															const C_DataConversionOptions* pOptions )
{
	Objectflags::C_Objectflags objectFlags( pOptions->GetObjectflagConfig(), pAftkNode );

	if (AftkHelpers::IsSceneNodeLight(pAftkNode))
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
		if( objectFlags.GetValue( E_Objectflag_Model ) )
		{
			ModelData::C_Model* pNewModel = new ModelData::C_Model( pAftkNode->GetName().c_str(), AftkHelpers::GetHMat43( pAftkNode ), pAftkNode );

			if(pCurrModel)
			{
				pCurrModel->AddChildNode( pNewModel );
				//LOGDEBUG( Tools::g_log, "%s->AddChild(%s)", pCurrPlatform->GetName().Chars(), pNewPlatformNode->GetName().Chars());
			}
			else
			{
				pSceneDescription->GetModelData().AddRootNode( pNewModel );
				//LOGDEBUG( Tools::g_log, "AddRootNode(%s)", pNewPlatformNode->GetName().Chars());
			}

			pCurrModel = pNewModel;

			ModelData::C_ModelElement* pNewElement = new ModelData::C_ModelElement( pAftkNode->GetName().c_str(), AftkHelpers::GetHMat43( pAftkNode ), pAftkNode );
			pCurrModel->AddRootNode( pNewElement );
			pCurrElement = pNewElement;
		}

		if( objectFlags.GetValue( E_Objectflag_Element ) )
		{
			ModelData::C_ModelElement* pNewElement = new ModelData::C_ModelElement( pAftkNode->GetName().c_str(), AftkHelpers::GetHMat43( pAftkNode ), pAftkNode );

			if(pCurrElement)
			{
				pCurrElement->AddChildNode( pNewElement );
				//LOGDEBUG( Tools::g_log, "%s->AddChild(%s)", pCurrPlatform->GetName().Chars(), pNewPlatformNode->GetName().Chars());
			}
			else
			{
				if(pCurrModel)
				{
					pCurrModel->AddRootNode( pNewElement );
				}
				else
				{
					assert(0);
				}
				//LOGDEBUG( Tools::g_log, "AddRootNode(%s)", pNewPlatformNode->GetName().Chars());
			}

			pCurrElement = pNewElement;
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

				//LOGDEBUG( Tools::g_log, "Add Aftk Node '%s' to platform '%s'", pAftkNode->GetName().c_str(), pCurrPlatform->GetName().Chars() );
				pCurrElement->AddAftkNode( pAftkNode );
			}
			else
			{
				//LOGDEBUG( Tools::g_log, "Add Aftk Node '%s' to static mesh", pAftkNode->GetName().c_str());

				//S32 routeNo = GetRouteNo(pOptions, pAftkNode);
				pSceneDescription->AddStaticMeshNode( pAftkNode );
			}
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

