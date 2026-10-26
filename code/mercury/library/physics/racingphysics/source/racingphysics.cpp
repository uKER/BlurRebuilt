#include "stdafx.h"
#include "racingphysics/racingphysics.h"

namespace RacingPhysics
{

XtDb*				gXtDb;
XtPack*				gXtPack;
XtCallbackContext	gXtContext('RPHS');

void xt_open_callback(XtPackOpenStatus *status,void *context)
{
	LOGWARN( g_RacingPhysicsLog,"Xt Open Status: %s",status->message);
}

void CreateXt(XtDb* database)
{
	XtInitialise();
	
	Sys::I_File* pFile = Sys::GetDefaultFileSystem()->Open("gamedata\\Xt\\racingphysics.bin", Sys::E_FileAccess_Read);

	if (pFile)
	{
		int length = (int) pFile->Length();
		U8* data = new U8[length];

		if (data)
		{
			if (pFile->Read(data, length))
			{
				XtPackOpenParams params;
				params.callback=xt_open_callback;

				if (!database)
					gXtDb = XtDbCreate(XtGetApplicationSchema(), 5000, &gXtContext);
				else
					gXtDb = database;

				RP_ASSERT(gXtDb);
				if (gXtDb)
					gXtPack	= XtPackOpen(gXtDb, data, length, &params);

				if (!database)
					XtNetworkStart(gXtDb);
			}
			delete [] data;
		}

		delete pFile;
	}
}

void DestroyXt()
{
	XtShutdown();
}

void UpdateXt()
{
	XtNetworkUpdate();
}

}
