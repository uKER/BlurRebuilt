#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "XtWin/WndCore.h"
#include "XtWin/AsyncOpWin.h"
#include "XtWin/SvnXt.h"
#include "XtWin/SvnAuthDlg.h"
#include "XtWin/ConflictWin.h"
#include <svn_wrapper.h>
#include <Xt/Impl/FindFile.h>

//
// SvnXtStorage
//

struct SvnXtContext;

class SvnXtStorage : public Xt::Storage
{
public:
	SvnXtStorage(SvnXtContext* ctx);

public:
	bool SetMimeType(const char* filename);

public:
	virtual bool New(const char* filename);
	virtual bool Dirty(const char* filename);
	virtual bool Rename(const char* oldFilename, const char* newFilename);
	virtual bool Remove(const char* filename);

public:
	SvnXtContext* _ctx;
};

//
// SvnXtContext
//

class SvnXtVcs;

struct SvnXtContext
{
	SvnXtParams*	params;
	SvnXtVcs*		vcs;
	Vcs::Svn*		svn;
	SvnXtStorage	storage;
	AsyncOp*		op;
	bool			underVersionControl;

public:
	SvnXtContext();
	~SvnXtContext();

public:
	void CheckUnderVersionControl();
};

//
// SvnXtStorage
//

SvnXtStorage::SvnXtStorage(SvnXtContext* ctx) :
	_ctx(ctx)
{
}

bool SvnXtStorage::SetMimeType(const char* filename)
{
	if (_ctx->underVersionControl)
	{
		Vcs::Status status;

		if (!(status = _ctx->svn->SetProperty(filename, "svn:mime-type", "application/octet-stream")))
		{
			_ctx->op->NewMessage(kAoiWarning, "Failed to set mime-type for: " + XtString(filename));
			_ctx->op->AppendMessage(status.text);

			return false;
		}
	}

	return true;
}

bool SvnXtStorage::New(const char* filename)
{
	if (_ctx->underVersionControl)
	{
		Vcs::Status status;

		if (!(status = _ctx->svn->Add(filename)))
		{
			_ctx->op->NewMessage(kAoiError, "Failed to add: " + XtString(filename));
			_ctx->op->AppendMessage(status.text);

			return false;
		}
	}

	_ctx->op->NewMessage(kAoiInfo, "Saved: " + XtString(filename));

	SetMimeType(filename);

	return true;
}

bool SvnXtStorage::Dirty(const char* filename)
{
	_ctx->op->NewMessage(kAoiInfo, "Saved: " + XtString(filename));

	SetMimeType(filename);

	return true;
}

bool SvnXtStorage::Rename(const char* oldFilename, const char* newFilename)
{
	bool result = true;

	if (_ctx->underVersionControl)
	{
		Vcs::Status status;

		if (!(status = _ctx->svn->Delete(oldFilename)))
		{
			_ctx->op->NewMessage(kAoiError, XtString("Failed to delete: ") + oldFilename);
			_ctx->op->AppendMessage(status.text);

			result = false;
		}

		if (!(status = _ctx->svn->Add(newFilename)))
		{
			_ctx->op->NewMessage(kAoiError, "Failed to add: " + XtString(newFilename));
			_ctx->op->AppendMessage(status.text);

			result = false;
		}
		else
		{
			_ctx->op->NewMessage(kAoiInfo, "Added: " + XtString(newFilename));

			SetMimeType(newFilename);
		}
	}
	else
	{
		if (!DeleteFile(oldFilename))
		{
			_ctx->op->NewMessage(kAoiError, XtString("Failed to delete: ") + oldFilename);

			result = false;
		}

		_ctx->op->NewMessage(kAoiInfo, "Saved: " + XtString(newFilename));
	}

	return result;
}

bool SvnXtStorage::Remove(const char* filename)
{
	if (_ctx->underVersionControl)
	{
		Vcs::Status status;

		if (!(status = _ctx->svn->Delete(filename)))
		{
			_ctx->op->NewMessage(kAoiError, "Failed to delete: " + XtString(filename));
			_ctx->op->AppendMessage(status.text);

			return false;
		}
	}
	else
	{
		if (!DeleteFile(filename))
		{
			_ctx->op->NewMessage(kAoiError, "Failed to delete: " + XtString(filename));

			return false;
		}
	}

	_ctx->op->NewMessage(kAoiInfo, "Deleted: " + XtString(filename));

	return true;
}

//
// SvnXtVcs
// 

class SvnXtVcs : public Vcs::VcsContext
{
public:
	SvnXtVcs(SvnXtContext* context) :
		_ctx(context)
	{
	}

public:
	virtual bool IsCancelled()
	{
		return _ctx->op->IsCancelled();
	}

