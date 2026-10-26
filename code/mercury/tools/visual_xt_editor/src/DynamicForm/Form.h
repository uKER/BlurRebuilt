#ifndef XT_EDITOR_DYNAMIC_FORM_H
#define XT_EDITOR_DYNAMIC_FORM_H

#include "Xt/XtTool.h"


/*namespace Xt
{
	struct DomRecord;
	struct DomBlock;
	struct DomDb;
}

struct LtStruct;*/

class wxAuiManager;

class C_Frame;

namespace DynamicForm
{

class C_Node;


class C_Form : public wxScrolledWindow
{
public:
    C_Form(C_Frame* pFrame, wxWindow* pParent, const wxWindowID id,
               const wxPoint& pos, const wxSize& size,
               long style);
	~C_Form();

	void		LoadRecord(Xt::DomRecord* pRecord);
	void		LoadBlock(Xt::DomBlock* pBlock, LtStruct* pLtStruct);
	Xt::DomDb*	GetDomDb() { return m_pDomDb; }
	void		UpdateLayout();
	void		ClearForm();

	C_Frame*	GetFrame() { return m_pFrame; }

	static void		DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, C_Form* doc);
protected:

	C_Frame*		m_pFrame;
	C_Node*			m_pRoot;
	Xt::DomBlock*	m_pBlock;
	wxString		m_PreviousType;
	Xt::DomDb*		m_pDomDb;
	Xt::DomMonitor	m_DomMonitor;
};

} // DynamicForm

#endif // XT_EDITOR_DYNAMIC_FORM_H