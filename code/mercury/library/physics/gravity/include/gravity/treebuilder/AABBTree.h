#ifndef NEWAABBTREE_H
#define NEWAABBTREE_H

#include "gravity/treebuilder/GenericTree.h"
#include "gravity/treebuilder/GenericTree.inl"
//#include "SortedArray.h"
//#include "Primitive.h"
#include "gravity/narrowphase/OptimisedAABBTree.h"
#include "gravity/sharedphysics/list.h"


namespace Baking
{

inline F32 Score(S32 leftCount, S32 rightCount, S32 activeCount, F32 midLow, F32 midHigh, F32 low, F32 high)
{
	F32 lc = (F32)leftCount;
	F32 rc = (F32)rightCount;
	F32 ac = (F32)activeCount;
	/*F32 range = high - low;
	F32 midRange(midHigh - midLow);
	F32 proportionOverlap = midRange/range;*/
	F32 scoreA = Maths::Abs(lc - rc) + 1.5f * ac;
	
	//F32 overlapScore = 200.f * proportionOverlap * F32(activeCount);
	return scoreA;// + overlapScore;
}

inline F32 Score2(S32 leftCount, S32 rightCount, S32 activeCount, F32 midLow, F32 midHigh, F32 low, F32 high)
{
	F32 front = (F32)leftCount;
	F32 back = (F32)rightCount;
	F32 straddle = (F32)activeCount;

	F32 total = front + back + straddle;

	F32 frontProportion = front/total;
	F32 backProportion = back/total;
	F32 straddleProportion = straddle/total;

	F32 score(frontProportion + straddleProportion);
	return score;


}




class AABBTree;

class AABBTreeNode: public GenericTreeNode<AABBTreeNode>
{
public:

	//The constructor for the leave node
	AABBTreeNode(U32 startIndex, U32 size):	
											GenericTreeNode<AABBTreeNode>(startIndex, size),
											m_NumDescendants(0)
	{
	}

	Collision::C_AxisBoxMinMax aabb;

	//Counts the number of descendant nodes this node has...
	U32 m_NumDescendants;

	friend class AABBTree;

};

class AABBBuildTreeTriangle : public SharedPhysics::C_DoubleListElem<AABBBuildTreeTriangle>
{
public:
	U32 m_IndexA, m_IndexB, m_IndexC;
	U32 m_NormalIndex;
	U32 m_CollisionFlags;
	U32 m_UserDataFlags;
	U16 m_PhysicsMaterialIndex;
	U16 m_EffectMaterialIndex;
	U8 m_ActiveEdgeFlags;
	MathsStorage::HenDN3 m_EdgeNormA;
	MathsStorage::HenDN3 m_EdgeNormB;
	MathsStorage::HenDN3 m_EdgeNormC;

	template <typename Serial>
	void Serialise(Serial& s);

	template <typename Serial>
	void DeSerialise(Serial& s);

	NOALIAS __forceinline bool ActiveEdge(const U32 edge) const
	{
		return (m_ActiveEdgeFlags & (1 << edge)) != 0;
	}

	NOALIAS FORCE_INLINE bool operator > (const AABBBuildTreeTriangle& tri)
	{
		return (m_CollisionFlags > tri.m_CollisionFlags) || 
			(m_CollisionFlags == tri.m_CollisionFlags && m_PhysicsMaterialIndex > tri.m_PhysicsMaterialIndex) ||
			(m_CollisionFlags == tri.m_CollisionFlags && m_PhysicsMaterialIndex == tri.m_PhysicsMaterialIndex && 
			m_EffectMaterialIndex > tri.m_EffectMaterialIndex) || 
			(m_CollisionFlags == tri.m_CollisionFlags && m_PhysicsMaterialIndex == tri.m_PhysicsMaterialIndex && 
			m_EffectMaterialIndex == tri.m_EffectMaterialIndex && m_UserDataFlags > tri.m_UserDataFlags);
	}

