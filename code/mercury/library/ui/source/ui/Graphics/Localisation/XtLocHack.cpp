#include "stdafx.h"

#include <system/DebugLog.h>

#ifdef _USE_BIZFILE
#include "system\resource\bizfile.h"
#else
#include "ui/UiInterface.h"
#endif

#include "ui/Graphics/Localisation/XtLocHack.h"

XtLocHack::XtLocHack()
{
	_dataToc = NULL;
	_dataLang = NULL;

	Free();
}

XtLocHack::~XtLocHack()
{
	Free();
}

bool XtLocHack::Load(const Sys::C_FilePath& filename, const char* language, Sys::I_FileSystem* fs )
{
	//Free();

#ifdef _USE_BIZFILE
	BizFile f;
	if (!f.OpenDD(filename, false))
		return false;
#else
	if ( !fs )
		fs = Sys::GetDefaultFileSystem();

	if ( !fs )
		return false;

	Sys::I_File* f;

	f = fs->Open( filename );
	if(!f)
		return false;
#endif

	XtLocHeader header;
	
#ifdef _USE_BIZFILE
	if (!f.Read(&header, sizeof(header)))
		return false;
#else
	if(!f->Read(&header, sizeof(header)))
		return false;
#endif

	if ((header.tag != kXtLocTag) || (header.version != 1))
		return false;

	if ((_dataToc = ReadBlock(f, &header.toc)) == 0)
		return false;

	_toc	= (XtLocToc*)(_dataToc);
	_langs	= (XtLocLang*)(_dataToc + _toc->langOffset);

	Setup(&_lookup, (XtLocTable*)(_dataToc + _toc->lookupOffset));

	XtLocLang* lang = FindLang(language);

	if (!lang)
		return false;

	_dataLang = ReadBlock(f, &lang->data);

	if (!_dataLang)
		return false;

	Setup(&_translation, (XtLocTable*)_dataLang);

#ifndef _USE_BIZFILE
	// Clean up
	delete f;
#endif

	return true;
}

bool XtLocHack::LoadFromMemory(const U8* pMemory, const U32 size, const char* language)
{
	assert(pMemory);
	assert(size);

	XtLocHeader* pHeader =  (XtLocHeader*)(pMemory);
	if ((pHeader->tag != kXtLocTag) || (pHeader->version != 1))
	{
		assert(!"Cant load in sorcery strings from memory");
		return false;
	}

	// allocate table
	XtU8* tableData = new XtU8[pHeader->toc.size];
	if (NULL == tableData)
		return false;

	memcpy(tableData, pMemory+pHeader->toc.offset, pHeader->toc.size);
	_dataToc = tableData;

	_toc	= (XtLocToc*)(_dataToc);
	_langs	= (XtLocLang*)(_dataToc + _toc->langOffset);

	Setup(&_lookup, (XtLocTable*)(_dataToc + _toc->lookupOffset));

	XtLocLang* lang = FindLang(language);

	if (!lang)
		return false;

	XtU8 * langData = new XtU8[lang->data.size];
	if (NULL == langData)
		return false;
	
	// copy data from original buffer.
	memcpy(langData, pMemory+lang->data.offset, lang->data.size);

	_dataLang = langData;

	Setup(&_translation, (XtLocTable*)_dataLang);

	return true;
}

void XtLocHack::Free()
{
	_toc		= 0;
	_langs		= 0;

	delete [] _dataToc;
	delete [] _dataLang;
	_dataToc		= NULL;
	_dataLang	= NULL;

	_lookup.count	= 0;
	_lookup.entries	= 0;
	_lookup.base	= 0;

	_translation.count		= 0;
	_translation.entries	= 0;
	_translation.base		= 0;
}

