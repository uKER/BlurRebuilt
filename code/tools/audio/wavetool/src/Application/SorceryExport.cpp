#include "Core/StdAfx.h"
#include "Application/SorceryApp.h"
#include "Application/SorceryView.h"
#include "Application/SorceryExport.h"
#include "Xt/XtTool.h"
#include "Xt/Impl/XtSimpleFile.h"
#include <xbdm.h>

struct SorceryBuildParams : public XtBuildParams
{
	AsyncOp*	op;
	int			warnings;
	int			errors;

public:
	SorceryBuildParams() :
		op(0),
		warnings(0),
		errors(0)
	{
	}

public:
	virtual void Output(XtSeverity severity, const char* text)
	{
		switch(severity)
		{
			case kXtSevInfo:	op->NewMessage(kAoiInfo, text);		break;
			case kXtSevWarning:	op->NewMessage(kAoiWarning, text);	break;
			case kXtSevError:	op->NewMessage(kAoiError, text);	break;
			case kXtSevTrace:	op->AppendMessage(text);			break;
		}

		if (severity == kXtSevWarning)
			warnings++;

		if (severity == kXtSevError)
			errors++;
	}

	virtual void Progress(float fraction)
	{
		op->SetProgress(fraction);
	}

	virtual bool IsCancelled()
	{
		return op->IsCancelled();
	}
};

struct SorceryExportParams
{
	SorceryApp*		app;
	bool			dataOnly;
	bool			dontDeploy;
	Xt::DomPack*	pack;
};

bool SorceryExportSub(AsyncOp* op, SorceryExportParams* sep)
{
	SorceryBuildParams params;

	params.db = sep->app->_db;
	params.op = op;

	if (sep->dataOnly)
	{
		params.doData			= true;
		params.doLocalisation	= false;
		params.doDeploy			= true;
	}

	if (sep->dontDeploy)
		params.doDeploy = false;

	if (sep->pack)
	{
		params.overridePacks = true;

		XtUt::AppendEx(&params.packs, sep->pack);
	}

	params.overridePlatforms = true;

	for(XtUt::List<Xt::BuildPlatform>::Iterator itPlatform(&sep->app->_db->build.platforms); itPlatform; ++itPlatform)
	{
		bool found = false;

		for(XtUt::Array<XtString>::Iterator itSkip(&sep->app->_skipPlatforms); itSkip; ++itSkip)
		{
			if (itSkip == itPlatform->name)
			{
				found = true;
				break;
			}
		}

		if (!found)
			XtUt::AppendEx(&params.platforms, itPlatform->name);
	}

	op->SetTitle("Exporting...");
	op->NewMessage(kAoiInfo, "Exporting:");
	op->AppendMessage("Database: " + sep->app->_db->path);

	XtBuild(&params);

	if ((params.warnings > 0) || (params.errors > 0))
	{
		op->SetTitle("Export failed");

		XtString msg = Format("Export failed, %i warning(s) and %i error(s)", params.warnings, params.errors);

		op->NewMessage(kAoiWarning, msg);
	}
	else
	{
		op->SetTitle("Export complete");
		op->NewMessage(kAoiInfo, "Export complete");
	}

	return true;
}

void SorceryExport(SorceryApp& app, bool dataOnly, bool dontDeploy, Xt::DomPack* pack)
{
	SorceryExportParams params;

	params.app			= &app;
	params.dataOnly		= dataOnly;
	params.dontDeploy	= dontDeploy;
	params.pack			= pack;

	AsyncInvoke(app._window, SorceryExportSub, &params);
}