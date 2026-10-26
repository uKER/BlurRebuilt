#pragma once

#ifndef EDITOR_LIB_STATUS_H
#define EDITOR_LIB_STATUS_H

namespace EditorLib
{
	enum E_Status
	{
		E_Status_Failure,
		E_Status_Success,
		E_Status_NotImplemented,
		E_Status_OutOfMemory,
		E_Status_Cancelled
	};

	class C_Status
	{
	public:
		INLINE C_Status();
		INLINE C_Status(const C_Status& rhs);
		INLINE C_Status(E_Status status);

		INLINE const C_Status& operator=(const C_Status& rhs);
		
		INLINE const C_Status& operator=(E_Status status);

		INLINE operator bool() const;

		INLINE operator E_Status() const;

	private:
		E_Status m_errorState;
	};

	#include "status.inl"
}

#endif
