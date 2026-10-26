#ifndef EDITOR_RENDER_SCENE_H
#define EDITOR_RENDER_SCENE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "core/utility/Singleton.h"
#include "sigslot/SigSlot.h"

namespace Editor
{
	class C_DgNodeSelection;
	class C_ActorScene;
	class C_AftkRefScene;

	struct C_RenderRegPolicy
	{
		template<typename DERIVED>
		static bool Register(C_RegInfo* pRegInfo) 
		{ 
			DERIVED::SimpleRendererRegister();
			return true; 
		}
	};

	typedef Ut::Vector<EditorLib::I_DgNode*>	DgNodeArray;
	typedef C_Factory<AftkRender::C_SceneGraphNode, C_RegInfo, C_RenderRegPolicy>	C_DgNodeFactory;


	class C_RenderScene
	{
	public:

		C_RenderScene();
		virtual ~C_RenderScene();

		C_DgNodeFactory& GetDgNodeFactory();

		AftkRender::C_SceneGraphNode*	GetSceneGraphNode(const EditorLib::I_DgNode* pNode) const;

		C_ActorScene&		GetActorScene();
		const C_ActorScene&	GetActorScene() const;

		const Ut::Vector<C_AftkRefScene*>& GetAftkRefScenes() const;
		
		const C_DgNodeSelection&		GetSelection() const;
		const Maths::C_AxisBox&			GetSelectionBounds() const;
		const Maths::C_AxisBox&			GetWorldBounds() const;

		bool IsVisible(const EditorLib::I_DgNode* pNode);

		void	AttachToRenderers(AftkRender::E_RendererPass pass);

		void	SetNodeVisibility(bool visible);
		void	SetSelectionVisibility(bool visible);
		void	SetInverseSelectionVisibility(bool visible);
		void	ShowPreviouslyHiddenNodes();
		
		//needed for selection!
		void	HideUnselectableNodes();	//hides nodes that are unselectable
		void	ShowUnselectableNodes();	//shows hidden nodes that are unselectable 

	private:

		struct C_Flags
		{
			bool pendingSync	: 1;
			bool viewEnabled	: 1;
			bool viewDirty		: 1;
		};

		class C_Slots : public sigslot::has_slots<>
		{
		public:
			C_Slots(C_RenderScene* pRenderScene);
			void Connect();
			void Disconnect();

			void OnLevelOpening(const EditorLib::I_Level& level);
			void OnLevelOpened(const EditorLib::I_Level& level);
			void OnLevelClosing(const EditorLib::I_Level& level);
			void OnLevelClosed(const EditorLib::I_Level& level);

			void OnSelectionChangedSlot();

			void OnDgNodeDestroySlot(const EditorLib::I_DgNode& dgNode);
			void OnDgNodeUpdatedSlot(const EditorLib::I_DgNode& dgNode, const EditorLib::C_UpdateHintCollection* pHc);

			void OnAftkRefLoadedSlot(
				const Btl::String
				& originalFilePath, 
				const Aftk::C_Scene& aftkScene,
				const EditorLib::C_SceneNode& sceneNode,
				const EditorLib::AftkDgMap& aftkDgMap);
			void OnAftkRefUnloadedSlot(const Btl::String& originalFilePath);

			void OnCoreEnableViews(bool enable);
			void OnCoreDirtyViews(EditorLib::E_DirtyEventType e);

		private:
			C_RenderScene*	m_pRenderScene;
		};

		void OnLevelOpening(const EditorLib::I_Level& level);
		void OnLevelOpened(const EditorLib::I_Level& level);
		void OnLevelClosing(const EditorLib::I_Level& level);
		void OnLevelClosed(const EditorLib::I_Level& level);

		void OnSelectionChanged();

		void OnDgNodeDestroy(const EditorLib::I_DgNode& dgNode);
		void OnDgNodeUpdated(const EditorLib::I_DgNode& dgNode);
		void OnDgNodeResolved(const EditorLib::I_DgNode& dgNode);

		void OnAftkRefLoaded(const Btl::String& originalFilePath, const Aftk::C_Scene& aftkScene, const EditorLib::C_SceneNode& sceneNode, const EditorLib::AftkDgMap& aftkDgMap);
		void OnAftkRefUnloaded(const Btl::String& originalFilePath);

		void OnCoreEnableViews(bool enable);
		void OnCoreDirtyViews(EditorLib::E_DirtyEventType e);

		void UpdateLayerVisibility(U32 layerMask, U32 nodeMask);

		C_Slots							m_slots;
		C_Flags							m_flags;

		C_DgNodeFactory*				m_pDgNodeFactory;

		C_ActorScene*					m_pActorScene;
		Ut::Vector<C_AftkRefScene*>		m_aftkRefScenes;

		C_DgNodeSelection*				m_pSelection;
		DgNodeArray						m_hiddenNodes;

		mutable Maths::C_AxisBox		m_selectionBounds;
		mutable Maths::C_AxisBox		m_worldBounds;
	};
}

#endif