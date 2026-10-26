#ifndef EDITOR_WX_SCARAB_FORM_CONTEXT_H
#define EDITOR_WX_SCARAB_FORM_CONTEXT_H

class wxAuiManager;

class C_XtDomWriter;
class ScarabForm;

namespace Editor
{

class C_WxScarabFormContext : public ScarabFormContext
{
public:
	C_WxScarabFormContext(C_WxScarabForm* pForm);
	virtual ~C_WxScarabFormContext();

public:
	virtual void OpenRecord(Xt::DomRecord* record);
	virtual void BeginDropper(LarvaField* field, Xt::DomElement* element);

	C_WxScarabForm* m_pWxScarabForm;
};

}

#endif
