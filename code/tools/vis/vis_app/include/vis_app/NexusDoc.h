// NexusDoc.h : interface of the NexusDoc class
//


#pragma once

class NexusDoc : public CDocument
{
protected: // create from serialization only
	NexusDoc();
	DECLARE_DYNCREATE(NexusDoc)

// Attributes
public:

// Operations
public:

// Overrides
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

// Implementation
public:
	virtual ~NexusDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	const Nexus::NexusSystem* NexusSystemPtr() const;
	Nexus::NexusSystem* NexusSystemPtr();

public:
	void SetMenuPtr(CExtMenuControlBar* pMenu);

	Nexus::NexusSystem* GetNexusSystemPtr();

protected:
	Nexus::NexusSystem m_nexusSystem;

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	afx_msg void OnShadingWireframeall();
	afx_msg void OnFileImport();
	afx_msg void OnFileExport();
};


