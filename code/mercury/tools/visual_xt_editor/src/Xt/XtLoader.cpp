
#include "stdafx.h"

#include "XtLoader.h"
#include "xt\xttool.h"

#include "src/Frame.h"
#include "src/Preview/PreviewWindow.h"

#include "src/Record/GenericRecord.h"

class C_XtStorage : public Xt::Storage
{
public:
	virtual bool Start() {return true;};
	virtual bool New(Xt::DomRecord* record);
	virtual bool Dirty(Xt::DomRecord* record);
	virtual bool Rename(Xt::DomRecord* record, const char* oldFilename) {return true;};
	virtual bool Remove(Xt::DomRecord* record) {return true;};
	virtual bool IsModified(const char* filename) {return true;};
	virtual bool Stop() {return true;};
};

bool C_XtStorage::New(Xt::DomRecord* record)
{
	return Xt::Export(record);
}

bool C_XtStorage::Dirty(Xt::DomRecord* record)
{
	return Xt::Export(record);
}

C_XtLoader::C_XtLoader() : m_Open(false)
{
	m_pDomDb = new Xt::DomDb;
	m_pStorage = new C_XtStorage;

}

C_XtLoader::~C_XtLoader()
{
	delete m_pDomDb;
	delete m_pStorage;
}

bool AppendFormatString(XtString& output, const XtString& format, XtUt::Dictionary<XtString, XtString>* substitutions)
{
	const char* start	= format;
	const char* ptr		= format;

	for(;;)
	{
		while((*ptr != '%') && (*ptr != '\0'))
			ptr++;

		if (!XtsAppend(output, start, ptr - start))
			return false;

		if (*ptr == '\0')
			break;

		start = ptr + 1;

		for(ptr++; (*ptr != '%') && (*ptr != '\0'); )
			ptr++;

		if (*ptr == '\0')
			return false;

		if (start != ptr)
		{
			XtString code(start, ptr - start);

			if (XtString* translated = At(substitutions, code))
			{
				if (!XtsAppend(output, *translated, translated->Length()))
					return false;
			}
		}
		else
		{
			if (!XtsAppend(output, "%", 1))
				return false;
		}

		start	= ptr + 1;
		ptr		= start;
	}

	return true;
}


bool C_XtLoader::Open(const wxString& path)
{
	if(m_Open)
	{
		Close();
	}
	
	wxLogMessage(_T("Loading Xt database '%s'..."), path.c_str());

	XtString xtPath(path.c_str());

	if(!Xt::Open(m_pDomDb, xtPath, m_pStorage, false, NULL, NULL))
	{
		wxLogMessage(_T("Database failed to load"));
		return false;	// Failed
	}
	else
		wxLogMessage(_T("Database loaded successfully!"));

	Xt::DomPack* pack = 0;

	wxLogMessage(_T("Parsing database..."));

	m_RecordCount = 0;

	Xt::DomBuild* pDomBuild = &m_pDomDb->build;

	XtUt::Dictionary<XtString, XtString> stageSubstitutions;
	if (!XtUt::Add(&stageSubstitutions, "db", new XtString(m_pDomDb->path)))
		return false;

	XtString flaskPath;
	m_Packs.Clear();

	// look for binary flask paths
	for(XtUt::List<Xt::BuildStage>::Iterator it(&(pDomBuild->staging)); it; ++it)
	{
		if(it->name == "pc")
		{
			AppendFormatString(flaskPath, it->root, &stageSubstitutions);

			for(XtUt::Dictionary<XtId, Xt::DomPack>::Iterator itPack(&m_pDomDb->packs); itPack; ++itPack)
			{
				// format the pack filename
				XtString packFile = flaskPath;
				XtUt::Dictionary<XtString, XtString> packSubstitutions;
				if (!XtUt::Add(&packSubstitutions, "pack", new XtString(itPack->name)))
					return false;
				AppendFormatString(packFile, pDomBuild->structure.packs, &packSubstitutions);

				// store in list of packs
				m_Packs.PushBack(Util::C_String((const char*)packFile));
			}
		}
	}

	for(XtUt::Dictionary<XtId, Xt::DomPack>::Iterator it(&(m_pDomDb->packs)); it; ++it)
	{
		for(XtUt::LinkList<Xt::DomRecord>::Iterator pRecordIterator(&(it->records)); pRecordIterator; ++pRecordIterator)
		{
			ParseElement(pRecordIterator);
		}
	}
	wxLogMessage(_T("Database parsed successfully, found %d records"), m_RecordCount);
	
	m_Open = true;

	return m_Open;
}

void C_XtLoader::Close()
{
	m_Open = false;
	Xt::Close(m_pDomDb, NULL);
}

bool C_XtLoader::Save()
{
	if(m_Open)
	{
		Xt::OpenDbContext odbc;

		odbc.callback	= 0;//SorceryLoadDbProgress;
		odbc.context	= 0;//op;

		for(XtUt::Dictionary<XtId, Xt::DomPack>::Iterator it(&m_pDomDb->packs); it; ++it)
			Free(&it->filters);
		
		bool result = Xt::Flush(m_pDomDb, &odbc);

		for(XtUt::Dictionary<XtId, Xt::DomRecord>::Iterator it(&m_pDomDb->records); it; ++it)
			Xt::Notify(it, &Xt::DomRecordChanged(it));

		return result;
	}
	else
		return false;
}

void C_XtLoader::ParseElement(Xt::DomElement* pElement)
{
	switch(pElement->type)
	{
		case Xt::kDomRecord:
			ParseRecord((Xt::DomRecord*)pElement);
			break;
		case Xt::kDomField:
			ParseField((Xt::DomField*)pElement);
			break;
		case Xt::kDomBlock:
			ParseBlock((Xt::DomBlock*)pElement);
			break;
		case Xt::kDomArray:
			ParseArray((Xt::DomArray*)pElement);
			break;
	}
}


void C_XtLoader::ParseRecord(Xt::DomRecord* pRecord)
{
	wxString name((const char*)pRecord->name);
	m_pRecord = C_Frame::Instance().AddRecord(pRecord, name);
	++m_RecordCount;
	
	ParseElement(&pRecord->root);
}

void C_XtLoader::ParseField(Xt::DomField* pField)
{
	
}

void C_XtLoader::ParseBlock(Xt::DomBlock* pBlock)
{
	for(XtUt::Dictionary<XtString, Xt::DomElement>::Iterator pElementIterator(&(pBlock->elements)); pElementIterator; ++pElementIterator)
	{
		ParseElement(pElementIterator);
	}	
}

void C_XtLoader::ParseArray(Xt::DomArray* pArray)
{
	S32 index = 0;
	for(XtUt::List<Xt::DomElement>::Iterator pArrayIterator(&(pArray->elements)); pArrayIterator; ++pArrayIterator)
	{
		++index;
		
		ParseElement(pArrayIterator);
	}
}

