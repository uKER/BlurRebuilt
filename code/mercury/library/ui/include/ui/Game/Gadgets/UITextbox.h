// [Blur2PC] Recovered retail PC UITextBox (type 0x24, vtable 0x00ea2d90).
#pragma once
#include "ui/Game/Gadgets/Buttons.h"
#include "ui/Graphics/Font/FontStyle.h"
#include "ui/Game/UiXt.h"

namespace UI {
class UITextBox : public C_GadgetBase {
public:
    DECLARE_GADGET(E_UIButtonType_TextBox, "TEXTBOX")
    virtual ~UITextBox();
    virtual void Init();
    virtual void Setup(lua_State* state);
    virtual void FrameUpdate();
    virtual void Render();
    void SetText(const WCHAR* text);
    const WCHAR* GetText() const { return m_Text; }
    void SetLimit(U16 limit);
    // XT field names/layout recovered from retail 0x010c6aa0.
    bool m_Hidden, m_ClearOnReturn, m_AutoSize;
    XtLocId m_DefaultText;
    C_FontStylePasses m_Styles;
    XtSz8 m_BackingColour;
    C_UISpriteColours m_ResolvedBackingColour;
    XtF32 m_FontHeight;
    U16 m_MaxCharacters;
private:
    void ToggleEditing();
    WCHAR m_Text[256];
    WCHAR m_Masked[256];
    U16 m_Length;
    bool m_Editing, m_Caret;
    F32 m_CaretTime;
    U32 m_InputGeneration;
};
}
