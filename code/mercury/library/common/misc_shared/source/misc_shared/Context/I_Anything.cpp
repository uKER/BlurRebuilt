#include "MiscSharedPch.h"

#include "misc_shared/Context/I_Anything.h"


/**
@brief Interface and helpers for generic RTTI system.

Provides a subset of RTTI functionality, in a similar manner to the I_Unknown
interface in COM.

To add RTTI to a class, derive from C_Anything and supply a user-defined
type identifier. It is the responsibility of the user to ensure that this
identifier is unique within the applicable context.

@todo Multiple-inheritence versions of C_Anything if and when they're needed.
*/
namespace Sys
{

/**
@param typeUid unique type identifier.
*/
I_Anything::I_Anything(const C_AnythingUid typeUid) : 
	m_TypeUid(typeUid) 
{
	// intentionally blank
}

I_Anything::~I_Anything()
{
	// intentionally blank
}

/**
@brief Determines if the object is compatible with this type.
Derived classes should implement this to test the type id and forward to the
parent class if it fails. Note that C_Anything does this automatically.
@param typeUid type identifier.
@returns true if QueryInterface will succeed.
*/
bool I_Anything::IsTypeOf(const C_AnythingUid typeUid) const 
{
	return false;
}

/**
@brief Obtains a pointer to the requested class.
Derived classes should implement this to test the type id and forward to the
parent class if it fails. Note that C_Anything does this automatically.
@param typeUid type identifier.
@param ppDest receives the pointer.
@returns true if the class was available.
*/
bool I_Anything::QueryInterfaceImpl(const C_AnythingUid typeUid, void** ppDest) 
{
	*ppDest = NULL; 
	return false;
}


} // namespace Sys
