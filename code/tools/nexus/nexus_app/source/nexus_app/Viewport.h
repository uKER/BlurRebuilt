#ifndef NEXUS_VIEWPORT_H
#define NEXUS_VIEWPORT_H

namespace Nexus
{
	class C_Viewport : public CWnd
	{
	public:
		C_Viewport();

	protected: // create from serialization only
		DECLARE_DYNCREATE(C_Viewport)

	// Attributes
	public:

	// Operations
	public:

	protected:
	// Overrides
		public:
		virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
		BOOL DataObjectToList(COleDataObject* const pDataObject);
		BOOL ProcessDroppedFile( HDROP hDrop );

	// Implementation
	public:
		virtual ~C_Viewport();
	#ifdef _DEBUG
		virtual void AssertValid() const;
		virtual void Dump(CDumpContext& dc) const;
	#endif

	public:
		void Pick();

		C_ViewportDisplay* GetDisplayPtr();

	protected:
		void MouseMoveEvent(UINT nFlags, CPoint point);
		void MouseButtonEvent(
			int buttonId, 
			E_MouseButtonAction action, 
			UINT nFlags, 
			CPoint point);
		void MouseButtonStateChanged(int buttonId);
		void ZoomCamera(F32 distancePx);
		

		S32	m_mouseXOld;
		S32	m_mouseYOld;

		bool m_initialPaint;

		C_ViewportDisplay m_display;

	// Generated message map functions
	protected:
		afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
		DECLARE_MESSAGE_MAP()
	public:
		virtual void OnFinalRelease();
		afx_msg void OnPaint();
		afx_msg void OnSize(UINT nType, int cx, int cy);
	protected:
		
	public:
		afx_msg void OnMouseMove(UINT nFlags, CPoint point);
		afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
		afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
		afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
		afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
		afx_msg void OnMButtonDblClk(UINT nFlags, CPoint point);
		afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
		afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
		afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
		afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
		afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
		afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	};
}

#endif