	bool SameIndices(AABBBuildTreeTriangle& other)
	{
		return (m_IndexA == other.m_IndexA || m_IndexA == other.m_IndexB || m_IndexA == other.m_IndexC) &&
			(m_IndexB == other.m_IndexA || m_IndexB == other.m_IndexB || m_IndexB == other.m_IndexC) && 
			(m_IndexC == other.m_IndexA || m_IndexC == other.m_IndexB || m_IndexC == other.m_IndexC);
	}

	void MergeTriangle(AABBBuildTreeTriangle& tri)
	{
		m_CollisionFlags |= tri.m_CollisionFlags;
		m_UserDataFlags |= tri.m_UserDataFlags;
	}

	U32 GetSpareIndex(const U32 indexA, const U32 indexB)
	{
		if(m_IndexA != indexA && m_IndexA != indexB)
			return m_IndexA;
		if(m_IndexB != indexA && m_IndexB != indexB)
			return m_IndexB;
		return m_IndexC;

	}
};

template <typename Serial>
void AABBBuildTreeTriangle::Serialise(Serial& s)
{
	s<<m_IndexA << m_IndexB << m_IndexC << m_NormalIndex << m_CollisionFlags << m_UserDataFlags << m_PhysicsMaterialIndex << m_EffectMaterialIndex << m_ActiveEdgeFlags
		<< m_EdgeNormA << m_EdgeNormB << m_EdgeNormC;
}

template <typename Serial>
void AABBBuildTreeTriangle::DeSerialise(Serial& s)
{
	s >> m_IndexA >> m_IndexB >> m_IndexC >> m_NormalIndex >>m_CollisionFlags >> m_UserDataFlags >> m_PhysicsMaterialIndex >> m_EffectMaterialIndex >> m_ActiveEdgeFlags;
	if(s.m_Stream.GetVersionNumber() >= 7)
		s >> m_EdgeNormA >> m_EdgeNormB >> m_EdgeNormC;
}

class C_EdgeEvent
{
public:
	Maths::HVec3 m_Max;
	Maths::HVec3 m_Min;
	F32 value;
	F32 range;
	bool bStart;

	C_EdgeEvent() : m_Max(Maths::UNDEFINED), m_Min(Maths::UNDEFINED)
	{
	}

	bool operator < (const C_EdgeEvent& ev) const
	{
		return (value < ev.value);/* || 
			(value == ev.value && bStart);*/
	}
	bool operator <= (const C_EdgeEvent& ev) const
	{
		return (value <= ev.value);
	}

	bool operator > (const C_EdgeEvent& ev) const
	{
		return (value > ev.value); /*|| 
			(value == ev.value && !bStart);*/
	}

	bool operator >= (const C_EdgeEvent& ev) const
	{
		return (value >= ev.value); 
	}

	bool operator == (const C_EdgeEvent& ev) const
	{
		return value == ev.value;
	}
};

class C_TreeVertex
{
public:
	Maths::HF32 m_Projection;
	Maths::HVec3 m_Vertex;
	U32 m_Index;
	U32 m_RemappedIndex;

	C_TreeVertex() : m_Projection(Maths::UNDEFINED), m_Vertex(Maths::UNDEFINED), m_Index(0),
		m_RemappedIndex(0)
	{
	}

	C_TreeVertex(const Maths::HVec3 PCREF vertex, const U32 index, const Maths::HF32 PCREF closeScale) : m_Projection(Maths::UNDEFINED), m_Vertex(Maths::UNDEFINED), m_Index(index),
		m_RemappedIndex(index)
	{
		const Maths::HVec3 scaledVertex(vertex*closeScale);
		m_Vertex = (scaledVertex.RoundToIntNearest())/closeScale;
		m_Projection = m_Vertex.X().Abs()+ m_Vertex.Y().Abs() + m_Vertex.Z().Abs();
	}

