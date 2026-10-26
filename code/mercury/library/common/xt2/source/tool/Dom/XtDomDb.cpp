#include "Common/XtPch.h"
#include <Xt/XtTool.h>
#include <Xt/Impl/FindFile.h>
#include "../Export/XtFlaskHelpers.h"

//
// Database File Management
//

namespace
{
	class ScOpenPack : public Xt::StatusCallback
	{
	public:
		ScOpenPack(StatusCallback* status, float low, float high) :
			_status(status),
			_low(low),
			_high(high)
		{
		}

	public:
		virtual void Status(const Xt::StatusMessage& msg)
		{
			if (_status)
				_status->Status(msg);
		}

		virtual void Progress(float progress)
		{
			if (_status)
				_status->Progress(XtUt::Lerp(_low, _high, progress));
		}

	private:
		StatusCallback*	_status;
		float			_low;
		float			_high;
	};

	template<typename T> __forceinline void Swap(T& Lhs, T& Rhs)
	{
		const T Tmp(Lhs);
		Lhs = Rhs;
		Rhs = Tmp;
	}
}

namespace Xt
{

	//
	// Xt::DomDb::Open
	//

	bool OpenConfig(DomDb* db, const XtString& path, StatusCallback* status, int& version);

	bool OpenSchemaSub(DomDb* db, const XtString& path, StatusCallback* status, Grimoire::Dom* grim)
	{
		FindFile ff;

		if (ff.Begin(path + "*"))
		{
			do
			{
				if (ff->cFileName[0] == '.')
					continue;

				if (ff->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					if (!OpenSchemaSub(db, path + ff->cFileName + "\\", status, grim))
						return false;
				}

				XtString fnPath;
				XtString fnFile;
				XtString fnExt;

				if (!XtsSplitFileName(ff->cFileName, fnPath, fnFile, fnExt))
					continue;

				if (_stricmp(fnExt, ".txt") != 0)
					continue;

				if (!GrimoireImport(path + ff->cFileName, grim, status))
					return false;
			}
			while(ff.Next());

			ff.End();
		}

		return true;
	}

	bool OpenSchema(DomDb* db, const XtString& path, StatusCallback* status)
	{
		Grimoire::Dom grim;

		grim.path = path;

		if (!OpenSchemaSub(db, path, status, &grim))
			return false;

		if ((db->schema = CreateLarvaSchema(&grim, status)) == 0)
		{
			StatusCallback::Error(status, "Failed to create larva schema");

			return false;
		}

		if ((db->exportSchema = new ExportSchema) == 0)
			return false;

		if (!CreateExportSchema(&grim, db->exportSchema, status))
 			return false;

		return true;
	}

	bool OpenDbProperties(DomDb* db, const XtString& path, StatusCallback* status)
	{
		if (!Import2(&db->properties, path, 0, status))
		{
			//StatusCallback::Error(status, "Failed to load '%s'", path.Text());
			//return false;

			StatusCallback::Warning(status, "Fail to load database properties '%s'", path.Text());
		}

		return true;
	}