	virtual bool GetLogin(Sz8& username, Sz8& password, const Sz8& realm, bool& maySave)
	{
		SvnAuthDlg dlg(_ctx->op->Window());

		dlg.SetAuthNotice(realm.Text());
		dlg.SetUsername(username.Text());
		dlg.SetPassword(password.Text());
		dlg.SetRemember(false);
		dlg.SetCanRemember(maySave);

		if (!dlg.Modal())
			return false;

		username	= dlg.Username().Text();
		password	= dlg.Password().Text();
		maySave		= maySave ? dlg.Remember() : false;

		return true;
	}

private:
	SvnXtContext* _ctx;
};

//
// SvnXtStatus
//

class SvnXtStatus : public Xt::StatusCallback
{
public:
	SvnXtStatus(SvnXtContext* ctx, float low = 0.0f, float high = 1.0f) :
		_ctx(ctx),
		_low(low),
		_high(high)
	{
	}

public:
	virtual void Status(const Xt::StatusMessage& msg)
	{
		switch(msg.severity)
		{
			case Xt::StatusMessage::kInfo:		_ctx->op->NewMessage(kAoiInfo, msg.message);	break;
			case Xt::StatusMessage::kWarning:	_ctx->op->NewMessage(kAoiWarning, msg.message);	break;
			case Xt::StatusMessage::kError:		_ctx->op->NewMessage(kAoiError, msg.message);	break;
			case Xt::StatusMessage::kTrace:		_ctx->op->AppendMessage(msg.message);			break;
		}
	}

	virtual void Progress(float progress)
	{
		_ctx->op->SetProgress(_low + ((_high - _low) * progress));
	}

	virtual bool IsCancelled()
	{
		return _ctx->op->IsCancelled();
	}

public:
	void SetRange(float low, float high)
	{
		_low	= low;
		_high	= high;
	}

private:
	SvnXtContext*	_ctx;
	float			_low;
	float			_high;
};

//
// SvnXtContext::SvnXtContext
//

SvnXtContext::SvnXtContext() :
	params(0),
	vcs(0),
	svn(0),
	storage(this),
	op(0),
	underVersionControl(true)
{
	vcs = new SvnXtVcs(this);
	svn = new Vcs::Svn(vcs);
}

SvnXtContext::~SvnXtContext()
{
	delete svn;
}

void SvnXtContext::CheckUnderVersionControl()
{
	Vcs::Status status = svn->Info(params->path, Vcs::kDepth_Empty);

	underVersionControl = (status.code != Vcs::Status::kNotAWorkingCopy);
}

//
// SvnXtOp_Open
//

bool SvnXtOp_Open(SvnXtContext* ctx)
{
	if (ctx->underVersionControl)
	{
		ConflictParams cp;

		cp.path = ctx->params->path;

		if (!SorceryResolveConflictsAsync(ctx->op, &cp))
			return false;
	}

	ctx->op->SetTitle("Loading...");
	ctx->op->NewMessage(kAoiInfo, "Loading:");
	ctx->op->AppendMessage(ctx->params->path);

	DWORD startTime = GetTickCount();

	Xt::OpenResult success = Open(ctx->params->db, ctx->params->path, &ctx->storage, &SvnXtStatus(ctx));

	DWORD endTime	= GetTickCount();
	float totalTime	= (endTime - startTime) / 1000.0f;

	DebugLn("Loaded %s in %.2f seconds", ctx->params->path.Text(), totalTime);

	if (success != Xt::kOpenSuccess)
	{
		ctx->op->SetTitle("Load failed");
		ctx->op->NewMessage(kAoiError, "Load failed.");

		return false;
	}

	ctx->op->SetTitle("Load complete");
	ctx->op->NewMessage(kAoiInfo, Format("Load complete, took %.2f seconds.", totalTime));

	return true;
}

//
// SvnXtOp_Save
//

bool SvnXtOp_Save(SvnXtContext* ctx)
{
	ctx->op->SetTitle("Saving...");
	ctx->op->NewMessage(kAoiInfo, "Saving:");
	ctx->op->AppendMessage(ctx->params->path);

	bool result = Flush(ctx->params->db, &ctx->storage, &SvnXtStatus(ctx));

	if (!result)
	{
		ctx->op->SetTitle("Save failed");
		ctx->op->NewMessage(kAoiError, "Save failed");

		return false;
	}

	ctx->op->SetTitle("Save complete");
	ctx->op->NewMessage(kAoiInfo, "Save complete");

	return true;
}

//
// SvnXtOp_Update
//

