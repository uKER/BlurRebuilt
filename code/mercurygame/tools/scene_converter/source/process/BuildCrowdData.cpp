#include "stdafx.h"

#include <converter_shared/helpers/AftkHelpers.h>

#include <AftkUt/AftkUt.h>

#include <data/DataTypes.h>
#include <data/ConversionOptions.h>
#include <data/AftkSceneData.h>
#include <data/SceneDescription.h>
#include <data/CrowdData.h>
#include <data/PVSData.h>

#include <process/BuildCrowdData.h>
#include <converter_shared/RenderDataConverter.h>
#include <converter_shared/renderdataconversion/NodeInform.h>

#include <RouteNo.h>

#include <system/misc/Random.h>

#include <Btl/Utility/Hash.h>

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

bool HasSHLighting(const Aftk::C_SceneNode &node)
{
	const Aftk::I_Plug *pPlug = node.GetPlugPtr("luxCustom");
	if (!pPlug)
		return false;

	return true;
}

int HexToI(char c)
{
	int v = c-'0';
	if (v>=0 && v<10)
		return v;
	v = c-'a';
	if (v>=0 && v<6)
		return v + 10;
	v = c-'A';
	if (v>=0 && v<6)
		return v + 10;
	return -1;
}

U8 *ConvertHexToBin(const std::string &name, int *piDataSize)
{
	int c = name.length()/2;
	U8 *pData = new U8[c];
	*piDataSize = c;
	U8 *pBinData = pData;

	// convert hex number to binary data
	const char *psz = name.c_str();
	for (int i=0; i<c; i++, pBinData++, psz+=2)
	{
		int v1 = HexToI(psz[0]);
		if (v1 == -1)
			return false;
		int v2 = HexToI(psz[1]);
		if (v2 == -1)
			return false;
		*pBinData = (v1 << 4) + v2;
	}
	return pData;
}

bool GetLuxCustomData(const Aftk::C_SceneNode &node, float *pOutValue, int iOffset)
{
	const Aftk::I_Plug *pPlug = node.GetPlugPtr("luxCustom");
	
	std::string text;
	if (!pPlug || !pPlug->GetValue(text))
		return false;

	int iSize;
	float *pData = (float*)ConvertHexToBin(text, &iSize);
	if (iSize<4)
	{
		delete pData;
		return false;		// not floating point data
	}

	if (iOffset >= iSize/4)
	{
		/*PrintWarning("Reading past the end of LuxCustomData with size %d at offset %d on node %s.\n",
			iSize/4, iOffset, node.GetName().c_str());*/

		delete pData;
		return false;
	}

	int i;
	for (i=0; i<16; i++)
	{
		if (i+iOffset >= iSize/4)
			break;
		pOutValue[i] = pData[iOffset + i];
	}
	for (;i<16; i++)
		pOutValue[i] = 0;

	delete pData;
	return true;
}

const Aftk::C_SceneNode* GetSceneNodeFromBlindData(const Aftk::C_Scene* pScene, const Aftk::C_BlindDataResource* pBlindDataResourcePtr)
{
	if (pScene && pBlindDataResourcePtr)
	{
		const Aftk::C_BlindDataNode& blindData = pBlindDataResourcePtr->GetContent();
		const Aftk::C_GenericAttribute& attr = blindData.GetAttribute(0);

		if (attr.GetDataType() == Aftk::E_GenericAttributeType_S32)
		{
			S32 sceneNodeID = -1;
			attr.GetValue(sceneNodeID);

			if (sceneNodeID >= 0)
				return pScene->GetSceneNodePtrByUniqueID(sceneNodeID);
		}
	}

	return NULL;
}

C_ProcessBuildCrowdData::C_ProcessBuildCrowdData()
{
}

const Util::C_String C_ProcessBuildCrowdData::GetName() const
{
	return Util::C_String("C_ProcessBuildCrowdData");
}

