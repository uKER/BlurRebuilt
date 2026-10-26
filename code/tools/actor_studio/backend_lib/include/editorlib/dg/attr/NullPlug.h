#ifndef EDITOR_LIB_NULL_PLUG_H
#define EDITOR_LIB_NULL_PLUG_H

#ifdef _MSC_VER_
#pragma once
#endif

#include <EditorLib/Dg/Attr/PlugBase.h>

namespace EditorLib
{
	class C_NullPlug : public C_PlugBase<C_NamedPlugPolicy, C_ChildPolicy, C_NonConnectablePolicy>
	{
	//From I_Attribute
	public:
		///\brief Get the name of this attribute.
		///
		///@return Attribute name;
		INLINE virtual const char* GetName() const;

		///\brief Set the value of this attribute.
		///
		///@param value Attribute value.
		INLINE virtual void GetValue(U32& value, bool update = true) const;
		INLINE virtual void GetValue(U64& value, bool update = true) const;
		INLINE virtual void GetValue(S32& value, bool update = true) const;
		INLINE virtual void GetValue(S64& value, bool update = true) const;
		INLINE virtual void GetValue(F32& value, bool update = true) const;
		INLINE virtual void GetValue(F64& value, bool update = true) const;
		INLINE virtual void GetValue(bool& value, bool update = true) const;
		INLINE virtual void GetValue(Btl::String& value, bool update = true) const;
		INLINE virtual void GetValue(const Btl::String*& value, bool update = true) const;
		INLINE virtual void GetValue(MathsStorage::Matrix44& value, bool update = true) const;
		INLINE virtual void GetValue(const MathsStorage::Matrix44*& value, bool update = true) const;

		///\brief Get the value type of this attribute.
		///
		///This might be, U32, bool etc.
		INLINE virtual Util::E_VarType GetDataType() const;

	public:
		///\brief Constructor.
		///
		C_NullPlug(
			I_DgNode* pParent,
			E_PlugType type,
			const char* name);

		C_NullPlug(
			I_Plug* pParent,
			E_PlugType type,
			const char* name);

		///\brief Destructor.
		///
		virtual ~C_NullPlug();

	protected:
	};

	#include "NullPlug.inl"
}

#endif
