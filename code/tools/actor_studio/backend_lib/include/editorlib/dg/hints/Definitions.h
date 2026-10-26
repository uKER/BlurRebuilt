#ifndef EDITOR_LIB_DG_DEFINITIONS_H
#define EDITOR_LIB_DG_DEFINITIONS_H

#ifdef _MSC_VER_
#pragma once
#endif

#include <misc_shared/UID/FourCC.h>

namespace EditorLib
{
	class I_UpdateHint;
		
	enum E_UpdateHint
	{
		E_UpdateHint_HiBits = FOURCC_HI20('UPHN'), 

		E_UpdateHint_Name,
		E_UpdateHint_Transform, 

		E_UpdateHint_End, 
		E_UpdateHint_Begin = E_UpdateHint_HiBits + 1, 
		E_UpdateHint_Count = E_UpdateHint_End - E_UpdateHint_Begin
	};

	enum E_ImmediateHint
	{
		E_ImmediateHint_HiBits = FOURCC_HI20('IMHN'), 

		E_ImmediateHint_PostInitialise, 
		E_ImmediateHint_Reparent,
		E_ImmediateHint_Transform, 

		E_ImmediateHint_End, 
		E_ImmediateHint_Begin = E_ImmediateHint_HiBits + 1, 
		E_ImmediateHint_Count = E_ImmediateHint_End - E_ImmediateHint_Begin
	};
}

#endif
