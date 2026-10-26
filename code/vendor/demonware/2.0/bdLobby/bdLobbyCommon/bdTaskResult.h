// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS


#ifndef BD_TASK_RESULT_H
#define BD_TASK_RESULT_H

#include <bdCore/bdCore.h>

class bdTaskResult
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS
		
		bdTaskResult();

		virtual ~bdTaskResult();

		virtual bdBool deserialize(bdByteBufferRef buffer) = 0;
		
		virtual bdUInt sizeOf() = 0;

};

#endif // BD_TASK_RESULT_H
