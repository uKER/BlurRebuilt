#ifndef LOCFORM_H
#define LOCFORM_H

#ifdef _MSC_VER
#pragma once
#endif

class LocFormMdiWindow : public MdiWindow
{
public:
	LocFormMdiWindow(class SorceryView& view);
	virtual ~LocFormMdiWindow();

public:
	virtual bool PreCreateWindow(CREATESTRUCT* cs);
	virtual LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);

public:
	void UpdateWindowTitle();
	void AdjustSizeToForm();

public:
	void PopulateFromDom();
	void UpdateFromDom();

private:
	void UpdateItemText(int index, Xt::DomBlock* block);
	void UpdateItemString(int index, Xt::DomBlock* block);

private:
	Xt::DomArray* GetStrings();
	Xt::DomRecord* GetRecord();
	const char* GetWhere();

public:
	void ImportIni(const char* filename);
	void ExportIni(const char* filename);

public:
	void ImportCsv(const char* filename);
	void ExportCsv(const char* filename);

public:
	void ImportTxt(const char* filename);
	void ExportTxt(const char* filename);

public:
	bool SetPair(const XtString& name, const XtString& text, const char* language = 0, bool createIfDoesntExist = true);
	Xt::DomBlock* GetBlockByName(const XtString& name, bool createIfDoesntExist);

public:
	inline SorceryView& View()
	{
		return _view;
	}

private:
	SorceryView&	_view;
	HWND			_list;
	HWND			_combo;
	HWND			_add;
	HWND			_remove;
	HWND			_import;
	HWND			_export;
	HWND			_filter;

private:
	int			_language;
	XtString	_filterText;
};

#endif // LOCFORM_H