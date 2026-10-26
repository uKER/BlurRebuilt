#include "Common/XtPch.h"
#include "Xt/XtTool.h"
#include "XtFlaskHelpers.h"
#include "Xt/Impl/XtLocFile.h"

struct XtLocExpEntry
{
	XtU32	id;
	XtU8*	text;
	int		byteCount;
	int		offset;

public:
	XtLocExpEntry() :
		id(0),
		text(0),
		byteCount(0),
		offset(0)
	{
	}

	~XtLocExpEntry()
	{
		delete [] text;
	}
};

struct XtLocExpLang
{
	XtString					name;
	XtUt::List<XtLocExpEntry>	strings;
	XtU8*						stringTable;
	int							stringTableSize;
	int							fileOffset;
	int							fileSize;

public:
	XtLocExpLang() :
		stringTable(0),
		stringTableSize(0),
		fileOffset(0),
		fileSize(0)
	{
	}

	~XtLocExpLang()
	{
		delete [] stringTable;
	}
};

bool XtLocExport_MakeLang(Xt::DomArray* strings, XtLocExpLang* lang, const XtString& fieldName, bool isUnicode, XtEndian endian)
{
	XtUt::Reserve(&lang->strings, Count(strings));

	for(Xt::DomArray::IndexIterator it(strings); it; ++it)
	{
		const XtString&	field = AsText(At(AsBlock(it), fieldName));

		XtLocExpEntry* entry = AppendEx(&lang->strings, new XtLocExpEntry);

		if (!entry)
			return false;
	
		entry->id = it.Key();

		if (isUnicode)
		{
			Sz16	uni		= MbToWc(field);
			int		length	= (int)uni.Length();

			entry->byteCount	= (length + 1) * 2;
			entry->text			= new XtU8[entry->byteCount];

			if (!entry->text)
				return false;

			wchar_t*		text	= (wchar_t*)entry->text;
			const wchar_t*	p		= uni;

			for(int i = 0; i < length; i++, p++)
				text[i] = EndianSwizzle<XtU16>(*p, endian);

			text[length] = EndianSwizzle<XtU16>(L'\0', endian);
		}
		else
		{
			entry->byteCount = field.Length() + 1;

			if ((entry->text = new XtU8[entry->byteCount]) == 0)
				return false;

			memcpy(entry->text, (const char*)field, entry->byteCount);
		}
	}

	return true;
}

bool XtLocExport_Sort_ById(const XtLocExpEntry* lhs, const XtLocExpEntry* rhs)
{
	return lhs->id < rhs->id;
}

bool XtLocExport_Sort_ByText(const XtLocExpEntry* lhs, const XtLocExpEntry* rhs)
{
	int result = memcmp(lhs->text, rhs->text, XtUt::Min(lhs->byteCount, rhs->byteCount));

	return result ? (result < 0) : (lhs->byteCount < rhs->byteCount);
}

bool XtLocExport_Sort(XtLocExpLang* lang, bool byId)
{
	if (!byId)
		return XtUt::MergeSort(&lang->strings, XtLocExport_Sort_ByText);

	return XtUt::MergeSort(&lang->strings, XtLocExport_Sort_ById);
}

bool XtLocExport_MakeStringTable(XtLocExpLang* lang)
{
	lang->stringTableSize = 0;

	for(size_t i = 0; i < lang->strings.count; i++)
		lang->stringTableSize += lang->strings[i]->byteCount;

	if ((lang->stringTable = new XtU8[lang->stringTableSize]) == 0)
		return false;

	int offset = 0;

	for(size_t i = 0; i < lang->strings.count; i++)
	{
		XtLocExpEntry* entry = lang->strings[i];

		memcpy(lang->stringTable + offset, entry->text, entry->byteCount);

		entry->offset = offset;

		offset += entry->byteCount;
	}

	assert(offset <= lang->stringTableSize);

	return true;
}

