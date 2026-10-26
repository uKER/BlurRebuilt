#ifndef __PHYSICS_GJK_H__
#define __PHYSICS_GJK_H__

namespace Impact
{

class ConvexBase;
class HullPersistentData;

/// determine whether two convex hulls intersect using the GJK algorithm
class GJK
{
	/// part of the internal simplex datastructure
	struct SimplexItem : public Maths::HVec3
	{
		SimplexItem() :
			Maths::HVec3(Maths::UNDEFINED)
		{
		}

		SimplexItem &operator =(const Maths::HVec3 &v)
		{
			*(Maths::HVec3*)this = v;
			return *this;
		}

		int iP1, iP2;
	};
	/// internal simplex representation
	typedef Ut::UtStackVector<SimplexItem,4> Simplex;

public:
					/// construct using a two convex hulls
					GJK(ConvexBase *pHull1, ConvexBase *pHull2);

					/// calculate whether the two hulls intersect. returns true if they do
	bool			Process(HullPersistentData *pPersist);

					/// draw the minkowski sum into the debug renderer by projecting a bunch points in random directions
	void			DebugDrawMinkowski();
private:
					/// calculate the maximum point on the minkowski sum of the two hulls, along a support axis
	void			ProjectSupportAxis(const Maths::C_Normal &vNormal, SimplexItem &SI);
					/// determine the new search direction given a simplex with 2 points
	void			DetermineSearchDirection2(const Maths::HVec3 &vA, const Maths::HVec3 &vB);
					/// determine the new search direction given a simplex with 3 points
	void			DetermineSearchDirection3(const Maths::HVec3 &vA, const Maths::HVec3 &vB, const Maths::HVec3 &vC);
					/// determine the new search direction given a simplex with 4 points
	bool			DetermineSearchDirection4(const Maths::HVec3 &vA, const Maths::HVec3 &vB, const Maths::HVec3 &vC, const Maths::HVec3 &vD);
					/// test whether the simplex in the PersistentData still encapsulates the origin
	void			TestPersistentSimplex(HullPersistentData *pPersist);

	ConvexBase		*m_pHull1;
	ConvexBase		*m_pHull2;

	Maths::C_Normal	m_vDir;		// current search direction
	Simplex			m_Simplex;	// current simplex
};

};

#endif