struct SvnXtUpdateDbProgressContext
{
	AsyncOp*	op;
	bool		hasConflicts;

public:
	SvnXtUpdateDbProgressContext(AsyncOp* op_) :
		op(op_),
		hasConflicts(false)
	{
	}
};

void SvnXtUpdateDbInfo(const Vcs::StatusEntry& info, void* baton)
{
	SvnXtUpdateDbProgressContext* ctx = (SvnXtUpdateDbProgressContext*)baton;

	if (info.textStatus == Vcs::kStatus_Unversioned)
	{
		if (_stricmp(SplitFileExt(info.path), ".db-lock") != 0)
		{
			ctx->op->NewMessage(kAoiWarning, "Unversioned:");
			ctx->op->AppendMessage(info.path);
		}
	}
	else if (info.textStatus == Vcs::kStatus_Conflicted)
	{
		ctx->hasConflicts = true;

		ctx->op->NewMessage(kAoiError, "Conflict:");
		ctx->op->AppendMessage(info.path);
	}
}

void SvnXtUpdateDbProgress(const Vcs::Notify& notify, void* baton)
{
	SvnXtUpdateDbProgressContext* ctx = (SvnXtUpdateDbProgressContext*)baton;

	if (!notify.error)
	{
		ctx->op->NewMessage(kAoiError, XtString("Error: ") + notify.path);
		ctx->op->AppendMessage(notify.error.text);
	}
	else
		ctx->op->NewMessage(kAoiInfo, notify.AsText());

	if (notify.contentState == Vcs::kState_Conflicted)
		ctx->hasConflicts = true;
}

bool SvnXtOp_Update(SvnXtContext* ctx)
{
	if (ctx->params->db->open)
	{
		if (!SvnXtOp_Save(ctx))
			return false;

		ctx->op->NewMessage(kAoiInfo, "Closing db...");

		DWORD timeStart = GetTickCount();

		Xt::Free(ctx->params->db);

		ctx->op->AppendMessage(Format("Closed, took %.2f seconds", (GetTickCount() - timeStart) / 1000.0f));
	}

	XtSimpleFile lock;

	if (!lock.Open(ctx->params->path + ".db-lock", true))
	{
		ctx->op->SetTitle("Update failed");
		ctx->op->NewMessage(kAoiError, "Database already open in another application");

		return false;
	}

	ctx->op->SetTitle("Updating...");
	ctx->op->NewMessage(kAoiInfo, "Updating:");
	ctx->op->AppendMessage(ctx->params->path);

	SvnXtUpdateDbProgressContext ctxUpd(ctx->op);

	Vcs::Status status = ctx->svn->Update(ctx->params->path.Text(), Vcs::kDepth_Infinity, Vcs::kRevision_Head, SvnXtUpdateDbProgress, &ctxUpd);

	if (status)
		status = ctx->svn->Info(ctx->params->path, Vcs::kDepth_Infinity, SvnXtUpdateDbInfo, &ctxUpd);

	if (!status)
	{
		ctx->op->SetTitle("Update failed");

		ctx->op->NewMessage(kAoiError, "Update failed");
		ctx->op->AppendMessage(status.text);

		return false;
	}

	ctx->op->NewMessage(kAoiInfo, "Update complete");

	lock.Close();

	return SvnXtOp_Open(ctx);
}

//
// SvnXtOp_Commit
//

void SvnXtCommitDbProgress(const Vcs::Notify& notify, void* baton)
{
	AsyncOp* op = (AsyncOp*)baton;

	if (!notify.error)
	{
		op->NewMessage(kAoiError, XtString("Failed: ") + notify.path);
		op->AppendMessage(notify.error.text);
	}
	else
		op->NewMessage(kAoiInfo, XtString("Success: ") + notify.path);
}

bool SvnXtOp_Commit(SvnXtContext* ctx)
{
	ctx->op->SetTitle("Commiting...");
	ctx->op->NewMessage(kAoiInfo, "Commiting:");
	ctx->op->AppendMessage(ctx->params->path);
	ctx->op->AppendMessage(ctx->params->comment);

	Vcs::Status status;

	if (ctx->params->files)
	{
		if (!(status = ctx->svn->Commit(ctx->params->files, ctx->params->comment, Vcs::kDepth_Empty, SvnXtCommitDbProgress, ctx->op)))
		{
			ctx->op->SetTitle("Commit failed");
			ctx->op->NewMessage(kAoiError, "Commit failed");
			ctx->op->AppendMessage(status.text);

			return false;
		}
	}
	else
	{
		if (!(status = ctx->svn->Commit(ctx->params->path, ctx->params->comment, Vcs::kDepth_Infinity, SvnXtCommitDbProgress, ctx->op)))
		{
			ctx->op->SetTitle("Commit failed");
			ctx->op->NewMessage(kAoiError, "Commit failed");
			ctx->op->AppendMessage(status.text);

			return false;
		}
	}

	ctx->op->SetTitle("Commit complete");
	ctx->op->NewMessage(kAoiInfo, "Commit complete");

	return true;
}

