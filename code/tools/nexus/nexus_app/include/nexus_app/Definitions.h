#ifndef NEXUS_DEFINITIONS_H
#define NEXUS_DEFINITIONS_H

namespace Nexus
{
	enum E_MouseButton
	{
		E_MouseButton_Left,
		E_MouseButton_Right,
		E_MouseButton_Middle,

		E_MouseButton_Count
	};

	enum E_MouseButtonAction
	{
		E_MouseButtonAction_Down,
		E_MouseButtonAction_Up,
		E_MouseButtonAction_DoubleClick,
	};

	enum E_ViewportState
	{
		E_ViewportState_NoAction,
		E_ViewportState_ViewTranslate,
		E_ViewportState_ViewRotate,
		E_ViewportState_ViewDolly
	};

	enum E_SelectionState
	{
		E_SelectionState_Single,
		E_SelectionState_Multi
	};
}

#define NEXUS_VIEW_CHECK_ERROR	if (!status) { Nexus::g_pMainFrame->OnError(status); return; }; 
#define NEXUS_DISABLE_IF_NOT_LOADED if (!Nexus::g_pViewportParent->GetDisplayManagerPtr()->GetRendererPtr())	{pCmdUI->Enable(0);return;}	if (!Nexus::g_pViewportParent->GetDisplayManagerPtr()->GetRendererPtr()->IsBuilt()){pCmdUI->Enable(0);return;} pCmdUI->Enable(1);

#endif
