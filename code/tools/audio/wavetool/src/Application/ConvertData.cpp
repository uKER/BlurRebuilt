#include "Core/StdAfx.h"
#include "SorceryApp.h"
#include "Xt/XtTool.h"

void DoConvertRecord(AsyncOp* op, Xt::DomDb* db, Xt::DomRecord* record)
{
	LtStruct* type = TypeOf(db->schema, record);

	if (!type)
	{
		op->NewMessage(kAoiError, "Missing record type: " + record->name + "." + record->root.type);
		return;
	}
}

void DoConvertDataSub(AsyncOp* op, SorceryApp* app)
{
	Xt::DomDb* db = app->_db;

	for(Xt::DomDb::Iterator itPack(db); itPack; ++itPack)
	{
		for(Xt::DomPack::Iterator itRecord(itPack); itRecord; ++itRecord)
			DoConvertRecord(op, db, itRecord);
	}

	/*XtUt::Ptr<Xt::DomPack> ancestor	= Xt::ImportPack("e:\\sorcery\\6-base.xtd", 0);
	XtUt::Ptr<Xt::DomPack> mine		= Xt::ImportPack("e:\\sorcery\\6-mine.xtd", 0);
	XtUt::Ptr<Xt::DomPack> theirs	= Xt::ImportPack("e:\\sorcery\\6-theirs.xtd", 0);

	bool mineResult = false;
	bool theirsResult = false;

	XtUt::Ptr<Xt::Change> rMine = Xt::Diff(ancestor, mine, &mineResult);
	XtUt::Ptr<Xt::Change> rTheirs = Xt::Diff(ancestor, theirs, &theirsResult);

	XtUt::Ptr<Xt::Change> rMerged = Xt::Merge(rMine, rTheirs);

	//VisualConflictResolver vcr(db->schema, rMine, ancestor, mine, 0);
	//vcr.DoModal(WinPoint(100, 100), WinSize(600, 400), app->_window);

	//VisualConflictResolver vcr(db->schema, rMerged, rMine, rTheirs, ancestor, mine, theirs);
	//vcr.DoModal(WinPoint(100, 100), WinSize(1200, 800), app->_window);

	if (!Xt::Patch(ancestor, rMerged))
	{
		DebugLn("PATCH FAIL");
	}

	ExportPack(ancestor, "e:\\sorcery\\out.xtd");*/
}

bool DoConvertData(AsyncOp* op, SorceryApp* app)
{
	op->SetTitle("Converting...");
	op->NewMessage(kAoiInfo, "Converting:");
	op->AppendMessage(app->_db->path);

	DWORD startTime	= GetTickCount();

	DoConvertDataSub(op, app);

	DWORD endTime	= GetTickCount();
	float totalTime	= (endTime - startTime) / 1000.0f;

	DebugLn("Converted %s in %.2f seconds", (const char*)app->_db->path, totalTime);

	char buf[256];
	_snprintf_s(buf, sizeof(buf), _TRUNCATE, "Convert complete, took %.2f seconds.", totalTime);

	op->SetTitle("Convert complete");
	op->NewMessage(kAoiInfo, buf);

	op->SetStayOpen();

	return true;
}

void ConvertData(SorceryApp& app)
{
	AsyncInvoke(app._window, DoConvertData, &app);

	//void TransTest(Xt::DomDb* db);
	//TransTest(&app._db);
}