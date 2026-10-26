#ifndef AFTK_I_PLUG_H
#define AFTK_I_PLUG_H

#ifdef _MSC_VER_
#pragma once
#endif

#define AFTK_MAX_DG_LOCATION_DEPTH 255

namespace Aftk
{
	struct C_PlugLogicalIndexList
	{
		C_PlugLogicalIndexList()
		{
			nIndices = 0;
		}

		U32 nIndices;
		//The logical indices for the plug.
		S32 indices[AFTK_MAX_DG_LOCATION_DEPTH];
	};

	struct C_PlugLocationInfo : C_PlugLogicalIndexList
	{
		//The top level parent of this plug.
		const C_Node* pTopLevelParent;
	};


	class I_Plug : public I_Object
	{
	public:
		typedef void(*OnChangedCallback)(void* pThis);

		///\brief Asignment operator
		///
		///@param rhs The class to copy
		///@return a reference to this node.
		const I_Plug& operator= (const I_Plug &rhs);

		///\brief Destructor.
		///
		~I_Plug();

		///\brief Equivilance test operator.
		///
		///@param rhs The class to test against.
		///@return bool Test result.
		bool operator==(const I_Plug &rhs) const;
		
		///\brief Negated equivilance test operator.
		///
		///@param rhs The class to test against.
		///@return bool Test result.
		bool operator!= (const I_Plug &rhs) const;

		bool operator< (const I_Plug &rhs) const;

		///\brief Connects this plug to another plug or shader node.
		///
		///@param pConnection Source connection.
		void SetSourceConnectionPtr(I_Object* pConnection);

		///\brief Get the type of compound object this plug represents.
		///
		///This function is obviously only valid if this is a compound plug.
		///
		///@return The compound plug type.
		E_GenericAttributeType GetCompoundType() const;

		///\brief Get the value of this attribute.
		///
		///@param value Attribute value.
		virtual bool GetValue(U8& value, bool evaluate = true) const = 0;

		///\brief Get the value of this attribute.
		///
		///@param value Attribute value.
		virtual bool GetValue(U16& value, bool evaluate = true) const = 0;

		///\brief Get the value of this attribute.
		///
		///@param value Attribute value.
		virtual bool GetValue(U32& value, bool evaluate = true) const = 0;

		///\brief Get the value of this attribute.
		///
		///@param value Attribute value.
		virtual bool GetValue(U64& value, bool evaluate = true) const = 0;

		///\brief Get the value of this attribute.
		///
		///@param value Attribute value.
		virtual bool GetValue(S8& value, bool evaluate = true) const = 0;

		///\brief Get the value of this attribute.
		///
		///@param value Attribute value.
		virtual bool GetValue(S16& value, bool evaluate = true) const = 0;

		///\brief Get the value of this attribute.
		///
		///@param value Attribute value.
		virtual bool GetValue(S32& value, bool evaluate = true) const = 0;

		///\brief Get the value of this attribute.
		///
		///@param value Attribute value.
		virtual bool GetValue(S64& value, bool evaluate = true) const = 0;

		///\brief Get the value of this attribute.
		///
		///@param value Attribute value.
		virtual bool GetValue(F32& value, bool evaluate = true) const = 0;

		///\brief Get the value of this attribute.
		///
		///@param value Attribute value.
		virtual bool GetValue(F64& value, bool evaluate = true) const = 0;

		///\brief Get the value of this attribute.
		///
		///@param value Attribute value.
		virtual bool GetValue(bool& value, bool evaluate = true) const = 0;

		///\brief Get the value of this attribute.
		///
		///@param value Attribute value.
		virtual bool GetValue(std::string& value, bool evaluate = true) const = 0;

		///\brief Get the value of this attribute.
		///
		///@param value Attribute value.
		virtual bool GetValue(Matrix44& value, bool evaluate = true) const = 0;

