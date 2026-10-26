#ifndef EDITOR_LIB_STATIC_PLUG_H
#define EDITOR_LIB_STATIC_PLUG_H

#ifdef _MSC_VER_
#pragma once
#endif

#include <EditorLib/Dg/Attr/PlugBase.h>
#include <EditorLib/Dg/Attr/PlugHelpers.h>

namespace EditorLib
{
	template<typename DATA_TYPE>
	class C_StaticPlug : public C_PlugBase<C_NamedPlugPolicy, C_ChildPolicy, C_ConnectablePolicy>
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

	//From C_StaticPlug
	public:
		INLINE C_StaticPlug(
			I_DgNode* pParent,
			void(*pOnInputChangedCallback)(I_Plug* pThis),
			E_PlugType type,
			const char* name);

		INLINE C_StaticPlug(
			I_Plug* pParent,
			void(*pOnInputChangedCallback)(I_Plug* pThis),
			E_PlugType type,
			const char* name);

		///\brief Destructor.
		///
		INLINE virtual ~C_StaticPlug();

		INLINE void SetValue(const DATA_TYPE& value, bool triggerTopLevelParentOnChanged = true);

		INLINE const DATA_TYPE& GetValue(bool update = true) const;

	protected:
		DATA_TYPE m_value;
	};

	#include "StaticPlug.inl"
}

#endif
