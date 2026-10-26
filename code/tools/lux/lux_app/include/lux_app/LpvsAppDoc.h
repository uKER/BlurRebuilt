// lpvsappdoc.h : interface of the LpvsAppDoc class
//


#pragma once

#include "lux_app/AppSettings.h"

class LpvsAppDoc : public CDocument
{
protected: // create from serialization only
	LpvsAppDoc();
	DECLARE_DYNCREATE(LpvsAppDoc)

// Attributes
public:

// Operations
public:

// Overrides
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive &ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
// Implementation
public:
	virtual ~LpvsAppDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	void UpdateLayerDialog();

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual void SetTitle(LPCTSTR lpszTitle);
};


