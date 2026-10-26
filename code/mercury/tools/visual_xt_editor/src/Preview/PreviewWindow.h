#ifndef XT_EDITOR_PREVIEW_WINDOW_H
#define XT_EDITOR_PREVIEW_WINDOW_H

// display library
#include "wx\Mercury\displaywnd.h"

// particle handle
#include <Render/Dynamic/Particle/EventInstance.h>

namespace DynamicEffect
{
	class C_Renderer;
}

namespace PostProcess
{
	class C_Manager;
	class C_HighDynamicRange;
	class C_Greyscale;
}

struct XtDb;
struct XtPack;
struct XtCallbackContext;

class C_PreviewWindow : public wxDisplayWindow
{
public:
	C_PreviewWindow( wxWindow *parent, wxWindowID, const wxPoint &pos, const wxSize &size );
	~C_PreviewWindow();

	virtual bool	OnCreate();
	virtual void	OnRender() const;
	virtual void	OnUpdate();
	virtual void	OnResize();

	void			OpenAssets(const Util::C_String& assetPath);
	void			OpenFlasks(const Ut::Vector<Util::C_String>& flaskPaths);
	void			CloseFlask();

	void			SpawnEvent(const U32 eventId, const Maths::C_QuatTrans& transform);
	void			RemoveEvent();

protected:

	PostProcess::C_Manager*					m_pPostProcessing;
	PostProcess::C_HighDynamicRange*		m_pHighDynamicRange;
	PostProcess::C_Greyscale*				m_pGreyscale;
	DynamicEffect::C_Renderer*				m_pDynamicEffectRenderer;
	DynamicEffect::C_EventInstanceHandle	m_ParticleHandle;
	U32										m_ParticleId;
	Maths::C_QuatTrans						m_ParticleTransform;
	bool									m_bParticleRepeating;

	XtDb*									m_pXtDb;
	Ut::Vector<XtPack*>						m_XtPacks;
	XtCallbackContext*						m_pXtCallbackContext;

	DECLARE_CLASS(C_PreviewWindow)
    DECLARE_EVENT_TABLE()
};

#endif // XT_EDITOR_PREVIEW_WINDOW_H