		///\brief Get the value of a compound.
		///
		///@param value Reference to variable to be filled with this compounds's value.
		template <class T, int N>
		void GetCompoundValue(UtMaths::BaseVector<T, N>& value) const
		{
			for (U32 i = 0; i < value.SIZE; i++)
				GetChildPtr(i)->GetValue(value[i]);
		}

		///\brief Returns the logical index of this plug in the array of plugs held by the parent node/plug.
		///
		///An index of -1 indicates that this plug is not parented.
		///
		///@return parent relative logical index.
		S32	GetLogicalIndex() const;

		///\brief Returns the source connectivity for this plug.
		///
		///@return Connection source.
		const I_Object*	GetSourceConnectionPtr() const;

		///\brief Returns the source connectivity for this plug.
		///
		///@return Connection source.
		I_Object* GetSourceConnectionPtr();

		///\brief Returns a flag indicating whether this plug is connected to anything.
		///
		///@return Connected flag.
		bool IsConnected() const;

		///\brief Returns a flag indicating whether this plug is a child of a compound.
		///
		///@return Child flag.
		bool IsPlugChild() const;

		I_Object* GetConnectionRoot() const;

		///\brief Return the number of children in this plug.
		///
		///@return The number of children.
		virtual U32	GetNumChildren() const = 0;

		///\brief Returns the parent object for this plug.
		///
		///Plugs can be attached to materials, shaders or other plugs.
		///
		///@return Parental object pointer.
		I_Object* GetParentPtr();

		///\brief Returns the parent object for this plug.
		///
		///Plugs can be attached to materials, shaders or other plugs.
		///
		///@return Parental object pointer.
		const I_Object*	GetParentPtr() const;

		///\brief Returns the nth child of this plug
		///
		///This function is only valid if this is a compound plug.
		///
		///@param instance The index of the child to fetch.
		///@return Parental plug pointer.
		virtual I_Plug* GetChildPtr(U32 instance)
		{
			//Lets do a const cast to save typing.
			return const_cast<I_Plug*>(((const I_Plug*)this)->GetChildPtr(instance));
		}

		///\brief Returns the nth child of this plug
		///
		///This function is only valid if this is a compound plug.
		///
		///@param instance The index of the child to fetch.
		///@return Parental plug pointer.
		virtual const I_Plug* GetChildPtr(U32 instance) const = 0;
		
		///\brief Returns a flag indicating whether this plug is a compound.
		///
		///@return Compound flag.
		bool IsCompound() const;

		///\brief Get the name of this attribute.
		///
		///@return Attribute name;
		virtual const std::string& GetName() const = 0;

		///\brief Returns true if this object implements the given type.
		///
		///@param The object type to test against.
		///@return true if we implement the given object type.
		virtual bool HasType(E_ObjType type) const;

		void Clear();

		///Determines whether this plug is valid. A plug is only valid if it refers to an attribute.
		///
		///@return bool I_Plug validity
		bool IsNull() const;

		///\brief Get the value type of this attribute.
		///
		///This might be, U32, bool etc.
		virtual E_GenericAttributeType GetDataType() const = 0;

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
		virtual E_ObjType GetType() const = 0;

		std::string GetPath() const;

		virtual I_Plug* Clone() const = 0;

		///\brief Displays the contents of the given plug object.
		///
		///Contents are displayed via the output manager. A callback to this can be registered in by a
		///call to 'SetOutputNotificationCallback'.
		///@param plug The plug to write.
		static void Output(const I_Plug& plug);

		const C_Node* GetTopLevelParentPtr() const;

		C_Node* GetTopLevelParentPtr()
		{
			//Lets do a const cast to save typing.
			return const_cast<C_Node*>(((const I_Plug*)this)->GetTopLevelParentPtr());
		}

		void GetLocationInfo(C_PlugLocationInfo& locationInfo) const;

		void GetLocationInfo(C_PlugLocationInfo& locationInfo)
		{
			//Lets do a const cast to save typing.
			((const I_Plug*)this)->GetLocationInfo(locationInfo);
		}