XtU32 XtLocHack::FindId(const char* name)
{
	int top		= 0;
	int bottom	= _lookup.count;

	while(top < bottom)
	{
		int			middle		= (top + bottom) / 2;
		XtLocEntry*	entry		= &_lookup.entries[middle];
		const char* entryName	= (const char*)(_lookup.base + entry->offset);
		int			result		= strcmp(entryName, name);

		if (result < 0)
			top = middle + 1;
		else if (result > 0)
			bottom = middle;
		else
			return entry->id;
	}

	// PR - for platform specific strings code-side, the above search will fail.
	// -- so add on the suffix and search again! Hmmmm ...

	char pName[ 512 ];	// should be plentiful
	pName[ 0 ] = '\0';

	#ifdef PLATFORM_PS3
	sprintf_s( pName, 512, "%s%s", name, "_PS3" );
	#endif
	#ifdef PLATFORM_PC
	sprintf_s( pName, 512, "%s%s", name, "_PC" );
	#endif

	// BOOOOooooo at copying and pasting code!

	top		= 0;
	bottom	= _lookup.count;

	while(top < bottom)
	{
		int			middle		= (top + bottom) / 2;
		XtLocEntry*	entry		= &_lookup.entries[middle];
		const char* entryName	= (const char*)(_lookup.base + entry->offset);
		int			result		= strcmp(entryName, pName);

		if (result < 0)
			top = middle + 1;
		else if (result > 0)
			bottom = middle;
		else
			return entry->id;
	}

	return 0;
}

const wchar_t* XtLocHack::FindText(XtU32 id, const bool null_if_not_found)
{
	const XtLocEntry* entry = FindEntry( id );

	if( !entry )
		return null_if_not_found ? NULL : L"";

	return (const wchar_t*)(_translation.base + entry->offset);
}

const char* XtLocHack::FindName(XtU32 id)
{
	// SLOW SLOW SLOW
	for(XtU32 i = 0; i < _lookup.count; i++)
	{
		XtLocEntry*	entry		= &_lookup.entries[ i ];
		if( entry->id == id )
			return (const char*)(_lookup.base + entry->offset);
	}

	return NULL;
}

const XtLocEntry* XtLocHack::FindEntry(XtU32 id)
{
	int top		= 0;
	int bottom	= _translation.count;

	while(top < bottom)
	{
		int			middle	= (top + bottom) / 2;
		XtLocEntry*	entry	= &_translation.entries[middle];

		if (entry->id < id)
			top = middle + 1;
		else if (entry->id > id)
			bottom = middle;
		else
			return entry;
	}

	return NULL;
}

#ifdef _USE_BIZFILE
XtU8* XtLocHack::ReadBlock(BizFile& f, XtLocBlock* block)
#else
XtU8* XtLocHack::ReadBlock(Sys::I_File* f, XtLocBlock* block)
#endif
{
	if (block->size > (4 * 1024 * 1024))
		return 0;

	XtU8* data = new XtU8[block->size];

	if (!data)
		return 0;

#ifdef _USE_BIZFILE
	f.SetFilePointer(block->offset, SEEK_SET);

	if (!f.Read(data, block->size))
#else
	f->SetPosition((Sys::C_StreamPos)block->offset);

	if (!f->Read(data, block->size))
#endif
	{
		delete [] data;
		return 0;
	}

	return data;
}

XtLocLang* XtLocHack::FindLang(const char* language) const
{
	if (!_toc || !_langs)
		return 0;

	for(XtU32 i = 0; i < _toc->langCount; i++)
	{
		if (stricmp(_langs[i].name, language) == 0)
			return &_langs[i];
	}

	// PR - if the language set to isn't supported we need to return the default Loc table that is loaded
	return &_langs[ 0 ];
}

void XtLocHack::Setup(Table* table, XtLocTable* loc)
{
	table->count	= loc->count;
	table->entries	= (XtLocEntry*)(((XtU8*)loc) + loc->offset);
	table->base		= (XtU8*)loc;
}

void XtLocLoad()
{
	XtLocHack loc;

	loc.Load("game:\\BLEHstrings.loc", "english");

	XtU32			id		= loc.FindId("IDS_YES");
	const wchar_t*	text	= loc.FindText(id);
#ifndef PLATFORM_PS3
	int j = 0;
#endif
}

// DG - Lookup for building LUA tables
XtU32 XtLocHack::Lookup_Count()
{
	return _lookup.count;
}

XtU32 XtLocHack::Lookup_ID(XtU32 index)
{
	XtLocEntry*	entry = &_translation.entries[index];

	return entry->id;
}

const char*	XtLocHack::Lookup_Name(XtU32 index)
{
	XtLocEntry*	entry		= &_lookup.entries[index];
	const char* entryName	= (const char*)(_lookup.base + entry->offset);
	return entryName;
}