	bool operator < (const C_TreeVertex& vertex)
	{
		//OK. This needs to work on a close scale, not an equal scale...
		/*const Maths::Scalar_Bool xEqX(m_Vertex.X().EQ(vertex.m_Vertex.X()));
		const Maths::Scalar_Bool yEqY(m_Vertex.Y().EQ(vertex.m_Vertex.Y()));

		return m_Vertex.X().LT(vertex.m_Vertex.X()) | 
			(xEqX & m_Vertex.Y().LT(vertex.m_Vertex.Y())) | 
			(xEqX & yEqY & m_Vertex.Z().LT(vertex.m_Vertex.Z()));*/
		return m_Projection < vertex.m_Projection;
	}
	
	bool operator <= (const C_TreeVertex& vertex)
	{
		/*const Maths::Scalar_Bool xEqX(m_Vertex.X().EQ(vertex.m_Vertex.X()));
		const Maths::Scalar_Bool yEqY(m_Vertex.Y().EQ(vertex.m_Vertex.Y()));

		return m_Vertex.X().LT(vertex.m_Vertex.X()) | 
			(xEqX & m_Vertex.Y().LT(vertex.m_Vertex.Y())) | 
			(xEqX & yEqY & m_Vertex.Z().LE(vertex.m_Vertex.Z()));*/
		return m_Projection <= vertex.m_Projection;
	}

	bool operator > (const C_TreeVertex& vertex)
	{
		/*const Maths::Scalar_Bool xEqX(m_Vertex.X().EQ(vertex.m_Vertex.X()));
		const Maths::Scalar_Bool yEqY(m_Vertex.Y().EQ(vertex.m_Vertex.Y()));

		return m_Vertex.X().GT(vertex.m_Vertex.X()) | 
			(xEqX & m_Vertex.Y().GT(vertex.m_Vertex.Y())) | 
			(xEqX & yEqY & m_Vertex.Z().GT(vertex.m_Vertex.Z()));*/
		return m_Projection > vertex.m_Projection;
	}
	
	bool operator >= (const C_TreeVertex& vertex)
	{
		/*const Maths::Scalar_Bool xEqX(m_Vertex.X().EQ(vertex.m_Vertex.X()));
		const Maths::Scalar_Bool yEqY(m_Vertex.Y().EQ(vertex.m_Vertex.Y()));

		return m_Vertex.X().GT(vertex.m_Vertex.X()) | 
			(xEqX & m_Vertex.Y().GT(vertex.m_Vertex.Y())) | 
			(xEqX & yEqY & m_Vertex.Z().GE(vertex.m_Vertex.Z()));*/
		return m_Projection >= vertex.m_Projection;

	}

	bool operator == (const C_TreeVertex& vertex)
	{
		const Maths::HVec3 dif(m_Vertex - vertex.m_Vertex);
		const Maths::HF32 d2(dif | dif);
		return (d2 < Maths::Constants<Maths::HF32>::Epsilon()) != 0;
	}

	bool operator != (const C_TreeVertex& vertex)
	{
		const Maths::HVec3 dif(m_Vertex - vertex.m_Vertex);
		const Maths::HF32 d2(dif | dif);
		return (d2 >= (Maths::Constants<Maths::HF32>::Epsilon())) != 0;
	}
};

class C_TreeVector
{
public:
	Maths::HVec3	m_Vector;
	U32				m_Index;
	U32				m_RemappedIndex;

	C_TreeVector() : m_Vector(Maths::UNDEFINED), m_Index(0), m_RemappedIndex(0)
	{
	}

	bool operator < (const C_TreeVector& r)
	{
		const Maths::Scalar_Bool xEqX(m_Vector.X().EQ(r.m_Vector.X()));
		const Maths::Scalar_Bool yEqY(m_Vector.Y().EQ(r.m_Vector.Y()));

		return m_Vector.X().LT(r.m_Vector.X()) | 
			(xEqX & m_Vector.Y().LT(r.m_Vector.Y())) | 
			(xEqX & yEqY & m_Vector.Z().LT(r.m_Vector.Z()));
	}
	
