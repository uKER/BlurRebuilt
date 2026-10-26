#ifndef EDITOR_LIB_PLUG_BASE_H
#define EDITOR_LIB_PLUG_BASE_H

#ifdef _MSC_VER_
#pragma once
#endif

//Only used to make flyweight declaration less painful.
#include <string>
//This sadly drags in a fair few things. Ideally move to our own flyweight impl.
#include <boost/flyweight.hpp>

#include <Btl/containers/vector.h>
#include <Btl/Utility/string.h>

#include <EditorLib/Dg/Attr/IPlug.h>


namespace EditorLib
{
	enum E_CompoundType
	{
		E_CompoundType_Vector4U32,
		E_CompoundType_Vector4U64,
		E_CompoundType_Vector4S32,
		E_CompoundType_Vector4S64,
		E_CompoundType_Vector4F32,
		E_CompoundType_Vector4F64,

		E_CompoundType_Vector3U32,
		E_CompoundType_Vector3U64,
		E_CompoundType_Vector3S32,
		E_CompoundType_Vector3S64,
		E_CompoundType_Vector3F32,
		E_CompoundType_Vector3F64,

		E_CompoundType_Vector2U32,
		E_CompoundType_Vector2U64,
		E_CompoundType_Vector2S32,
		E_CompoundType_Vector2S64,
		E_CompoundType_Vector2F32,
		E_CompoundType_Vector2F64,
		E_CompoundType_Unknown
	};

	class C_NamedPlugPolicy
	{
	public:
		void SetName(const char* name)
		{
			m_name = std::string(name);
		}

	protected:
		C_NamedPlugPolicy(const char* name)
		{
			m_name = std::string(name);
		}

		const char* _GetName() const
		{
			return m_name.get().c_str();
		}

		boost::flyweights::flyweight<std::string> m_name;
	};

	class C_UnnamedPlugPolicy
	{
	protected:
		C_UnnamedPlugPolicy(const char* name)
		{
		}

		const char* _GetName() const;
	};

	class C_ConnectablePolicy
	{
	protected:
		///\brief Connects this plug to another plug or shader node.
		///
		///@param pConnection Source connection.
		void _SetSourceConnection(I_Plug* pSource, I_Plug* pDestination, bool triggerTopLevelParentOnChanged = true)
		{
			if (pSource == pDestination)
			{
				BIZ_ASSERT(!"Plug source and destination cannot be the same!");
				return;
			}

			m_sourceConnection.Disconnect(pDestination);

			if (pSource)
				m_sourceConnection.Connect(pSource, pDestination);

			pDestination->InvokeDependentOnChanged(triggerTopLevelParentOnChanged);
		}

		///\brief Returns the source connectivity for this plug.
		///
		///@return Connection source.
		const I_Plug* _GetSourceConnection() const
		{
			return m_sourceConnection.GetSource();
		}

		///\brief Returns the source connectivity for this plug.
		///
		///@return Connection source.
		I_Plug* _GetSourceConnection()
		{
			return m_sourceConnection.GetSource();
		}

		C_PlugConnection m_sourceConnection;
	};

	class C_NonConnectablePolicy
	{
	protected:
		void _SetSourceConnection(I_Plug* pSource, I_Plug* pDestination, bool triggerTopLevelParentOnChanged = true)
		{
			if (pSource)
				BIZ_ASSERT(!"Invalid Connection Attempt!");
		}

		///\brief Returns the source connectivity for this plug.
		///
		///@return Connection source.
		const I_Plug* _GetSourceConnection() const
		{
			return 0;
		}

		///\brief Returns the source connectivity for this plug.
		///
		///@return Connection source.
		I_Plug* _GetSourceConnection()
		{
			return 0;
		}
	};

	class C_ChildPolicy
	{
	public:
		~C_ChildPolicy()
		{
			for (U32 i = 0; i < (U32)m_children.Size(); i++)
				SAFE_DELETE_PTR(m_children[i]);
		}

		INLINE void DeleteChildren()
		{
			for (U32 i = 0; i < (U32)m_children.Size(); i++)
				SAFE_DELETE_PTR(m_children[i]);
			m_children.Clear();
		}

	protected:
		///\brief Return the number of children in this plug.
		///
		///@return The number of children.
		INLINE U32	_GetNumChildren() const
		{
			return m_children.Size();
		}

		///\brief Returns the nth child of this plug
		///
		///This function is only valid if this is a compound plug.
		///
		///@param instance The index of the child to fetch.
		///@return Parental plug pointer.
		INLINE I_Plug* _GetChild(U32 idx)
		{
			return m_children[idx];
		}

