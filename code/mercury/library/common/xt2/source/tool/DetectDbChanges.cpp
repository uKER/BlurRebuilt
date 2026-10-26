#include "Common/XtPch.h"
#include "Xt/XtTool.h"

void DdcTest(const Sz8& dbPath)
{
	return;

	HANDLE h = CreateFile(dbPath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, 0);

	if (h == INVALID_HANDLE_VALUE)
		return;

	const int kBufSize = 16384;

	XtU8 buf[kBufSize];

	for(;;)
	{
		DWORD byteCount = 0;

		BOOL result = ReadDirectoryChangesW(h, buf, kBufSize, TRUE,
				FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION,
			&byteCount, 0, 0);

		if (result)
		{
			FILE_NOTIFY_INFORMATION* fni = (FILE_NOTIFY_INFORMATION*)buf;

			for(;;)
			{
				Sz16 filename(fni->FileName, fni->FileNameLength / 2);

				filename = MbToWc(dbPath) + filename;

				if (IndexOfStringI(filename, L".svn") < 0)
				{
					switch(fni->Action)
					{
						case FILE_ACTION_ADDED:				DebugLn("A %S", filename.Text());	break;
						case FILE_ACTION_REMOVED:			DebugLn("R %S", filename.Text());	break;
						case FILE_ACTION_MODIFIED:			DebugLn("M %S", filename.Text());	break;
						case FILE_ACTION_RENAMED_OLD_NAME:	DebugLn("rO %S", filename.Text());	break;
						case FILE_ACTION_RENAMED_NEW_NAME:	DebugLn("rN %S", filename.Text());	break;
						default:							DebugLn("? %S", filename.Text());	break;
					}
				}

				if (fni->NextEntryOffset <= 0)
					break;

				fni = (FILE_NOTIFY_INFORMATION*)(((XtU8*)fni) + fni->NextEntryOffset);
			}
		}
		else
		{
			DebugLn("... %i", GetLastError());
		}
	}

	CloseHandle(h);
}