bool C_ProcessBuildCrowdData::Process( Progress::I_ProgressBar* pProgress )
{
	Progress::UpdateSimple progress( pProgress, GetName() );

	const C_DataConversionOptions* pInputConversionOptions = (const C_DataConversionOptions*)GetInput( E_GraphData_ConversionOptions );
	assert(pInputConversionOptions);

	const C_DataSceneDescription* pInputSceneDescription = (const C_DataSceneDescription*)GetInput( E_GraphData_SceneDescription );
	assert(pInputSceneDescription);

	const C_DataPvs* pInputPVSData = (const C_DataPvs*)GetInput( E_GraphData_PvsData );

	C_DataCrowd* pCrowdData = (C_DataCrowd*)GetOutput( E_GraphData_CrowdData );
	assert(pCrowdData);

	bool rotateY180 = pInputConversionOptions->GetToolConfig().CrowdRotateY180();

	if (pInputSceneDescription->GetCrowdBaseInstanceNodes().Size() <= 0)
		return true;

	const Aftk::C_Scene* pScene = pInputSceneDescription->GetCrowdBaseInstanceNodes()[0]->GetScenePtr();

	if (!pScene)
		return true;

	RenderNodeInform nodeInform(pInputConversionOptions);

	S32 routeCount = pInputSceneDescription->GetRouteCount();

	pCrowdData->Seed(pInputConversionOptions->GetCrowdRandomSeed());

	Util::C_Random random;
	random.Seed(pInputConversionOptions->GetCrowdRandomSeed());

	Btl::Map<U32, U32>			behaviourMap;
	Btl::Map<std::string, U32>	characterMap;

	for (S32 i = 0; i < pInputSceneDescription->GetCrowdBaseInstanceNodes().Size(); ++i)
	{
		const Aftk::C_SceneNode* pCurrAftkNode = pInputSceneDescription->GetCrowdBaseInstanceNodes()[i];

		AnimatedCrowd::C_CrowdCharacterRef character;

		U32 behaviourHash = 0;
		bool storeSHNode = false;

		bool bStatus = false;
		AftkUt::C_NodeFlagParser nodeFlags(*pCurrAftkNode, &bStatus);
		if (bStatus)
		{
			if (_stricmp(nodeFlags.GetFlagGroupCategory().c_str(), "crowd") == 0)
			{
				std::string groupName = nodeFlags.GetFlagGroupName();

				if (_stricmp(groupName.c_str(), "Placeholder") == 0)
				{
					std::string behaviourString;
					
					if (nodeFlags.GetParameterValue("Behaviour", behaviourString))
						behaviourHash = Btl::Hash(behaviourString.c_str());

					nodeFlags.GetParameterValue("StoreSHNode", storeSHNode);
					if (storeSHNode)
						character.StoreSHNode(storeSHNode);
				}
			}
		}

		Btl::Map<U32, U32>::ConstIterator iter = behaviourMap.Find(behaviourHash);
		if (iter != behaviourMap.End())
		{
			character.SetBehaviourIndex((*iter).GetValue());
		}
		else
		{
			U32 behaviourIndex = pCrowdData->GetCrowdBehavioursSize();
			behaviourMap.Insert(behaviourHash, behaviourIndex);
			pCrowdData->AddCrowdBehaviour(behaviourHash);
			character.SetBehaviourIndex(behaviourIndex);
		}

		//Maths::HVec4 extents = AnimatedCrowd::GetExtentsFromBehaviourType(); // TODO: [rthomas]
		
		F32 extentsMinXZ = -2.f;
		F32 extentsMaxXZ = 2.f;
		F32 extentsMinY = -2.f;
		F32 extentsMaxY = 2.f;

		if (bStatus)
			nodeFlags.GetParameterValue("BoundsExtents", extentsMinXZ, extentsMaxXZ, extentsMinY, extentsMaxY);
		
		Maths::C_AxisBox axisBox(Maths::HVec3(extentsMinXZ, extentsMinY, extentsMinXZ), Maths::HVec3(extentsMaxXZ, extentsMaxY, extentsMaxXZ));
		character.SetAxisBox(axisBox);

		characterMap.Insert(((Aftk::C_Node*)pCurrAftkNode)->GetName(), pCrowdData->GetCrowdCharactersSize());
		pCrowdData->AddCrowdCharacter(character);
	}

	Btl::Vector<Btl::Pair<U32, const Aftk::C_SceneNode*> > instanceSHNodes;

	for (S32 i = 0; i < pInputSceneDescription->GetCrowdInstanceNodes().Size(); ++i)
	{
		const Aftk::C_SceneNode* pCurrAftkNode = pInputSceneDescription->GetCrowdInstanceNodes()[i];
		const Aftk::C_SceneNode* pCurrAftkSourceNode = AftkHelpers::InstanceGetSourceNode(pCurrAftkNode);

		Aftk::E_ObjType sourceNodeType = pCurrAftkSourceNode->GetType();

		AnimatedCrowd::C_CrowdInstance instance;

		instance.SetCharacterIndex(characterMap[((Aftk::C_Node*)pCurrAftkSourceNode)->GetName()]);

		const AnimatedCrowd::C_CrowdCharacterRef* pCharacterRef = pCrowdData->GetCrowdCharacter(instance.GetCharacterIndex());
		
		if (pCharacterRef && pCharacterRef->StoreSHNode())
		{
			// TODO: [rthomas]	Need to get my hands on the instance this is a placeholder for ...
			//					Then grab what that is an instance of and pass to SH node list
			const Aftk::C_SceneNode* pMeshNode = GetSceneNodeFromBlindData(pScene, pCurrAftkNode->GetBlindDataResourcePtr());
			instanceSHNodes.PushBack(Btl::Pair<U32, const Aftk::C_SceneNode*>(pCrowdData->GetCrowdInstancesSize(), pMeshNode));
		}

		instance.SetUniqueId(AftkUt::CrowdHelper::GetCrowdInstanceIndex(pCurrAftkNode));

		Aftk::Matrix44 worldMatrix = pCurrAftkNode->GetWorldMatrix();

		Maths::HMat43 worldTransform;
		Aftk::TemplateToNightmareMatrix43(worldTransform, worldMatrix);

		Maths::HVec3 scale;
		Maths::HVec3 shear;
		Maths::HVec3 rotate;
		Maths::HVec3 translate;

		Aftk::GetSShRT(scale, shear, rotate, translate, worldTransform, pCurrAftkNode->GetRotationOrder());

		instance.SetPosition(translate);
		
		{
			// calculate direction, set on instance
			Maths::HMat33 rotationX(Maths::RotX(rotate.X()));
			Maths::HMat33 rotationY(Maths::RotY(rotate.Y()));
			Maths::HMat33 rotationZ(Maths::RotZ(rotate.Z()));

			Maths::HMat33 rotationMat = (rotationX * rotationY) * rotationZ;
			Maths::HVec3 direction = Maths::HVec3(0.f, 0.f, 1.f) * rotationMat;
			instance.SetDirection(direction);
		}

		// TODO: [rthomas]	Consider removing scale from instance as I can't seem to scale them in maya...
		//					hence it remains (1.f, 1.f, 1.f)
		//					Actually, appears to be set on the base instance, so move to C_CrowdCharacter?
		Maths::HF32 largestScale = scale.X();
		if (Maths::Abs<Maths::HF32>(scale.Y()) > Maths::Abs<Maths::HF32>(largestScale))
			largestScale = scale.Y();
		if (Maths::Abs<Maths::HF32>(scale.Z()) > Maths::Abs<Maths::HF32>(largestScale))
			largestScale = scale.Z();

		instance.SetScale(largestScale);

		instance.SetRandomSeed(random.Rand());

		instance.SetRoute(nodeInform.GetRouteNo(pCurrAftkNode));

		pCrowdData->AddCrowdInstance(instance);
	}

	for (U32 i = 0, count = static_cast<U32>(instanceSHNodes.Size()); i < count; ++i)
	{
		if (instanceSHNodes[i].second && HasSHLighting(*(instanceSHNodes[i].second)))
		{
			AnimatedCrowd::C_CrowdSHNodeData shNodeData;
			shNodeData.SetInstanceIndex(instanceSHNodes[i].first);
			GetLuxCustomData(*(instanceSHNodes[i].second), shNodeData.GetFloatArrayPtr(), 0);
			pCrowdData->AddCrowdInstanceSHNode(shNodeData);
		}
	}

	characterMap.Clear();

	const Pvs::Build::PvsData* pPVSData = NULL;
	if (pInputPVSData)
		pPVSData = &(pInputPVSData->GetPVSData());

	pCrowdData->BuildBlocks(pPVSData);

	pCrowdData->SortBlocks(routeCount);

	pCrowdData->ConstructBlockGrid(AnimatedCrowd::CROWD_GRID_CELL_SIZE, AnimatedCrowd::CROWD_GRID_CELL_RANGE);

	return true;
}

