
#ifndef UI_UICHECKBOX_H
#define UI_UICHECKBOX_H

#include "ui\UIGameDefines.h"

#include "ui/Game/Gadgets/UIStaticText.h"

namespace UI
{

class UICheckbox : public UIStaticText
{
public:
	DECLARE_GADGET( E_UIGameButtonType_Checkbox, "CHECKBOX" )

	void			Init();
	void			PostInit();
	void			Setup( lua_State* pState );
	bool			MessageHandler( const Messaging::C_MessageBase *pMessage );
	void			Render();

	void			AssignCallback( const char* pName );
	void			Checked( const bool bChecked )
					{
						m_Checked = bChecked;
					}

	bool			IsChecked() const
					{
						return m_Checked;
					}

public:
	F32				m_IconSize;
	bool			m_Checked;

private:
	static const U32 m_MaxName = 24;
	char			m_CallbackName[m_MaxName]; // might allocate this rather than have it fixed size
};

}; // namespace UI

#endif // UI_UIAMAXCHECKBOX_H