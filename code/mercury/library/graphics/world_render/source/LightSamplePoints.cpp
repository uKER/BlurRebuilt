
#include "stdafx.h"

#include <system/serialise/C_FileBlock.h>
#include <world_render/WorldRender.h>
#include <world_render/LightSamplePoints.h>
#include <system/debug/I_DebugRender.h>
#include <System/Memory/MemoryTracker.h> 
#include <misc_shared/types/Float16.h>
#include "btl/containers/algorithm.h"
#include <core\maths\hvec\New\Vec4F32.h>
#include <core\maths\hvec\New\IncludeMinimal.h>
#include <core\maths\hvec\New\IncludeMinimal.inl>

namespace WorldRender
{

Debug::Bool g_RenderLightSamplePoints( "Graphics.WorldRender.LightSamplePoints.RenderLightSamplePoints", false );
Debug::Float g_RenderLightSamplePointSize( "Graphics.WorldRender.LightSamplePoints.Settings.LightSamplePointSize", 0.25f, Debug::Range(0.0f, 1.0f) );
Debug::Float g_RenderLightSamplePointDrawDistance( "Graphics.WorldRender.LightSamplePoints.Settings.LightSamplePointDrawDistance", 50.0f, Debug::Range(0.0f, 10000.0f) );

Debug::Bool g_RenderLightSamplePointValues( "Graphics.WorldRender.LightSamplePoints.Settings.LightSamplePointValues", false );


enum E_LightSamplePointsFileVersion
{
	E_LightSamplePointsFileVersion_Initial = 0,
	E_LightSamplePointsFileVersion = 1
};

//////////////////////////////////////////////////////////////////////////
// Compressed data
//////////////////////////////////////////////////////////////////////////
struct C_RGBSH_SunSkyFactorCompressedLightSampleData
{
	Half::F16 m_CompressData[14];

	void Decompress(C_LightSampleData& sampleData)
	{
		sampleData.m_SampleData[0] = m_CompressData[0];
		sampleData.m_SampleData[1] = m_CompressData[1];
		sampleData.m_SampleData[2] = m_CompressData[2];
		sampleData.m_SampleData[3] = m_CompressData[3];
		sampleData.m_SampleData[4] = m_CompressData[4];
		sampleData.m_SampleData[5] = m_CompressData[5];
		sampleData.m_SampleData[6] = m_CompressData[6];
		sampleData.m_SampleData[7] = m_CompressData[7];
		sampleData.m_SampleData[8] = m_CompressData[8];
		sampleData.m_SampleData[9] = m_CompressData[9];
		sampleData.m_SampleData[10] = m_CompressData[10];
		sampleData.m_SampleData[11] = m_CompressData[11];
		sampleData.m_SampleData[12] = m_CompressData[12];
		sampleData.m_SampleData[13] = m_CompressData[13];
		sampleData.m_SampleData[14] = 0.0f;
		sampleData.m_SampleData[15] = 0.0f;
	}
	template <typename T>
	inline void Deserialise( T& stream )
	{
		stream >> m_CompressData[0].m_U16;
		stream >> m_CompressData[1].m_U16;
		stream >> m_CompressData[2].m_U16;
		stream >> m_CompressData[3].m_U16;
		stream >> m_CompressData[4].m_U16;
		stream >> m_CompressData[5].m_U16;
		stream >> m_CompressData[6].m_U16;
		stream >> m_CompressData[7].m_U16;
		stream >> m_CompressData[8].m_U16;
		stream >> m_CompressData[9].m_U16;
		stream >> m_CompressData[10].m_U16;
		stream >> m_CompressData[11].m_U16;
		stream >> m_CompressData[12].m_U16;
		stream >> m_CompressData[13].m_U16;
	}
};

struct C_RGBSHCompressedLightSampleData
{
	Half::F16 m_CompressData[12];

	void Decompress(C_LightSampleData& sampleData)
	{
		sampleData.m_SampleData[0] = m_CompressData[0];
		sampleData.m_SampleData[1] = m_CompressData[1];
		sampleData.m_SampleData[2] = m_CompressData[2];
		sampleData.m_SampleData[3] = m_CompressData[3];
		sampleData.m_SampleData[4] = m_CompressData[4];
		sampleData.m_SampleData[5] = m_CompressData[5];
		sampleData.m_SampleData[6] = m_CompressData[6];
		sampleData.m_SampleData[7] = m_CompressData[7];
		sampleData.m_SampleData[8] = m_CompressData[8];
		sampleData.m_SampleData[9] = m_CompressData[9];
		sampleData.m_SampleData[10] = m_CompressData[10];
		sampleData.m_SampleData[11] = m_CompressData[11];
	}

