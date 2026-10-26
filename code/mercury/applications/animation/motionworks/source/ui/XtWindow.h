#ifndef XTWINDOW_H
#define XTWINDOW_H

#ifdef _MSC_VER
#pragma once
#endif

#include <xtwin/WndCore.h>
#include <xtwin/Scarab.h>
#include <XtWin/WndClasses.h>


class C_XtWindow: public wxWindow
{
public:
	C_XtWindow(
		wxWindow* parent,
		wxWindowID winId,
		const wxPoint& pos,
		const wxSize& size,
		U32 styleFlags
	);
	
	virtual ~C_XtWindow();

	void SetRecord( Xt::DomRecord* record );
	Xt::DomRecord* GetRecord() const;
	
	void SetDefaultCaption(const wxString& caption)	{ m_defaultCaption = caption; }
	void UpdateCaption();
	
	void OnSize( wxSizeEvent& e );
	void OnPaint( wxPaintEvent& e );

private:
	ScarabForm* CreateScarabForm();
	void ResizeScarabFormToFit();

	class C_XtScarabFormContext: public ScarabFormContext
	{
	public:
		C_XtScarabFormContext( C_XtWindow* win ):
			window( win )
		{
		}
		virtual void OpenRecord(Xt::DomRecord* record) {}
		
	private:
		C_XtWindow*		window;
	};

	wxString		m_defaultCaption;
	ScarabForm*		m_scarabForm;
	Xt::DomRecord*	m_record;
};




#endif
