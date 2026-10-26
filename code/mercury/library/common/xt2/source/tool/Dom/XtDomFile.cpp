#include "Common/XtPch.h"
#include <Xt/XtTool.h>

//
// XttExport helpers
//

namespace
{
	inline int XttsLen(const char* text)
	{
		int length = 0;

		for(; *text != '\0'; text++)
			length++;

		return length;
	}

	bool XttEsWrite(XtUt::StreamWriter& f, const char* text)
	{
		return f.Write(text, XttsLen(text));
	}

	bool XttEsWrite(XtUt::StreamWriter& f, const XtString& text)
	{
		return f.Write((const char*)text, text.Length());
	}

	bool XttEswTab(XtUt::StreamWriter& f, int length)
	{
		const int kMaxTabs = 512;
		char tabs[kMaxTabs];

		if (length < 0)
			length = 0;
		else if (length > kMaxTabs)
			length = kMaxTabs;

		memset(tabs, '\t', length);

		return f.Write(tabs, length);
	}

	bool XttEswEncodeWrite(XtUt::StreamWriter& f, const char* text)
	{
		if (!f.Write("\"", sizeof(char)))
			return false;

		const int kMaxBuf = 1024;

		char buf[kMaxBuf];

		char* bufPos = buf;
		char* bufEnd = buf + kMaxBuf;

		for(; *text != '\0'; text++)
		{
			if ((bufPos + 2) > bufEnd)
			{
				if (!f.Write(buf, bufPos - buf))
					return false;

				bufPos = buf;
			}

			switch(*text)
			{
				case '\'': *bufPos++ = '\''; *bufPos++ = '\''; break;
				case '\"': *bufPos++ = '\''; *bufPos++ = '\"'; break;
				case '\n': *bufPos++ = '\''; *bufPos++ = 'n'; break;
				case '\r': *bufPos++ = '\''; *bufPos++ = 'r'; break;
				case '\t': *bufPos++ = '\''; *bufPos++ = 't'; break;
				default: *bufPos++ = *text; break;
			}
		}

		return f.Write(buf, bufPos - buf) && f.Write("\"", sizeof(char));
	}

	bool XttEswIdWrite(XtUt::StreamWriter& f, XtId id)
	{
		const int kMaxBuf = 64;

		char buf[kMaxBuf];

		if (_snprintf_s(buf, sizeof(buf), _TRUNCATE, "%08X", id) < 0)
			return false;

		return XttEsWrite(f, buf);
	}

	bool XttEswXt(XtUt::StreamWriter& f, int indent)
	{
		return XttEswTab(f, indent) && XttEsWrite(f, "xt\r\n");
	}

	bool XttEswXtBundle(XtUt::StreamWriter& f, int indent)
	{
		return XttEswTab(f, indent) && XttEsWrite(f, "xtbundle\r\n");
	}

	bool XttEswComponent(XtUt::StreamWriter& f, int indent, const XtString& name, XtId id, const XtString& type)
	{
		return XttEswTab(f, indent) && XttEsWrite(f, "component ") &&
				XttEsWrite(f, name) && XttEsWrite(f, " ") &&
				XttEswIdWrite(f, id) && XttEsWrite(f, " ") &&
				XttEswEncodeWrite(f, type) && XttEsWrite(f, "\r\n");
	}

	bool XttEswBlock(XtUt::StreamWriter& f, int indent, const XtString& name)
	{
		return XttEswTab(f, indent) && XttEsWrite(f, "block ") && XttEsWrite(f, name) && XttEsWrite(f, "\r\n");
	}

	bool XttEswArray(XtUt::StreamWriter& f, int indent, const XtString& name)
	{
		return XttEswTab(f, indent) && XttEsWrite(f, "array ") && XttEsWrite(f, name) && XttEsWrite(f, "\r\n");
	}

	bool XttEswField(XtUt::StreamWriter& f, int indent, const XtString& name, const XtString& value)
	{
		if (!XttEswTab(f, indent) || !XttEsWrite(f, "field ") || !XttEsWrite(f, name) || !XttEsWrite(f, " "))
			return false;

		return XttEswEncodeWrite(f, value) && XttEsWrite(f, "\r\n");
	}

	bool XttEswRef(XtUt::StreamWriter& f, int indent, const XtString& name, XtId value)
	{
		if (!XttEswTab(f, indent) || !XttEsWrite(f, "ref ") || !XttEsWrite(f, name) || !XttEsWrite(f, " "))
			return false;

		return XttEswIdWrite(f, value) && XttEsWrite(f, "\r\n");
	}

	bool XttEswEnd2(XtUt::StreamWriter& f, int indent)
	{
		return XttEswTab(f, indent) && XttEsWrite(f, ";\r\n");
	}
}

//
// XttDomImport
//

namespace Xt
{

	// Xt::DomBlock::Import2

