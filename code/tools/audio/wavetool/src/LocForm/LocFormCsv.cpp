#include "Core/StdAfx.h"
#include "XtWin/Core.h"
#include "Application/SorceryApp.h"
#include "Application/SorceryView.h"
#include "Xt/XtTool.h"
#include "LocForm/LocForm.h"
#include "LocForm/LocCSVParser.h"

wchar_t* TranslateToUnicodeCsv(const char* ansiText)
{
	Sz16 out = MbToWc(ansiText);

	int length = out.Length();

	int			byteCount	= (length + 1) * 2 * 2;
	wchar_t*	uniText		= new wchar_t[byteCount];

	if (!uniText)
		return 0;

	const wchar_t* s = out.Text();
	wchar_t* d = uniText;

	for(int c = length; c > 0; c--, s++)
	{
		*d++ = *s;

		if (*s == L'\"')
			*d++ = '\"';
	}

	*d = L'\0';

	return uniText;
}

struct ImportCsvCallbackContext
{
	XtString filename;
	LocFormMdiWindow* locForm;
};

bool ImportCsvCallback(AsyncOp* op, ImportCsvCallbackContext* ctx)
{
	CLocCSVParser csv;

	op->SetTitle("Importing localisation text...");

	if (!csv.LoadFile(ctx->filename))
	{
		op->NewMessage(kAoiError, "Unable to open file:");
		op->AppendMessage(ctx->filename);
		return false;
	}

	csv.Parse(op, ctx->locForm);

	op->SetTitle("Import done");
	op->NewMessage(kAoiInfo, "Import done.");
	op->SetProgress(1.0f);

	return true;
}

void LocFormMdiWindow::ImportCsv(const char* filename)
{
	ImportCsvCallbackContext ctx;

	ctx.filename = filename;
	ctx.locForm = this;

	void NetSendHackPause();
	void NetSendHackResume();

	NetSendHackPause();

	AsyncInvoke(_view._app._window, ImportCsvCallback, &ctx);

	NetSendHackResume();
}

void LocFormMdiWindow::ExportCsv(const char* filename)
{
	FILE* f;

	if (fopen_s(&f, filename, "wt, ccs=UNICODE") != 0)
	{
		MessageBox(*this, XtString("Unable to open file:\r\n") + filename, _T("Sorcery"), MB_ICONERROR | MB_OK);
		return;
	}

	Xt::DomDb* db = _view._app._db;

	fwprintf(f, L"\"id\",");

	for(XtUt::List<Xt::DomLanguage>::Iterator it(&db->languages); it; ++it)
		fwprintf(f, L"\"%S\",", (const char*)it->name);

	fwprintf(f, L"\"comment\"\n");

	if (Xt::DomArray* strings = GetStrings())
	{
		for(Xt::DomArray::IndexIterator it(strings); it; ++it)
		{
			Xt::DomBlock* block = AsBlock(it);

			if (!block)
				continue;

			const char* name = AsText(At(block, "name"));

			if (*name == '\0')
				continue;

			fwprintf(f, L"\"%S\",", name);

			for(XtUt::List<Xt::DomLanguage>::Iterator it(&db->languages); it; ++it)
			{
				const char*	text	= AsText(At(block, it->name));
				wchar_t*	uniText	= TranslateToUnicodeCsv(text);

				fwprintf(f, L"\"%s\",", uniText);

				delete [] uniText;
			}

			const char*	textComment		= AsText(At(block, "comment"));
			wchar_t*	uniTextComment	= TranslateToUnicodeCsv(textComment);

			fwprintf(f, L"\"%s\"\n", uniTextComment);

			delete [] uniTextComment;
		}
	}

	fclose(f);
}