	bool operator <= (const C_TreeVector& r)
	{
		const Maths::Scalar_Bool xEqX(m_Vector.X().EQ(r.m_Vector.X()));
		const Maths::Scalar_Bool yEqY(m_Vector.Y().EQ(r.m_Vector.Y()));

		return m_Vector.X().LT(r.m_Vector.X()) | 
			(xEqX & m_Vector.Y().LT(r.m_Vector.Y())) | 
			(xEqX & yEqY & m_Vector.Z().LE(r.m_Vector.Z()));
	}

	bool operator > (const C_TreeVector& r)
	{
		const Maths::Scalar_Bool xEqX(m_Vector.X().EQ(r.m_Vector.X()));
		const Maths::Scalar_Bool yEqY(m_Vector.Y().EQ(r.m_Vector.Y()));

		return m_Vector.X().GT(r.m_Vector.X()) | 
			(xEqX & m_Vector.Y().GT(r.m_Vector.Y())) | 
			(xEqX & yEqY & m_Vector.Z().GT(r.m_Vector.Z()));
	}

	bool operator >= (const C_TreeVector& r)
	{
		const Maths::Scalar_Bool xEqX(m_Vector.X().EQ(r.m_Vector.X()));
		const Maths::Scalar_Bool yEqY(m_Vector.Y().EQ(r.m_Vector.Y()));

		return m_Vector.X().GT(r.m_Vector.X()) | 
			(xEqX & m_Vector.Y().GT(r.m_Vector.Y())) | 
			(xEqX & yEqY & m_Vector.Z().GE(r.m_Vector.Z()));
	}

	bool operator == (const C_TreeVector& r)
	{	
		const Maths::HVec3 dif(m_Vector - r.m_Vector);
		const Maths::HF32 d2(dif | dif);
		return (d2 < Maths::Constants<Maths::HF32>::Epsilon()) != 0;
	}

	bool operator != (const C_TreeVector& r)
	{
		const Maths::HVec3 dif(m_Vector - r.m_Vector);
		const Maths::HF32 d2(dif | dif);
		return (d2 >= (Maths::Constants<Maths::HF32>::Epsilon())) != 0;
	}
};

class AABBTree : public GenericTree<AABBTreeNode, AABBBuildTreeTriangle>
{
public:
	AABBTree(F32 splitRatio = 0.25f, U32 maxDepth = MAX_DEPTH): 
				GenericTree<AABBTreeNode, AABBBuildTreeTriangle>(),
				m_splitRatio(splitRatio),
				m_maxDepth(maxDepth),
				_m_indice(&m_Allocator),
				m_StreamSize(0),
				m_bSplitX(true), m_bSplitY(true), m_bSplitZ(true)

	{
	}

	virtual ~AABBTree()
	{
		int bob=0;
	}

	
	//Returns the number of new indices inserted in this node
	void setSplitRatio(F32 splitRatio)
	{
		m_splitRatio=splitRatio;
	}

	void setMaxDepth(U32 maxDepth)
	{
		m_maxDepth=maxDepth;
	}

	NOALIAS __forceinline void AddTriangles(AABBBuildTreeTriangle* __restrict indices, const U32 numIndices) __restrict
	{
		for(U32 a =0; a < numIndices; ++a)
			_m_indice.AddToEnd(indices + a);
	}

	NOALIAS __forceinline void AddTriangle(AABBBuildTreeTriangle& index) __restrict
	{
		_m_indice.AddToEnd(&index);
	}

	U32 GetNumTriangles()
	{
		return _m_indice.Size();
	}


