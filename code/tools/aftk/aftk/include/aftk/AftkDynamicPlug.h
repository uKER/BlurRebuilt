#ifndef AFTK_DYNAMIC_PLUG_H
#define AFTK_DYNAMIC_PLUG_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_Node;
	class C_ShaderNode;

	class C_DynamicPlug : public I_Plug
	{
	public:
		///\brief Constructor.
		///
		C_DynamicPlug();

		///\brief Copy constructor.
		///
		C_DynamicPlug(const C_DynamicPlug &rhs);

		///\brief Destructor.
		///
		~C_DynamicPlug();

		///\brief Assignment operator.
		///
		///@param rhs Class to copy.
		///@return Reference to this class.
		const C_DynamicPlug& operator=(const C_DynamicPlug &rhs);
		
		///\brief Equivilance test operator.
		///
		///@param rhs The class to test against.
		///@return bool Test result.
		bool operator==(const C_DynamicPlug &rhs) const;
		
		///\brief Negated equivilance test operator.
		///
		///@param rhs The class to test against.
		///@return bool Test result.
		bool operator!= (const C_DynamicPlug &rhs) const;

		///\brief Less than operator.
		///
		///This test is required in order to be able to store generic attributes within a sorted container.
		///
		///@param rhs The class to test against
		///@return bool Test result.
		bool operator<(const C_DynamicPlug &rhs) const;

		///\brief Sets the name of this attribute.
		///
		///@param name Attribute name.
		void SetName(const std::string &name);

		///\brief Get the name of this attribute.
		///
		///@return Attribute name;
		const std::string& GetName() const;

		///Determines whether this plug is valid. A plug is only valid if it refers to an attribute.
		///
		///@return bool I_Plug validity
		bool IsNull() const;

		S32 AddChildPtr(C_DynamicPlug* pChild);

		const I_Plug* GetChildPtr(U32 instance) const;

		U32 GetNumChildren() const;

		///\brief Set the value of this attribute.
		///
		///@param value Attribute value.
		void SetValue(const U8& value, bool evaluateDependants = true);

		///\brief Set the value of this attribute.
		///
		///@param value Attribute value.
		void SetValue(const U16& value, bool evaluateDependants = true);

		///\brief Set the value of this attribute.
		///
		///@param value Attribute value.
		void SetValue(const U32& value, bool evaluateDependants = true);

		///\brief Set the value of this attribute.
		///
		///@param value Attribute value.
		void SetValue(const U64& value, bool evaluateDependants = true);

		///\brief Set the value of this attribute.
		///
		///@param value Attribute value.
		void SetValue(const S8& value, bool evaluateDependants = true);

		///\brief Set the value of this attribute.
		///
		///@param value Attribute value.
		void SetValue(const S16& value, bool evaluateDependants = true);

		///\brief Set the value of this attribute.
		///
		///@param value Attribute value.
		void SetValue(const S32& value, bool evaluateDependants = true);

		///\brief Set the value of this attribute.
		///
		///@param value Attribute value.
		void SetValue(const S64& value, bool evaluateDependants = true);

		///\brief Set the value of this attribute.
		///
		///@param value Attribute value.
		void SetValue(const F32& value, bool evaluateDependants = true);

		///\brief Set the value of this attribute.
		///
		///@param value Attribute value.
		void SetValue(const F64& value, bool evaluateDependants = true);

		///\brief Set the value of this attribute.
		///
		///@param value Attribute value.
		void SetValue(const bool& value, bool evaluateDependants = true);

		///\brief Set the value of this attribute.
		///
		///@param value Attribute value.
		void SetValue(const std::string& value, bool evaluateDependants = true);

		///\brief Set the value of this attribute.
		///
		///@param value Attribute value.
		void SetValue(const Matrix44& value, bool evaluateDependants = true);

		///\brief Set the value of this attribute.
		///
		///@param value Attribute value.
		void SetValue(const Aftk::C_DynamicPlug& value, bool evaluateDependants = true);

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

		const C_GenericAttribute& GetAttribute() const;

		///\brief Get the value type of this attribute.
		///
		///This might be, U32, bool etc.
		E_GenericAttributeType GetDataType() const;

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

		void DeleteChildren();

		virtual I_Plug* Clone() const;

		virtual void Clear();
		
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

		bool IsEqualToAll(const I_Object &rhs, E_ObjType classToAbortTest = E_ObjType_Unknown, U32 flags = 0) const;

		template<class T>
		void SetValueT(const T& value, bool evaluateDependants)
		{
			m_attribute.SetValue(value);
			if (evaluateDependants)
				OnChanged();
		}

		C_GenericAttribute m_attribute;

		//Children, used if we are a compound plug
		Ut::UtVector< Ut::DynamicVector<I_Plug*, Ut::ArrayAllocator<I_Plug*> > >	
			m_children;

		friend class I_Object;
		friend class C_ShaderImageNode;
		friend class C_ShaderNode;
		friend class C_MaterialNode;
		friend class C_NativePlug;
		friend class C_NativeFtk;
		friend class C_NativeDynamicPlug;

	#endif
	};
}

#endif
