#ifndef AFTK_UT_SNAP_VERTICES_H
#define AFTK_UT_SNAP_VERTICES_H

#ifdef MSC_VER
#pragma once
#endif

namespace AftkUt
{
	struct I_SnapEquality
	{
		virtual bool Equal(
			const Aftk::Position3& p1,
			const Aftk::Position3& p2) = 0;

		//used for bounds checking in our internal kd-tree
		virtual F32 Threshold(const Aftk::Position3& p) = 0;
	};

	struct StaticSnapEquality : public I_SnapEquality
	{
		StaticSnapEquality(F32 threshold)
			: m_threshold(threshold)
		{ }

		inline bool Equal(
			const Aftk::Position3& p1,
			const Aftk::Position3& p2)
		{
			return Aftk::EqualityTestFVector(p1, p2, m_threshold*m_threshold);
		}

		inline F32 Threshold(const Aftk::Position3& p)
		{
			return m_threshold;
		}

		F32 m_threshold;
	};

	// @see http://www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm
	struct RelativeSnapEquality : public I_SnapEquality
	{
		RelativeSnapEquality(U32 maxUlps)
			: m_maxUlps(maxUlps)
		{ }

		inline bool Equal(
			const Aftk::Position3& p1,
			const Aftk::Position3& p2)
		{
			// Make sure maxUlps is non-negative and small enough that the
		    // default NAN won't compare as equal to anything.
		    assert(m_maxUlps>0 && m_maxUlps<4*1024*1024);

		    S32 x1=*(S32*)&p1[0]; S32 y1=*(S32*)&p1[1]; S32 z1=*(S32*)&p1[2];
			S32 x2=*(S32*)&p2[0]; S32 y2=*(S32*)&p2[1]; S32 z2=*(S32*)&p2[2];

			//order lexicographically as twos-complement ints
		    if (x1<0) x1 = 0x80000000-x1;
			if (y1<0) y1 = 0x80000000-y1;
			if (z1<0) z1 = 0x80000000-z1;
			if (x2<0) x2 = 0x80000000-x2;
			if (y2<0) y2 = 0x80000000-y2;
			if (z2<0) z2 = 0x80000000-z2;

			S32 dx=abs(x1-x2); S32 dy=abs(y1-y2); S32 dz=abs(z1-z2);
			return dx<m_maxUlps && dy<m_maxUlps && dz<m_maxUlps;
		}

		inline F32 Threshold(const Aftk::Position3& p)
		{
			return 0.05f;
		}

		S32 m_maxUlps;
	};

	class C_SnapVertices
	{
	public:
		static void BuildNodesToSnap(Aftk::SceneMeshNodePtrArray& nodes, Aftk::C_Scene& scene);

		static Aftk::C_Status Snap(
			const Aftk::SceneMeshNodePtrArray& nodes,
			I_SnapEquality& t = RelativeSnapEquality(4));
	};
}

#endif