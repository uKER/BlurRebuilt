#ifndef CONVERTER_SHARED_LUX_SAMPLE_LIST_H
#define CONVERTER_SHARED_LUX_SAMPLE_LIST_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/containers/vector.h>

#include <converter_shared/lux_interface/LuxSampleList.h>

namespace LuxSampleList
{

class C_BSPPointTreeNode
{
	S32 m_Index; // Index in NodeVector, if m_Index == -1 -> Leaf Node
	union
	{
		struct  
		{
			S32 m_PlaneIndex;
			C_BSPPointTreeNode* m_pFrontNode;
			C_BSPPointTreeNode* m_pBackNode;
		};
		struct  
		{
			S32 m_FirstPositionIndex;
			S32 m_PositionIndexCount;
		};
	};
	
	friend class C_BSPPointTree;
public:
	C_BSPPointTreeNode(){};
	//Branch Node
	C_BSPPointTreeNode(S32 nodeIndex, S32 planeIndex, C_BSPPointTreeNode* pFrontNode, C_BSPPointTreeNode* pBackNode)
	{
		m_Index = nodeIndex;
		m_PlaneIndex = planeIndex;
		m_pFrontNode = pFrontNode;
		m_pBackNode = pBackNode;
	}
	//Leaf Node
	C_BSPPointTreeNode(S32 firstPositionIndex, S32 positionIndexCount)
	{
		m_Index = -1;
		m_FirstPositionIndex = firstPositionIndex;
		m_PositionIndexCount = positionIndexCount;
	}
	~C_BSPPointTreeNode()
	{
		if (m_Index >= 0)
		{
			SAFE_DELETE_PTR(m_pFrontNode);
			SAFE_DELETE_PTR(m_pBackNode);
		}
		
	}
};

class C_BSPPointTree
{
	struct C_IndexPosition
	{
		U32 m_Index;
		Maths::HVec3 m_Position;
		C_IndexPosition(){};
		C_IndexPosition(U32 index, Maths::HVec3 position) : m_Index(index), m_Position(position) {};
	};
	Btl::Vector<Maths::C_Plane>			m_PlaneVector;
	Btl::Vector<C_BSPPointTreeNode>		m_NodeVector;
	U32									m_NumNodes;
	Btl::Vector<C_IndexPosition>		m_PositionVector;
	//Root Node
	C_BSPPointTreeNode* m_pRootNode;

	//Add Node
	C_BSPPointTreeNode* AddBuildBranchNode(S32 planeIndex, C_BSPPointTreeNode* pFrontNode, C_BSPPointTreeNode* pBackNode);
	C_BSPPointTreeNode* AddBuildLeafNode(Maths::HVec3* positions, U32 numPositions, Btl::Vector<U32>& indexPositionVector);
	//Added using a lineal memory vector
	C_BSPPointTreeNode* AddUnserializeNode();
	//Add Plane
	S32 AddPlane(Maths::C_Plane plane);
	//Internal Build
	C_BSPPointTreeNode* InternalBuild(S32 bucketSize, Maths::HVec3* positions, U32 numPositions, Btl::Vector<U32>& indexPositionVector );

	//Read Node
	template<typename Archive>
	C_BSPPointTreeNode* ReadNode(Archive& ar);
	//Write Node
	template<typename Archive>
	void WriteNode(C_BSPPointTreeNode* pNode, Archive& ar) const;
	
public:
	C_BSPPointTree()
	{
		m_pRootNode = NULL;
		m_NumNodes = 0;
	}

	~C_BSPPointTree()
	{
		SAFE_DELETE_PTR(m_pRootNode);
	}
	void Build(S32 bucketSize, Maths::HVec3* positions, U32 numPositions);
	
	bool Export(Sys::I_File* pFile ) const;
	void Read( Sys::I_File& file );

	
};


class C_SampleList;

// C_SampleData
class C_SampleData
{
public:
	C_SampleData();
	~C_SampleData();

	float operator[](int pos) const { return m_Data[pos]; }
public:
	F32 m_Data[16];

};

// C_Sample

class C_Sample
{
public:
	C_Sample();
	C_Sample( C_SampleList* pSampleList, const S32 index, const Maths::HVec3 PCREF position );
	~C_Sample();
	Maths::HVec3 GetPosition() const;

public:
	Maths::HVec3 m_Position;
	S32 m_Index;
	C_SampleList* m_pSampleList;
	S32 m_SampleDataIndex;
};

// C_SampleList

class C_SampleList
{
public:
	C_SampleList( const S32 dataElementCount = 0 );
	~C_SampleList();

	S32 AddSamplePoint( const Maths::HVec3 PCREF pos );

	bool WriteSamplePointsFile( const Util::C_String& filename );
	bool ReadSamplePointsFile( const Util::C_String& filename );
	bool ReadSampleDataFile( const Util::C_String& filename, const Util::C_String lightModelName);

	void CompressSampleData();

public:
	const S32 m_DataElementCount;

	Btl::Vector<C_Sample> m_SampleList;
	Btl::Vector<C_SampleData> m_SampleDataList;
};

} // LuxSampleList

#endif