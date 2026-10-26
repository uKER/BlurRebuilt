#pragma once


// CXMLEditView view

class CXMLEditView : public CRichEditView
{
	DECLARE_DYNCREATE(CXMLEditView)

protected:
	CXMLEditView();           // protected constructor used by dynamic creation
	virtual ~CXMLEditView();

public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	DECLARE_MESSAGE_MAP()
public:
protected:
public:
	bool changed;
protected:
public:
	afx_msg void OnEnChange();
	virtual void OnInitialUpdate();
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar,UINT nRepCnt,UINT nFlags);
protected:

public:
	afx_msg void OnAddTextKey();
};