	DomElement* Import2(const char* filename, XtFileTime* ft, StatusCallback* sc, XtUt::Block* pristine)
	{
		XtSimpleFile f;

		if (!f.Open(filename))
			return 0;

		if (ft)
			*ft = f.LastWriteTime();

		XtUt::MappedFileStorageReader fsr(&f);

		if (!fsr.IsMapped())
			return 0;

		XtUt::WeakBlock mb = fsr.GetBlock();

		XtUt::Ptr<DomElement> element = Parse2(&mb, sc, filename);

		if (!element)
			return 0;

		if (pristine)
		{
			size_t length = f.Length();

			if (!XtUt::Allocate(pristine, length))
				return 0;

			memcpy(pristine->start, mb.start, length);
		}

		return Pop(&element);
	}

	bool Import2(Xt::DomBlock* block, const char* filename, XtFileTime* ft, StatusCallback* sc)
	{
		XtSimpleFile f;

		if (!f.Open(filename))
			return 0;

		if (ft)
			*ft = f.LastWriteTime();

		XtUt::MappedFileStorageReader fsr(&f);

		if (!fsr.IsMapped())
			return 0;

		XtUt::WeakBlock mb = fsr.GetBlock();

		return Parse2(block, &mb, sc, filename);
	}

	// Xt::DomPack::ExportPack

	bool ExportPack(DomPack* pack, const char* filename, XtUt::Block* pristine)
	{
		return Export2(pack, filename, pristine);
	}

	// Xt::DomPack::Import

	DomPack* ImportPack(const char* filename, StatusCallback* status, XtUt::Block* pristine)
	{
		XtFileTime ft;

		XtUt::Ptr<DomElement> dom = Import2(filename, &ft, status, pristine);

		if (!dom || (dom->kind != kDomPack))
			return 0;

		DomPack* pack = (DomPack*)Pop(&dom);

		pack->filename = filename;
		pack->filetime = ft;

		return pack;
	}

}

//
// XtLoadSchema
//

// TODO: Move this...

XtSchema* XtLoadSchema(const char* filename)
{
	XtSimpleFile f;

	if (!f.Open(filename))
		return 0;

	XtUt::Block mb;

	if (ReadAllData(&XtUt::FileStorageReader(&f), &mb))
		return XtImportSchema(mb.start, Capacity(&mb));

	return 0;
}

namespace
{
	bool DoSerialiseElement(Xt::DomElement* element, XtUt::StreamWriter& f, int indent);

	// ** Field **

	bool DoSerialiseField(Xt::DomField* field, XtUt::StreamWriter& f, int indent)
	{
		return XttEsWrite(f, "field ") && XttEswEncodeWrite(f, field->value) && XttEsWrite(f, "\r\n");
	}

	// ** Ref **

	bool DoSerialiseRef(Xt::DomRef* ref, XtUt::StreamWriter& f, int indent)
	{
		return XttEsWrite(f, "ref ") && XttEswIdWrite(f, ref->value) && XttEsWrite(f, "\r\n");
	}

	// ** Block **

	bool DoSerialiseBlockSub(Xt::DomBlock* block, XtUt::StreamWriter& f, int indent)
	{
		for(Xt::DomBlock::Iterator it(block); it; ++it)
		{
			if (!XttEswTab(f, indent + 1) || !XttEsWrite(f, it.Key()) || !XttEsWrite(f, " "))
				return false;

			if (!DoSerialiseElement(it, f, indent + 1))
				return false;
		}

		return XttEswEnd2(f, indent);
	}

	bool DoSerialiseBlock(Xt::DomBlock* block, XtUt::StreamWriter& f, int indent)
	{
		if (!XttEsWrite(f, "block\r\n"))
			return false;

		return DoSerialiseBlockSub(block, f, indent);
	}

	// ** Array **

	bool DoSerialiseArray(Xt::DomArray* array, XtUt::StreamWriter& f, int indent)
	{
		if (!XttEsWrite(f, "array\r\n"))
			return false;

		for(Xt::DomArray::IndexIterator it(array); it; ++it)
		{
			if (!XttEswTab(f, indent + 1) || !XttEswIdWrite(f, it.Key()) || !XttEsWrite(f, " "))
				return false;

			if (!DoSerialiseElement(it, f, indent + 1))
				return false;
		}

		return XttEswEnd2(f, indent);
	}

	// ** Component **

	bool DoSerialiseComponent(Xt::DomComponent* component, XtUt::StreamWriter& f, int indent)
	{
		if (
				!XttEsWrite(f, "component ") ||
				!XttEswIdWrite(f, component->id) ||
				!XttEsWrite(f, " ") ||
				!XttEswEncodeWrite(f, component->type) ||
				!XttEsWrite(f, "\r\n")
			)
		{
			return false;
		}

		return DoSerialiseBlockSub(&component->data, f, indent);
	}

	// ** Record **

	bool DoSerialiseRecord(Xt::DomRecord* record, XtUt::StreamWriter& f, int indent)
	{
		if (
				!XttEsWrite(f, "record ") ||
				!XttEswIdWrite(f, record->id) ||
				!XttEsWrite(f, " ") ||
				!XttEswEncodeWrite(f, record->name) ||
				!XttEsWrite(f, " ") ||
				!XttEswEncodeWrite(f, record->root.type) ||
				!XttEsWrite(f, "\r\n")
			)
		{
			return false;
		}

		return DoSerialiseBlockSub(&record->root.data, f, indent);
	}

