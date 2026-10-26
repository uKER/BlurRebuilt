#include <nexus_app/stdafx.h>

#include "../../resource.h"
#include <nexus_app/NexusProgressDialog.h>

//Nexus includes
#include <nexus_app/NexusSystem.h>
#include <nexus_app/DisplayManager.h>

#include "..\..\include\nexus_app\mainfrm.h"
#include <nexus_app/NexusDoc.h>
#include <nexus_app/ViewportParent.h>
#include <mfc_extras/ChatRichEd.h>
#include <nexus_app/CommandEdit.h>
#include <nexus_app/CommandView.h>
#include <nexus_app/DockingPanelBottom.h>

//Disable warning C4005: 'M_PI' : macro redefinition
#ifdef NEXUS_MAYA
#pragma warning(push)
	#pragma warning(disable : 4005)
	#include <maya/MLibrary.h>
	#include <maya/MString.h>
#pragma warning(pop)
#endif

using namespace Nexus;

NexusSystem::NexusSystem()
{
	m_isCreated = false;
	m_sceneLoaded = false;
	m_pLogCallbackFunc = 0;
	g_pNexusSystem = this;
	memset(&m_keyboardState, 0, sizeof(m_keyboardState));
	memset(&m_mouseButtonState, 0, sizeof(m_mouseButtonState));
	memset(&m_timerFrequency, 0, sizeof(LARGE_INTEGER));
	m_activeViewportIndex = 0;

	Clear();
}

NexusSystem::~NexusSystem()
{
}

Tu::Status NexusSystem::Create()
{
	if (m_isCreated) 
		return Tu::Status::SUCCESS;

	Tu::Status status;
	if (!QueryPerformanceFrequency(&m_timerFrequency))
		return Tu::Status(Tu::Status::FAILURE, "Could not query performance counter.");

	status = m_nexusCore.Create();
	if (!status) return status;

	m_pProgressDialog = new NexusProgressDialog;
	m_nexusCore.SetProgressCallback((Nexus::Progress*)m_pProgressDialog);
	m_nexusCore.SetPrintMessageCallback(
		&g_pMainFrame->GetDockingPanelBottomPtr()->GetCommandViewPtr()->PrintStatic);
	m_nexusCore.SetPrintWarningCallback(
		&g_pMainFrame->GetDockingPanelBottomPtr()->GetCommandViewPtr()->PrintStatic);
	m_nexusCore.SetPrintErrorCallback(
		&g_pMainFrame->GetDockingPanelBottomPtr()->GetCommandViewPtr()->PrintStatic);
	m_nexusCore.SetSelectCallback(
		&OnSelect);

	m_isCreated = true;
	return Tu::Status::SUCCESS;
}

void NexusSystem::OnSelect(E_SelectionMode mode, const C_SelectedNodeInfo& selection)
{
	if (mode == E_SelectionMode_Face && g_pNexusSystem->GetState().showFaceInfoOnSelection)
	{
		AftkTasks::PrintSelectedFaceInfo(selection);
	}
	else if (mode == E_SelectionMode_Object && g_pNexusSystem->GetState().showObjectInfoOnSelection)
	{
		AftkTasks::PrintSelectedNodeInfo(selection);
	}
}

void NexusSystem::Clear()
{
	m_nexusCore.Clear();
	m_sceneLoaded = false;
	m_startTime.QuadPart = 0;
	m_animationPaused = true;
}

void NexusSystem::Destroy()
{
	Clear();

	m_nexusCore.Destroy();
	m_isCreated = false;
	delete m_pProgressDialog;
	m_pProgressDialog = 0;
}

bool NexusSystem::IsCreated() const
{
	return m_isCreated;
}

void NexusSystem::CleanUpAfterFailure()
{
}

const Core* NexusSystem::GetCorePtr() const
{
	return &m_nexusCore;
}

Core* NexusSystem::GetCorePtr()
{
	return &m_nexusCore;
}

void NexusSystem::NewScene()
{
	//Create renderer if needed.
	if (g_pViewportParent && 
		g_pViewportParent->GetDisplayManagerPtr() &&
		!g_pViewportParent->GetDisplayManagerPtr()->IsCreated())
	{
		Tu::Status status = g_pViewportParent->GetDisplayManagerPtr()->Create(
			g_pViewportParent->GetViewportPtr(0));
		if (!status)
		{
			g_pMainFrame->OnCriticalError(status);
			return;
		}
	}
	else
	{
		if (g_pViewportParent && g_pViewportParent->GetDisplayManagerPtr())
			g_pViewportParent->GetDisplayManagerPtr()->Clear();	
	}
	
	AftkTasks::NewScene();
}