//
// SvnXtOp_Recover
//

bool SvnXtOp_Recover(SvnXtContext* ctx)
{
	ctx->op->SetTitle("Recovering...");

	ctx->op->NewMessage(kAoiInfo, "Attempting database recovery on:");
	ctx->op->AppendMessage(ctx->params->path);

	// Close

	ctx->op->NewMessage(kAoiInfo, "Closing...");

	Xt::Free(ctx->params->db);

	// Lock

	XtSimpleFile lock;

	if (!lock.Open(ctx->params->path + ".db-lock", true))
	{
		ctx->op->SetTitle("Recovery failed");
		ctx->op->NewMessage(kAoiError, "Database already open in another application");

		return false;
	}

	// Clean

	ctx->op->NewMessage(kAoiInfo, "Cleaning:");
	ctx->op->AppendMessage(ctx->params->path);

	Vcs::Status status;

	if (!(status = ctx->svn->Cleanup(ctx->params->path)))
	{
		ctx->op->SetTitle("Recovery failed");
		ctx->op->NewMessage(kAoiError, "Clean failed");
		ctx->op->AppendMessage(status.text);

		return false;
	}

	// Mime types

	ctx->op->NewMessage(kAoiInfo, "Setting mime types...");

	FindFile ff;

	if (ff.Begin(ctx->params->path + "packs\\*.xtd"))
	{
		do
		{
			XtString filename = ctx->params->path + "packs\\" + ff->cFileName;

			Vcs::Status status;

			if (!(status = ctx->svn->SetProperty(filename.Text(), "svn:mime-type", "application/octet-stream")))
			{
				ctx->op->NewMessage(kAoiWarning, "Failed to set mime-type for: " + XtString(filename));
				ctx->op->AppendMessage(status.text);
			}
		}
		while(ff.Next());
	}

	// Update

	ctx->op->NewMessage(kAoiInfo, "Updating:");
	ctx->op->AppendMessage(ctx->params->path);

	SvnXtUpdateDbProgressContext ctxUpd(ctx->op);

	status = ctx->svn->Update(ctx->params->path.Text(), Vcs::kDepth_Infinity, Vcs::kRevision_Head, SvnXtUpdateDbProgress, &ctxUpd);

	if (status)
		status = ctx->svn->Info(ctx->params->path, Vcs::kDepth_Infinity, SvnXtUpdateDbInfo, &ctxUpd);

	if (!status)
	{
		ctx->op->SetTitle("Recovery failed");

		ctx->op->NewMessage(kAoiError, "Update failed");
		ctx->op->AppendMessage(status.text);

		return false;
	}

	// Reload

	ctx->op->NewMessage(kAoiInfo, "Recovery complete");

	lock.Close();

	return SvnXtOp_Open(ctx);
}

//
// SvnXtStub
//

struct SvnXtStub
{
	bool			(*callback)(SvnXtContext*);
	SvnXtParams*	params;

public:
	SvnXtStub(bool (*callback_)(SvnXtContext*), SvnXtParams* params_) :
		callback(callback_),
		params(params_)
	{
	}

public:
	bool Call(HWND parentWindow)
	{
		CleanUpPath(params->path, false, true);

		return AsyncInvoke(parentWindow, Stub, this, 0, params->noWindow);
	}

private:
	static bool Stub(AsyncOp* op, SvnXtStub* stub)
	{
		SvnXtContext ctx;

		ctx.params	= stub->params;
		ctx.op		= op;

		ctx.CheckUnderVersionControl();

		return stub->callback(&ctx);
	}
};

//
// SvnXt interface
//

bool SvnXtOpen(HWND parentWindow, SvnXtParams* params)
{
	return SvnXtStub(SvnXtOp_Open, params).Call(parentWindow);
}

bool SvnXtSave(HWND parentWindow, SvnXtParams* params)
{
	return SvnXtStub(SvnXtOp_Save, params).Call(parentWindow);
}

bool SvnXtUpdate(HWND parentWindow, SvnXtParams* params)
{
	return SvnXtStub(SvnXtOp_Update, params).Call(parentWindow);
}

bool SvnXtCommit(HWND parentWindow, SvnXtParams* params)
{
	return SvnXtStub(SvnXtOp_Commit, params).Call(parentWindow);
}

bool SvnXtRecover(HWND parentWindow, SvnXtParams* params)
{
	return SvnXtStub(SvnXtOp_Recover, params).Call(parentWindow);
}