	void RemapVerticesToLeafNodes();
	void RemoveDuplicatedVertices();
	void RemoveDuplicateTriangles();
	U32 SplitTriangle(U32& index, AABBBuildTreeTriangle& tri, AABBBuildTreeTriangle* pLeftTriangle, AABBBuildTreeTriangle* pRightTriangle, const Collision::C_Plane& plane, const U32 startIndex, const U32 endIndex,
		SharedPhysics::C_DoubleListAlloc<AABBBuildTreeTriangle>& m_indice, const Maths::HF32 PCREF planeThickness);
	U32 GetSplitStatus(const U32 index, AABBBuildTreeTriangle& tri, const Collision::C_Plane& p, const Maths::HF32 PCREF threshold);
	F32 GetScore(const Collision::C_Plane& p, const U32 startIndex, const U32 endIndex, const F32 cons, const Maths::HF32 PCREF recipParentVolume, const Maths::HF32 PCREF threshold);
	AABBTreeNode* CreateAABBNode(const U32 startIndex, const U32 size, SharedPhysics::C_DoubleAllocIterator<AABBBuildTreeTriangle> iter);
	
	Collision::C_Plane PickASeperatedSurfacePlane(const U32 startIndex, const U32 size, F32& score, const Maths::HF32 PCREF recipVolume, const Maths::HF32 PCREF threshold);
	Collision::C_Plane PickASeperatedEdgePlane(const U32 startIndex, const U32 size, F32& score, const Maths::HF32 PCREF recipVolume, const Maths::HF32 PCREF threshold);
	Collision::C_Plane PickASeperatedAlignAxisPlane(SharedPhysics::C_DoubleAllocIterator<AABBBuildTreeTriangle> iter, const U32 startIndex, const U32 size, const Collision::C_AxisBoxMinMax& aabb, F32& score, const Maths::HF32 PCREF recipVolume, Maths::HF32& threshold);
	
	void BuildAABBTree(const Maths::HF32 PCREF normalThreshold, const F32 triangleIncreaseThreshold = 0.4f);

	void CompressAABBTreeData();

	void ValidateTree(AABBTreeNode* pNode);

	template <typename Serial>
	void Serialise(Serial& s);

	const SharedPhysics::SortedArray<AABBBuildTreeTriangle*>& GetTriangles() const
	{
		return m_indicesArray;
	}

	Collision::C_TriangleStream& GetStream()
	{
		return m_Stream;
	}

	const U32 GetStreamSize()
	{
		return m_StreamSize;
	}

private:

	U32 CountVertices(AABBTreeNode* pNode, SharedPhysics::C_DoubleAllocIterator<AABBBuildTreeTriangle> iter);
	bool MatchNormal(AABBBuildTreeTriangle& tri);
	bool MatchNormal(Collision::AABBTreeTriangle& tri);
	void ExtractEdgeEvents(SharedPhysics::C_DoubleAllocIterator<AABBBuildTreeTriangle> iter, SharedPhysics::SortedArray<C_EdgeEvent>& edgeEvents, U32 axis, U32 startIndex, U32 endIndex);
	F32 PickBestEdgePoint(SharedPhysics::SortedArray<C_EdgeEvent>& edgeEvents, F32& score);
	F32 PickFatSubdivision(SharedPhysics::C_DoubleAllocIterator<AABBBuildTreeTriangle> iter, const U32 startIndex, const U32 size, 
								 F32& scoreFat, const U32 axis);

	AABBTreeNode* BuildAABBTree(const U32 startIndex, const U32 size, const U32 depth, S32& maxNewTriangles,
		SharedPhysics::C_DoubleListAlloc<AABBBuildTreeTriangle>& m_indice);

	void CaclulateNumChildrenNodes();

	void CalculateActiveEdgeFlags(const Maths::HF32 PCREF normThreshold);

	void CopyTrianglesToIndexArray();

	U32 CountNumNodes(AABBTreeNode* pNode);

	void TransferDataToStream();

	void ReSortTriangles(AABBTreeNode* pNode);

	template <typename Serial>
	void RecursiveSerialise(Serial& s, AABBTreeNode* pNode);
	void RemoveDegeneratedTriangle();
	void CalculateFaceNormals();
public:
	void SplitX(bool bUseX)
	{
		m_bSplitX = bUseX;
	}