bool XtLocExport(XtTargetProfile* profile, XtUt::Array<XtString>* expLangs, Xt::DomRecord* record, XtUt::Block* data)
{
	XtEndian endian = profile->endian;

	if (!record || !data)
		return false;

	Xt::DomArray* strings = AsArray(At(&record->root.data, "strings"));

	if (!strings)
		return false;

	XtUt::Array<XtLocExpLang> langs;

	if (!XtUt::Allocate(&langs, expLangs->count + 1, expLangs->count + 1))
		return false;

	langs[0].name = "name";

	for(int i = 0; i < (int)expLangs->count; i++)
		langs[i + 1].name = *At(expLangs, i);

	for(int i = 0; i < (int)langs.count; i++)
	{
		XtLocExpLang* lang = &langs[i];

		if (!XtLocExport_MakeLang(strings, lang, lang->name, i != 0, endian))
			return false;

		if (!XtLocExport_Sort(lang, i != 0))
			return false;

		if (!XtLocExport_MakeStringTable(lang))
			return false;
	}

	// Measure

	int totalSize = 0;
	
	totalSize += sizeof(XtLocHeader);
	
	int tocOffset	= totalSize;				totalSize += sizeof(XtLocToc);
	int langOffset	= totalSize - tocOffset;	totalSize += sizeof(XtLocLang) * (langs.count - 1);

	for(int i = 0; i < (int)langs.count; i++)
	{
		XtLocExpLang* lang = &langs[i];

		int size = 0;

		size += sizeof(XtLocTable);
		size += sizeof(XtLocEntry) * lang->strings.count;
		size += lang->stringTableSize;

		lang->fileOffset	= totalSize;
		lang->fileSize		= size;

		totalSize += size;
	}

	// Allocate

	if (!XtUt::Allocate(data, totalSize))
		return false;

	XtUt::Clear(data);

	// Setup

	XtLocHeader* header = (XtLocHeader*)(data->start);

	header->tag			= EndianSwizzle<XtU32>(kXtLocTag, endian);
	header->version		= EndianSwizzle<XtU32>(1, endian);
	header->toc.offset	= EndianSwizzle<XtU32>(tocOffset, endian);
	header->toc.size	= EndianSwizzle<XtU32>(langs[0].fileOffset + langs[0].fileSize - tocOffset, endian);

	XtLocToc* toc = (XtLocToc*)(data->start + tocOffset);

	toc->langOffset		= EndianSwizzle<XtU32>(langOffset, endian);
	toc->langCount		= EndianSwizzle<XtU32>(langs.count - 1, endian);
	toc->lookupOffset	= EndianSwizzle<XtU32>(langs[0].fileOffset - tocOffset, endian);

	for(int i = 0; i < (int)langs.count - 1; i++)
	{
		XtLocExpLang*	exp		= &langs[i + 1];
		XtLocLang*		lang	= &((XtLocLang*)(data->start + tocOffset + langOffset))[i];

		_snprintf_s(lang->name, sizeof(lang->name), _TRUNCATE, "%s", (const char*)exp->name);

		lang->data.offset	= EndianSwizzle<XtU32>(exp->fileOffset, endian);
		lang->data.size		= EndianSwizzle<XtU32>(exp->fileSize, endian);
	}

	for(int i = 0; i < (int)langs.count; i++)
	{
		XtLocExpLang*	expLang	= &langs[i];
		XtLocTable*		table	= (XtLocTable*)(data->start + expLang->fileOffset);

		table->count	= EndianSwizzle<XtU32>(expLang->strings.count, endian);
		table->offset	= EndianSwizzle<XtU32>(sizeof(XtLocTable), endian);

		XtLocEntry*	entries			= (XtLocEntry*)(((XtU8*)table) + sizeof(XtLocTable));
		int			stringsOffset	= sizeof(XtLocTable) + (sizeof(XtLocEntry) * expLang->strings.count);

		for(size_t j = 0; j < expLang->strings.count; j++)
		{
			XtLocExpEntry*	expEntry	= expLang->strings[j];
			XtLocEntry*		entry		= &entries[j];

			entry->id		= EndianSwizzle<XtU32>(expEntry->id, endian);
			entry->offset	= EndianSwizzle<XtU32>(stringsOffset + expEntry->offset, endian);
		}

		memcpy(data->start + expLang->fileOffset + stringsOffset, expLang->stringTable, expLang->stringTableSize);
	}

	return true;
}