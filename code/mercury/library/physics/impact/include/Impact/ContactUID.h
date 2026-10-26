#ifndef __PHYSICS_CONTACT_UID_H__
#define __PHYSICS_CONTACT_UID_H__

namespace Impact
{

/// the unique identifier of a contact
/// is a combination of the edge indices of the two collidinb objects
struct ContactUID
{
	union
	{
		struct Edges
		{
			U8 inEdge1;
			U8 outEdge1;
			U8 inEdge2;
			U8 outEdge2;
		} e;
		int key;
	};
};

};

#endif