Tu::Status NexusSystem::OpenScene(const std::string& fileName, bool import, bool& cancelled)
{
	Tu::Status status;

	if (g_pViewportParent && g_pViewportParent->GetDisplayManagerPtr())
		g_pViewportParent->GetDisplayManagerPtr()->Clear();

	{
		Nexus::StringArray arg;
		arg.PushBack(fileName);
		//Removed unused uvs if from xsi.
		arg.PushBack(std::string("cleanuvs=false"));
		if (import)
		{
			status = m_nexusCore.ProgressCallback()->ExecuteTask(AftkTasks::ImportScene, arg);
		}
		else
		{
			NewScene();
			status = m_nexusCore.ProgressCallback()->ExecuteTask(AftkTasks::OpenScene, arg);
		}
		cancelled = m_nexusCore.ProgressCallback()->Cancelled();
		if (!status) return status;
	}

	cancelled = m_nexusCore.ProgressCallback()->Cancelled();
	if (cancelled) 
		return Tu::Status::SUCCESS;
	else	
		if (!status) return status;

	//Fit scene within viewport.
	g_pViewportParent->GetDisplayManagerPtr()->FrameAll();

	const Aftk::C_Scene* pAftkScene = m_nexusCore.GetAftkScenePtr();

	AftkTasks::PrintSceneBounds();
	
	cancelled = m_nexusCore.ProgressCallback()->Cancelled();

	SetTime( GetCorePtr()->GetAftkScenePtr()->GetStartTime() );

	return Tu::Status::SUCCESS;
}

void NexusSystem::UpdateSceneNodeSelection()
{
	if (!g_pViewportParent->GetDisplayManagerPtr()->GetRendererPtr())
		return;

	g_pViewportParent->GetDisplayManagerPtr()->GetRendererPtr()->SetSelected(
		(NexusRender::E_SelectionMode)m_state.selectionMode, 
		m_nexusCore.GetSelected());
}

Tu::Status NexusSystem::SaveScene(const std::string& fileName, bool& cancelled)
{
	Nexus::StringArray arg;
	arg.PushBack(fileName);
	Tu::Status status;
	{
		Nexus::StringArray arg;
		status = m_nexusCore.ProgressCallback()->ExecuteTask(AftkTasks::ValidateScene, arg);
		cancelled = m_nexusCore.ProgressCallback()->Cancelled();
		if (!status) return status;
	}
	status = m_nexusCore.ProgressCallback()->ExecuteTask(AftkTasks::SaveScene, arg);
	cancelled = m_nexusCore.ProgressCallback()->Cancelled();

#pragma message("TODO: Copy crop sheet files when saving scene to a new location")
	return status;
}

void NexusSystem::SetLogCallback(void (*pLogCallbackFunc)(const std::string& text))
{
	m_pLogCallbackFunc = pLogCallbackFunc;
}

C_ApplicationState& NexusSystem::GetState()
{
	return m_state;
}

const C_ApplicationState& NexusSystem::GetState() const
{
	return m_state;
}

bool NexusSystem::AnimationPaused() const
{
	return m_animationPaused;
}

void NexusSystem::ToggleAnimation()
{
	m_animationPaused = !m_animationPaused;
}

void NexusSystem::ResetAnimation()
{
	SetTime(GetCorePtr()->GetAftkScenePtr()->GetStartTime());
}

void NexusSystem::FrameSkip(S32 skip)
{
	F32 frameRate = GetCorePtr()->GetAftkScenePtr()->GetFrameRate();
	F32 startTime = GetCorePtr()->GetAftkScenePtr()->GetStartTime();
	F32 endTime = GetCorePtr()->GetAftkScenePtr()->GetEndTime();
	F32 currentTime = m_nexusCore.GetAftkScenePtr()->GetCurrentTime();

	if (frameRate > 0.01f)
	{
		currentTime += (1 / frameRate * skip);
	}
	if (currentTime < startTime)
	{
		currentTime = endTime - (startTime - currentTime); 
	}
	else if (currentTime > endTime)
	{
		currentTime = startTime + currentTime - endTime;
	}

	SetTime(currentTime);
}

void NexusSystem::SetTime(F32 time)
{
	m_nexusCore.GetAftkScenePtr()->SetCurrentTime(time);
	g_pViewportParent->GetDisplayManagerPtr()->GetRendererPtr()->SetTime(time);
}

void NexusSystem::UpdateTime(F32 time)
{
   	if (m_animationPaused)
		return;

	F32 currentTime = m_nexusCore.GetAftkScenePtr()->GetCurrentTime();
	if (currentTime + time > GetCorePtr()->GetAftkScenePtr()->GetDuration())
	{
		F32 startTime = GetCorePtr()->GetAftkScenePtr()->GetStartTime();
		F32 endTime = GetCorePtr()->GetAftkScenePtr()->GetEndTime();
		F32 duration = GetCorePtr()->GetAftkScenePtr()->GetDuration();

		F32 newTime = (currentTime + time);
		U32 integerComp = (U32)(newTime / duration);
		newTime /= (F32)integerComp;
		currentTime = startTime + newTime - endTime;

		if (currentTime < startTime)
		{
			currentTime = endTime - (startTime - currentTime); 
		}
	}
	else if (currentTime + time >= GetCorePtr()->GetAftkScenePtr()->GetEndTime())
	{
		F32 startTime = GetCorePtr()->GetAftkScenePtr()->GetStartTime();
		F32 endTime = GetCorePtr()->GetAftkScenePtr()->GetEndTime();
		F32 newTime = (currentTime + time);
		currentTime = startTime + newTime - endTime;
	}
	else
	{
 		currentTime += time;
	}

	SetTime(currentTime);
}