	bool OpenPacks(DomDb* db, const XtString& path, StatusCallback* status)
	{
		FindFile ff;

		const XtI64 kMinByteCount = 1024 * 128;

		int estimatedPackCount		= 1;
		XtI64 estimatedByteCount	= 0;
		XtI64 actualByteCount		= 0;

		if (ff.Begin(path + "*.xtd"))
		{
			do
			{
				estimatedPackCount++;

				XtI64 size = ((XtI64)ff->nFileSizeHigh << 32) | (XtI64)ff->nFileSizeLow;

				estimatedByteCount	+= XtUt::Max(size, kMinByteCount);
				actualByteCount		+= size;
			}
			while(ff.Next());

			ff.End();
		}

		bool totalResult = true;

		if (ff.Begin(path + "*.xtd"))
		{
			XtI64 byteProgress	= 0;
			DWORD timeStart		= GetTickCount();

			do
			{
				if (status && status->IsCancelled())
				{
					StatusCallback::Error(status, "Cancelled.");
					return false;
				}

				XtI64	byteSize		= XtUt::Max(((XtI64)ff->nFileSizeHigh << 32) | (XtI64)ff->nFileSizeLow, kMinByteCount);
				float	progressLow		= byteProgress / (float)estimatedByteCount;
				float	progressHigh	= (byteProgress + byteSize) / (float)estimatedByteCount;

				byteProgress += byteSize;

				StatusCallback::Progress(status, progressLow);

				XtString filename = path + ff->cFileName;

				XtUt::Block pristine;

				XtUt::Ptr<DomPack> pack = ImportPack(filename, &ScOpenPack(status, progressLow, progressHigh), &pristine);

				if (!pack)
				{
					totalResult = false;
					continue;
				}

				if (DomPack* dup = FindPackById(db, pack->id))
				{
					totalResult = false;
					StatusCallback::Error(status, "Failed to open pack file");
					StatusCallback::Trace(status, "Error: Duplicate pack id");
					StatusCallback::Trace(status, "Pack 1: %s[%08X] {%s}", dup->name.Text(), dup->id, dup->filename.Text());
					StatusCallback::Trace(status, "Pack 2: %s[%08X] {%s}", pack->name.Text(), pack->id, pack->filename.Text());
					continue;
				}

				bool dupFound = false;

				for(DomPack::Iterator itRecord(pack); itRecord; ++itRecord)
				{
					if (DomRecord* dup = FindRecordById(db, itRecord->id))
					{
						totalResult	= false;
						dupFound	= true;
						StatusCallback::Error(status, "Failed to open pack file");
						StatusCallback::Trace(status, "Error: Duplicate record id");
						StatusCallback::Trace(status, "File: %s", filename.Text());
						StatusCallback::Trace(status, "Record 1: %s.%s[%08X]", dup->name.Text(), dup->root.type.Text(), dup->id);
						StatusCallback::Trace(status, "Record 2: %s.%s[%08X]", itRecord->name.Text(), itRecord->root.type.Text(), itRecord->id);
						break;
					}
				}

				if (dupFound)
					continue;

				DomPack* newPack = Pop(&pack);

				if (!AddPack(db, newPack))
				{
					totalResult = false;
					StatusCallback::Error(status, "Failed to open pack file");
					StatusCallback::Trace(status, "Error: Failed to add pack to db");
					StatusCallback::Trace(status ,"File: %s", filename.Text());
					continue;
				}

				XtUt::Block* newPristine = new XtUt::Block;

				if (!Insert(&db->pristine, newPack->id, newPristine))
				{
					totalResult = false;
					StatusCallback::Error(status, "Failed to open pack file");
					StatusCallback::Trace(status, "Error: Failed to add pristine pack file");
					StatusCallback::Trace(status ,"File: %s", filename.Text());
					continue;
				}

				XtUt::ReplaceData(newPristine, &pristine);
			}
			while(ff.Next());

			float duration = (GetTickCount() - timeStart) / 1000.0f;

			DebugLn("XtDb: Open packs took %.2f seconds, %.2f MB/s", duration, ((float)actualByteCount / duration) / (1024.0f * 1024.0f));

			ff.End();
		}

		return totalResult;
	}

	OpenResult Open(DomDb* db, const XtString& path, Storage* storage, StatusCallback* status)
	{
		assert(db != 0);

		XtLog("XTDB", "Open %s", path.Text());

		if (db->open)
			Free(db);

		db->path = path;

		CleanUpPath(db->path, false, true);

		XtSimpleFile lock;

		OpenResult result = kOpenSuccess;

		DWORD attr = GetFileAttributes(db->path);

		if ((attr == INVALID_FILE_ATTRIBUTES) || ((attr & FILE_ATTRIBUTE_DIRECTORY) == 0))
		{
			Xt::StatusCallback::Error(status, "Database '%s' does not exist.", (const char*)db->path);
			result = kOpenFailed;
		}
		else
		{
			if (!db->lock.Open(db->path + ".db-lock", true))
			{
				Xt::StatusCallback::Error(status, "Database '%s' is already open in another application.", (const char*)db->path);
				result = kOpenLocked;
			}
			else
			{
				if (!lock.Open(db->path + ".lock", true))
				{
					Xt::StatusCallback::Error(status, "Database '%s' is locked for reading by another application.", (const char*)db->path);
					result = kOpenLocked;
				}
				else
				{
					int version = 0;

					if (
							!OpenConfig(db, db->path + "config.xtp", status, version) ||
							!OpenSchema(db, db->path + "schema\\", status) ||
							!OpenDbProperties(db, db->path + "db.xtd", status) ||
							!OpenPacks(db, db->path + "packs\\", status)
						)
					{
						result = kOpenFailed;
					}
					else
					{
						db->open	= true;
						db->dirty	= false;

						Notify(db, &DomDbOpened(db));
					}
				}
			}

			lock.Close();

			DeleteFile(db->path + ".lock");
		}

		if (result != kOpenSuccess)
			Free(db);

		return result;
	}

	//
	// Xt::DomDb::Flush
	//

