// UIMaker2Doc.h : interface of the CUIMaker2Doc class
//


#pragma once

class CUIMaker2Doc : public CDocument
{
protected: // create from serialization only
	CUIMaker2Doc();
	DECLARE_DYNCREATE(CUIMaker2Doc)

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
	virtual ~CUIMaker2Doc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnBuild();
	afx_msg void OnBundle();

	afx_msg void OnCopyToServer();
	afx_msg void OnCopyToConsole();

	afx_msg void OnBundleEA();
	afx_msg void OnCopyEAToConsole();
public:

};
