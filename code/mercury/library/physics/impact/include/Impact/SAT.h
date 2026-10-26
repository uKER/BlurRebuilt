#ifndef __PHYSICS_SAT_H__
#define __PHYSICS_SAT_H__

namespace Impact
{

class HullPersistentData;
class ConvexBase;

/// find the separating axis between two bodies with the lowest penetration depth
class SAT
{
public:
	/// structure containing the result
	struct Result
	{
		Maths::HVec3	vSeparatingAxis;	// separating axis with lowest penetration depth
		float			fPenetration;		// penetration depth
		int				iPlane[2];			// touching plane indices for hull 1 and 2
		int				iReference;			// index of the hull containing the reference plane
	};

					/// construct using a two convex hulls
					SAT(ConvexBase *pHull1, ConvexBase *pHull2);

					/// find the SAT
	void			Process(Result &result, HullPersistentData *pPersist);

					/// calculate the penetration depth for a given axis
	void			TestAxis(const Maths::C_Plane &vPlane);

private:
	float			m_fBias;
	float			m_fPenetration;
	Maths::HVec3	m_vSeparatingAxis;
	ConvexBase		*m_pHull[2];
	Maths::HVec3	m_vOppOrigin;
	int				m_iCurHull, m_iCurPlane, m_iHull, m_iPlane;
};

};

#endif
