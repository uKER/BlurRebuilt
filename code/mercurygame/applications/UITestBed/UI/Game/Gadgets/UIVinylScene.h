#ifndef UI_VINYL_SCENE
#define UI_VINYL_SCENE

#include <ui\Game\Gadgets\BaseUIShape.h>

#include "ui\UiManager.h"
#include "VinylManager.h"
#include "VinylList.h"

namespace UI
{

class UIVinylScene : public C_GadgetBase
{
public:
	DECLARE_GADGET( E_UIGameButtonType_VinylScene, "VINYL_SCENE" )

	void		Init();
	bool		MessageHandler(const Messaging::C_MessageBase *pMessage);
	void		Render();

// Exposed for Xt
public:
	XtF32							m_AlphaStep;
	XtF32							m_RotateStep;
	XtF32							m_TranslateStep;
	XtF32							m_ShearStep;
	XtF32							m_ScaleStep;

	XtF32							m_AlphaNudge;
	XtF32							m_RotateNudge;
	XtF32							m_TranslateNudge;
	XtF32							m_ShearNudge;
	XtF32							m_ScaleNudge;

private:
	void							RenderList( const C_VinylManager::E_List list );

	F32								m_ActiveTimer;
};

}; // namespace UI

#endif // UI_VINYL_SCENE
