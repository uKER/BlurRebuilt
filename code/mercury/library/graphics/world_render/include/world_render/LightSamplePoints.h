#ifndef LIGHTSAMPLEPOINTS_H
#define LIGHTSAMPLEPOINTS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/Containers/vector.h>
#include <btl/Containers/List.h>

namespace DbUtil{ class I_DebugRender;};

namespace WorldRender
{

enum E_LightSampleDataType
{
	E_LightSampleDataType_Unknown = -1,
	E_LightSampleDataType_RGBSH_SunSkyFactor = 0,
	E_LightSampleDataType_SunSkySH = 1,
	E_LightSampleDataType_RGBSH = 2,
	E_LightSampleDataType_Count
};

class C_LightSamplePoints;


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
};

class C_BSPPointTree
{
public:
	struct C_IndexPosition
	{
		Maths::StorageVec4F32_F16 m_Position;	// w component is 16-bit sampleindex
		U32 m_iSampleIndex;
	};
private:
	Btl::Vector<Maths::C_Plane>			m_PlaneVector;
	Btl::Vector<C_BSPPointTreeNode>		m_NodeVector;
	U32									m_NumNodes;
	Btl::Vector<C_IndexPosition>		m_PositionVector;
	//Root Node
	C_BSPPointTreeNode* m_pRootNode;

	//Added using a lineal memory vector
	C_BSPPointTreeNode* AddUnserializeNode();

	//Read Node
	template<typename Archive>
	C_BSPPointTreeNode* ReadNode(Archive& ar);

	friend class C_LightSamplePoints;

public:
	C_BSPPointTree()
	{
		m_pRootNode = NULL;
		m_NumNodes = 0;

	}
	C_BSPPointTreeNode* GetRootNode()
	{
		return m_pRootNode;
	}
	const Maths::C_Plane& GetPlane(U32 index) const
	{
		return m_PlaneVector[index];
	}
	void Read( Sys::I_File& file );

	const C_IndexPosition* GetClosestSample( const Maths::HVec3& position, F32& distance ) const;

};


class C_LightSamplePoint
{
public:
	Maths::HVec3 m_Position;
	U32	m_SampleIndex;
};

class C_LightSampleData
{
public:
	F32 m_SampleData[16];
	C_LightSampleData()
	{
		for (U32 i = 0; i < 16; i++)
			m_SampleData[i] = 0.f;
	}
	void Lerp(const C_LightSampleData& destLightSampleData, float t)
	{
		for (U32 i = 0; i < 16; i++)
		{
			m_SampleData[i] = m_SampleData[i] * (1.f - t) + destLightSampleData.m_SampleData[i] * t;
		}
	}
};

class C_LightSamplePoints
{
friend class C_LightSampler;
public:
	C_LightSamplePoints();
	~C_LightSamplePoints();
	
	bool Read( Sys::I_File* pFile );
	void DebugRender( DbUtil::I_DebugRender* pDebugRender, const Maths::HMat43 PCREF m, const Maths::HVec3 PCREF vCamera ) const;
	bool GetClosestSample(C_LightSampleData& result, F32& distance, const Maths::HVec3 PCREF vPosition ) const;
	bool GetClosestSamplePoint(C_LightSamplePoint& result, F32& distance, const Maths::HVec3 PCREF vPosition ) const;

	const S32 GetLightSamplePointCount() const;
	const C_LightSamplePoint GetLightSamplePoint(const S32 index) const;

	void GetSampleData( C_LightSampleData& result, const U32 index ) const;

private:
	//Vector of Light Sample Points
	Btl::Vector<U32> m_LightSampleCompressMap;
	//Vector of Light Sample Data
	//Btl::Vector<C_LightSampleData> m_LightSampleData;
	//fast access BSPPointTree 
	C_BSPPointTree m_BSPPointTree;

	//Light Model
	E_LightSampleDataType	m_LightModel;

	//Compress sample data
	void* m_pCompressLightSampleData;
};


} // namespace WorldRender

#endif // WORLD_RENDER_LEVEL_H

