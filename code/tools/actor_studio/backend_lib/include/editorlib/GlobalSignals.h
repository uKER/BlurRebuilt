#ifndef EDITORLIB_GLOBAL_SIGNALS_H
#define EDITORLIB_GLOBAL_SIGNALS_H

#ifndef _MSC_VER
#pragma once
#endif

#include <sigslot/sigslot.h>
		  
namespace EditorLib
{
	enum E_DatabaseEvent
	{
		E_DatabaseEvent_Opened,
		E_DatabaseEvent_Closing,
	};

	enum E_PackEvent
	{
		E_PackEvent_Opening,
		E_PackEvent_Opened,
		E_PackEvent_Closing,
		E_PackEvent_Closed,
	};

	enum E_LevelEvent
	{
		E_LevelEvent_ActiveLevelChanging,
		E_LevelEvent_ActiveLevelChanged,
		E_LevelEvent_ReadOnlyFlagChanged,
	};

	enum E_LayerEvent
	{
		E_LayerEvent_Added,
		E_LayerEvent_Removed,
		E_LayerEvent_Deserialised,
	};

	enum E_DgNodeTransform
	{
		E_DgNodeTransform_Rotate,
		E_DgNodeTransform_EndRotate,
		E_DgNodeTransform_Move,
		E_DgNodeTransform_EndMove,
	};

	struct C_GlobalSignals
	{
		////// Signals from C_Core //////
		sigslot::signal0<> quit;

		////// Signals from C_AftkRefManager //////
		sigslot::signal4<
			const Btl::String&, 
			const Aftk::C_Scene&, 
			const C_SceneNode&,
			const AftkDgMap&>								aftkRefLoaded;
		sigslot::signal1<const Btl::String&>				aftkRefUnloaded;

		sigslot::signal2<Xt::DomDb&, E_DatabaseEvent>		databaseEvent;

		sigslot::signal2<const C_LevelPack&, E_PackEvent>	packEvent;
		
		////// Signals from I_Level //////
		sigslot::signal2<const I_Level*, E_LevelEvent>		levelEvent;
		sigslot::signal1<const I_Level&>					levelOpening;
		sigslot::signal1<const I_Level&>					levelOpened;
		sigslot::signal1<const I_Level&>					levelClosing;
		sigslot::signal1<const I_Level&>					levelClosed;

		sigslot::signal1<const Btl::String&>				levelAftkRefAdded;
		sigslot::signal1<const Btl::String&>				levelAftkRefRemoved;
		sigslot::signal1<const I_Level&>					levelNameChanged;

		//signals from C_Layer
		sigslot::signal2<const I_Layer&, E_LayerEvent>		layerEvent;
		sigslot::signal2<const I_Layer&, const I_DgNode&>	layerNodeAdded;
		sigslot::signal2<const I_Layer&, const I_DgNode&>	layerNodeRemoved;

		////// Signals from C_DgManager //////
		sigslot::signal1<const I_DgNode&>					dgNodeCreated;
		sigslot::signal1<const I_DgNode&>					dgNodeDestroy;
		
		////// Signals from C_Core //////
		sigslot::signal0<>									coreSelectionChanged;
		sigslot::signal2<const Btl::String&, const void*>	coreContextChanged;
		sigslot::signal1<bool>								coreEnableViews;
		sigslot::signal1<E_DirtyEventType>					coreDirtyViews;

		////// Signals from Dg node and its derived classes. //////
		sigslot::signal2<const I_DgNode&, const C_UpdateHintCollection*>	dgNodeUpdated;

		////// Signals from manipulator contexts for GameMessaging. //////
		sigslot::signal2<const I_DgNode&, E_DgNodeTransform> dgNodeTransform;

	private:
		//Prevent construction outside of C_Core.
		C_GlobalSignals();
		~C_GlobalSignals();

		class C_Slots : public sigslot::has_slots<>
		{
		public:
			C_Slots(C_GlobalSignals* pGlobalSignals);

			void Connect();
			void Disconnect();

			////// Signals from C_Core //////
			void QuitSlot();

			////// Signals from C_AftkRefManager //////
			void AftkRefLoadedSlot(
				const Btl::String& originalFilePath, 
				const Aftk::C_Scene& aftkScene, 
				const EditorLib::C_SceneNode& sceneNode, 
				const EditorLib::AftkDgMap& aftkDgMap);
			void AftkRefUnloadedSlot(const Btl::String& originalFilePath);

			void DatabaseEventSlot(Xt::DomDb& db, E_DatabaseEvent evt);

			////// Signals from C_XtLevel //////
			void LevelOpeningSlot(const I_Level& level);
			void LevelOpenedSlot(const I_Level& level);
			void LevelClosingSlot(const I_Level& level);
			void LevelClosedSlot(const I_Level& level);

			void LevelAftkRefAddedSlot(const Btl::String& originalFilePath);
			void LevelAftkRefRemovedSlot(const Btl::String& originalFilePath);
			void LevelNameChangedSlot(const I_Level& level);

			//signals from C_Layer
			void LayerEventSlot(const I_Layer& layer, E_LayerEvent e);

			void LayerNodeAddedSlot(const I_Layer& layer, const I_DgNode& dgNode);
			void LayerNodeRemovedSlot(const I_Layer& layer, const I_DgNode& dgNode);

			////// Signals from C_DgManager //////
			void DgNodeCreatedSlot(const I_DgNode& dgNode);
			void DgNodeDestroySlot(const I_DgNode& dgNode);
			
			////// Signals from C_Core //////
			void CoreSelectionChangedSlot();
			void CoreContextChangedSlot(const Btl::String& context, const void* contextData);
			void CoreEnableViewsSlot(bool enable);
			void CoreDirtyViewsSlot(E_DirtyEventType eventType);

			////// Signals from Dg node and its derived classes. //////
			void DgNodeUpdatedSlot(const I_DgNode& dgNode, const C_UpdateHintCollection* pHints);

		private:
			HANDLE				m_mainThreadId;
			C_GlobalSignals*	m_pGlobalSignals;
		};

		C_Slots m_slots;
		FRIEND_CLASS C_Core;
	};
}

#endif
