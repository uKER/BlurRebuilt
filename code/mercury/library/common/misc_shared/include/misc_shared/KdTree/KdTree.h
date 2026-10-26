#ifndef __MISCSHARED_KDTREE_H__
#define __MISCSHARED_KDTREE_H__

#include "misc_shared/KdTree/KdNode.h"
#include "misc_shared/KdTree/KdTreeBuilder.h"

namespace Util
{

/// Kd-Tree containing points
/// allows 'nearest point' query
template<typename T>
class KdTree
{
public:
	KdTree()
		: m_nTests(0)
		, m_bBuilt(false)
	{
	}

	/// add a primitive
	void PushBack(const T &item)
	{
		m_Primitives.PushBack(item);
		m_bBuilt = false;
	}

	int Size() const
	{
		return m_Primitives.Size();
	}

	T& operator [](int i)
	{
		return m_Primitives[i];
	}

	const T& operator [](int i) const
	{
		return m_Primitives[i];
	}

	T& At(int i)
	{
		return m_Primitives[i];
	}

	const T& At(int i) const
	{
		return m_Primitives[i];
	}

	/// build tree
	void Build() const
	{
		if (m_bBuilt)
			return;

		m_bBuilt = true;
#if TOOL
		KdBuilder builder;
		builder.SetParameters(4, 10, 0, 1);

		Box3Vector &prims = builder.GetPrimBounds();
		prims.Resize(m_Primitives.Size());
		for (int i=0; i<prims.Size(); i++)
		{
			Maths::C_AxisBox box;
			box.UpdateExtents(m_Primitives[i]);
			prims[i] = box;
		}

		m_Data = builder.Build();
#else
		assert(0); // Kd tree is supposed to be built offline, saved to this and loaded back in in-game
#endif
	}

	/// return index of item nearest to the given position
	int FindNearest(const Maths::HVec3 &v) const
	{
		Build();
		KdNode *pNode = &m_Data.m_Nodes[0];
		KdNode *pNextNode, *pNextNode1, *pNextNode2;

		KdNode			*stack[KDNODE_STACK_SIZE];
		float			fstack[KDNODE_STACK_SIZE];
		int				iStackPos = 0;
		float			fNearest = FLT_MAX;
		int				iNearest = 0;

		while (1)
		{
			while (!KDNODE_ISLEAF(pNode))
			{
				// get node data
				int iAxis = KDNODE_DIMENSION(pNode);
				float d = pNode->fSplitCoordinate - v[iAxis];

				// calculate next near/far node pointers
				pNextNode = pNode+1;
				pNextNode1 = KDPTR_OFFSET(pNextNode, 0);
				pNextNode2 = KDPTR_OFFSET(pNextNode, KDNODE_OFFSET(pNode));

				// totally on the left side?
				if (d>fNearest)
				{
					pNode = pNextNode1;
					continue;
				}
				// totally on the right side?
				if (d<-fNearest)
				{
					pNode = pNextNode2;
					continue;
				}

				// do both sides
				if (d > 0)
				{
					stack[iStackPos] = pNextNode2;
					fstack[iStackPos] = d;
					pNode = pNextNode1;
				}
				else
				{
					stack[iStackPos] = pNextNode1;
					fstack[iStackPos] = -d;
					pNode = pNextNode2;
				}
				iStackPos++;
			}

			// have a leafnode
			if (pNode->iFlagAndNumPrimitives & 0x7fffffff)
			{
				int iTri, iPrim = 0;

				// keep processing mailboxes in sequences until we hit one with the high-bit of the primitive index set
				for (KdMailbox *pBox = &m_Data.m_Mailboxes[pNode->iPrimitive]; iPrim>=0; pBox++)
				{
					iPrim = pBox->iPrimitive;
					iTri = iPrim & 0x7fffffff;
					const T *pTri = &m_Primitives[ iTri ];
					float f = pTri->Distance(v);
					m_nTests++;
					if (f < fNearest)
					{
						fNearest = f;
						iNearest = iTri;
					}
				}
			}

			// pop
			while (1)
			{
				if (!iStackPos)
					return iNearest; // nothing else to traverse any more...

				iStackPos--;
				pNode = stack[iStackPos];
				if (fstack[iStackPos] < fNearest)
					break;
			}
		}
	}

	/// return index of item nearest to the given position
	template<typename T2>
	int Intersect(const T2 &v, int *pResults, int iMaxResults) const
	{
		Build();

		if (m_Data.m_Nodes.Size() == 0)
			return 0;

		KdNode *pNode = &m_Data.m_Nodes[0];
		KdNode *pNextNode, *pNextNode1, *pNextNode2;

		KdNode			*stack[KDNODE_STACK_SIZE];
		int				iStackPos = 0;
		int				nResults = 0;
		m_nTests = 0;

		// convert v to a C_AxisBox
		Maths::C_AxisBox vbox;
		vbox.UpdateExtents(v);

		while (1)
		{
			while (!KDNODE_ISLEAF(pNode))
			{
				// get node data
				int iAxis = KDNODE_DIMENSION(pNode);

				// calculate next near/far node pointers
				pNextNode = pNode+1;
				pNextNode1 = KDPTR_OFFSET(pNextNode, 0);
				pNextNode2 = KDPTR_OFFSET(pNextNode, KDNODE_OFFSET(pNode));

				// totally on the left side?
				if (pNode->fSplitCoordinate > vbox.Max()[iAxis])
				{
					pNode = pNextNode1;
					continue;
				}
				// totally on the right side?
				if (pNode->fSplitCoordinate < vbox.Min()[iAxis])
				{
					pNode = pNextNode2;
					continue;
				}

				// do both sides
				if (pNode->fSplitCoordinate > vbox.Centre()[iAxis])
				{
					stack[iStackPos] = pNextNode2;
					pNode = pNextNode1;
				}
				else
				{
					stack[iStackPos] = pNextNode1;
					pNode = pNextNode2;
				}
				iStackPos++;
			}

			// have a leafnode
			if (pNode->iFlagAndNumPrimitives & 0x7fffffff)
			{
				int iTri, iPrim = 0;

				// keep processing mailboxes in sequences until we hit one with the high-bit of the primitive index set
				for (KdMailbox *pBox = &m_Data.m_Mailboxes[pNode->iPrimitive]; iPrim>=0; pBox++)
				{
					iPrim = pBox->iPrimitive;
					iTri = iPrim & 0x7fffffff;
					const T *pTri = &m_Primitives[ iTri ];
					m_nTests++;
					if (pTri->Intersects(v))
					{
						pResults[nResults] = iTri;
						nResults++;
						if (nResults == iMaxResults)
							return nResults;
					}
				}
			}

			// pop
			if (!iStackPos)
				return nResults;

			iStackPos--;
			pNode = stack[iStackPos];
		}
	}

	template<typename Archive>
	void Serialise(Archive &ar)
	{
		Build();
		ar << m_Data << m_Primitives;
	}

	template<typename Archive>
	void Deserialise(Archive &ar)
	{
		ar >> m_Data >> m_Primitives;
		m_bBuilt = true;
	}

private:
	mutable KdTreeData m_Data;
	Btl::Vector<T>	m_Primitives;
	mutable int m_nTests;
	mutable bool m_bBuilt;
};

}

#endif
