#include "core/stdafx.h"

#pragma warning(disable:4355)
#include "xt\xttool.h"

#include "XtWin/WndCore.h"
#include "Application/SorceryApp.h"
#include "Application/SorceryView.h"

#include "LocCSVParser.h"
#include "LocForm.h"

#include <wchar.h>




char *MkStr(char *format, ...)
{
	static char dst[256];

	va_list argptr;
	va_start(argptr, format);

	vsprintf_s(dst, 256, format, argptr);

	va_end(argptr);

	return dst;
}


CLocCSVParser::CLocCSVParser() :
	mBuf(0),
	mBufPtr(0),
	mBufPtrW(0),
	mUnicode(false)
{
}

CLocCSVParser::~CLocCSVParser()
{
	delete []mBuf;
}

inline WCHAR CLocCSVParser::NextChar()
{
	if (mUnicode)
		return *(mBufPtrW++);
	else
		return btowc(*(mBufPtr++));
}

inline void CLocCSVParser::SkipChar()
{
	if (mUnicode)
		mBufPtrW++;
	else
		mBufPtr++;
}

inline WCHAR CLocCSVParser::PeekChar()
{
	if (mUnicode)
		return *mBufPtrW;
	else
		return btowc(*mBufPtr);
}

bool CLocCSVParser::EndOfLine()
{
	if (PeekChar() != L'\r' && PeekChar() != 0)
		return false;

	return true;
}

bool CLocCSVParser::EndOfFile()
{
	if (PeekChar() != 0)
		return false;

	return true;
}

bool CLocCSVParser::SkipToNextLine()
{
	while (!EndOfLine())
		SkipChar();

	if (!EndOfFile())
	{
		SkipChar();
		if (PeekChar() == L'\n')
			SkipChar();
	}

	return true;
}

XtString CLocCSVParser::ReadParam()
{
	XtString param;

	bool quote = false;

	const int bufSize = 64;
	WCHAR buf[bufSize];
	int bufIndex = 0;

	for (; !EndOfLine(); )
	{
		WCHAR str;
		str = NextChar();

		if (str == L'\"')
		{
			if (quote && PeekChar() == L'\"')
				SkipChar();
			else
			{
				quote = !quote;
				continue;
			}
		}

		if (str == L',' && !quote)
			break;

		buf[bufIndex++] = str;
		if (bufIndex == bufSize)
		{
			param += WcToMb(Sz16(buf, bufSize));
			bufIndex = 0;
		}
	}

	if (bufIndex)
		param += WcToMb(Sz16(buf, bufIndex));

	return param;
}

bool CLocCSVParser::LoadFile(const char* fileName)
{
	FILE* f;

	//if (fopen_s(&f, fileName, "rt, ccs=UNICODE") != 0)
	if (fopen_s(&f, fileName, "rb") != 0)
		return false;

	fpos_t pos;
	fseek(f, 0, SEEK_END);
	fgetpos(f, &pos);
	fseek(f, 0, SEEK_SET);

	mBufferSize = (int) pos;
	mBuf = new BYTE[mBufferSize+2];
	mBuf[mBufferSize] = 0;
	mBuf[mBufferSize+1] = 0;
	int read = (int) fread(mBuf, 1, mBufferSize, f);

	fclose(f);

	if (read != mBufferSize)
		return false;

	mBufPtr = (CHAR*) mBuf;
	mBufPtrW = (WCHAR*) mBuf;

	if (*mBufPtrW == 0xfffe || *mBufPtrW == 0xfeff)
	{
		mUnicode = true;
		mBufPtrW++;	// Skip Unicode header
	}

	return true;
}

bool CLocCSVParser::Parse(AsyncOp* op, LocFormMdiWindow* locForm)
{
	mAsyncOp	= op;
	mLocForm	= locForm;

	ReadFirstLine();

	while (!EndOfFile())
	{
		ReadLine();
	}

	return true;
}

bool CLocCSVParser::ReadFirstLine()
{
	ReadParam();	// "id"

	while (!EndOfLine())
	{
		XtString lang = ReadParam();

		bool exists = false;
		for(XtUt::List<Xt::DomLanguage>::Iterator it(&mLocForm->View()._app._db->languages); it; ++it)
		{
			if (_stricmp(lang, it->name) == 0)
				exists = true;
		}

		if (!_stricmp(lang, "comment"))
			exists = true;

		XtUt::AppendEx(&mLanguages, exists ? lang : "");

		if (!exists)
			mAsyncOp->NewMessage(kAoiWarning, "Ignoring language: " + lang);
	}

	SkipToNextLine();

	mAsyncOp->NewMessage(kAoiInfo, "Importing:");

	for (XtUt::Array<XtString>::Iterator it(&mLanguages); it; ++it)
	{
		XtString lang = *it;

		if (*lang)
			mAsyncOp->AppendMessage(lang);
	}

	return true;
}

bool CLocCSVParser::ReadLine()
{
	XtString idu = ReadParam();
	XtString id = idu;

	Xt::DomBlock* block = mLocForm->GetBlockByName(id, false);

	if (block)
	{
		//DebugLn("[%s]", (const char*) id);

		for (XtUt::Array<XtString>::Iterator it(&mLanguages); it; ++it)
		{
			XtString lang = *it;

			XtString string = ReadParam();

			if (*lang)
				Set(block, lang, string);
		}
	}
	else
		mAsyncOp->NewMessage(kAoiWarning, "ID [" + id + "] - Not Found");

	SkipToNextLine();

	int pos;
	if (mUnicode)
		pos = (int) ((BYTE*) mBufPtrW - mBuf);
	else
		pos = (int) ((BYTE*) mBufPtr - mBuf);

	mAsyncOp->SetProgress((float) pos / (float) mBufferSize);

	return true;
}
