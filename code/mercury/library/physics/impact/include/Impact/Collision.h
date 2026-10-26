#ifndef __PHYSICS_COLLISION_H__
#define __PHYSICS_COLLISION_H__

// need to define one of these
#define TWO_FRICTION_AXIS
//#define SINGLE_FRICTION_MANIFOLD

namespace Impact
{

class I_PersistentData;
class Body;
class Shape;

#define MAX_CONTACTS 24

/// a contact point of two bodies that are in collision
class Contact
{
public:
	Contact() : m_vPos(Maths::UNDEFINED), m_vNormal(Maths::UNDEFINED) {}

	Maths::HVec3	m_vPos;
	Maths::C_Normal m_vNormal;
	float			m_fAccNormalImpulse;
	union
	{
	float			m_fSeparation;	// negative separation = penetration
	float			m_fBias;
	};
	float			m_invkNormal;
	int				m_uid;
#ifndef SINGLE_FRICTION_MANIFOLD
	float			m_fAccTangentImpulseX;
	float			m_fAccTangentImpulseY;
	float			m_invkTangent[2];
#endif

	Impact::Contact &operator=(const Impact::Contact &in)
	{
		m_vPos = in.m_vPos;
		m_vNormal = in.m_vNormal;
		m_fSeparation = in.m_fSeparation;
		m_uid = in.m_uid;
		return *this;
	}
};

// this defines the maximum number of contacts
typedef Ut::UtVector<Ut::StackVector<Contact,MAX_CONTACTS> > ContactVector;

/// calculate the collision between two bodies
/// the return value is the number of contacts
/// the 'contacts' vector contains the actual contact data
int Collide(ContactVector &contacts, Body *pBody1, Body *pBody2, I_PersistentData **ppPersist);

/// as above, but takes a separate body and shape *
int Collide(ContactVector &contacts, Body *pBody[2], Shape *pShape[2], I_PersistentData **ppPersist);

/// display contacts for debugging purposes
void DisplayContacts(ContactVector &contacts);

};

#endif
