#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "XtWin/SendEmail.h"
#include "XtWin/AsyncOpWin.h"
#include "XtWin/ConflictWin.h"

#include <svn_wrapper.h>

enum ConflictResolution
{
	kCrLeave,
	kCrMerge
};

struct ConflictItem
{
	ConflictResolution	resolution;
	XtString			path;
	XtString			merged;
	XtString			ancestor;
	XtString			mine;
	XtString			theirs;

public:
	ConflictItem() :
		resolution(kCrLeave)
	{
	}
};

struct ConflictList
{
	ConflictParams*				params;
	XtUt::List<ConflictItem>	items;
};

void PopulateListCallback(const Vcs::StatusEntry& info, void* baton)
{
	ConflictList* list = (ConflictList*)baton;

	if (info.textStatus != Vcs::kStatus_Conflicted)
		return;

	ConflictItem* item = XtUt::AppendEx(&list->items, new ConflictItem);

	if (!item)
		return;

	int pathLength = (int)list->params->path.Length();

	const char* filename = info.path + (((int)strlen(info.path) >= pathLength) ? pathLength : 0);

	XtString ext;
	SplitPath(filename, item->path, item->merged, ext);

	item->merged += ext;

	item->ancestor	= info.conflictOld ? info.conflictOld : "";
	item->mine		= info.conflictWrk ? info.conflictWrk : item->merged;
	item->theirs	= info.conflictNew ? info.conflictNew : "";

	if (_stricmp(ext, ".xtd") == 0)
		item->resolution = kCrMerge;
}

namespace
{
	struct DumpContext
	{
		AsyncOp* op;
		Xt::DomPack* ancestor;
		Xt::DomPack* mine;
		Xt::DomPack* theirs;
		bool warned;
		FILE* logFp;
	};

	void DoNewMessage(AsyncOp* op, FILE* logFp, AsyncOpIcon icon, const Sz8& text)//, const char* msg, ...)
	{
		/*va_list vl;
		va_start(vl, msg);
		
		Sz8 text = FormatV(msg, vl);

		va_end(vl);*/

		if (op)
			op->NewMessage(icon, text);

		if (logFp)
		{
			switch(icon)
			{
				case kAoiNone:		fprintf(logFp, "[   ] %s\n", text.Text()); break;
				case kAoiInfo:		fprintf(logFp, "[INF] %s\n", text.Text()); break;
				case kAoiWarning:	fprintf(logFp, "[WRN] %s\n", text.Text()); break;
				case kAoiError:		fprintf(logFp, "[ERR] %s\n", text.Text()); break;
			}
		}
	}

	void DoAppendMessage(AsyncOp* op, FILE* logFp, const Sz8& text)//const char* msg, ...)
	{
		/*va_list vl;
		va_start(vl, msg);
		
		Sz8 text = FormatV(msg, vl);

		va_end(vl);*/

		if (op)
			op->AppendMessage(text);

		if (logFp)
			fprintf(logFp, "      %s\n", text.Text());
	}

	XtString FindRecordName(DumpContext* dc, XtId id)
	{
		if (Xt::DomRecord* record = dc->ancestor->records[id])
			return record->name + "." + record->root.type;

		if (Xt::DomRecord* record = dc->mine->records[id])
			return record->name + "." + record->root.type;

		if (Xt::DomRecord* record = dc->theirs->records[id])
			return record->name + "." + record->root.type;

		return "(unknown record)";
	}

	void Dump(DumpContext* dc, Xt::Change* change, const XtString& path)
	{
		if (!change)
			return;

		switch(change->type)
		{
			case Xt::kChangeRemove:
			break;

			case Xt::kChangeLeaf:
			break;

			case Xt::kChangeBlock:
				for(Xt::ChangeBlock::Iterator itChange(&((Xt::ChangeBlock*)change)->changes); itChange; ++itChange)
					Dump(dc, itChange, path + "/" + itChange.Key());
			break;

			case Xt::kChangeArray:
				Dump(dc, ((Xt::ChangeArray*)change)->index, path + " *Index* ");

				for(Xt::ChangeArray::Iterator itChange(&((Xt::ChangeArray*)change)->changes); itChange; ++itChange)
					Dump(dc, itChange, path + "/#" + Format("%08X", itChange.Key()));
			break;

			case Xt::kChangeComponent:
				Dump(dc, ((Xt::ChangeComponent*)change)->id, path + " *Id* ");
				Dump(dc, ((Xt::ChangeComponent*)change)->type, path + " *Type* ");
				Dump(dc, ((Xt::ChangeComponent*)change)->data, path + " *Data* ");
			break;

			case Xt::kChangeRecord:
				Dump(dc, ((Xt::ChangeRecord*)change)->name, path + " *Name* ");
				Dump(dc, ((Xt::ChangeRecord*)change)->root, path + " *Root* ");
			break;

			case Xt::kChangePack:
				Dump(dc, ((Xt::ChangePack*)change)->name, path + " *Name* ");

				for(Xt::ChangePack::Iterator itChange(&((Xt::ChangePack*)change)->changes); itChange; ++itChange)
					Dump(dc, itChange, FindRecordName(dc, itChange.Key()));
			break;

			case Xt::kChangeId:
			break;

			case Xt::kChangeString:
			break;

			case Xt::kChangeIndex:
			break;

			case Xt::kChangeConflict:
				if (!dc->warned)
				{
					DoNewMessage(dc->op, dc->logFp, kAoiWarning, "Conflicts autoresolved:");
					dc->warned = true;
				}

				DoAppendMessage(dc->op, dc->logFp, path);
			break;
		}
	}

