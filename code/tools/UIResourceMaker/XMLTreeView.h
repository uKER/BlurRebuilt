#pragma once

class CXMLTreeView : public CTreeView
{
	DECLARE_DYNCREATE(CXMLTreeView)

protected:
	CXMLTreeView();           // protected constructor used by dynamic creation
	virtual ~CXMLTreeView();

public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	DECLARE_MESSAGE_MAP()
public:
//	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnDropFiles(HDROP hDropInfo);
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);

	void BuildTree();
	void ClearTree();
	void RefreshImages();
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);

private:
	CImageList ImageList;
	CBitmap bm,bm1,bm2,bm3,bm4;
	char mSelectedName[256];
	char mSelectedExtension[32];

	void RecBuildDir(const char *dir);

public:
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
};


