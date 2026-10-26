#include "Core/StdAfx.h"
#include "XtWin/Core.h"
#include "Application/SorceryApp.h"
#include "Application/SorceryView.h"
#include "Xt/XtTool.h"
#include "LocForm/LocForm.h"
#include "LocForm/LocCSVParser.h"
#include <io.h>

wchar_t* TranslateToUnicode(const char* ansiText);

struct ImportTxtCallbackContext
{
	XtString filename;
	LocFormMdiWindow* locForm;
};

void RemoveNewLines(wchar_t* str)
{
	for(; *str != L'\0'; str++)
	{
		switch(*str)
		{
			case L'\r':
			case L'\n':
			case L'\t':
				*str = L' ';
			break;
		}
	}
}

wchar_t* TokHack(wchar_t* str, const wchar_t* delim, wchar_t** ctx)
{
	if (!str)
		str = *ctx;
	else
		*ctx = 0;

	wchar_t* tok = str;

	if (*tok == L'\0')
	{
		*ctx = 0;
		return 0;
	}

	for(; *str != L'\0'; str++)
	{
		for(const wchar_t* p = delim; *p != L'\0'; p++)
		{
			if (*p == *str)
			{
				*str = '\0';
				*ctx = str + 1;
				return tok;
			}
		}
	}

	*ctx = str;
	return tok;
}

void ImportTextRemoveExtraQuotes(wchar_t* text)
{
	if (*text != L'\"')
		return;

	for(const wchar_t* p = text + 1; *p != L'\0'; p++)
	{
		if ((p[0] == L'\"') && (p[1] == L'\0'))
			break;

		if ((p[0] == L'\"') && (p[1] == L'\"'))
			p++;

		*text++ = *p;
	}

	*text = L'\0';
}

bool ImportTxtCallback(AsyncOp* op, ImportTxtCallbackContext* ctx)
{
	op->SetTitle("Importing localisation text...");

	FILE* f;

	if (fopen_s(&f, ctx->filename, "rt, ccs=UTF-8") != 0)
	{
		op->NewMessage(kAoiError, "Unable to open file:");
		op->AppendMessage(ctx->filename);
		return false;
	}

	setvbuf(f, 0, _IOFBF, 48 * 1024);

	wchar_t buf[8192];

	XtUt::Array<XtString> languages;
	XtUt::Array<XtString> bad;

	int fileLength = _filelength(_fileno(f));
	int filePos = 0;

	for(int line = 0; !feof(f); line++)
	{
		if (fgetws(buf, sizeof(buf) / sizeof(wchar_t), f) == 0)
			break;

		filePos += (int)wcslen(buf);

		if (line == 0)
		{
			wchar_t* tokCtx	= 0;
			wchar_t* tok	= TokHack(buf, L"\t\n", &tokCtx);

			int validCount = 0;

			for(int col = 0; tok; col++, tok = TokHack(0, L"\t\n", &tokCtx))
			{
				if (col == 0)
					continue;

				char lang[64];
				_snprintf_s(lang, sizeof(lang), _TRUNCATE, "%S", tok);

				bool exists = false;

				if (!_stricmp(lang, "comment"))
					exists = true;
				else
				{
					for(XtUt::List<Xt::DomLanguage>::Iterator it(&ctx->locForm->View()._app._db->languages); it; ++it)
					{
						if (_stricmp(lang, it->name) == 0)
							exists = true;
					}
				}

				if (!XtUt::AppendEx(&languages, XtString(exists ? lang : "")))
				{
					op->NewMessage(kAoiError, "Out of memory!");
					return false;
				}

				if (!exists)
				{
					if (!XtUt::AppendEx(&bad, XtString(lang)))
					{
						op->NewMessage(kAoiError, "Out of memory!");
						return false;
					}
				}
				else
					validCount++;
			}

			if (bad.count > 0)
			{
				op->NewMessage(kAoiWarning, "Ignoring:");

				for (XtUt::Array<XtString>::Iterator it(&bad); it; ++it)
					op->AppendMessage(*it);
			}

			if (validCount > 0)
			{
				op->NewMessage(kAoiInfo, "Importing:");

				for (XtUt::Array<XtString>::Iterator it(&languages); it; ++it)
				{
					if (**it)
						op->AppendMessage(*it);
				}
			}
			else
			{
				op->NewMessage(kAoiError, "No languages to import!");
				return false;
			}
		}
		else
		{
			wchar_t* tokCtx	= 0;
			wchar_t* tok	= TokHack(buf, L"\t\n", &tokCtx);

			int validCount = 0;

			Xt::DomBlock* block = 0;

			for(int col = 0; tok; col++, tok = TokHack(0, L"\t\n", &tokCtx))
			{
				if (col == 0)
				{
					char id[256];
					_snprintf_s(id, sizeof(id), _TRUNCATE, "%S", tok);

					if (!*id)
						break;

					if ((block = ctx->locForm->GetBlockByName(id, false)) == 0)
					{
						op->NewMessage(kAoiWarning, XtString("Id [") + id + "] - Not Found");
						break;
					}

					continue;
				}
				else if ((col >= 1) && (col <= (int)languages.count) && block)
				{
					if (*languages[col - 1])
					{
						ImportTextRemoveExtraQuotes(tok);

						XtString encoded = WcToMb(Sz16(tok, (int)wcslen(tok)));

						Set(block, languages[col - 1], encoded);
					}
				}
			}
		}

		op->SetProgress((float)filePos / (float)fileLength);
	}

	fclose(f);

	op->SetTitle("Import done");
	op->NewMessage(kAoiInfo, "Import done.");
	op->SetProgress(1.0f);

	return true;
}

void LocFormMdiWindow::ImportTxt(const char* filename)
{
	ImportTxtCallbackContext ctx;

	ctx.filename = filename;
	ctx.locForm = this;

	void NetSendHackPause();
	void NetSendHackResume();

	NetSendHackPause();

	AsyncInvoke(_view._app._window, ImportTxtCallback, &ctx);

	NetSendHackResume();
}

void LocFormMdiWindow::ExportTxt(const char* filename)
{
	FILE* f;

	if (fopen_s(&f, filename, "wt, ccs=UTF-8") != 0)
	{
		MessageBox(*this, XtString("Unable to open file:\r\n") + filename, _T("Sorcery"), MB_ICONERROR | MB_OK);
		return;
	}

	Xt::DomDb* db = _view._app._db;

	fwprintf(f, L"id\t");

	for(XtUt::List<Xt::DomLanguage>::Iterator it(&db->languages); it; ++it)
		fwprintf(f, L"%S\t", (const char*)it->name);

	fwprintf(f, L"comment\n");

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

			fwprintf(f, L"%S\t", name);

			for(XtUt::List<Xt::DomLanguage>::Iterator it(&db->languages); it; ++it)
			{
				const char*	text	= AsText(At(block, it->name));
				wchar_t*	uniText	= TranslateToUnicode(text);

				RemoveNewLines(uniText);

				fwprintf(f, L"%s\t", uniText);

				delete [] uniText;
			}

			const char*	textComment		= AsText(At(block, "comment"));
			wchar_t*	uniTextComment	= TranslateToUnicode(textComment);

			RemoveNewLines(uniTextComment);

			fwprintf(f, L"%s\n", uniTextComment);

			delete [] uniTextComment;
		}
	}

	fclose(f);
}