	void DumpChanges(AsyncOp* op, FILE* logFp, Xt::DomPack* ancestor, Xt::DomPack* mine, Xt::DomPack* theirs, Xt::Change* merged)
	{
		DumpContext dc;

		dc.op		= op;
		dc.ancestor	= ancestor;
		dc.mine		= mine;
		dc.theirs	= theirs;
		dc.warned	= false;
		dc.logFp	= logFp;

		Dump(&dc, merged, "");
	}
}

class ResolveConflictsStatus : public Xt::StatusCallback
{
public:
	ResolveConflictsStatus(AsyncOp* op) :
		_op(op)
	{
	}

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

private:
	AsyncOp* _op;
};

bool SorceryResolveConflictsAsync(AsyncOp* op, ConflictParams* params)
{
	ConflictList list;

	list.params = params;

	ResolveConflictsStatus stat(op);

	if (op)
	{
		op->SetTitle("Automatic Conflict Resolver");
		DoNewMessage(op, 0, kAoiInfo, "Building conflict list...");
	}

	// Check db version

	XtString projName;

	{
		XtString configFileName = params->path + "config.xtp";

		Xt::DomBlock block;

		if (!Import2(&block, configFileName, 0, &stat))
		{
			DoNewMessage(op, 0, kAoiError, "Failed to load config file");
			DoAppendMessage(op, 0, configFileName);

			return false;
		}

		projName = AsText(At(&block, "name"));

		if (atoi(AsText(At(&block, "__version"))) > XtGetDbVersion())
		{
			DoNewMessage(op, 0, kAoiError, "Db version out of date, please grab the latest tools");

			return false;
		}
	}

	// Do conflicts

	{
		Vcs::Svn svn;

		if (svn)
		{
			Vcs::Status status;

			if (!(status = svn.Info(params->path, Vcs::kDepth_Infinity, PopulateListCallback, &list)))
			{
				DoNewMessage(op, 0, kAoiError, "Failed to build list of conflicted files");
				DoAppendMessage(op, 0, status.text);
			}
		}

		DoAppendMessage(op, 0, "Done.");
	}

	//
	// START HACK
	//
	projName = XtsStripBoth(projName, ' ');
	if (!*projName) projName = "Unnamed";
	XtString hackPath;
	FILE* logFp = 0;
	if (Count(&list.items) > 0)
	{
		CreateDirectory("C:\\ConflictHistory", 0);

		SYSTEMTIME time;
		GetLocalTime(&time);

		hackPath = Format("C:\\ConflictHistory\\%s %04i-%02i-%02i %02i.%02i.%02i", projName.Text(), time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);

		CreateDirectory(hackPath, 0);

		if (fopen_s(&logFp, hackPath + "\\conflict-log.txt", "wt") != 0)
			logFp = 0;
	}
	//
	// END HACK
	//

	bool result = true;

	for(XtUt::List<ConflictItem>::Iterator it(&list.items); it; ++it)
	{
		if (it->resolution != kCrMerge)
		{
			DoNewMessage(op, logFp, kAoiError, "Cannot automatically merge:");
			DoAppendMessage(op, logFp, params->path + it->path + it->merged);
			result = false;
			continue;
		}

		// START HACK
		{
			CopyFile(params->path + it->path + it->ancestor, hackPath + "\\" + it->ancestor + "-base", FALSE);
			CopyFile(params->path + it->path + it->mine, hackPath + "\\" + it->mine + "-mine", FALSE);
			CopyFile(params->path + it->path + it->theirs, hackPath + "\\" + it->theirs + "-theirs", FALSE);
		}
		// END HACK

		DoNewMessage(op, logFp, kAoiInfo, "Resolving conflicts for: " + it->path + it->merged);

		DoAppendMessage(op, logFp, "Ancestor - " + params->path + it->path + it->ancestor);

		XtUt::Ptr<Xt::DomPack> ancestor = Xt::ImportPack(params->path + it->path + it->ancestor, &stat);

		if (!ancestor)
		{
			DoNewMessage(op, logFp, kAoiError, "Failed to load:");
			DoAppendMessage(op, logFp, params->path + it->path + it->ancestor);

			result = false;

			continue;
		}

		DoAppendMessage(op, logFp, "Mine - " + params->path + it->path + it->mine);

		XtUt::Ptr<Xt::DomPack> mine = Xt::ImportPack(params->path + it->path + it->mine, &stat);

		if (!mine)
		{
			DoNewMessage(op, logFp, kAoiError, "Failed to load:");
			DoAppendMessage(op, logFp, params->path + it->path + it->mine);

			result = false;

			continue;
		}

		DoAppendMessage(op, logFp, "Theirs - " + params->path + it->path + it->theirs);

		XtUt::Ptr<Xt::DomPack> theirs = Xt::ImportPack(params->path + it->path + it->theirs, &stat);

		if (!theirs)
		{
			DoNewMessage(op, logFp, kAoiError, "Failed to load:");
			DoAppendMessage(op, logFp, params->path + it->path + it->theirs);

			result = false;

			continue;
		}

		bool mineResult		= false;
		bool theirsResult	= false;

		XtUt::Ptr<Xt::Change> rMine = Xt::Diff(ancestor, mine, &mineResult);
		XtUt::Ptr<Xt::Change> rTheirs = Xt::Diff(ancestor, theirs, &theirsResult);

		if (!mineResult)
		{
			DoNewMessage(op, logFp, kAoiError, "Failed to diff ancestor vs mine:");
			DoAppendMessage(op, logFp, params->path + it->path + it->merged);

			result = false;

			continue;
		}

		if (!theirsResult)
		{
			DoNewMessage(op, logFp, kAoiError, "Failed to diff ancestor vs theirs:");
			DoAppendMessage(op, logFp, params->path + it->path + it->merged);

			result = false;

			continue;
		}

		XtUt::Ptr<Xt::Change> rMerged = Xt::Merge(rMine, rTheirs);

		if (!rMerged)
		{
			DoNewMessage(op, logFp, kAoiError, "Failed to merge:");
			DoAppendMessage(op, logFp, params->path + it->path + it->merged);

			result = false;

			continue;
		}

		DumpChanges(op, logFp, ancestor, mine, theirs, rMerged);

		if (!Xt::Patch(ancestor, rMerged))
		{
			DoNewMessage(op, logFp, kAoiError, "Failed to patch:");
			DoAppendMessage(op, logFp, params->path + it->path + it->merged);

			result = false;

			continue;
		}

		if (!RecycleFile(params->path + it->path + it->ancestor))
		{
			DoNewMessage(op, logFp, kAoiError, "Recycle file failed:");
			DoAppendMessage(op, logFp, params->path + it->path + it->ancestor);
		}

		if (!RecycleFile(params->path + it->path + it->mine))
		{
			DoNewMessage(op, logFp, kAoiError, "Recycle file failed:");
			DoAppendMessage(op, logFp, params->path + it->path + it->mine);
		}

		if (!RecycleFile(params->path + it->path + it->theirs))
		{
			DoNewMessage(op, logFp, kAoiError, "Recycle file failed:");
			DoAppendMessage(op, logFp, params->path + it->path + it->theirs);
		}

		if (!ExportPack(ancestor, params->path + it->path + it->merged))
		{
			DoNewMessage(op, logFp, kAoiError, "Export pack failed:");
			DoAppendMessage(op, logFp, params->path + it->path + it->merged);
		}

		// START HACK
		{
			CopyFile(params->path + it->path + it->merged, hackPath + "\\" + it->merged + "-out", FALSE);
		}
		// END HACK

		DoNewMessage(op, logFp, kAoiInfo, "Resolved:");
		DoAppendMessage(op, logFp, params->path + it->path + it->merged);

		if (op)
			op->SetStayOpen();
	}

	if (logFp)
		fclose(logFp);

	return result;
}

bool SorceryResolveConflicts(ConflictParams* params)
{
	if (!AsyncInvoke(params->parentWindow, SorceryResolveConflictsAsync, params))
		return false;

	return true;
}