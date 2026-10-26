#ifndef AFTK_NULL_PLUG_H
#define AFTK_NULL_PLUG_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_Node;
	class C_ShaderNode;
	class C_SceneNode;
	class C_NativeNullPlug;
	class C_AnimationCurveNode;
	class C_ClipSourceNode;
	class C_BlendWeightedNode;
	class C_UnitConversionNode;
	class C_Track;
	class C_SceneAimConstraintNode;
	class C_SceneLookAtConstraintNode;
	class C_SceneCameraNode;
	class C_DistanceDimNode;
	class C_ScenePointConstraintNode;
	class C_SceneOrientConstraintNode;

	class C_NullPlug : public I_Plug
	{
	public:
		///\brief Destructor.
		///
		~C_NullPlug();

		///\brief Assignment operator.
		///
		///@param rhs Class to copy.
		///@return Reference to this class.
		const C_NullPlug& operator=(const C_NullPlug &rhs);
		
		///\brief Equivilance test operator.
		///
		///@param rhs The class to test against.
		///@return bool Test result.
		bool operator==(const C_NullPlug &rhs) const;
		
		///\brief Negated equivilance test operator.
		///
		///@param rhs The class to test against.
		///@return bool Test result.
		bool operator!= (const C_NullPlug &rhs) const;

		///\brief Less than operator.
		///
		///This test is required in order to be able to store generic attributes within a sorted container.
		///
		///@param rhs The class to test against
		///@return bool Test result.
		bool operator<(const C_NullPlug &rhs) const;

		///\brief Get the name of this attribute.
		///
		///@return Attribute name;
		const std::string& GetName() const;
		
		///Determines whether this plug is valid. A plug is only valid if it refers to an attribute.
		///
		///@return bool plug validity
		bool IsNull() const;

		const I_Plug* GetChildPtr(U32 instance) const;

		I_Plug* GetChildPtr(U32 instance);

		U32 GetNumChildren() const;

		///\brief Get the value of this attribute.
		///
		///@param value Attribute value.
		virtual bool GetValue(U8& value, bool evaluate = true) const;

		///\brief Get the value of this attribute.
		///
		///@param value Attribute value.
		virtual bool GetValue(U16& value, bool evaluate = true) const;

		///\brief Get the value of this attribute.
		///
		///@param value Attribute value.
		virtual bool GetValue(U32& value, bool evaluate = true) const;

		///\brief Get the value of this attribute.
		///
		///@param value Attribute value.
		virtual bool GetValue(U64& value, bool evaluate = true) const;

		///\brief Get the value of this attribute.
		///
		///@param value Attribute value.
		virtual bool GetValue(S8& value, bool evaluate = true) const;

		///\brief Get the value of this attribute.
		///
		///@param value Attribute value.
		virtual bool GetValue(S16& value, bool evaluate = true) const;

		///\brief Get the value of this attribute.
		///
		///@param value Attribute value.
		virtual bool GetValue(S32& value, bool evaluate = true) const;

		///\brief Get the value of this attribute.
		///
		///@param value Attribute value.
		virtual bool GetValue(S64& value, bool evaluate = true) const;

		///\brief Get the value of this attribute.
		///
		///@param value Attribute value.
		virtual bool GetValue(F32& value, bool evaluate = true) const;

		///\brief Get the value of this attribute.
		///
		///@param value Attribute value.
		virtual bool GetValue(F64& value, bool evaluate = true) const;

		///\brief Get the value of this attribute.
		///
		///@param value Attribute value.
		virtual bool GetValue(bool& value, bool evaluate = true) const;

		///\brief Get the value of this attribute.
		///
		///@param value Attribute value.
		virtual bool GetValue(std::string& value, bool evaluate = true) const;

		///\brief Get the value of this attribute.
		///
		///@param value Attribute value.
		virtual bool GetValue(Matrix44& value, bool evaluate = true) const;

		///\brief Get the value type of this attribute.
		///
		///This might be, U32, bool, Vector3F32 etc.
		virtual E_GenericAttributeType GetDataType() const;

		virtual U32 GetHash() const;

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
		virtual E_ObjType GetType() const;

		///\brief Returns true if this object implements the given type.
		///
		///@param The object type to test against.
		///@return true if we implement the given object type.
		virtual bool HasType(E_ObjType type) const;

		virtual I_Plug* Clone() const;

		void Clear();

		///\brief Equivilance operator for this class only.
		///
		///@param rhs The node for comparison.
		///@return returns 'true' if this node and aftkNode are equivilant. Otherwise 'false' is returned.
		bool IsEqualToThis(const I_Object &rhs, U32 flags = 0) const;

		//New
		void* operator new (size_t size);

		//Delete.
		void operator delete(void* p);
		
	#ifndef DOXYGEN_SHOULD_SKIP_THIS
	protected:
		///\brief Constructor.
		///
		///@param onChanged callback
		C_NullPlug(void(*pOnChangedCallback)(void* pThis));

		void OneTimeInit();

		void ClearThis();

		S32 AddChildPtr(I_Plug* pChild);

		void DeleteChildren();

		bool IsEqualToAll(const I_Object& rhs, E_ObjType classToAbortTest = E_ObjType_Unknown, U32 flags = 0) const;

		///\brief Sets the name of this attribute.
		///
		///@param name Attribute name.
		void SetName(const std::string &name);

		void SetChildrenLocked(bool lock);

		bool m_childrenLocked;

		std::string m_name;

		//Children, used if we are a compound plug
		Ut::UtVector< Ut::DynamicVector<I_Plug*, Ut::ArrayAllocator<I_Plug*> > >	
			m_children;

		FRIEND_CLASS I_Object;
		FRIEND_CLASS C_ShaderImageNode;
		FRIEND_CLASS C_ShaderNode;
		FRIEND_CLASS C_MaterialNode;
		FRIEND_CLASS C_NativePlug;
		FRIEND_CLASS C_NativeFtk;
		FRIEND_CLASS C_SceneNode;
		FRIEND_CLASS C_NativeNullPlug;
		FRIEND_CLASS C_AnimationCurveNode;
		FRIEND_CLASS C_ClipSourceNode;
		FRIEND_CLASS C_BlendWeightedNode;
		FRIEND_CLASS C_UnitConversionNode;
		FRIEND_CLASS C_Track;
		FRIEND_CLASS C_SceneAimConstraintNode;
		FRIEND_CLASS C_SceneLookAtConstraintNode;
		FRIEND_CLASS C_SceneCameraNode;
		FRIEND_CLASS C_DistanceDimNode;
		FRIEND_CLASS C_ScenePointConstraintNode;
		FRIEND_CLASS C_SceneOrientConstraintNode;
	#endif
	};
}

#endif
