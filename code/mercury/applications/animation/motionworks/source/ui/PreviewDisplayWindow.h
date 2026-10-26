#ifndef PREVIEWDISPLAYWINDOW_H
#define PREVIEWDISPLAYWINDOW_H


#include "DisplayWindow.h"

class C_PreviewSection;

class C_PreviewDisplayWindow: public C_DisplayWindow
{
public:
	DECLARE_CLASS(C_PreviewDisplayWindow)
	
	C_PreviewDisplayWindow( C_Camera& camera, wxWindow *parent, wxWindowID, const wxPoint &pos, const wxSize &size );
	
	void OnKeyUp( wxKeyEvent& e );
	void OnKeyDown( wxKeyEvent& e );
	void OnMouseEvent(wxMouseEvent& e);
	void OnSize(wxSizeEvent& e);
	
protected:

	virtual bool OnCreate();

	virtual void OnUpdate();
	//virtual void OnRender() const;

private:

	bool m_debounce;

};

#endif

