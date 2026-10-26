// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS


#ifndef BD_SESSION_PARAMS_H
#define BD_SESSION_PARAMS_H

#include <bdCore/bdCore.h>

/// Class to contain session parameters when searching for online games.
class bdSessionParams
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS
		
		bdSessionParams();

		virtual ~bdSessionParams();

		virtual void serialize(bdByteBuffer &buffer)const;

		// Override this function in any subclass
		virtual bdUInt sizeOf();

};


#endif // BD_SESSION_PARAMS_H