	template <typename T>
	inline void Deserialise( T& stream )
	{
		stream >> m_CompressData[0].m_U16;
		stream >> m_CompressData[1].m_U16;
		stream >> m_CompressData[2].m_U16;
		stream >> m_CompressData[3].m_U16;
		stream >> m_CompressData[4].m_U16;
		stream >> m_CompressData[5].m_U16;
		stream >> m_CompressData[6].m_U16;
		stream >> m_CompressData[7].m_U16;
		stream >> m_CompressData[8].m_U16;
		stream >> m_CompressData[9].m_U16;
		stream >> m_CompressData[10].m_U16;
		stream >> m_CompressData[11].m_U16;
	}
};

// C_RenderLevel

C_LightSamplePoints::C_LightSamplePoints()
{
	m_pCompressLightSampleData = NULL;
}

C_LightSamplePoints::~C_LightSamplePoints()
{
	if (m_pCompressLightSampleData)
	{
		switch(m_LightModel)
		{
		case E_LightSampleDataType_RGBSH_SunSkyFactor:
			delete [] (C_RGBSH_SunSkyFactorCompressedLightSampleData*) m_pCompressLightSampleData;
			break;
		case E_LightSampleDataType_RGBSH:
			delete [] (C_RGBSHCompressedLightSampleData*) m_pCompressLightSampleData;
			break;
		}
	}
}


bool C_LightSamplePoints::Read( Sys::I_File* pFile )
{
	bool success = true;

	U32 fileVersion = E_LightSamplePointsFileVersion_Initial;
	// Reading LightSamplePoints
	while ( pFile->GetPosition() < pFile->Length() )
	{
		Sys::C_FileBlock block( pFile );

		if( block.HeaderId() == "Header" )
		{
			Sys::BinaryDeserialiser<Ut::NoEndianSwap> ar( block );
			ar >> fileVersion;

			if (fileVersion < E_LightSamplePointsFileVersion)
			{
				LOGWARN(g_Log, "LightSamplePoint old version data");
				//MEMORY_TRACKER_POP("graphics.worldrender.lightsamplepoints");
				return false;
			}
			S32 aux;
			ar >> aux;
			if (aux == -1)
			{
				LOGWARN(g_Log, "LightSamplePoint unknown lightsampledata format");
				m_LightModel = E_LightSampleDataType_Unknown;
				//MEMORY_TRACKER_POP("graphics.worldrender.lightsamplepoints");
				return false;
			}
			m_LightModel = E_LightSampleDataType(aux);
		}
		else			
		if( block.HeaderId() == "LSPoint" )
		{			
			MEMORY_TRACKER_PUSH_SUB("indicies");

			Sys::BinaryDeserialiser<Ut::NoEndianSwap> ar( block );
			//Read Points
			S32 numLightSamplePoints;
			ar >> numLightSamplePoints;
			m_LightSampleCompressMap.Resize(numLightSamplePoints);

			for (S32 i = 0; i < numLightSamplePoints; i++)
			{
 				Half::F16 f16Aux; //Reading old data
 				ar >> f16Aux.m_U16;
 				ar >> f16Aux.m_U16;
 				ar >> f16Aux.m_U16;

				ar >> m_LightSampleCompressMap[i];
			}

			MEMORY_TRACKER_POP_SUB("indicies");
		}
		else
		if( block.HeaderId() == "LSData" )
		{
			MEMORY_TRACKER_PUSH_SUB("data");
			Sys::BinaryDeserialiser<Ut::NoEndianSwap> ar( block );
			//Read Samples Data
			S32 numLightSampleData;
			ar >> numLightSampleData;

			if(numLightSampleData > 0)
			{
				//Depends of the light model
				switch (m_LightModel)
				{
				case E_LightSampleDataType_RGBSH_SunSkyFactor:
					{
						C_RGBSH_SunSkyFactorCompressedLightSampleData* pCompressData;
						pCompressData = new C_RGBSH_SunSkyFactorCompressedLightSampleData[numLightSampleData];

						for (S32 i = 0; i < numLightSampleData; i++)
						{
							ar >> pCompressData[i];
						}

						m_pCompressLightSampleData = pCompressData;
					}
					break;
				case E_LightSampleDataType_RGBSH:
					{
						C_RGBSHCompressedLightSampleData* pCompressData;
						pCompressData = new C_RGBSHCompressedLightSampleData[numLightSampleData];

						for (S32 i = 0; i < numLightSampleData; i++)
						{
							ar >> pCompressData[i];
						}

						m_pCompressLightSampleData = pCompressData;
					}
					break;
				case E_LightSampleDataType_SunSkySH:
				default:
					LOGERROR(g_Log, "Light model is not supported");

					MEMORY_TRACKER_POP_SUB("data");
					return false;
				}
			}
			MEMORY_TRACKER_POP_SUB("data");
		}
		else
		if( block.HeaderId() == "LSTree" )
		{			
			MEMORY_TRACKER_PUSH_SUB("tree");
			m_BSPPointTree.Read(block);
			MEMORY_TRACKER_POP_SUB("tree");
		}
		else
		{
			LOGWARN( g_Log, "Unknown Block Header '%s'", block.HeaderId().Chars() );
		}
	}

	return true;
}

bool C_LightSamplePoints::GetClosestSample(C_LightSampleData& result, F32& distance, const Maths::HVec3 PCREF vPosition ) const
{
	//First implementation, really slow
	//Maths::HF32 closest = distance;//Maths::Constants<Maths::HF32>::Max();

/*	
	for (S32 i = 0; i < m_LightSamplePoint.Size(); i++)
	{
		if ((m_LightSamplePoint[i].m_Position - position).LengthSquared() < closest)
		{
			closest = (m_LightSamplePoint[i].m_Position - position).LengthSquared();
			result = &m_LightSampleData[m_LightSamplePoint[i].m_SampleIndex];
		}
	}*/


	//Second implementation
	const C_BSPPointTree::C_IndexPosition* pClosest = m_BSPPointTree.GetClosestSample(vPosition, distance);
	if (pClosest == NULL)
		return false;
	U32 closestIndex = pClosest->m_iSampleIndex;
	U32 dataIndex = m_LightSampleCompressMap[closestIndex];

	GetSampleData(result, dataIndex);
	
	//DbUtil::I_DebugRender* debugRender = DbUtil::DefaultDebugRenderSys()->Deferred();
	//debugRender->Draw3dStar(m_LightSamplePoint[closestIndex].m_Position, 1.f, Maths::RGBA(255,0,0,255));

	return true;
}

void C_LightSamplePoints::GetSampleData( C_LightSampleData& result, const U32 index ) const
{
	//Decompress sample data
	switch(m_LightModel)
	{
	case E_LightSampleDataType_RGBSH_SunSkyFactor:
		{
			C_RGBSH_SunSkyFactorCompressedLightSampleData* pCompressSampleData;
			pCompressSampleData = (C_RGBSH_SunSkyFactorCompressedLightSampleData*)m_pCompressLightSampleData;

			pCompressSampleData[index].Decompress(result);
		}
		break;
	case E_LightSampleDataType_RGBSH:
		{
			C_RGBSHCompressedLightSampleData* pCompressSampleData;
			pCompressSampleData = (C_RGBSHCompressedLightSampleData*)m_pCompressLightSampleData;

			pCompressSampleData[index].Decompress(result);
		}
		break;
	}
}

bool C_LightSamplePoints::GetClosestSamplePoint(C_LightSamplePoint& result, F32& distance, const Maths::HVec3 PCREF vPosition ) const
{
	const C_BSPPointTree::C_IndexPosition* pClosest = m_BSPPointTree.GetClosestSample(vPosition, distance);
	if (pClosest != NULL)
	{
		result.m_SampleIndex = m_LightSampleCompressMap[pClosest->m_iSampleIndex];
		result.m_Position = (Maths::HVec3)Maths::Load(&pClosest->m_Position);
		return true;
	}
	return false;
}

void C_LightSamplePoints::DebugRender( DbUtil::I_DebugRender* pDebugRender, const Maths::HMat43 PCREF m, const Maths::HVec3 PCREF vCamera ) const
{
	#ifndef CFG_GOLD
	if (g_RenderLightSamplePoints)
	{
		F32 size = g_RenderLightSamplePointSize;
		F32 drawDistanceSquared = (F32)g_RenderLightSamplePointDrawDistance * (F32)g_RenderLightSamplePointDrawDistance;

		//pDebugRender->SetDepthEnable(false);
		Btl::Vector<C_BSPPointTree::C_IndexPosition>::ConstIterator it = m_BSPPointTree.m_PositionVector.Begin();

		while (it != m_BSPPointTree.m_PositionVector.End())
		{
			const Maths::HVec3 position = (Maths::HVec3)Maths::Load(&(*it).m_Position);
			if(position.DistanceSquared(vCamera) < drawDistanceSquared )
			{
				//Render lightsampler debug
				pDebugRender->Draw3dCross(position, size, Maths::RGBA(1.f,0.f,0.f,1.f));

				if(g_RenderLightSamplePointValues)
				{
					C_LightSampleData lightSampleData;
					GetSampleData( lightSampleData, m_LightSampleCompressMap[it->m_iSampleIndex]);
					Util::C_String str;
					str.Printf( "[%0.2f, %0.2f, %0.2f, %0.2f]\n[%0.2f, %0.2f, %0.2f, %0.2f]\n[%0.2f, %0.2f, %0.2f, %0.2f]\n[%0.2f, %0.2f, %0.2f, %0.2f]", 
							lightSampleData.m_SampleData[0], lightSampleData.m_SampleData[1], lightSampleData.m_SampleData[2], lightSampleData.m_SampleData[3], 
							lightSampleData.m_SampleData[4], lightSampleData.m_SampleData[5], lightSampleData.m_SampleData[6], lightSampleData.m_SampleData[7], 
							lightSampleData.m_SampleData[8], lightSampleData.m_SampleData[9], lightSampleData.m_SampleData[10], lightSampleData.m_SampleData[11], 
							lightSampleData.m_SampleData[12], lightSampleData.m_SampleData[13], lightSampleData.m_SampleData[14], lightSampleData.m_SampleData[15] );
						
					pDebugRender->Draw3dText( position, 0.1f, str );
				}
			}
			
			++it;
		}
		//pDebugRender->SetDepthEnable(true);
	}
	#endif
}

const S32 C_LightSamplePoints::GetLightSamplePointCount() const
{
	return m_LightSampleCompressMap.Size();
}

const C_LightSamplePoint C_LightSamplePoints::GetLightSamplePoint(const S32 index) const
{
	C_LightSamplePoint result;
	result.m_SampleIndex = m_LightSampleCompressMap[m_BSPPointTree.m_PositionVector[index].m_iSampleIndex];
	result.m_Position = (Maths::HVec3)Maths::Load(&m_BSPPointTree.m_PositionVector[index].m_Position);
	return result;
}

//////////////////////////////////////////////////////////////////////////
// BSPPointNodes
//////////////////////////////////////////////////////////////////////////
void C_BSPPointTree::Read( Sys::I_File& file )
{
	Sys::BinaryDeserialiser<Ut::NoEndianSwap> ar( file );

	//Deserialize Plane Vector
	S32 planeVectorSize;
	ar >> planeVectorSize;
	m_PlaneVector.Resize(planeVectorSize);
	for (S32 i = 0; i < planeVectorSize; i++)
		ar >> m_PlaneVector[i];

	//Deserialize Position Vector
	S32 positionVectorSize;
	ar >> positionVectorSize;
	m_PositionVector.Resize(positionVectorSize);
	for (S32 i = 0; i < positionVectorSize; i++)
	{
		U16 *p = (U16*)&m_PositionVector[i].m_Position;
		U32 iSampleIdx;
		ar >> iSampleIdx;
		ar >> p[0];
		ar >> p[1];
		ar >> p[2];
		m_PositionVector[i].m_iSampleIndex = iSampleIdx;
	}

	//Get NumNodes
	S32 numNodes;
	ar >> numNodes;
	m_NodeVector.Resize(numNodes);
	m_NumNodes = 0;

	//Read
	m_pRootNode = ReadNode(ar);
}

C_BSPPointTreeNode* C_BSPPointTree::AddUnserializeNode()
{
	C_BSPPointTreeNode* pNode = &m_NodeVector[m_NumNodes];
	m_NumNodes++;
	return pNode;
}

//Read Node
template<typename Archive>
C_BSPPointTreeNode* C_BSPPointTree::ReadNode(Archive& ar)
{
	S32 index;
	ar >> index;
	switch (index)
	{
	case -1:
		{
			C_BSPPointTreeNode* pNode = AddUnserializeNode();
			//Leaf
			pNode->m_Index = -1;
			ar >> pNode->m_FirstPositionIndex >> pNode->m_PositionIndexCount;

			return pNode;
		}
		break;
	case 1:
		{
			C_BSPPointTreeNode* pNode = AddUnserializeNode();
			//Branch
			pNode->m_Index = 1;
			ar >> pNode->m_PlaneIndex;
			//Read front and back
			pNode->m_pFrontNode = ReadNode(ar);
			pNode->m_pBackNode = ReadNode(ar);

			return pNode;
		}
		break;
	case 0:
		return NULL; //Null node
		break;
	}

	return NULL;
}

struct StackItem
{
	const C_BSPPointTreeNode	*m_pNode;
	float				m_fPlaneDistance;
};

const C_BSPPointTree::C_IndexPosition* C_BSPPointTree::GetClosestSample( const Maths::HVec3& position, F32& distance ) const
{
	const C_BSPPointTree::C_IndexPosition* result = NULL;
	Maths::HF32 closest = distance * distance;//Maths::Constants<Maths::HF32>::Max();

	// push the rootnode onto the stack
	Ut::UtStackVector<StackItem, 32> m_Stack;
	m_Stack.PushBack();
	m_Stack.Back().m_pNode = m_pRootNode;
	m_Stack.Back().m_fPlaneDistance = 0;
	F32 planeDist;

	while (m_Stack.Size())
	{
		// compare the distance of this node against the current distance and cull if it's too far away
		if (m_Stack.Back().m_fPlaneDistance > closest)
		{
			m_Stack.PopBack();
			continue;
		}

		// get the current node
		const C_BSPPointTreeNode *pNode = m_Stack.Back().m_pNode;
		m_Stack.PopBack();
		if (!pNode)
			continue;

		// traverse splitnodes on the near-side of the tree until we reach a leafnode
		// (inner loop)
		while ((pNode->m_Index != -1) )
		{
			Maths::Store(GetPlane(pNode->m_PlaneIndex).Distance( position ), &planeDist);
			int idx = ((*(int*)&planeDist)>>31)&1;
			const C_BSPPointTreeNode *pOnNode = ((C_BSPPointTreeNode**)&pNode->m_pFrontNode)[idx];
			const C_BSPPointTreeNode *pOffNode = ((C_BSPPointTreeNode**)&pNode->m_pFrontNode)[idx^1];

			// push far-side of the tree onto the stack
			m_Stack.PushBack();
			m_Stack.Back().m_pNode = pOffNode;
			m_Stack.Back().m_fPlaneDistance = planeDist * planeDist;//Maths::Abs(planeDist);

			// traverse down near-side of the tree
			pNode = pOnNode;
		}

		// leafnode - find nearest sample position
		const C_IndexPosition* pCurrSample = &m_PositionVector[pNode->m_FirstPositionIndex];
		// n - 1 elements (prefetch)
		for(S32 i=0; i< pNode->m_PositionIndexCount - 1; i++)
		{
			Maths::PrefetchNext(pCurrSample);
			Maths::HVec3 vPos = (Maths::HVec3)Maths::Load(&pCurrSample->m_Position);
			Maths::HF32 currDist = vPos.DistanceSquared( position );
			if( currDist < closest )
			{
				closest = currDist;
				result = pCurrSample;
			}
			pCurrSample++;
		}
		// last element (no prefetch)
		Maths::HVec3 vPos = (Maths::HVec3)Maths::Load(&pCurrSample->m_Position);
		Maths::HF32 currDist = vPos.DistanceSquared( position );
		if( currDist < closest )
		{
			closest = currDist;
			result = pCurrSample;
		}
	}

	distance = Maths::Sqrt(closest);

	return result;
}
} // namespace WorldRender
