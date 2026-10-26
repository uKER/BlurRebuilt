#ifndef AFTK_STATIC_PLUG_H
#define AFTK_STATIC_PLUG_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	template<class T>
	class C_NativeStaticPlug;

	///
	///Note that for compatibility with I_Plug we need to provide run-time type information.
	template<class T>
	class C_StaticPlug : public C_NullPlug
	{
	public:
		///\brief Constructor.
		///
		C_StaticPlug(E_GenericAttributeType type, void(*pOnChangedCallback)(void* pThis));

		///\brief Destructor.
		///
		~C_StaticPlug();

		///\brief Asignment operator
		///
		///@param rhs The class to copy.
		///@return A reference to this class.
		const C_StaticPlug& operator=(const C_StaticPlug &rhs);

		///\brief Equivilance test operator.
		///
		///@param rhs The class to test against.
		///@return bool Test result.
		bool operator==(const C_StaticPlug &rhs) const;
		
		///\brief Negated equivilance test operator.
		///
		///@param rhs The class to test against.
		///@return bool Test result.
		bool operator!= (const C_StaticPlug &rhs) const;

		///\brief Less than operator.
		///
		///This test is required in order to be able to store generic attributes within a sorted container.
		///
		///@param rhs The class to test against
		///@return bool Test result.
		bool operator<(const C_StaticPlug &rhs) const;

		///\brief Overwrites the contents of the generic attribute using another generic attribute.
		///
		///@param rhs The generic attribute to copy.
		void SetValue(const T& value, bool evaluateDependants = true);

		///\brief Get the value of this attribute.
		///
		///@param value Attribute value.
		bool GetValue(U8& value, bool evaluate = true) const;

		///\brief Get the value of this attribute.
		///
		///@param value Attribute value.
		bool GetValue(U16& value, bool evaluate = true) const;

		///\brief Get the value of this attribute.
		///
		///@param value Attribute value.
		bool GetValue(U32& value, bool evaluate = true) const;

		///\brief Get the value of this attribute.
		///
		///@param value Attribute value.
		bool GetValue(U64& value, bool evaluate = true) const;

		///\brief Get the value of this attribute.
		///
		///@param value Attribute value.
		bool GetValue(S8& value, bool evaluate = true) const;

		///\brief Get the value of this attribute.
		///
		///@param value Attribute value.
		bool GetValue(S16& value, bool evaluate = true) const;

		///\brief Get the value of this attribute.
		///
		///@param value Attribute value.
		bool GetValue(S32& value, bool evaluate = true) const;

		///\brief Get the value of this attribute.
		///
		///@param value Attribute value.
		bool GetValue(S64& value, bool evaluate = true) const;

		///\brief Get the value of this attribute.
		///
		///@param value Attribute value.
		bool GetValue(F32& value, bool evaluate = true) const;

		///\brief Get the value of this attribute.
		///
		///@param value Attribute value.
		bool GetValue(F64& value, bool evaluate = true) const;

		///\brief Get the value of this attribute.
		///
		///@param value Attribute value.
		bool GetValue(bool& value, bool evaluate = true) const;

		///\brief Get the value of this attribute.
		///
		///@param value Attribute value.
		bool GetValue(std::string& value, bool evaluate = true) const;

		///\brief Get the value of this attribute.
		///
		///@param value Attribute value.
		bool GetValue(Matrix44& value, bool evaluate = true) const;
	
		///Determines whether this plug is valid. A plug is only valid if it refers to an attribute.
		///
		///@return bool plug validity
		bool IsNull() const;

		///\brief Get the value type of this attribute.
		///
		///This might be, U32, bool, Vector3F32 etc.
		E_GenericAttributeType GetDataType() const;

		U32 GetHash() const;

		///\brief Equivilance operator which operates on entire class heirachy
		///
		///@param rhs The node for comparison.
		///@return returns 'true' if this node and aftkNode are equivilant. Otherwise 'false' is returned.
		virtual bool IsEqualTo(const I_Object &rhs, E_ObjType classToAbortTest = E_ObjType_Unknown, U32 flags = 0) const;

		///\brief Get this objects's type enum.
		///
		///AFTK node types are all identified by the enumerators of E_ObjType. This function returns the
		///enum indicating the type of this object. For example, spline, camera actor etc.
		///
		///@return object's type enum.
		E_ObjType GetType() const;

		///\brief Returns true if this object implements the given type.
		///
		///@param The object type to test against.
		///@return true if we implement the given object type.
		bool HasType(E_ObjType type) const;

		virtual I_Plug* Clone() const;

		void Clear();

		///\brief Equivilance operator for this class only.
		///
		///@param rhs The node for comparison.
		///@return returns 'true' if this node and aftkNode are equivilant. Otherwise 'false' is returned.
		bool IsEqualToThis(const I_Object& rhs, U32 flags = 0) const;

		//New
		void* operator new (size_t size);

		//Delete.
		void operator delete(void* p);
		
	#ifndef DOXYGEN_SHOULD_SKIP_THIS
	protected:
		void OneTimeInit();

		void ClearThis();

		bool IsEqualToAll(const I_Object& rhs, E_ObjType classToAbortTest = E_ObjType_Unknown, U32 flags = 0) const;

		T						m_value;
		E_GenericAttributeType	m_dataType;

		FRIEND_CLASS C_NativePlug;
		FRIEND_CLASS C_NativeFtk;
		FRIEND_CLASS C_NativeStaticPlug<T>;
	#endif
	};

	#include "AftkStaticPlug.inl"
}

#endif
