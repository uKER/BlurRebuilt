#include <stdafx.h>
#include "gravity/treebuilder/KDTree.h"
#include "gravity/treebuilder/GenericTree.inl"
#include "gravity/sharedphysics/SortedArray.inl"

//using namespace Maths;
using namespace SharedPhysics;

namespace Baking
{

/*//Get the lowest score of the seperated planes perpendidular to the axis 
F32 KDTree::PickEdgeNode(const SortedArray<KDTreeEdgeNode>& edges, const U32 axis, const F32 recipSA, const MathsStorage::Float3& d , const MathsStorage::Float3& min, const MathsStorage::Float3& max, F32& bestDist)
{
	U32 numBelow = 0;
	U32 numAbove = edges.m_numElements>>1;	//number of triangle and each triangle contributes to two edges

	
	F32 bestCost(Maths::Constants<F32>::Max());	

	U32 otherAxis0=m_otherAxis[axis][0];
	U32 otherAxis1=m_otherAxis[axis][1];
			
	F32 t1=d.v[otherAxis0] * d.v[otherAxis1];
	F32 t2=d.v[otherAxis0] + d.v[otherAxis1];

	F32 minDist=min.v[axis];
	F32 maxDist=max.v[axis];

	for(S32 i=0; i<edges.m_numElements; ++i)
	{
		KDTreeEdgeNode& e=edges.m_pElems[i];
		
		if(e.m_type == END) // the edge node is the end point of the triangle
		{
			--numAbove;
		}
		else	// the edge node is the start point of triangle
		{
			++numBelow;
		}

		if(e.m_dist > minDist && e.m_dist < maxDist)
		{
			//Below is the cost function using surface area heuristic
			
			const F32 belowSA= t1+(e.m_dist-minDist)*t2;
			const F32 aboveSA= t1+(maxDist-e.m_dist)*t2;

			const F32 below=belowSA*numBelow;
			const F32 above=aboveSA*numAbove;

			const F32 p=m_intersectCost*recipSA;

			const F32 cost=m_traversalCost + p*( below + above );

			if(cost < bestCost)
			{
				bestCost=cost;
				bestDist=e.m_dist;
			}
		}
		
	}
	return bestCost;
}


U32 KDTree::GetSplitStatus(const U32 index, const U32 axis, const F32 dist, const Maths::HF32 PCREF threshold)
{
	KDTreeTriangle& tri = m_indice[index];
	const F32 epsilon(1e-3f);
	
	if(tri.m_maxf.v[axis] <= (dist + epsilon))
		return POINT_IN_FRONT_PLANE;
	else if(tri.m_minf.v[axis] >= (dist - epsilon))
		return POINT_IN_BACK_PLANE;
	else
		return POINT_STRADDLE_PLANE;;
}

Debug::Bool g_DrawSplitTrianglesKD("Test.DrawKDSplitTriangles", false);

U32 KDTree::SplitTriangle(U32& index, const U32 axis, const F32 dist, const U32 startIndex, const U32 endIndex)
{
	using namespace Maths;

	KDTreeTriangle& tri=m_indice[index];
	
	U32 temp0=tri.m_IndexA;
	U32 temp1=tri.m_IndexB;
	U32 temp2=tri.m_IndexC;
	U32 material = tri.m_MaterialIndex;
	U32 flags = tri.m_CollisionFlags;

	const F32 epi(Maths::Constants<F32>::Epsilon2());
	const HF32 zero(Maths::Constants<HF32>::Zero());
	const HVec3 a(m_vertice[temp0]);
	const HVec3 b(m_vertice[temp1]);
	const HVec3 c(m_vertice[temp2]);

	HVec3 t0(a);
	HVec3 t1(b);
	HVec3 t2(c);

	MathsStorage::Float3 af;
	MathsStorage::Float3 bf;
	MathsStorage::Float3 cf;

	Store(a, &af);
	Store(b, &bf);
	Store(c, &cf);

	F32 s0(af.v[axis]);
	F32 s1(bf.v[axis]);
	F32 s2(cf.v[axis]);


	if((s1*s0<0) && (s1*s2<0))// b in one side while a and c in different side
	{
		t0=b;
		t1=a;
		
		U32 temp=temp0;
		temp0=temp1;
		temp1=temp;
		
		F32 s=s0;
		s0=s1;
		s1=s;
	}
	else if((s2*s0<0)&& (s1*s2<0))//c in one side while a and b in different side
	{
		t0=c;
		t2=a;
		
		U32 temp=temp0;
		temp0=temp2;
		temp2=temp;
		
		F32 s=s0;
		s0=s2;
		s2=s;
	}


	const HVec3 t0t1(t1-t0);
	const HVec3 t0t2(t2-t0);

	const HVec3 ab(b-a);
	const HVec3 ac(c-a);
	const HVec3 tp(t0t1^t0t2);
	const HVec3 op(ab^ac);
	const HF32 res(tp|op);

	KDTreeTriangle front;
	KDTreeTriangle mid;
	KDTreeTriangle back;

	KDTreeTriangle* pFront=&front;
	KDTreeTriangle* pBack=&back;
	KDTreeTriangle* pMid=NULL;

	bool midInFront=true;

	if(Abs(s1)<epi)
	{
		//t1 is one of the split point
		
		F32 t=(dist-s0)/(s2-s0);
		//vmid=t0+(t2-t0)*t;
		Maths::HVec3 vmid(t0 + t0t2*t);
		//IntersectSegmentPlane(t0, t2, p, vmid);
		m_vertice.AddToEnd(&vmid);
		U32 temp3=m_vertice.m_numElements-1;

		//if tp and op in the same direction, res is 1, if tp and op oppisite direction, res is -1
		if(res.GT(zero))
		{
			//the wiring order is the same as the original triangle
			front.m_IndexA=temp3; front.m_IndexB=temp1; front.m_IndexC=temp2; 
			front.m_MaterialIndex = material; front.m_CollisionFlags = flags;//triangle mid_t1_t2
			back.m_IndexA=temp3; back.m_IndexB=temp0; back.m_IndexC=temp1;//triangle mid_t0_t1
			back.m_MaterialIndex = material; back.m_CollisionFlags = flags;
		}
		else
		{
			//the wiring order is not the same as the original triangle
			front.m_IndexA=temp3; front.m_IndexB=temp1; front.m_IndexC=temp0;//triangle mid_t1_t0
			front.m_MaterialIndex = material; front.m_CollisionFlags = flags;
			back.m_IndexA=temp3; back.m_IndexB=temp2; back.m_IndexC=temp1;	//triangle mid_t2_t1
			front.m_MaterialIndex = material; front.m_CollisionFlags = flags;
		}

	}
	else if(Abs(s2)<epi)
	{
		//t2 is one of the split point
		const F32 t=(dist-s0)/(s1-s0);
		Maths::HVec3 vmid(t0 + t0t1*t);
		//IntersectSegmentPlane(t0, t1, p, vmid);
		m_vertice.AddToEnd(&vmid);
		U32 temp3=m_vertice.m_numElements-1;

		//if tp and op in the same direction, res is 1, if tp and op oppisite direction, res is -1
		if(res.GT(zero))
		{
			//the wiring order is the same as the original
			front.m_IndexA=temp3; front.m_IndexB=temp1; front.m_IndexC=temp2; //triangle mid_t1_t2
			front.m_MaterialIndex = material; front.m_CollisionFlags = flags;
			back.m_IndexA=temp3; back.m_IndexB=temp2; back.m_IndexC=temp0; //triangle mid_t2_t0
			back.m_MaterialIndex = material; back.m_CollisionFlags = flags;

		}
		else
		{
			//the wiring order is not the same as the original triangle
			front.m_IndexA=temp3; front.m_IndexB=temp2; front.m_IndexC=temp1; //triangle mid_t2_t1
			front.m_CollisionFlags = flags; front.m_MaterialIndex = material;
			back.m_IndexA=temp3;back.m_IndexB=temp0;back.m_IndexC=temp2; //triangle mid_t0_t2
			back.m_CollisionFlags = flags; back.m_MaterialIndex = material;
		}
	}
	else
	{
	
		const F32 tmid1=(dist-s0)/(s1-s0);
		const F32 tmid2=(dist-s0)/(s2-s0);
		HVec3 mid1(t0 + t0t1*tmid1);
		HVec3 mid2(t0 + t0t2*tmid2);
		//IntersectSegmentPlane(t0, t1, p, mid1);
		//IntersectSegmentPlane(t0, t2, p, mid2);

		m_vertice.AddToEnd(&mid1);
		m_vertice.AddToEnd(&mid2);

		U32 temp3=m_vertice.m_numElements-2;
		U32 temp4=m_vertice.m_numElements-1;

		
		pMid=&mid;
		
		//if tp and op in the same direction, res is 1, if tp and op oppisite direction, res is -1
		//Assume t0 is behind the plane
		if(res.GT(zero))
		{
			//the wiring order is the same as the original

			front.m_IndexA=temp4; front.m_IndexB=temp1; front.m_IndexC=temp2;	//trinagle mid2_t1_t2
			front.m_CollisionFlags = flags; front.m_MaterialIndex = material;
			mid.m_IndexA=temp4;	mid.m_IndexB=temp3;	mid.m_IndexC=temp1;	 //triangle mid2_mid1_t1
			mid.m_CollisionFlags = flags; mid.m_MaterialIndex = material;
			back.m_IndexA=temp4; back.m_IndexB=temp0; back.m_IndexC=temp3;	//triangle mid2_t0_mid1
			back.m_CollisionFlags = flags; back.m_MaterialIndex = material;

		}
		else
		{
			//the wiring order is not the same as the original triangle

			front.m_IndexA=temp4; front.m_IndexB=temp2; front.m_IndexC=temp1;	//trinagle mid2_t2_t1
			front.m_CollisionFlags = flags; front.m_MaterialIndex = material;
			mid.m_IndexA=temp4;	mid.m_IndexB=temp1;	mid.m_IndexC=temp3;	//triangle mid2_t1_mid1
			mid.m_CollisionFlags = flags; mid.m_MaterialIndex = material;
			back.m_IndexA=temp4; back.m_IndexB=temp3;back.m_IndexC=temp0;	//triangle mid2_mid1_t0
			back.m_CollisionFlags = flags; back.m_MaterialIndex = material;
		}
	}

 // if t0 is in front of the plane, swap the front and back pointer
	if((s0-dist)<0)
	{	
		pFront=&back;
		pBack=&front;
		midInFront=false;
		
	}

	const RGBA black(0, 0, 0, 255);
	const RGBA blue(0, 64, 255, 255);

	m_indice.Remove(index, 1);

	//Insert the front triangle in the front of m_indice
	pFront->SetWorldBound(m_vertice);
	m_indice.Insert(pFront, startIndex);

	index+=1;

	//if the triangle is split into three
	U32 newInds = 0;
	if(pMid)
	{
		pMid->SetWorldBound(m_vertice);
		
		newInds += 1;
		if(midInFront)
		{
			m_indice.Insert(pMid, startIndex);
			index+=1;
		}
		else
		{
			m_indice.Insert(pMid, endIndex);
		}
	}

	pBack->SetWorldBound(m_vertice);
	m_indice.Insert(pBack, endIndex);

	
	return newInds + 1;
}

KDTreeNode* KDTree::CreateKDTreeNode(const U32 startIndex, const U32 numTriangle, const Maths::HVec3 PCREF min, const Maths::HVec3 PCREF max)
{
	KDTreeNode* node=new KDTreeNode(startIndex, numTriangle);

	//Axis axis;
	//F32 dist;

	const Maths::HVec3 d(max-min);
	MathsStorage::Float3 minf;
	MathsStorage::Float3 maxf;
	MathsStorage::Float3 df;
	Store(min, &minf);
	Store(max, &maxf);
	Store(d, &df);
	F32 t1=df.v[Y]*df.v[Z];
	F32 t2=df.v[Y]+df.v[Z];
	F32 SA=t1 + df.v[X]*t2;
	F32 recipSA=1.f/SA;

	edgesX.m_numElements=0;
	edgesY.m_numElements=0;
	edgesZ.m_numElements=0;

	U32 endIndex=startIndex+numTriangle;
	//all the sorting is based the m_dist value
	for(U32 i=startIndex; i< endIndex; ++i)
	{
		KDTreeTriangle& t=m_indice[i];

		CreateEdgeNode(t, X, i, edgesX);
		CreateEdgeNode(t, Y, i, edgesY);
		CreateEdgeNode(t, Z, i, edgesZ);
	}

	int Stack[4096];
	edgesX.QuickSort(Stack, 10);
	edgesY.QuickSort(Stack, 10);
	edgesZ.QuickSort(Stack, 10);
	
	F32 bestDistX(0.f);
	F32 bestDistY(0.f);
	F32 bestDistZ(0.f);


	F32 costX=PickEdgeNode(edgesX, X ,recipSA, df, minf, maxf, bestDistX);
	F32 costY=PickEdgeNode(edgesY, Y, recipSA, df, minf, maxf, bestDistY);
	F32 costZ=PickEdgeNode(edgesZ, Z, recipSA, df, minf, maxf, bestDistZ);

	
	if(costX < costY && costX < costZ)
	{
		node->m_axis=X;
		node->m_dist=bestDistX;
		
	}
	else if(costY < costZ)
	{
		node->m_axis=Y;
		node->m_dist=bestDistY;
	}
	else
	{
		node->m_axis=Z;
		node->m_dist=bestDistZ;
	}

	return node;
}


void KDTree::CreateEdgeNode(KDTreeTriangle& tri, U32 axis, U32 index, SortedArray<KDTreeEdgeNode>& edges)
{
	//KDTreeEdgeNode edgeStart;
	//KDTreeEdgeNode edgeEnd;

	KDTreeEdgeNode* edgeStart=(KDTreeEdgeNode*)edges.GetNextFreeElem();
	edgeStart->m_dist=tri.m_minf.v[axis];
	edgeStart->m_index=index;
	edgeStart->m_type=START;

	KDTreeEdgeNode* edgeEnd=(KDTreeEdgeNode*)edges.GetNextFreeElem();

	edgeEnd->m_dist=tri.m_maxf.v[axis];
	edgeEnd->m_index=index;
	edgeEnd->m_type=END;
	
	//edges.AddToEnd(&edgeStart);
	//edges.AddToEnd(&edgeEnd);
	
}


void KDTree::BuildKDTree()
{
	RemoveDuplicatedVertice();
	RemoveDegeneratedTriangle();

	for(S32 i=0; i< m_indice.m_numElements; ++i)
	{
		KDTreeTriangle& tri=m_indice.m_pElems[i];
		tri.SetWorldBound(m_vertice);
	}
	
	m_pRoot = BuildKDTree(0, m_indice.m_numElements, 0);
}

KDTreeNode* KDTree::BuildKDTree(U32 startIndex, U32 numTriangle, U32 depth)
{

	using namespace Maths;

	if(depth >= m_maxDepth || numTriangle <=MIN_LEAF_SIZE)
	{
		return new KDTreeNode(startIndex, numTriangle);
	}

	U32 endIndex=startIndex+numTriangle;

	Maths::HVec3 min(Maths::Constants<Maths::HVec3>::Max());
	Maths::HVec3 max(-Maths::Constants<Maths::HVec3>::Max());

	for(U32 k=startIndex; k<endIndex; ++k)
	{
		KDTreeTriangle& tri=m_indice.m_pElems[k];
		min=Min(tri.m_min, min);
		max=Max(tri.m_max, max);
	}

	
	KDTreeNode* node=CreateKDTreeNode(startIndex, numTriangle, min, max);

	node->min=min;
	node->max=max;

	const HF32 half(Maths::Constants<HF32>::Half());

	const HVec3 c((max + min)*half);
	const HVec3 t((max - c) * m_splitRatio);
	const HF32 threshold(t|t);

	//U32 endIndex=startIndex+size;
	
	U32 i=startIndex;
	U32 j=endIndex-1;

	U32 splitIndex = i;
	while(i<=j)
	{
		bool inBackPlane=false;
		while(!inBackPlane && i <= j)
		{
		
			switch(GetSplitStatus(i, node->m_axis, node->m_dist, threshold ))
			{
			case POINT_IN_FRONT_PLANE:
				{
					i++;
					splitIndex = i;
					break;
				}
			case POINT_IN_BACK_PLANE:
				{	
					inBackPlane=true;
					break;
				}
			default: //point straddle the plane
				{
					//Split the triangle and update i
					U32 newInds = SplitTriangle(i, node->m_axis, node->m_dist, startIndex, endIndex);
					j+= (newInds - 1);
					endIndex += newInds;
					numTriangle += newInds;
					node->m_sizeIndice += newInds;
					splitIndex = i;
					
					
					
					//inBackPlane = true;
					//i ++;
					break;
				}
			}
		}
		bool inFrontPlane=false;
		while(!inFrontPlane && i <= j)
		{
			switch(GetSplitStatus(j,node->m_axis, node->m_dist, threshold))
			{
			case POINT_IN_FRONT_PLANE:
				{
					inFrontPlane=true;
					break;
				}
			case POINT_IN_BACK_PLANE:
				{	
					splitIndex = j;
					j--;

					break;
				}
			default: //point straddle the plane
				{
					//split the triangle and update j
					U32 newInds = SplitTriangle(j, node->m_axis, node->m_dist, startIndex, endIndex);
					endIndex += newInds;
					//i += (newInds - 1);
					splitIndex = j;
					j--;
					node->m_sizeIndice += newInds;
					
					
					//inFrontPlane = true;
					//j--;
					break;
				}
			}
		}
	
		if(i < j)
		{
			KDTreeTriangle tri = m_indice[i];
			m_indice[i] = m_indice[j];
			m_indice[j] = tri;
		}
	}


	if(splitIndex == endIndex || splitIndex == startIndex)
		return node;

	const U32 mid = splitIndex - startIndex;
	const U32 right = endIndex - splitIndex;
	node->m_leftNode=BuildKDTree(startIndex, mid, depth+1);
	node->m_sizeIndice = node->m_leftNode->m_sizeIndice + right;
	const U32 rightStart = node->m_startIndex + (node->m_sizeIndice - right);
	node->m_rightNode=BuildKDTree(rightStart, right, depth+1);
	node->m_sizeIndice = node->m_leftNode->m_sizeIndice + node->m_rightNode->m_sizeIndice;
	return node;
}

U32 CalculateNumDescendants(KDTreeNode* node)
{
	if(node->m_leftNode == NULL)
	{
		node->m_NumDescendants = 0;
	}
	else
	{
		node->m_NumDescendants = CalculateNumDescendants(node->m_leftNode) + CalculateNumDescendants(node->m_rightNode);
	}
	return node->m_NumDescendants + 1;
}

void KDTree::CaclulateNumChildrenNodes()
{
	KDTreeNode* node = this->m_pRoot;
	if(node->m_leftNode == NULL)
		node->m_NumDescendants = 0;
	else
		node->m_NumDescendants = CalculateNumDescendants(node->m_leftNode) + CalculateNumDescendants(node->m_rightNode);
}

U32 KDTree::CountNumNodes(KDTreeNode* pNode)
{
	if(pNode->m_leftNode == NULL)
		return 1;
	return 1 + CountNumNodes(pNode->m_leftNode) + CountNumNodes(pNode->m_rightNode);
}
*/

};
