#ifndef __PHYSICS_PERSISTENTDATA_H__
#define __PHYSICS_PERSISTENTDATA_H__

namespace Impact
{

class I_PersistentData
{
public:
	virtual ~I_PersistentData() {};
};

// data that is persistent between frames, to help speed up collision using frame coherence
class HullPersistentData : public I_PersistentData
{
public:
	HullPersistentData()
	{
		p1[0] = -1;
		iHull = 0;
		iPlane = 0;
	}

	Maths::C_Normal	normal;			// separating axis
	U8				p1[4], p2[4];	// GJK: indices of the 4 points that make up the simplex (p1[0] == -1 means not set)
	U8				iHull, iPlane;	// last best hull and plane of SAT
};

template<typename T>
T *GetPersist(I_PersistentData **ppPersist)
{
	if (!*ppPersist)
		*ppPersist = new T;
	return (T*)*ppPersist;
}

};

#endif
