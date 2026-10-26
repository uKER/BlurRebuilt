#ifndef XT_EDITOR_XTLOADER_H
#define XT_EDITOR_XTLOADER_H

namespace Xt
{
	struct DomDb;
	struct DomElement;
	struct DomRecord;
	struct DomField;
	struct DomBlock;
	struct DomArray;
}

class C_XtStorage;

class C_Record;

class C_XtLoader
{
public:
	C_XtLoader();
	~C_XtLoader();

	bool	Open(const wxString& path);
	bool	Save();
	void	Close();

	const Ut::Vector<Util::C_String>&	GetPacks() const { return m_Packs; }

private:
	void					ParseElement(Xt::DomElement* pElement);
	void					ParseRecord(Xt::DomRecord* pRecord);
	void					ParseBlock(Xt::DomBlock* pBlock);
	void					ParseArray(Xt::DomArray* pArray);
	void					ParseField(Xt::DomField* pField);


	Xt::DomDb*					m_pDomDb;
	C_XtStorage*				m_pStorage;

	C_Record*					m_pRecord;
	wxString					m_CurrentType;

	U32							m_RecordCount;
	Ut::Vector<Util::C_String>	m_Packs;

	bool					m_Open;

};


#endif
