#ifndef EDITOR_LIB_IDG_OBJECT_H
#define EDITOR_LIB_IDG_OBJECT_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace EditorLib
{
	class I_DgObject : public Sys::RefObj
	{
	public:
		I_DgObject();
		virtual ~I_DgObject();

		///\brief Get this objects's type enum.
		///
		///Node types are all identified by the enumerators of E_ObjType. This function returns the
		///enum indicating the type of this object.
		///
		///@return object's type enum.
		virtual E_ObjType GetType() const = 0;

		virtual const char* GetTypeName() const = 0;

		///\brief Returns true if this object implements the given type.
		///
		///@param The object type to test against.
		///@return true if we implement the given object type.
		virtual bool IsA(E_ObjType type) const;
	};
}

#endif
