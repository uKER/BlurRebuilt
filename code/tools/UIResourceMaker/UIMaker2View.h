// UIMaker2View.h : interface of the CUIMaker2View class
//


#pragma once


class CUIMaker2View : public CScrollView
{
protected: // create from serialization only
	CUIMaker2View();
	DECLARE_DYNCREATE(CUIMaker2View)

// Attributes
public:
	CUIMaker2Doc* GetDocument() const;

// Operations
public:

// Overrides
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// Implementation
public:
	virtual ~CUIMaker2View();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in UIMaker2View.cpp
inline CUIMaker2Doc* CUIMaker2View::GetDocument() const
   { return reinterpret_cast<CUIMaker2Doc*>(m_pDocument); }
#endif