	bool Flush_Config(DomDb* db);

	bool Flush_Sub(DomDb* db, Storage* storage, StatusCallback* status)
	{
		assert(db != 0);

		if (!db->open)
			return false;

		XtSimpleFile lock;

		if (!lock.Open(db->path + ".lock", true))
			return false;

		// TODO: For now, don't flush config file
		//if (!Flush_Config(db))
		//	return false;

		bool totalResult = true;

		// Remove deleted packs

		XtUt::Array<XtString> remaining;

		for(XtUt::Array<XtString>::Iterator itPack(&db->removedPacks); itPack; ++itPack)
		{
			if (!storage->Remove(*itPack))
			{
				XtUt::AppendEx(&remaining, *itPack);
				totalResult = false;
			}
		}

		XtUt::Replace(&db->removedPacks, &remaining);

		// Save db

		if (db->dirty)
		{
			XtString dbFilename = Format("%sdb.xtd", db->path.Text());

			if (!Xt::Export2(&db->properties, dbFilename) || !storage->Dirty(dbFilename))
				totalResult = false;
		}

		// Save packs

		for(XtUt::Dictionary<XtId, DomPack>::Iterator itPack(&db->packs); itPack; ++itPack)
		{
			if (!itPack->dirty)
				continue;

			XtLog("XTDB", "Pack %s dirty, dirty records are:", itPack->name.Text());

			for(Xt::DomPack::Iterator itRecord(itPack); itRecord; ++itRecord)
			{
				if (itRecord->dirty)
					XtLog("XTDB", "    %s (%s)", itRecord->name.Text(), itRecord->root.type.Text());
			}

			if (*itPack->filename)
			{
				XtSimpleFile f;

				if (f.Open(itPack->filename, false))
				{
					if (itPack->filetime != f.LastWriteTime())
					{
						Xt::StatusCallback::Warning(status, "Pack file is newer on disk than in memory, overwriting anyway: %s", itPack->filename.Text());
					}
				}
			}

			XtString packFilename = Format("%spacks\\%s-%08X.xtd", (const char*)db->path, (const char*)itPack->name, itPack->id);

			if (_stricmp(packFilename, itPack->filename) == 0)
				packFilename = itPack->filename;

			XtString packFilenameNew = packFilename + ".new";

			bool success = false;

			XtUt::Block pristine;

			if (Xt::ExportPack(itPack, packFilenameNew, &pristine))
			{
				if (MoveFileEx(packFilenameNew, packFilename, MOVEFILE_REPLACE_EXISTING))
					success = true;
			}

			if (success)
			{
				if (!*itPack->filename)
					success = storage->New(packFilename);
				else if (itPack->filename == packFilename)
					success = storage->Dirty(packFilename);
				else
					success = storage->Rename(itPack->filename, packFilename);

				if (success)
				{
					for(DomPack::Iterator itRecord(itPack); itRecord; ++itRecord)
						itRecord->dirty = false;

					itPack->filename	= packFilename;
					itPack->dirty		= false;

					XtUt::Remove(&db->pristine, FirstOf(&db->pristine, itPack->id), LastOf(&db->pristine, itPack->id));

					XtUt::Block* newPristine = new XtUt::Block;

					if (!XtUt::Insert(&db->pristine, itPack->id, newPristine))
					{
						Xt::StatusCallback::Error(status, "Failed to update pristine copy for: %s", packFilename.Text());
						totalResult = false;
					}

					XtUt::ReplaceData(newPristine, &pristine);

					// Record latest filetime

					XtSimpleFile f;

					if (f.Open(itPack->filename, false))
					{
						itPack->filetime = f.LastWriteTime();
					}
				}
				else
					totalResult = false;
			}
			else
			{
				Xt::StatusCallback::Error(status, "Failed to save: %s", packFilename.Text());
				totalResult = false;
			}
		}

		if (totalResult)
			db->dirty = false;

		return totalResult;
	}

	bool Flush(DomDb* db, Storage* storage, StatusCallback* status)
	{
		if (!db->open)
			return false;

		XtLog("XTDB", "Flush %s", db->path.Text());

		bool result = Flush_Sub(db, storage, status);

		DeleteFile(db->path + ".lock");

		return result;
	}

	bool ReloadSchema(DomDb* db, StatusCallback* status)
	{
		if (!db)
			return false;

		DomDb tmp;

		if (!OpenSchema(&tmp, db->path + "schema\\", status))
			return false;

		::Swap(db->schema, tmp.schema);
		::Swap(db->exportSchema, tmp.exportSchema);

		return true;
	}

}