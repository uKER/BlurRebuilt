#include <core/core.h>
#include <cxxtest/TestSuite.h>

#include <system/serialise.h>

#include <misc_shared/kdtree/KdTree.h>
#include <system/misc/random.h>
#include <system/serialise/C_MemoryStream.h>

class KdTreeTest : public CxxTest::TestSuite
{
public:

	// compare two sets of results. they don't necessarily have to be in the same order
	bool FindResult(int *pA, int n, int x)
	{
		for (int i=0; i<n; i++)
		{
			if (pA[i] == x)
				return true;
		}

		return false;
	}

	void CompareResults(int *pA, int *pB, int nA, int nB)
	{
		for (int i=0; i<nA; i++)
		{
			TS_ASSERT(FindResult(pB, nB, pA[i]));
		}		
	}

	/// brute-force function to compare the kd-tree against
	int BruteForce(Util::KdTree<Maths::HVec3> *pTree, Maths::HVec3 v)
	{
		float fNearest = FLT_MAX;
		int iNearest;
		for (int i=0; i<pTree->Size(); i++)
		{
			float f = pTree->At(i).Distance(v);
			if (f<fNearest)
			{
				fNearest = f;
				iNearest = i;
			}
		}

		return iNearest;
	}

	/// test kd tree building, serialising and deserialising and finding the clostest point
	void testKdTreeFullTest()
	{
		Util::KdTree<Maths::HVec3> kdtree;

		// put boxes in random places
		Util::C_Random random;
		Maths::HVec3 vNear = random.SignedCubeRand(1000.f);
		for (int i=0; i<1000; i++)
			kdtree.PushBack(random.SignedCubeRand(1000.f));
		kdtree.PushBack(Maths::HVec3(0.1f, -0.1f, 0.2f));
		kdtree.PushBack(vNear);

		// serialise the data to memory
		Sys::C_MemoryStream memstream(1024*16);
		Sys::BinarySerialiser<Ut::NoEndianSwap> ar(memstream);
		ar << kdtree;
		
		// deserialise the data
		Sys::C_MemoryStream memstream2(memstream.GetBufferPtr(), (U32)memstream.GetBufferSize());
		Sys::BinaryDeserialiser<Ut::NoEndianSwap> ar2(memstream2);
		Util::KdTree<Maths::HVec3> kdtree2;
		ar2 >> kdtree2;

		// perform queries
		for (int i=0; i<10; i++)
		{
			vNear = random.SignedCubeRand(1000.f);
//			vNear = vNear + Maths::HVec3(0, 500000.f, 0);

			int x = kdtree2.FindNearest(vNear);		// check 'FindNearest' function
			float f1 = kdtree2[x].Distance(vNear);
			int x2 = BruteForce(&kdtree2, vNear);
			float f2 = kdtree2[x2].Distance(vNear);

			TS_ASSERT_EQUALS(x, x2);

			kdtree2.PushBack(vNear);	// test re-building the tree with more data
		}
	}

	int BruteForce(Util::KdTree<Maths::C_AxisBox> *pTree, Maths::HVec3 v, int *pResults, int iMaxResults)
	{
		int nResults = 0;
		for (int i=0; i<pTree->Size(); i++)
		{
			if (pTree->At(i).Intersects(v))
			{
				pResults[nResults] = i;
				nResults++;
				if (nResults == iMaxResults)
					return nResults;
			}
		}
		return nResults;
	}

	/// build a tree out of axis aligned boxes and then find all the boxes that intersect with a given point
	void testKdTreeBoxPointIntersection()
	{
		Util::KdTree<Maths::C_AxisBox> kdtree;

		// put boxes in random places
		Util::C_Random random;
		Maths::HVec3 v(60.1f, 60.2f, 70.3f);
		for (int i=0; i<1000; i++)
		{
			Maths::HVec3 vCenter = random.SignedCubeRand(500.f);
			kdtree.PushBack(Maths::C_AxisBox(vCenter - v, vCenter + v));
		}

		// perform queries
		int resultsA[10], resultsB[10];
		for (int i=0; i<100; i++)
		{
			Maths::HVec3 vNear = random.SignedCubeRand(500.f);

			int nResultsA = kdtree.Intersect(vNear, resultsA, 10);
			int nResultsB = BruteForce(&kdtree, vNear, resultsB, 10);

			CompareResults(resultsA, resultsB, nResultsA, nResultsB);
		}
	}

	/// example of a user defined primitive that is used to build a kd-tree out of
	class MyPoint
	{
	public:
		/// initialise by passing a HVec3 into KdTree::PushBack
		MyPoint(const Maths::HVec3 v)
			: m_v(v)
		{
		}

		/// function which performs an intersection test on the primitive in a leaf and the parameter passed into KdTree::Intersect
		bool Intersects(const Maths::C_AxisBox &b) const
		{
			return b.Intersects(m_v);
		}

		/// function which returns this primitive's bounding box. this is used to build the kd-tree
		operator const Maths::C_AxisBox () const
		{
			return Maths::C_AxisBox(m_v,m_v);
		}

	private:
		Maths::HVec3 m_v;
	};

	int BruteForce(Util::KdTree<MyPoint> *pTree, Maths::C_AxisBox v, int *pResults, int iMaxResults)
	{
		int nResults = 0;
		for (int i=0; i<pTree->Size(); i++)
		{
			if (pTree->At(i).Intersects(v))
			{
				pResults[nResults] = i;
				nResults++;
				if (nResults == iMaxResults)
					return nResults;
			}
		}
		return nResults;
	}

	/// build a tree out of points and then find all the points contained in a given bounding box
	/// (this uses a custom datatype called MyPoint)
	void testKdTreePointBoxIntersection()
	{
		Util::KdTree<MyPoint> kdtree;

		// put boxes in random places
		Util::C_Random random;
		Maths::HVec3 v(60.1f, 60.2f, 70.3f);
		for (int i=0; i<1000; i++)
		{
			Maths::HVec3 vCenter = random.SignedCubeRand(500.f);
			kdtree.PushBack(vCenter);
		}

		// perform queries
		int resultsA[10], resultsB[10];
		for (int i=0; i<100; i++)
		{
			Maths::HVec3 vNear = random.SignedCubeRand(500.f);

			Maths::C_AxisBox bbox = Maths::C_AxisBox(vNear-v, vNear+v);
			int nResultsA = kdtree.Intersect(bbox, resultsA, 10);
			int nResultsB = BruteForce(&kdtree, bbox, resultsB, 10);

			CompareResults(resultsA, resultsB, nResultsA, nResultsB);
		}
	}

};

