/////////////////////////////////////////////////////////////
// C_LiveryEditor
/////////////////////////////////////////////////////////////

#ifndef __LIVERY_EDITOR_H__
#define __LIVERY_EDITOR_H__

//#include "graphics\resourcebundler.h"

#include "Canvas.h"

#include <Display/Bundler/I_ResourceFactory.h>
#include <Display/Bundler/I_ResourceBundle.h>
#include <Display/Bundler/ResourceBundleSingle.h>

namespace Vehicle
{

namespace Livery
{

/* 
===============================================================
					class C_LiveryEditor

NOTES: 
===============================================================
*/
class C_LiveryEditor
{
public:

	static C_LiveryEditor& Instance() {return s_instance;}

	C_LiveryEditor();

	~C_LiveryEditor();

	//! Initialise - textures etc
	void Initialise();

	//! Kill everything - textures etc
	void Shutdown();

	//! Clears all canvases
	void ClearAll();

	//! Clears a given canvas
	void ClearCanvas(E_Canvas p_Canvas);

	//! Updates current canvas
	void Update();

	//! Renders current canvas panel
	void Render();

	//! Adds a layer to the active canvas
	void AddLayer();

	//! Edits a canvas texture
	void EditCanvas(E_Canvas p_Canvas, bool updateTexture = false);

	//! Renders to a canvas texture
	void RenderCanvas(E_Canvas p_Canvas, float fFlash = 0.0f);

	//! Sets the active canvas
	void SetActiveCanvas(E_Canvas p_Canvas);

	//! Selects next layer
	void NextLayer();

	//! Selects previous layer
	void PreviousLayer();

	//! Returns active canvas
	C_Canvas* GetActiveCanvas();

	//! Copies current layer
	void CopyLayer();

	//! Pastes layer
	void PasteLayer();

	//! Flips active layer horizontally
	void FlipHorizontal();

	//! Flips active layer vertically
	void FlipVertical();

	//! Mirrors active layer horizontally
	void MirrorHorizontal();

	//! Mirrors active layer vertically
	void MirrorVertical();

	//! Debug display
	void DebugDisplay();

	//! Debug: Displays a canvas
	void Debug_DisplayCanvas(E_Canvas p_Canvas);

	//! Sets texture
	//LPDIRECT3DTEXTURE9 GetTexture(CarClusterCode p_Code);

	//! Saves livery
	bool Save(const char* p_pPath, const char* p_pName);

	//! Loads livery
	bool Load(const char* p_pPath, const char* p_pName);

	//! Sets force look at
	void SetForceLookAt(bool p_ForceLookAt) { m_ForceLookAt = p_ForceLookAt; }

	//! Returns true if livery options menu is active
	bool MenuActive() { return m_MenuActive; }
	
	//! Flags menu as active (disable camera controls
	void SetMenuActive(bool p_Active) { m_MenuActive = p_Active; }

	//! Returns current colour for selected layer
	MathsStorage::Float4 GetCurrentColour();

	//const BizPackedResource* GetResource() { return m_pLiveryResource; }
	const Display::ResourceBundler::I_ResourceBundle* GetResource() { return m_pLiveryResource; }

public:
	
	static C_LiveryEditor		s_instance;

	int							m_Version;

	bool						m_Initialised;						//!< Has the system been initialised
	bool						m_RenderOnce;
	bool						m_ForceLookAt;
	bool						m_MenuActive;

	float						m_FlashLayer;
	float						m_FlashLayerMinTime;
	float						m_FlashLayerMaxTime;

	E_Canvas					m_CurrentCanvas;

	C_Layer						m_CopiedLayer;

	Ut::UtVector<Ut::ArrayVector<C_Canvas, E_Canvas_Max> >	m_Canvases;	//!< Canvas targets

	//BizPackedResource*			m_pLiveryResource;					//!< Livery texture bank
	Util::C_String									m_BundleName;
	Display::ResourceBundler::I_ResourceBundle*		m_pLiveryResource;
};

} //namespace Livery

} //namespace Vehicle

#endif