		///\brief Returns the nth child of this plug
		///
		///This function is only valid if this is a compound plug.
		///
		///@param instance The index of the child to fetch.
		///@return Parental plug pointer.
		INLINE const I_Plug* _GetChild(U32 idx) const
		{
			return m_children[idx];
		}

		INLINE bool _AddChild(I_Plug* pChild)
		{
			m_children.PushBack(pChild);
			return true;
		}

		Btl::Vector<I_Plug*>	m_children;
	};

	class C_NoChildPolicy
	{
	protected:
		C_NoChildPolicy(I_Plug** ppChildren, U32 nChildren)
		{
		}

		///\brief Return the number of children in this plug.
		///
		///@return The number of children.
		INLINE U32	_GetNumChildren() const
		{
			return 0;
		}

		///\brief Returns the nth child of this plug
		///
		///This function is only valid if this is a compound plug.
		///
		///@param instance The index of the child to fetch.
		///@return Parental plug pointer.
		INLINE I_Plug* _GetChild(U32 idx)
		{
			return 0;
		}

		///\brief Returns the nth child of this plug
		///
		///This function is only valid if this is a compound plug.
		///
		///@param instance The index of the child to fetch.
		///@return Parental plug pointer.
		INLINE const I_Plug* _GetChild(U32 idx) const
		{
			return 0;
		}

		INLINE bool _AddChild(I_Plug* pChild)
		{
			return false;
		}

	protected:
		Btl::Vector<I_Plug*>	m_children;
	};

	template<typename NAME_POLICY, typename CHILD_POLICY, typename CONNECTION_POLICY>
	class C_PlugBase : public I_Plug, public NAME_POLICY, public CHILD_POLICY, public CONNECTION_POLICY
	{
	//From I_Plug
	public:
		///\brief Returns the source connectivity for this plug.
		///
		///@return Connection source.
		virtual I_Plug* GetSourceConnection();

		///\brief Returns the source connectivity for this plug.
		///
		///@return Connection source.
		virtual const I_Plug* GetSourceConnection() const;

		///\brief Return the number of children in this plug.
		///
		///@return The number of children.
		virtual U32	GetNumChildren() const;

		virtual I_Plug* GetParentPlug();

		virtual I_DgNode* GetParentNode();

		virtual const I_Plug* GetParentPlug() const;

		virtual const I_DgNode* GetParentNode() const;

		///\brief Returns the nth child of this plug
		///
		///This function is only valid if this is a compound plug.
		///
		///@param instance The index of the child to fetch.
		///@return Parental plug pointer.
		virtual I_Plug* GetChild(U32 idx);

		///\brief Returns the nth child of this plug
		///
		///This function is only valid if this is a compound plug.
		///
		///@param instance The index of the child to fetch.
		///@return Parental plug pointer.
		virtual const I_Plug* GetChild(U32 idx) const;

		virtual bool AddChild(I_Plug* pChild);

		virtual void SetSourceConnection(I_Plug* pSource, bool triggerTopLevelParentOnChanged = true);

		virtual C_ConnectedPlugs& GetConnections();

		virtual const C_ConnectedPlugs& GetConnections() const;

		virtual void InvokeDependentOnChanged(bool triggerTopLevelParentOnChanged);

		virtual bool IsNodeDirty() const;

		virtual bool IsTopLevel() const;

		virtual void ResetDirty();

	//From C_PlugBase
	public:
		typedef void(*OnInputPlugChangedCallback)(I_Plug* pPlug);

		void SetOnInputPlugChangedCallback(OnInputPlugChangedCallback pFunc);

		void Clear();

		virtual ~C_PlugBase();

	//From C_PlugBase
	protected:
		C_PlugBase(
			I_DgNode* pParent,
			void(*pOnInputPlugChangedCallback)(I_Plug* pThis),
			E_PlugType type,
			const char* name);
		C_PlugBase(
			I_Plug* pParent,
			void(*pOnInputPlugChangedCallback)(I_Plug* pThis),
			E_PlugType type,
			const char* name);

		OnInputPlugChangedCallback		m_pOnInputPlugChangedCallback;

		union Flags
		{
			struct
			{
				E_PlugType type : 2;
				bool isDirtyOutput : 1;
				bool isTopLevelPlug : 1;
			};
			U8 data;
		};

		union Parent
		{
			I_DgNode*	pDgNode;
			I_Plug*		pPlug;
		};
		
		Parent			m_parent;
		Flags			m_flags;

		C_ConnectedPlugs	m_connections;
	};

	#include "PlugBase.inl"
}

#endif