	// ** Pack **

	bool DoSerialisePack(Xt::DomPack* pack, XtUt::StreamWriter& f, int indent)
	{
		if (
				!XttEsWrite(f, "pack ") ||
				!XttEswIdWrite(f, pack->id) ||
				!XttEsWrite(f, " ") ||
				!XttEswEncodeWrite(f, pack->name) ||
				!XttEsWrite(f, "\r\n")
			)
		{
			return false;
		}

		// Properties

		if (!XttEswTab(f, indent + 1) || !XttEsWrite(f, "properties\r\n"))
			return false;

		if (!DoSerialiseBlockSub(&pack->properties, f, indent + 1))
			return false;

		// Records

		if (!XttEswTab(f, indent + 1) || !XttEsWrite(f, "records\r\n"))
			return false;

		for(Xt::DomPack::Iterator it(pack); it; ++it)
		{
			if (!XttEswTab(f, indent + 2))
				return false;

			if (!DoSerialiseRecord(it, f, indent + 2))
				return false;
		}

		if (!XttEswEnd2(f, indent + 1))
			return false;

		return XttEswEnd2(f, indent);
	}

	bool DoSerialiseDb(Xt::DomDb* db, XtUt::StreamWriter& f, int indent)
	{
		return false;
	}

	bool DoSerialiseElement(Xt::DomElement* element, XtUt::StreamWriter& f, int indent)
	{
		if (element)
		{
			switch(element->kind)
			{
				case Xt::kDomField:		return DoSerialiseField((Xt::DomField*)element, f, indent);
				case Xt::kDomRef:		return DoSerialiseRef((Xt::DomRef*)element, f, indent);
				case Xt::kDomBlock:		return DoSerialiseBlock((Xt::DomBlock*)element, f, indent);
				case Xt::kDomArray:		return DoSerialiseArray((Xt::DomArray*)element, f, indent);
				case Xt::kDomComponent:	return DoSerialiseComponent((Xt::DomComponent*)element, f, indent);
				case Xt::kDomRecord:	return DoSerialiseRecord((Xt::DomRecord*)element, f, indent);
				case Xt::kDomPack:		return DoSerialisePack((Xt::DomPack*)element, f, indent);
				case Xt::kDomDb:		return DoSerialiseDb((Xt::DomDb*)element, f, indent);
			}
		}

		return false;
	}
}

namespace Xt
{

	bool Serialise2(DomElement* element, XtString& data)
	{
		XtUt::Block mb;

		XtUt::MemoryStorageWriter msw(&mb);
		XtUt::WholeBlockWriter wbw(&msw, 1024 * 1024);
		XtUt::StreamWriter w(&wbw);

		if (!XttEswXt(w, 0))
			return false;

		if (!DoSerialiseElement(element, w, 0))
			return false;

		if (!wbw.Flush(true))
			return false;

		return data.Set((const char*)mb.start, XtUt::Used(&mb));
	}

	bool Export2(DomElement* element, const char* filename, XtUt::Block* pristine)
	{
		XtSimpleFile f;

		if (!f.Open(filename, true))
			return false;

		XtUt::Block mb;
		XtUt::MemoryStorageWriter msw(&mb);

		XtUt::FileStorageWriter fsw(&f);

		XtUt::WholeBlockWriter wbw(pristine ? (XtUt::StorageWriter*)&msw : (XtUt::StorageWriter*)&fsw, 1024 * 1024);
		XtUt::StreamWriter w(&wbw);

		if (!XttEswXt(w, 0))
			return false;

		if (!DoSerialiseElement(element, w, 0))
			return false;

		if (!wbw.Flush(true))
			return false;

		if (pristine)
		{
			if (!fsw.Write(mb.start, XtUt::Used(&mb)))
				return false;

			XtUt::ReplaceData(pristine, &mb);
		}

		return true;
	}

	bool Serialise(Bundle* bundle, XtString& data)
	{
		XtUt::Block mb;

		XtUt::MemoryStorageWriter msw(&mb);
		XtUt::WholeBlockWriter wbw(&msw, 1024 * 1024);
		XtUt::StreamWriter w(&wbw);

		if (!XttEswXtBundle(w, 0))
			return false;

		if (bundle)
		{
			for(Bundle::Iterator it(bundle); it; ++it)
			{
				if (
						!XttEsWrite(w, "\t") ||
						!XttEswEncodeWrite(w, it->type) ||
						!XttEsWrite(w, " ") ||
						!XttEswEncodeWrite(w, it->field) ||
						!XttEsWrite(w, " ") ||
						!DoSerialiseElement(it->element, w, 1)
					)
				{
					return false;
				}
			}
		}

		if (!XttEswEnd2(w, 0))
			return false;

		if (!wbw.Flush(true))
			return false;

		return data.Set((const char*)mb.start, XtUt::Used(&mb));

		return false;
	}

}