	void SplitY(bool bUseY)
	{
		m_bSplitY = bUseY;
	}

	void SplitZ(bool bUseZ)
	{
		m_bSplitZ = bUseZ;
	}

private:
	SharedPhysics::C_AllocationManager<AABBBuildTreeTriangle, 32, false> m_Allocator;
	SharedPhysics::C_DoubleListAlloc<AABBBuildTreeTriangle> _m_indice;
	SharedPhysics::SortedArray<AABBBuildTreeTriangle*> m_indicesArray;
	SharedPhysics::SortedArray<Collision::C_ActiveEdgeNormals> m_TriangleEdgeNormals;
	//SharedPhysics::SortedArray<AABBBuildTreeTriangle> m_indice;
	F32 m_splitRatio;
	U32 m_maxDepth;
	Collision::C_TriangleStream m_Stream;
	U32 m_StreamSize;
	bool m_bSplitX, m_bSplitY, m_bSplitZ;
};

inline F32 Score(S32 leftCount, S32 rightCount, S32 activeCount, F32 midLow, F32 midHigh, F32 low, F32 high, SharedPhysics::SortedArray<Baking::C_EdgeEvent>& edgeEvents
				 , S32 startIndex, S32 endIndex)
{
	F32 lc = (F32)leftCount;
	F32 rc = (F32)rightCount;
	F32 ac = (F32)activeCount;
	F32 range = high - low;
	F32 midRange(midHigh - midLow);
	F32 proportionOverlap = midRange/range;
	F32 scoreA = Maths::Abs(lc - rc);

	Maths::HVec3 lowMin(Maths::Constants<Maths::HVec3>::Max());
	Maths::HVec3 lowMax(-Maths::Constants<Maths::HVec3>::Max());
	Maths::HVec3 hiMin(Maths::Constants<Maths::HVec3>::Max());
	Maths::HVec3 hiMax(-Maths::Constants<Maths::HVec3>::Max());

	for(S32 a = startIndex; a <= endIndex; ++a)
	{
		C_EdgeEvent& ev = edgeEvents[a];
		//if(ev.bStart)
		{
			lowMin = Maths::Min(ev.m_Min, lowMin);
			lowMax = Maths::Max(ev.m_Max, lowMax);
		}

		//parMin = Maths::Min(ev.m_Min, lowMin);
		//parMax = Maths::Max(ev.m_Max, lowMax);
	}

	for(S32 a = endIndex + 1; a < edgeEvents.m_numElements; ++a)
	{
		C_EdgeEvent& ev = edgeEvents[a];
		//if(ev.bStart)
		{
			hiMin = Maths::Min(ev.m_Min, hiMin);
			hiMax = Maths::Max(ev.m_Max, hiMax);
		}
		//parMin = Maths::Min(ev.m_Min, lowMin);
		//parMax = Maths::Max(ev.m_Max, lowMax);
	}

	Maths::HVec3 parMin(Maths::Min(lowMin, hiMin));
	Maths::HVec3 parMax(Maths::Max(lowMax, hiMax));
	const Maths::HVec3 parRange(Maths::Max(parMax - parMin, Maths::Constants<Maths::HVec3>::Zero()));
	const Maths::HF32 parVolume(parRange.X() * parRange.Y() * parRange.Z());

	const Maths::HVec3 overlapMin(Maths::Max(hiMin, lowMin));
	const Maths::HVec3 overlapMax(Maths::Min(hiMax, lowMax));

	const Maths::HVec3 overlapRange(Maths::Max(overlapMax - overlapMin, Maths::Constants<Maths::HVec3>::Zero()));

	const Maths::HF32 overlapVolume(overlapRange.X() * overlapRange.Y() * overlapRange.Z());

	
	//F32 overlapScore = 200.f * proportionOverlap * F32(activeCount);
	return (scoreA * parVolume) + (overlapVolume * ac); // + overlapScore;
}



};

#endif



