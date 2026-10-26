#ifndef FRAMEWORK_DEBUG_LAYER_H
#define FRAMEWORK_DEBUG_LAYER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Framework/Layer/Layer.h>
#include <Framework/Debug/MemoryBudgets.h>
#include <DebugSystems/DebugHUDMenu.h>
#include <DebugSystems/Frame.h>

//#define FILEDIALOG_SAMPLE_ON

//Forwarding DebugSystem classes to avoid include dependencies
namespace DebugSystems
{
	class C_DebugHUDSubscriber;
	//class C_DebugHUDMenu;
	class C_DebugHUDPerf;
	class C_DebugHUDMemory;
	class C_DebugHUDResource;
	//class C_Frame;
	class C_FileDialog;
	class C_DebugHUDTextureViewer;
	class C_FileMonitor;
}

namespace Render { namespace Font { class C_FontRenderer;}};

namespace Framework
{

class C_DebugMenuController;
class C_DebugTextureViewerController;
class C_FileDialogController;

class C_DebugLayerSkin
{
public:
	DebugSystems::C_FrameSkin m_FrameSkin;
	DebugSystems::C_DebugHUDMenuSkin m_DebugHUDMenuSkin;
};

class C_DebugLayer : public C_Layer
{
public:
	C_DebugLayer();
	C_DebugLayer(C_DebugLayerSkin& debugLayerSkin);
	virtual ~C_DebugLayer();

	HRESULT ProcessMessage(const E_LayerMessage message);
	HRESULT Create();
	HRESULT Destroy();
	HRESULT GlobalUpdate();
	HRESULT RenderFullScreenDebug();
	HRESULT Render3dDebug();

	Render::Font::C_FontRenderer * GetFontRenderer() { return m_pFontRenderer; }
	C_MemoryBudgets * GetMemoryBudgets(){ return &m_MemoryBudgets; }

private:
	S32 GetLayerPriority() const { return Framework::E_LayerPriority_Last; }
	bool IsPauseable() const { return false; }
	void DumpBuildInfo();
	void ShowBuildInfo();
	void ShowIpAddress();
	void ShowStringMsg();
	void DrawCameraHelper();
	void DrawArtTextureSizeHelper();
	void InitFontRenderer();

	//DebugLayer Skin
	C_DebugLayerSkin m_DebugLayerSkin;
	//Log Subscriber debug system
	DebugSystems::C_DebugHUDSubscriber*		m_pDebugHUDSubscriber;
	//Menu debug system
	DebugSystems::C_DebugHUDMenu*			m_pDebugHUDMenuSystem;			
	//Perf debug system
	DebugSystems::C_DebugHUDPerf*			m_pDebugHUDPerfSystem;	
	//Memory debug system
	DebugSystems::C_DebugHUDMemory*			m_pDebugHUDMemorySystem;
	//Resource debug system
	DebugSystems::C_DebugHUDResource*		m_pDebugHUDResourceSystem;
	//Texture Viewer debug system
	DebugSystems::C_DebugHUDTextureViewer*	m_pDebugHUDTextureViewer;
	//DebugMenuController
	C_DebugMenuController*					m_pDebugHUDMenuController;
	//DebugTextureViewer Controller
	C_DebugTextureViewerController*			m_pDebugTextureViewerController;
	//Debug Frame
	DebugSystems::C_Frame*					m_pDebugFrame;
	DebugSystems::C_FileMonitor*			m_pFileMonitor;

#ifdef FILEDIALOG_SAMPLE_ON
	//FileDialogController
	C_FileDialogController*					m_pFileDialogController;

	//FileDialog sample
	DebugSystems::C_FileDialog*				m_pFileDialog; 
#endif

	//Font Renderer
	Render::Font::C_FontRenderer*			m_pFontRenderer;
	bool									m_Freeze;
	C_MemoryBudgets							m_MemoryBudgets;
};

extern Debug::Bool g_DebugKeyCombos;

} // namespace Framework

#endif // FRAMEWORK_DEBUG_LAYER_H
