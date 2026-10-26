#include "stdafx.h"
#include <sigslot/sigslot.h>

#include <xtwin/WndCore.h>
#include <xtwin/WndClasses.h>
#include <xtwin/Scarab.h>

#include <dialogueman/Components/Scarab_Form/WxScarabForm.h>
#include <dialogueman/Components/Scarab_Form/WxScarabFormContext.h>


namespace DMan
{

C_WxScarabFormContext::C_WxScarabFormContext(C_WxScarabForm* pForm) : ScarabFormContext()
{
	m_pWxScarabForm = pForm;
}

C_WxScarabFormContext::~C_WxScarabFormContext()
{
}

void C_WxScarabFormContext::OpenRecord(Xt::DomRecord* record)
{
}
void C_WxScarabFormContext::BeginDropper(LarvaField* field, Xt::DomElement* element)
{
}

}
