#include "Core/StdAfx.h"
#include "SorceryApp.h"
#include "SorceryView.h"
#include "Resource/Resource.h"
#include "Xt/XtTool.h"
#include "XtWin/SvnXt.h"
#include <crtdbg.h>

XT_APPLICATION
XT_END_APPLICATION

//
// SorceryApp
//

SorceryApp::SorceryApp() :
	_window(*this),
	_db(0)
{
	_db = new Xt::DomDb;

	Monitor(&_window._monitor, _db);
}

SorceryApp::~SorceryApp()
{
	_documents.Clear();

	delete _db;
}

bool SorceryApp::Initialise()
{
	_dbPath = (__argc >= 1) ? __argv[1] : "";

	if (strchr(_dbPath, ':') == 0)
	{
		TCHAR cwd[MAX_PATH] = _T("");
		GetCurrentDirectory(MAX_PATH, cwd);

		_dbPath = XtString(cwd) + "\\" + _dbPath;
	}

	CleanUpPath(_dbPath, false, true);

	if (!ArteryStartup(false, "sorcery"))
		return false;

	if (!_window.Create())
		return false;

	ShowWindow(_window, SW_SHOW);

	//
	{
		void LcsTest();
		LcsTest();

		void DdcTest(const Sz8& dbPath);
		DdcTest(_dbPath);
	}
	//

	if (!SvnXtOpen(_window, &SvnXtParams(_db, _dbPath)))
	{
		if (MessageBox(_window,
				"Sorcery failed to load the database:\r\n\r\n" +
				_dbPath +
				"\r\n\r\nWould you like to attempt a recovery to fix the problem?",
				_T("Sorcery"),
				MB_YESNO | MB_ICONQUESTION
			) == IDYES)
		{
			if (!SvnRecover())
				return false;
		}
		else
		{
			return false;
		}
	}

	_window.Explorer()->ResetView();

	return true;
}

void SorceryApp::Destroy()
{
	_window.Destroy();

	ArteryCleanup();
}

bool SorceryApp::SaveProject()
{
	if (_window.Explorer()->Root() && _window.Explorer()->Root()->Item())
		_window.Explorer()->Root()->Item()->GenerateFilters(0, "");

	bool result = SvnXtSave(_window, &SvnXtParams(_db, _dbPath));

	for(Xt::DomDb::Iterator itPack(_db); itPack; ++itPack)
	{
		for(Xt::DomPack::Iterator itRecord(itPack); itRecord; ++itRecord)
		{
			Notify(itRecord, &Xt::DomRecordChanged(itRecord));
		}
	}

	return result;
}

void SorceryApp::SvnUpdate()
{
	for(LinkIterator<SorceryView> it(_window._views); it; )
	{
		SorceryView* view = it;

		++it;

		view->Close();
	}

	if (_window.Explorer()->Root() && _window.Explorer()->Root()->Item())
		_window.Explorer()->Root()->Item()->GenerateFilters(0, "");

	_window.Explorer()->ClearTree();

	SvnXtUpdate(_window, &SvnXtParams(_db, _dbPath));

	_window.Explorer()->ResetView();
}

void SorceryApp::SvnCommit()
{
	if (!SaveProject())
		return;

	CommitParams params;

	params.path			= _db->path;
	params.parentWindow	= _window;

	if (SorceryCommit(&params) == kCrProceed)
	{
		SvnXtParams xp(_db, _dbPath, params.comment);

		xp.files = &params.files;

		SvnXtCommit(_window, &xp);
	}
}

bool SorceryApp::SvnRecover()
{
	for(LinkIterator<SorceryView> it(_window._views); it; )
	{
		SorceryView* view = it;

		++it;

		view->Close();
	}

	_window.Explorer()->ClearTree();

	if (!SvnXtRecover(_window, &SvnXtParams(_db, _dbPath)))
	{
		MessageBox(_window, _T("The recovery attempt failed to fix the issue"), _T("Sorcery"), MB_OK  | MB_ICONERROR);

		return false;
	}

	MessageBox(_window, _T("The recovery attempt was successful"), _T("Sorcery"), MB_OK | MB_ICONINFORMATION);

	_window.Explorer()->ResetView();

	return true;
}

class SorceryXtStatus : public Xt::StatusCallback
{
public:
	SorceryXtStatus(AsyncOp* op) : _op(op) { }

public:
	virtual void Status(const Xt::StatusMessage& msg)
	{
		switch(msg.severity)
		{
			case Xt::StatusMessage::kInfo:		_op->NewMessage(kAoiInfo, msg.message);		break;
			case Xt::StatusMessage::kWarning:	_op->NewMessage(kAoiWarning, msg.message);	break;
			case Xt::StatusMessage::kError:		_op->NewMessage(kAoiError, msg.message);	break;
			case Xt::StatusMessage::kTrace:		_op->AppendMessage(msg.message);			break;
		}
	}

	virtual void Progress(float progress)
	{
		_op->SetProgress(progress);
	}

	virtual bool IsCancelled()
	{
		return _op->IsCancelled();
	}

private:
	AsyncOp* _op;
};

XtUt::Array<XtId> gReloadSchemaOpenRecords;

bool SorceryReloadSchema(AsyncOp* op, SorceryApp* app)
{
	op->SetTitle("Reloading Schema...");
	op->NewMessage(kAoiInfo, "Reloading Schema:");
	op->AppendMessage(app->_dbPath);

	DWORD startTime = GetTickCount();

	ReloadSchema(app->_db, &SorceryXtStatus(op));

	DWORD endTime	= GetTickCount();
	float totalTime	= (endTime - startTime) / 1000.0f;

	DebugLn("Schema loaded %s in %.2f seconds", app->_dbPath.Text(), totalTime);

	char buf[256];
	_snprintf_s(buf, sizeof(buf), _TRUNCATE, "Schema reload complete, took %.2f seconds.", totalTime);

	op->SetTitle("Schema reload complete");
	op->NewMessage(kAoiInfo, buf);

	app->_window.Explorer()->ResetView();

	return true;
}

void SorceryApp::ReloadSchema()
{
	_window._finder.Destroy();

	if (_db->open)
	{
		Free(&gReloadSchemaOpenRecords);

		for(LinkIterator<SorceryView> it(_window._views); it; )
		{
			SorceryView* view = it;

			if (view->_document)
				XtUt::AppendEx(&gReloadSchemaOpenRecords, view->_document->_id);

			++it;

			view->Close();
		}
	}

	AsyncInvoke(_window, SorceryReloadSchema, this);

	if (_db->open)
	{
		for(XtUt::Array<XtId>::Iterator it(&gReloadSchemaOpenRecords); it; ++it)
		{
			if (Xt::DomRecord* record = FindRecordById(_db, it))
				_window.OpenRecord(record);
		}
	}
}