#ifndef DMAN_WX_SCARAB_FORM_CONTEXT_H
#define DMAN_WX_SCARAB_FORM_CONTEXT_H

#ifdef _MSC_VER
#pragma once
#endif

class ScarabForm;

namespace DMan
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

#endif //DMAN_WX_SCARAB_FORM_CONTEXT_H
