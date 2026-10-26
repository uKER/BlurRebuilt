#ifndef XTLOCHACK_H
#define XTLOCHACK_H

#ifdef _MSC_VER
#pragma once
#endif

#include <system/serialise/I_File.h>
#include <xt/xtgame.h>
#include <xt/impl/xtlocfile.h>

struct XtLocBlock;
struct XtLocToc;
struct XtLocLang;
struct XtLocTable;
struct XtLocEntry;

class XtLocHack
{
public:
	XtLocHack();
	~XtLocHack();

public:
	bool Load(const Sys::C_FilePath& filename, const char* language, Sys::I_FileSystem* fs = NULL);
	bool LoadFromMemory(const U8* pMemory, const U32 size, const char* language);

	void Free();

public:
	XtU32			Lookup_Count();
	XtU32			Lookup_ID(XtU32 index);
	const char*		Lookup_Name(XtU32 index);

	XtU32			FindId(const char* name);
	const char*		FindName(XtU32 id);
	const wchar_t*	FindText(XtU32 id, bool null_if_not_found = false);

private:
#ifdef _USE_BIZFILE
	XtU8* ReadBlock(BizFile& f, XtLocBlock* block);
#else
	XtU8* ReadBlock(Sys::I_File* f, XtLocBlock* block);
#endif

private:
	struct Table
	{
		XtU32		count;
		XtLocEntry*	entries;
		XtU8*		base;
	};

private:
	XtLocLang*			FindLang(const char* language) const;
	const XtLocEntry*	FindEntry(XtU32 id);
	void				Setup(Table* table, XtLocTable* loc);

private:
	XtU8* _dataToc;
	XtU8* _dataLang;

private:
	XtLocToc*	_toc;
	XtLocLang*	_langs;
	
	Table		_lookup;
	Table		_translation;
};

#endif // XTLOCHACK_H