E_ViewportState NexusSystem::GetViewportState() const
{
	return m_viewportState;
}

U32 NexusSystem::GetActiveViewportIndex() const
{
	return m_activeViewportIndex;
}

void NexusSystem::MouseButtonEvent(
	U32 viewportIndex,
	int buttonId, 
	E_MouseButtonAction action,
	UINT nFlags,
	CPoint point)
{
	switch (action)
	{
	case E_MouseButtonAction_Down:
		m_mouseButtonState[buttonId] = true;
		MouseButtonStateChanged(viewportIndex, buttonId);
		break;

	case E_MouseButtonAction_Up:
		m_mouseButtonState[buttonId] = false;
		MouseButtonStateChanged(viewportIndex, buttonId);
		break;

	case E_MouseButtonAction_DoubleClick:
	default:
		break;
	};
}

void NexusSystem::MouseButtonStateChanged(
	U32 viewportIndex,
	int buttonId)
{
	if ((buttonId == E_MouseButton_Left)
		&&
		(m_mouseButtonState[buttonId]))
	{
		if (GetKeyState(VK_CONTROL)&0x8000)
		{
			m_selectionState = E_SelectionState_Multi;
		}
		else
		{
			m_selectionState = E_SelectionState_Single;
		}

		if (GetKeyState(VK_MENU)&0x8000)
		{
			m_viewportState = E_ViewportState_ViewRotate;
		}
		else
		{
			g_pViewportParent->GetViewportPtr(viewportIndex)->Pick();
		}
	}
	else if ((buttonId == E_MouseButton_Middle)
		&&
		(m_mouseButtonState[buttonId]))
	{
		if (GetKeyState(VK_MENU)&0x8000)
		{
			if (m_viewportState != E_ViewportState_ViewTranslate)
			{
				m_viewportState = E_ViewportState_ViewTranslate;
			}
		}
	}
	else if ((buttonId == E_MouseButton_Right)
		&&
		(m_mouseButtonState[buttonId]))
	{
		if (GetKeyState(VK_MENU)&0x8000)
		{
			if (m_viewportState != E_ViewportState_ViewDolly)
				m_viewportState = E_ViewportState_ViewDolly;
		}
	}
	else
	{
		if (m_viewportState != E_ViewportState_NoAction)
			m_viewportState = E_ViewportState_NoAction;
	}
}


void NexusSystem::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	m_keyboardState[nChar] = true;
	KeyboardEvent(nChar, nRepCnt, nFlags, true);
}

void NexusSystem::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	m_keyboardState[nChar] = false;
	KeyboardEvent(nChar, nRepCnt, nFlags, false);
}

void NexusSystem::KeyboardEvent(UINT nChar, UINT nRepCnt, UINT nFlags, bool pressed)
{
	bool viewInvalidated = false;

	switch (nChar)
	{
		case 'C':
		case 'c':
		{
			g_pViewportParent->GetDisplayManagerPtr()->GetViewportDisplayPtr(m_activeViewportIndex)->CentreCamera();
			viewInvalidated = true;
			break;
		}

		case 'A':
		case 'a':
		{
			g_pViewportParent->GetDisplayManagerPtr()->FrameAll();
			viewInvalidated = true;
			break;
		}

		case 'F':
		case 'f':
		{
			g_pViewportParent->GetDisplayManagerPtr()->GetViewportDisplayPtr(m_activeViewportIndex)->FrameSelected();
			viewInvalidated = true;
			break;
		}

		case 'R':
		case 'r':
		{
			ResetAnimation();
			viewInvalidated = true;
			break;
		}

		case VK_LEFT:
		{
			S32 t = (S32)(m_nexusCore.GetAftkScenePtr()->GetFrameRate()*0.25);
			FrameSkip(-t);
			viewInvalidated = true;
			break;
		}

		case VK_RIGHT:
		{
			S32 t = (S32)(m_nexusCore.GetAftkScenePtr()->GetFrameRate()*0.25);
			FrameSkip(t);
			viewInvalidated = true;
			break;
		}

		case ' ':
		{
			if (pressed)
			{
				ToggleAnimation();
				viewInvalidated = true;
			}
			break;
		}
	}

	if (viewInvalidated)
		g_pDocument->UpdateAllViews(0);
}

E_SelectionState NexusSystem::GetSelectionState() const
{
	return m_selectionState;
}

LARGE_INTEGER NexusSystem::GetFrameStartTime() const
{
	return m_startTime;
}

void NexusSystem::SetFrameStartTime(LARGE_INTEGER startTime)
{
	m_startTime = startTime;
}

LARGE_INTEGER NexusSystem::GetTimerFrequency() const
{
	return m_timerFrequency;
}