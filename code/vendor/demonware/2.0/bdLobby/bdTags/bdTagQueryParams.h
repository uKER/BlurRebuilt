// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS


#ifndef BD_TAG_QUERY_PARAMS_H
#define BD_TAG_QUERY_PARAMS_H

#include <bdCore/bdCore.h>

/// Class to contain session parameters when searching for online games.
/// FIXME: This class is a (almost) direct copy of bdMatchMaking\bdSessionParams
///		   Investigae if bdTags should instead use that class in a sensible fashion.
class bdTagQueryParams
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS
		
		bdTagQueryParams();

		virtual ~bdTagQueryParams();

		virtual void serialize(bdByteBuffer &buffer) const = 0;

		virtual bdUInt sizeOf() const = 0;
};

#endif // BD_TAG_QUERY_PARAMS_H
