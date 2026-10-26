#include "Common/XtPch.h"
#include "XtGameImpl.h"

#if defined(PLATFORM_PC) || defined(PLATFORM_PS3)

#if defined(PLATFORM_PS3)
#include <cell/atomic.h>
#include <sys/synchronization.h>
#endif

#include <System/Artery/Artery.h>
#include <System/Artery/ArterySync.h>
#include <System/Debug/Variables.h>

#include <Xt/Impl/XtUt.h>

#define ARTERY_FOURCC(a, b, c, d) ((a << 24) | (b << 16) | (c << 8) | d)

struct XtNetMsg
{
	ArteryMessage msg;
};

XtDb*					gXtNetworkDb;
ArteryLock				gXtNetCs;
XtUt::List<XtNetMsg>	gXtNetMsgs;

void XtNetProc(ArteryConnection con, void* context)
{
	// TODO: Potential unbounded cross thread access here; but in practice shouldn't happen

	XtU8*	data	= 0;
	int		length	= 0;

	if (XtExportSchema(gXtNetworkDb->schema, data, length))
	{
		XtU32 endian = (XtGetEndian() == kXtBigEndian) ? ARTERY_FOURCC('B', 'I', 'G', 'E') : ARTERY_FOURCC('L', 'T', 'L', 'E');

		ArteryMessageBuilder b;

		b.Write(endian);
		b.Write((XtU32)length);
		b.Write(data, length);

		ArteryMessage msg;

		if (b.Build(msg))
			con.Send(msg);

		delete [] data;
	}

	while(con.IsConnected())
	{
		ArteryMessage msg;

		if (con.Receive(msg))
		{
			XtNetMsg* nm = new XtNetMsg;

			if (!nm)
				continue;

			nm->msg = msg;

			gXtNetCs.Wait();

			AppendEx(&gXtNetMsgs, nm);

			gXtNetCs.Release();
		}
	}
}

void XtNetworkStart(XtDb* db)
{
	gXtNetworkDb = db;

	ArteryRegister("Sorcery", XtNetProc, 0);
}

DECLARE_DEBUG_BOOL(gXtReportNetStatus, "Xt.ReportNetStatus", false, Debug::Persist());

void XtNetStatusCallback(XtPackOpenStatus* status, void* context)
{
	if (gXtReportNetStatus)
		DebugLn("XT-NET: %s", status->message);
}

bool XtNetworkUpdate()
{
	if (!gXtNetworkDb)
		return false;

	bool result = false;

	gXtNetCs.Wait();

	XtPackOpenParams pop;

	pop.callback = XtNetStatusCallback;

	for(XtUt::List<XtNetMsg>::Iterator it(&gXtNetMsgs); it; ++it)
	{
		XtPack* pack = XtPackOpen(gXtNetworkDb, it->msg.Data(), it->msg.DataLength(), &pop);

		if (pack)
		{
			result = true;

			delete pack; // HACK: Need an XtPackFree //XtPackClose(gXtNetworkDb, pack);
		}
	}

	Free(&gXtNetMsgs);

	gXtNetCs.Release();

	return result;
}

struct XtDumpMemoryStatsItem
{
	const char* name;
	XtU32 bytes;
	XtU32 count;
	XtU32 data;
	XtU32 strings;
	XtU32 overhead;

public:
	XtDumpMemoryStatsItem(const char* name) : name(name), bytes(0), count(0), data(0), strings(0), overhead(0) { }
};

struct XtDumpMemoryStatsCompare
{
	bool operator()(const char* lhs, const char* rhs) const
	{
		return strcmp(lhs, rhs) < 0;
	}
};

void XtDumpMemoryStats(void*)
{
	if (!gXtNetworkDb)
		return;

	// Count

	XtUt::Dictionary<const char*, XtDumpMemoryStatsItem, XtDumpMemoryStatsCompare> totals;

	XtDb* db = gXtNetworkDb;

	for(XtU32 i = 0; i < db->count; i++)
	{
		XtRecord* record = db->index[i];

		if (!record->object)
			continue;

		XtDumpMemoryStatsItem* item = XtUt::At(&totals, record->type->name);

		if (!item)
		{
			item = new XtDumpMemoryStatsItem(record->type->name);

			if (!XtUt::Insert(&totals, item->name, item))
				item = 0;
		}

		if (item)
		{
			XtRecordHeader* header = XtGetRecordHeader(record);

			XtU32 overheadBytes	= (XtU32)((XtU8*)header - header->top);	// [Blur2PC][x64] byte distance
			XtU32 recordBytes	= XtGetRecordSize(record);

			item->bytes		+= header->size;
			item->data		+= recordBytes;
			item->strings	+= header->size - (overheadBytes + recordBytes);
			item->overhead	+= overheadBytes;
			item->count++;
		}
	}

	// Sort

	XtUt::Dictionary<XtU32, XtDumpMemoryStatsItem> sorted;

	for(XtUt::Dictionary<const char*, XtDumpMemoryStatsItem, XtDumpMemoryStatsCompare>::Iterator it(&totals); it; ++it)
		XtUt::Insert(&sorted, it->bytes, new XtDumpMemoryStatsItem(*it));

	// Display

	DebugLn("** Xt Memory Stats **");

	XtU32 totalBytes	= 0;
	XtU32 totalCount	= 0;
	XtU32 totalData		= 0;
	XtU32 totalStrings	= 0;
	XtU32 totalOverhead	= 0;

	for(XtUt::Dictionary<XtU32, XtDumpMemoryStatsItem>::Iterator it(&sorted); it; ++it)
	{
		DebugLn("  %-32.32s %-6.1fk (%i items, %.1fk data, %.1fk strings, %.1fk overhead)", it->name, it->bytes / 1024.0f, it->count, it->data / 1024.0f, it->strings / 1024.0f, it->overhead / 1024.0f);

		totalBytes		+= it->bytes;
		totalCount		+= it->count;
		totalData		+= it->data;
		totalStrings	+= it->strings;
		totalOverhead	+= it->overhead;
	}

	DebugLn("** Total: %.1fk (%i items, %.1fk data, %.1fk strings, %.1fk overhead) **", totalBytes / 1024.0f, totalCount, totalData / 1024.0f, totalStrings / 1024.0f, totalOverhead / 1024.0f);
}

DECLARE_DEBUG_FUNCTION(gXtDumpMemoryStats, "Xt.DumpMemoryStats", XtDumpMemoryStats, 0);

#else

void XtNetworkStart(XtDb* db)
{
}

bool XtNetworkUpdate()
{
	return false;
}

#endif
