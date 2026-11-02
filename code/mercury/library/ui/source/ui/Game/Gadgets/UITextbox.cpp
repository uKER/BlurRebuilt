// [Blur2PC] Retail PC account text entry. Recovered function names are descriptive.
#include "stdafx.h"
#include "ui/Game/Gadgets/UITextbox.h"
#include "ui/Game/Gadgets/ButtonMessages.h"
#include "ui/Game/UIScreenControl.h"
#include "ui/Graphics/Font/Font.h"
#include "ui/Graphics/SimpleTimer.h"
#include "ui/Game/UILocaliser.h"
#include <input/KeyboardPC.h>

XT_TYPE(UI::UITextBox, "UITextBox")
    XT_BASE(UI::C_GadgetBase)
    XT_FIELD(m_MaxCharacters, "max_characters")
    XT_FIELD(m_DefaultText, "default_text")
    XT_FIELD(m_FontHeight, "font_height")
    XT_FIELD(m_Hidden, "hidden")
    XT_FIELD(m_AutoSize, "autosize")
    XT_FIELD(m_ClearOnReturn, "clear_on_return")
    XT_FIELD(m_BackingColour, "backing_colour")
    XT_FIELD(m_Styles, "styles")
XT_END_TYPE

namespace UI {
UITextBox::~UITextBox() {
    SecureZeroMemory(m_Text,sizeof(m_Text));
    SecureZeroMemory(m_Masked,sizeof(m_Masked));
}
// Recovered UITextBox::Init, retail 0x00784460; caret period at 0x00ea8e68.
void UITextBox::Init() {
    C_GadgetBase::Init();
    memset(m_Text,0,sizeof(m_Text));memset(m_Masked,0,sizeof(m_Masked));
    m_Length=0;m_Editing=false;m_Caret=false;m_CaretTime=0.5f;
    m_Hidden=false;m_ClearOnReturn=false;m_AutoSize=false;m_DefaultText.id=0;
    m_BackingColour=XtSz8("");m_MaxCharacters=255;m_FontHeight=0;m_InputGeneration=0;
}
// Recovered UITextBox::Setup, retail 0x007844d0.
void UITextBox::Setup(lua_State* state) {
    C_GadgetBase::Setup(state);m_Styles.Setup(state);
    SetLimit(m_MaxCharacters);
    m_ResolvedBackingColour.SetByName(m_BackingColour);
    AddSubscriber(E_UiMsg_FrameUpdate);
}
// Recovered UITextBox::SetLimit / SetText, retail 0x00785b10 / 0x007845f0.
void UITextBox::SetLimit(U16 limit) {m_MaxCharacters=limit>255?255:limit;}
void UITextBox::SetText(const WCHAR* text) {
    m_Length=0;
    while(text && text[m_Length] && m_Length<m_MaxCharacters) {
        m_Text[m_Length]=text[m_Length];m_Masked[m_Length]=L'*';++m_Length;
    }
    m_Text[m_Length]=m_Masked[m_Length]=0;
}
// Recovered UITextBox::ToggleEditing, retail 0x00784d20.
void UITextBox::ToggleEditing() {
    m_Editing=!m_Editing;m_Caret=m_Editing;m_CaretTime=0.5f;
    if(m_Editing)SetText(L"");
}
// Recovered UITextBox::FrameUpdate, retail 0x00785180. WM_CHAR supplies the
// character event snapshot on this port; timer advancement remains elapsed-time based.
void UITextBox::FrameUpdate() {
    if(!m_Activated || !m_Selected)return;
    C_ScreenControl& screen=C_ScreenControlManager::Instance().Get(m_ID.Context());
    if(!screen.Active() || !(C_ScreenControlManager::Instance().InputMessageMask() & screen.GetMessageMask()))return;
    WCHAR characters[256];
    U32 count=Input::C_KeyboardPC::ReadTextCharacters(characters,256,m_InputGeneration);
    for(U32 i=0;i<count;++i) {
        WCHAR c=characters[i];
        if(c==L'\t') { _UI_MESSAGE2_MASK(E_UiMsg_TextTab, 0, true, screen.GetMessageMask());break; }
        if(c==L'\b') {
            if(m_Length) {--m_Length;m_Text[m_Length]=m_Masked[m_Length]=0;}
        } else if(c==L'\r') {
            ToggleEditing();
        } else if(c>=32 && c!=127) {
            if(!m_Editing)ToggleEditing();
            if(m_Length<m_MaxCharacters && c!=L'~' && c!=L'%' && gFont.GetFont(m_Styles.GetFontStyle(0).Font())->LookupGlyph(c,true)) {
                m_Text[m_Length]=c;m_Masked[m_Length]=L'*';++m_Length;
                m_Text[m_Length]=m_Masked[m_Length]=0;
            }
        }
    }
    SecureZeroMemory(characters,sizeof(characters));
    if(m_Editing) {
        m_CaretTime-=theFETimer.FrameTime();
        while(m_CaretTime<=0) {m_CaretTime+=0.5f;m_Caret=!m_Caret;}
    }
}
// Recovered UITextBox::Render, retail 0x007880c0: backing, default hint,
// password mask and a blinking append cursor in the original SCUI field.
void UITextBox::Render() {
    if(!m_Activated)return;
    if(!m_Selected) {m_Editing=false;m_Caret=false;}
    C_UIAutoTransform transform(this);
    C_GadgetBase::Render();
    Maths::HVec3 position=Maths::HVec3(m_Position)+gSimple2D.GetJustifiedCorner(m_Justify,0,m_Panel)*m_Size;
    gSimple2D.AddSprite(m_Layer,position,m_Size,m_ResolvedBackingColour,m_Panel,E_SpriteType_Sprite);
    gSimple2D.AddTexUVs(0,GetTextureParam(0));
    gSimple2D.AddJustification(E_Justify_TopLeft);
    WCHAR display[258];
    const WCHAR* text=m_Hidden?m_Masked:m_Text;
    if(!m_Editing && !m_Length && m_DefaultText.id) {
        C_TextStringID hint(m_DefaultText.id);
        gFont.DrawText(m_Layer,position,E_Justify_TopLeft,hint,m_SpriteColours,m_FontHeight,m_Styles,m_Size.x,m_Panel);
    } else {
        wcscpy_s(display,text);
        if(m_Editing && m_Caret)wcscat_s(display,L"|");
        gFont.DrawText(m_Layer,position,E_Justify_TopLeft,display,m_SpriteColours,m_FontHeight,m_Styles,m_Size.x,m_Panel);
        SecureZeroMemory(display,sizeof(display));
    }
}
}
