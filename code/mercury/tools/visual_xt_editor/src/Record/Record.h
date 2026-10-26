#ifndef XT_EDITOR_RECORD_H
#define XT_EDITOR_RECORD_H

namespace Xt
{
	struct	DomRecord;
};

namespace DynamicForm
{
	class C_Form;
};

class C_ExplorerItemData;

enum E_RecordType
{
	E_Record_ParticleEvent,
	E_Record_ParticleEffect
};

class C_Record
{
public:
	C_Record(Xt::DomRecord* pDomRecord, DynamicForm::C_Form* pForm) : m_pDomRecord(pDomRecord), m_pForm(pForm) {}
	virtual ~C_Record() {}

	void					SetName(const wxString& name);

	const wxString&			GetName() const { return m_Name; }
	const wxString&			GetPath() const { return m_Path; }
	E_RecordType			GetRecordType() const { return m_eType; }
	const Xt::DomRecord*	GetDomRecord() const { return m_pDomRecord; }
	Xt::DomRecord*			GetDomRecord() { return m_pDomRecord; }
	DynamicForm::C_Form*	GetForm() { return m_pForm; }

	virtual bool			Create() { return true; }
	virtual void			Destroy() {}

	virtual bool			Open() { return true; }
	virtual void			Close() {}

	virtual void			Select() {}
protected:

	wxString				m_Name;
	wxString				m_Path;
	E_RecordType			m_eType;
	Xt::DomRecord*			m_pDomRecord;
	DynamicForm::C_Form*	m_pForm;
};

#endif // XT_EDITOR_RECORD_H