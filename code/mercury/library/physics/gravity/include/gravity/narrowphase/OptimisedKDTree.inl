#ifndef OPTIMISEDKDTREE_INL
#define OPTIMISEDKDTREE_INL

#include "gravity/narrowphase/OptimisedKDTree.h"

namespace Collision
{
	template <typename Serial>
	void KDTreeTriangle::DeSerialise(Serial& s)
	{
		s >> m_IndexA >> m_IndexB >> m_IndexC >> m_NormalIndex >> m_CollisionFlags >> m_MaterialIndex;
	}


	template <typename Serial>
	void OptimisedKDNode::DeSerialise(Serial& s)
	{
		//Get bounding box
		s >> m_axis >> m_dist;
		s >> m_NumIndices >> m_StartIndex;
	}

	template <typename Serial>
	void OptimisedKDTree::DeSerialise(Serial& s)
	{
		s >> m_NumVertices >> m_NumNormals >> m_NumTriangles >> m_NumNodes;

		if(m_pVertices)
			delete [] m_pVertices;
		if(m_pNormals)
			delete [] m_pNormals;
		if(m_pTriangles)
			delete [] m_pTriangles;
		if(m_pNodes)
			delete [] m_pNodes;

		//m_pVertices = new MathsStorage::Float3[m_NumVertices];
		//m_pNormals = new MathsStorage::Float3[m_NumNormals];
		m_pVertices = new MathsStorage::Float3[m_NumVertices+1]; // JASOND: Padded to prevent unaligned vector loading crash!
		m_pNormals = new MathsStorage::Float3[m_NumNormals+1]; // JASOND: Padded to prevent unaligned vector loading crash!
		m_pTriangles = new KDTreeTriangle[m_NumTriangles];
		m_pNodes = new OptimisedKDNode[m_NumNodes];

		for(U32 a = 0; a < m_NumVertices; ++a)
		{
			s >> m_pVertices[a];
		}

		for(U32 a = 0; a < m_NumNormals; ++a)
		{
			s >> m_pNormals[a];
		}

		for(U32 a = 0; a < m_NumTriangles; ++a)
		{
			m_pTriangles[a].DeSerialise(s);
		}

		for(U32 a = 0; a < m_NumNodes; ++a)
		{
			m_pNodes[a].DeSerialise(s);
		}
	}

	template<bool (*TriCollisions)(const MathsStorage::Float3* const __restrict verts, const MathsStorage::Float3* const __restrict normals, const Collision::KDTreeTriangle* const __restrict tris, const U32 numTris, Collision::C_Ray& ray)>
	bool OptimisedKDTree::RayTracing(Collision::C_Ray &ray, SharedPhysics::C_SafeStack<true>& stack)
	{
		using namespace Maths;

		const HF32 half(Maths::Constants<HF32>::Half());
		stack.Push(m_pNodes, 0);
		S32 size = 1;
		OptimisedKDNode* pStackTop = m_pNodes;
		bool collide = false;
		U32 colour = 0;
		while(size--)
		{
			OptimisedKDNode* node = pStackTop;
			if(size > 0)
				pStackTop = stack.Pop<OptimisedKDNode>(size-1);
			if(node->m_NumIndices == 0)
			{
				//Test to see which side to push into!
				U32 startSide(Side(ray.m_start, node));
				U32 endSide(Side(ray.m_result.m_contact, node));
				if(startSide == 0)
				{
					stack.Push( node + node->m_RightChildOffset, size++);
					stack.Push( node + 1, size++);
					pStackTop = node + 1;
				}
				else if(startSide < 0)
				{
					if(endSide >= 0)
					{
						stack.Push( node + node->m_RightChildOffset, size++ );
					}
					stack.Push( node + 1, size++);
					pStackTop = node + 1;
				}
				else
				{
					if(endSide <= 0)
					{
						stack.Push( node + 1, size++ );
					}
					stack.Push(node + node->m_RightChildOffset, size++);
					pStackTop = node + node->m_RightChildOffset;
				}
			}
			else
			{
				//Do collision detection
				//manager.GetCollider(meshObj, obj);
				collide |= TriCollisions(m_pVertices, m_pNormals, m_pTriangles + node->m_StartIndex, node->m_NumIndices, ray);				
				for(U32 a = 0; a < node->m_NumIndices; ++a)
				{
					Collision::KDTreeTriangle& tri(m_pTriangles[node->m_StartIndex + a]);
					const Maths::HVec3 posA(Maths::Load<Maths::HVec3>(&m_pVertices[tri.m_IndexA]));
					const Maths::HVec3 posB(Maths::Load<Maths::HVec3>(&m_pVertices[tri.m_IndexB]));
					const Maths::HVec3 posC(Maths::Load<Maths::HVec3>(&m_pVertices[tri.m_IndexC]));

					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangle(posA, posB, posC, RGBA(255 - colour, 0, colour, 255));
					colour += 5;

				}
			}
		}
		return collide;
	}
};

#endif

