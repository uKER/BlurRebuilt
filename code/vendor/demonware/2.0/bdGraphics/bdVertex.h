// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE		: Provide a platform independent vertex representation.

#ifndef BD_VERTEX_H
#define BD_VERTEX_H

#include <bdCore/bdCore.h>

class bdVertex
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		bdFloat32 m_x;
		bdFloat32 m_y;
		bdFloat32 m_z;

};

#endif // BD_VERTEX_H