		///\brief Equivilance operator for this class only.
		///
		///@param rhs The node for comparison.
		///@return returns 'true' if this node and aftkNode are equivilant. Otherwise 'false' is returned.
		bool IsEqualToThis(const I_Object& rhs, U32 flags = 0) const;

		void OnChanged()
		{
			AFTK_ASSERT(m_pParent);

			if (m_pParent->HasType(E_ObjType_IPlug))
			{
				//Inform our parent that something's changed.
				((I_Plug*)m_pParent)->OnChanged();
			}
			if (m_pOnChangedCallback)
				m_pOnChangedCallback(this);

			//Re-evaluate all target plugs.
			for (U32 i = 0; i < (U32)m_connectedPlugs.Size(); i++)
				m_connectedPlugs[i]->OnChanged();
		}

		void SetLogicalIndex(S32 instance);

		void SetParentPtr(I_Object* pParent);

	#ifndef DOXYGEN_SHOULD_SKIP_THIS
	protected:
		I_Plug(void(*OnChangedCallback)(void* pThis));

		void OneTimeInit(void(*pOnChangedCallback)(void* pThis));

		void ClearThis();

		bool IsEqualToAll(const I_Object& rhs, E_ObjType classToAbortTest = E_ObjType_Unknown, U32 flags = 0) const;

		void DisconnectFromObject();

		static void GetLocationIndexList(
			C_PlugLogicalIndexList& locationIndexList,
			const I_Object* pObject);

		template<class OUTPUT_TYPE>
		bool EvaluateSourceConnection(OUTPUT_TYPE& output) const
		{
			if (!m_pSourceConnection)
			{
				AFTK_ASSERT(m_pSourceConnection);
				return false;
			}

			if (m_pSourceConnection->HasType(E_ObjType_IEvaluatable))
				return ((const I_EvaluatableNode*)m_pSourceConnection)->Evaluate(output);
			else if (m_pSourceConnection->HasType(E_ObjType_IPlug))
				return ((const I_Plug*)m_pSourceConnection)->GetValue(output, true);

			return false;
		}

		template<class OUTPUT_TYPE>
		bool EvaluateConnectivity(OUTPUT_TYPE& value, bool& evalResult) const
		{
			//std::cout << "ESC " << this->GetPath() << std::endl;
			//If our parent is a plug with a connection then this drives our value.
			if (m_pParent->HasType(E_ObjType_IPlug) && ((I_Plug*)m_pParent)->m_pSourceConnection)
			{
				I_Plug* pParent = (I_Plug*)m_pParent;

				//Only plugs can output vector values.
				if (pParent->m_pSourceConnection->HasType(E_ObjType_IPlug))
				{
					I_Plug* pParentConnection = (I_Plug*)pParent->m_pSourceConnection;
					AFTK_ASSERT(m_logicalIndex >= 0);
					if (m_logicalIndex < (S32)pParentConnection->GetNumChildren())
					{
						//Evaluate from our parent's connection's child at our logical index.
						evalResult = pParentConnection->GetChildPtr(m_logicalIndex)->GetValue(value, true);
						//Connectivity evaluated.
						return true;
					}
				}
			}
			else if (m_pSourceConnection)
			{
				evalResult = EvaluateSourceConnection(value);
				//Connectivity evaluated.
				return true;
			}

			//Return false if no connectivity to evaluate.
			evalResult = true;
			return false;
		}

		//Our parent shader/material
		I_Object* m_pParent;

		//The plug that we are the destination of
		I_Object* m_pSourceConnection;

		S32	m_logicalIndex;

		//Callback function that's triggered whenever the plug value or connection is altered.
		void(*m_pOnChangedCallback)(void* pThis);

		friend class I_Object;
		friend class C_NativeIPlug;
		friend class C_DynamicPlug;
		friend class C_NullPlug;
		friend class C_ShaderImageNode;
		friend class C_ShaderNode;
		friend class C_MaterialNode;
		friend class C_NativePlug;
		friend class C_NativeFtk;
		friend class C_NativeDynamicPlug;
		friend class C_SceneNode;
	#endif
	};
}

#endif
