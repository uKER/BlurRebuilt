#include "StdAfx.h"
#include <Xt/XtGame.h>
#include "system\serialise.h"
//#include "AudioDebug.h"
#include "SorceryIntegration.h"

//
// based on code from GW3
//


XtDb*				gXtDb;
XtCallbackContext	gXtContext('AUD_');
XtPack*				gXtPack;

static Debug::Logger &g_Log = Debug::GetRootLogger();


//-----------------------------------------------------------------------------
// GameDbOpenCallback
//-----------------------------------------------------------------------------
void GameDbOpenCallback(struct XtPackOpenStatus* status, void* context)
{
	LOGINFO(g_Log, "XtOpen: %s", status->message);
}


//-----------------------------------------------------------------------------
// SorceryDbInit
//-----------------------------------------------------------------------------
void SorceryDbInit()
{
	if (!XtInitialise())
		return;

	gXtDb = XtDbCreate(XtGetApplicationSchema(), 200, &gXtContext);

	XtNetworkStart(gXtDb);

	Util::C_String strDbName = "GAME:\\data\\data.bin";

	Sys::I_File* pFile = Sys::C_FileSystem::Instance().Open(strDbName.Chars());

	if (pFile)
	{
		int	length	= pFile->Length();
		U8*	data	= new U8[length];

		if (data)
		{
			if (pFile->Read(data, length))
			{
				XtPackOpenParams params;

				params.callback = GameDbOpenCallback;

				gXtPack = XtPackOpen(gXtDb, data, length, &params);

				if (gXtPack)
					LOGINFO(g_Log, "Opened sorcery database '%s' ok\n", strDbName.Chars());
				else
					LOGWARN(g_Log, "XtPackOpen failed on sorcery database '%s'\n", strDbName.Chars());
			}
			else
				LOGWARN(g_Log, "Opened sorcery database file '%s' but could not read from it\n", strDbName.Chars());

			delete [] data;
		}
		else
			LOGWARN(g_Log, "Could not allocate %d bytes for buffer for reading in sorcery database\n", length);

		delete pFile;
	}
	else
		LOGWARN(g_Log, "Failed to open sorcery database file '%s'\n", strDbName.Chars());
}