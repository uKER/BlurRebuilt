#ifndef EDITOR_LIB_SCENE_NODE_H
#define EDITOR_LIB_SCENE_NODE_H

#ifdef _MSC_VER_
#pragma once
#endif

#include <EditorLib/Dg/Attr/IPlug.h>
#include <EditorLib/Dg/Attr/NullPlug.h>
#include <EditorLib/Dg/Attr/StaticPlug.h>
#include <EditorLib/Dg/IDgObject.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/DgNodeBase.h>

namespace EditorLib
{
	class C_SceneNode : public C_DgNodeBase
	{
	//From I_DgObject
	public:
		virtual const char*		GetTypeName() const;

		virtual E_ObjType		GetType() const;

		virtual bool			IsA(E_ObjType type) const;

	//From I_DgNode
	public:
		virtual void			GetPlugs(Btl::Vector<I_Plug*>& plugs);

		virtual bool			Update(C_UpdateHintCollection* pHc = 0);

		virtual bool			DirtyDependents(bool setThisDirty = true);

		virtual bool			IsNonXtDirty() const;

		virtual void			ResetNonXtDirty();

	//From C_SceneNode
	public:
		virtual void AddChild(C_SceneNode* pChild);	

		virtual C_SceneNode*			GetParent();

		virtual const C_SceneNode*		GetParent() const;

		virtual C_SceneNode*			GetChild();

		virtual const C_SceneNode*		GetChild() const;

		virtual C_SceneNode*			GetPreviousSibling();

		virtual const C_SceneNode*		GetPreviousSibling() const;

		virtual C_SceneNode*			GetNextSibling();

		virtual const C_SceneNode*		GetNextSibling() const;

		virtual C_NullPlug&				GetTranslatePlug();

		virtual C_NullPlug&				GetRotatePlug();
		
		virtual C_NullPlug&				GetScalePlug();
		
		virtual C_NullPlug&				GetShearPlug();

		virtual C_StaticPlug<MathsStorage::Matrix44>&		
										GetWorldMatrixPlug();

		virtual C_StaticPlug<MathsStorage::Matrix44>&		
										GetLocalMatrixPlug();

		virtual const C_NullPlug&		GetTranslatePlug() const;

		virtual const C_NullPlug&		GetRotatePlug() const;
		
		virtual const C_NullPlug&		GetScalePlug() const;
		
		virtual const  C_NullPlug&		GetShearPlug() const;

		virtual const C_StaticPlug<MathsStorage::Matrix44>&		
										GetLocalMatrixPlug() const;

		virtual const C_StaticPlug<MathsStorage::Matrix44>&		
										GetWorldMatrixPlug() const;

		virtual void					SetLocalMatrix(const Matrix44& matrix, bool compensateChildren = false);

		virtual void					SetWorldMatrix(const Matrix44& matrix, bool compensateChildren = false);

		virtual const Matrix44&			GetLocalMatrix() const;

		virtual const Matrix44&			GetWorldMatrix() const;

	//From C_SceneNode
	public:
		struct SceneNodeFlags
		{
			bool canReparent : 1;
			bool canAddChildren : 1;
		};

		union SceneNodeDirtyFlags
		{
			struct
			{
				mutable bool transformViaImmediateDirty : 1;
				mutable bool parentViaImmediateDirty : 1;
			};
			U32 data;
		};

		static const char*		GetStaticTypeName();

		static E_ObjType		GetStaticType();

		static void*			Create();

		SceneNodeFlags&			GetSceneNodeFlags();

		const SceneNodeFlags&	GetSceneNodeFlags() const;

		virtual void			PostInitialiseImmediatePropagate();

	//From C_SceneNode
	protected:
		enum E_StaticPlug
		{
			E_StaticPlug_Translate,
			E_StaticPlug_Rotate,
			E_StaticPlug_Scale,
			E_StaticPlug_Shear,
			E_StaticPlug_LocalMatrix,
			E_StaticPlug_WorldMatrix,

			E_StaticPlug_Count
		};

		C_SceneNode();
		virtual ~C_SceneNode();

		virtual bool VetoParentChange(const C_SceneNode* pCandidateParent) const;

		virtual void OnStoreTransform() {}

		virtual void OnStoreParent() {}

		virtual void ImmediatePropagate(const C_ImmediateHintCollection& hc, bool dirtyDependents = true);

		void OnTransformPlugsChanged();

		I_Plug* m_staticPlugs[E_StaticPlug_Count];

		C_SceneNode*				m_pParent;
		C_SceneNode*				m_pChild;
		C_SceneNode*				m_pPrevious;
		C_SceneNode*				m_pNext;

		SceneNodeFlags				m_sceneNodeFlags;
		SceneNodeDirtyFlags			m_sceneNodeDirtyFlags;

	private:
		static void OnSceneNodePlugChanged(I_Plug* pPlug);

		FRIEND_CLASS C_SceneNodeHelpers;
	};
}

#endif
