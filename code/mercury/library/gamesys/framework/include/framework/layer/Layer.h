#ifndef FRAMEWORK_LAYER_H
#define FRAMEWORK_LAYER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Framework/Application/ApplicationTypes.h"

namespace Framework
{

class C_LayerManager;

//! E_LayerMessage

enum E_LayerMessage
{
	E_LayerMessage_Create = 0,
	E_LayerMessage_Destroy,
	E_LayerMessage_Resize,

	E_LayerMessage_Update,
	E_LayerMessage_GlobalUpdate,

	E_LayerMessage_BeginFrame,
	E_LayerMessage_BeginSplitscreen,
	E_LayerMessage_ScheduleOnce,
	E_LayerMessage_ScheduleSplitscreen,
	E_LayerMessage_RenderOnce,
	E_LayerMessage_PreRenderSplitscreen,
	E_LayerMessage_RenderSplitscreenWithAA,
	
	E_LayerMessage_BeginRenderSplitscreenWithoutAA,
	E_LayerMessage_RenderSplitscreenWithoutAA,
	E_LayerMessage_EndRenderSplitscreenWithoutAA,

	E_LayerMessage_RenderFullscreen,
	E_LayerMessage_RenderFullscreenDebug,
	E_LayerMessage_EndFrame,
	E_LayerMessage_ResetDebugRender
	
};

enum E_LayerState
{
	E_LayerState_Constructed = 0,
	E_LayerState_PendingCreate,
	E_LayerState_Ready,
	E_LayerState_Running,
	E_LayerState_PendingDestroy,
	E_LayerState_PendingAsyncDestroy,
	E_LayerState_Destroyed
};

enum E_LayerPriority
{
	E_LayerPriority_First = -1000,
	E_LayerPriority_Default = 0,
	E_LayerPriority_Last = 1000
};

//! C_Layer

class C_Layer
{
	friend class C_LayerManager;

public:

	C_Layer(const char* name);
	virtual ~C_Layer();

	const char* GetName() const;

	E_LayerState GetLayerState() const;

	virtual S32 GetLayerPriority() const;

	void EnableLayer();

	void DisableLayer();

private:
	void SetLayerState( const E_LayerState layerState );

	virtual HRESULT ProcessMessage(const E_LayerMessage message) = 0;

private:
	E_LayerState m_LayerState;
	bool m_Enabled;
	const char* m_Name;
};

} // namespace Framework

#endif // FRAMEWORK_LAYER_H
