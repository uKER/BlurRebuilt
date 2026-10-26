#include "Core/StdAfx.h"
#include "XtWin/Core.h"
#include "Application/SorceryApp.h"
#include "Application/SorceryView.h"
#include "Xt/XtTool.h"
#include "LocForm/LocForm.h"

int MyIsSpace(wint_t c)
{
	return (c != '\r') && (c != '\n') && iswspace(c);
}

wchar_t* TranslateToUnicode(const char* ansiText)
{
	Sz16 out = MbToWc(ansiText);

	int length = out.Length();

	int			byteCount	= (length + 1) * 2;
	wchar_t*	uniText		= new wchar_t[byteCount];

	if (!uniText)
		return 0;

	memcpy(uniText, out.Text(), byteCount);

	return uniText;
}

void LocFormMdiWindow::ImportIni(const char* filename)
{
	XtSimpleFile f;

	if (!f.Open(filename))
		return;

	XtUt::Block mb;

	if (!ReadAllData(&XtUt::FileStorageReader(&f), &mb))
		return;

	wchar_t* p = (wchar_t*)mb.start;
	wchar_t* e = (wchar_t*)mb.end;

	wchar_t* idStart		= 0;
	wchar_t* idEnd		= 0;
	wchar_t* valueStart	= 0;
	wchar_t* valueEnd	= 0;

	if ((*p == 0xFFFE) || (*p == 0xFEFF))
		p++;

	while(p < e)
	{
		while((p < e) && MyIsSpace(*p))
			p++;

		idStart = p;

		while((p < e) && !MyIsSpace(*p) && (*p != '='))
			p++;

		idEnd = p;

		while((p < e) && MyIsSpace(*p))
			p++;

		if ((p < e) && (*p == '='))
		{
			p++;

			while((p < e) && MyIsSpace(*p))
				p++;
			
			valueStart = p;
		}
		else
			valueStart = 0;

		while((p < e) && ((*p != '\r') && (*p != '\n')))
			p++;

		valueEnd = p;

		while((p < e) && ((*p == '\r') || (*p == '\n')))
			p++;

		if (valueStart)
		{
			while((valueStart < valueEnd) && MyIsSpace(*(valueEnd - 1)))
				valueEnd--;

			XtString id = Format("%.*S", idEnd - idStart, idStart);

			if (XtsIsIdentifier(id))
			{
				XtString value = WcToMb(Sz16(valueStart, (int)(valueEnd - valueStart)));

				SetPair(id, value, 0, false);
			}
		}
	}
}

void LocFormMdiWindow::ExportIni(const char* filename)
{
	FILE* f;

	if (fopen_s(&f, filename, "wt, ccs=UNICODE") != 0)
	{
		MessageBox(*this, XtString("Unable to open file:\r\n") + filename, _T("Sorcery"), MB_ICONERROR | MB_OK);
		return;
	}

	const char* where = GetWhere();

	if (Xt::DomArray* strings = GetStrings())
	{
		for(Xt::DomArray::IndexIterator it(strings); it; ++it)
		{
			Xt::DomBlock* block = AsBlock(it);

			if (!block)
				continue;

			const char* name = AsText(At(block, "name"));
			const char* text = AsText(At(block, where));

			if (*name == '\0')
				continue;

			wchar_t* uniText = TranslateToUnicode(text);

			if (uniText)
				fwprintf(f, L"%S=%s\n", name, uniText);

			delete [] uniText;
		}
	}

	fclose(f);
}