#include "stdafx.h"

#include <converter_shared/ProcessGraph.h>

#include <data/DataTypes.h>
#include <data/NullData.h>

C_DataNull::C_DataNull( const Util::C_String& name )
	: m_Name(name)
{
}

// inherited from C_Data
const Util::C_String C_DataNull::GetName() const 
{
	return m_Name; 
}
