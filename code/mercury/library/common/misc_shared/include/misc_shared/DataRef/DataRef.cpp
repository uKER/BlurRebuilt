#include "MiscSharedPch.h"

#include "DataRef.h"


/**
@brief Smart pointer functionality for abstracting data sources.

The C_Ptr smart pointer provides a means to refer to data from arbitrary
sources without worrying about the underlying memory management model.

Users must provide one or more implementations of I_Storage as backend data
sources, together with functions to create C_Ptr objects using these storage
types. There are some generic storage types exposed through the
FactoryCreate() and SorceryLookup() functions, which can be customised
through template parameters.

For deserialisation, the DeserialisePtr() template function should be 
overridden to the data type.
*/
namespace DataRef
{
} // namespace DataRef
