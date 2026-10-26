#include "stdafx.h"
#include <Tools_Shared/PlatformHelpers.h>
#include <Tools_Shared/File_Copy/FileCopy.h>

#include <EditorLib/Xt/XtObject.h>
#include <EditorLib/Xt/XtChange.h>
#include <EditorLib/Xt/XtManager.h>
#include <EditorLib/Xt/Export/XtBuildLevelPack.h>
#include <EditorLib/Dg/DgNodeIterator.h>
#include <EditorLib/Misc/AsyncHelpers.h>
#include <EditorLib/CoreConfig.h>
#include <EditorLib/Core.h>
#include <EditorLib/Level/XtLevel.h>

#include <Tools_Shared/PlatformHelpers.h>

namespace EditorLib
{
namespace XtBuildLevelPack
{

static const XtString kActorDataName = "actors";
static const XtString kNewActorDataName = "actorpack";

Btl::String BuildWorkingFolder(XtId packId, Util::E_Platform platform)
{
	Xt::DomField *pOutputField = 0;
	{
		//Find pack property block
		Xt::DomPack* pPack = EditorLib::C_Core::Instance().GetXtManager().FindPack(packId);
		if (!pPack)
		{
			LOGERROR(C_Core::Instance().GetLogger(), "Failed to read export location - invalid pack specified (id %08x)", packId);
			return "";
		}
		Xt::DomBlock *pPropsBlock = &pPack->properties;
		Xt::DomBlock *pEditorPropsBlock = Xt::AsBlock(Xt::At(&pPack->properties, "__ASProperties"));
		pOutputField = Xt::AsField(Xt::At(pEditorPropsBlock, "outputFolder"));
	}

	if (!pOutputField || pOutputField->value == kXtNullString)
	{
		LOGERROR(C_Core::Instance().GetLogger(), "No export location has been specified in pack (id %08x)", packId);
		return "";
	}

	const C_CoreConfig& coreConfig = C_Core::Instance().GetConfig();

	Btl::String rootProjectFolder = coreConfig.rootProjectPath;
	rootProjectFolder = Misc::RemoveTrailingSlash(rootProjectFolder);

	Btl::String partialOutputPath = pOutputField->value.Text();
	partialOutputPath = Misc::RemoveTrailingSlash(partialOutputPath);

	return
		rootProjectFolder + 
		Btl::String("/working/") +
		Util::GetPlatformString(platform) + Btl::String("/") +
		partialOutputPath;
}

Btl::String BuildTempFolder(XtId packId)
{
	Xt::DomField *pOutputField = 0;
	{
		//Find pack property block
		Xt::DomPack* pPack = EditorLib::C_Core::Instance().GetXtManager().FindPack(packId);
		if (!pPack)
			return "";

		Xt::DomBlock *pPropsBlock = &pPack->properties;
		Xt::DomBlock *pEditorPropsBlock = Xt::AsBlock(Xt::At(&pPack->properties, "__ASProperties"));
		pOutputField = Xt::AsField(Xt::At(pEditorPropsBlock, "outputFolder"));
	}

	if (!pOutputField)
		return "";

	const C_CoreConfig& coreConfig = C_Core::Instance().GetConfig();

	Btl::String rootProjectFolder = coreConfig.rootProjectPath;
	rootProjectFolder = Misc::RemoveTrailingSlash(rootProjectFolder);

	Btl::String partialOutputPath = pOutputField->value.Text();
	partialOutputPath = Misc::RemoveTrailingSlash(partialOutputPath);

	return
		rootProjectFolder + 
		Btl::String("/temp/") +
		partialOutputPath;
}

C_Status BuildPackBlocks(
	XtId packId,
	XtUt::Block& endianSwappedData,
	XtUt::Block& nonEndianSwappedData)
{
	Xt::DomDb* pDb = C_Core::Instance().GetXtManager().GetDb();
	assert(pDb);

	Debug::Logger& logger = C_Core::Instance().GetLogger();

	Xt::DomPack* pPack = EditorLib::C_Core::Instance().GetXtManager().FindPack(packId);
	if (!pPack)
		return E_Status_Failure;

	//Generate binary data.
	bool success = true;
	{
		XtTargetProfile profile;
		profile.endian = kXtBigEndian;

		XtPackBuildParams params;
		if (!XtPackBuild(&profile, pDb, pPack, &endianSwappedData, &params))
		{
			LOGFATAL(logger, "XT: failed to build pack (endian swapped version).\r\n");
			success = false;
		}

		profile.endian = kXtLittleEndian;

		if (!XtPackBuild(&profile, pDb, pPack, &nonEndianSwappedData, &params))
		{
			LOGFATAL(logger, "XT: failed to build pack.(non-endian swapped version)\r\n");
			success = false;
		}
	}
	
	return success ? E_Status_Success : E_Status_Failure;
}

struct SorceryBuildParams : public XtBuildParams
{
	AsyncOp*	op;
	int			warnings;
	int			errors;
	bool		interactive;

public:
	SorceryBuildParams() :
		op(0),
		warnings(0),
		errors(0),
		interactive(true)
	{
	}

public:
	virtual void Output(XtSeverity severity, const char* text)
	{

		//Avoid modal dialog.
		if (!interactive)
			severity = kXtSevInfo;

		if (op)
		{
			switch(severity)
			{
				case kXtSevInfo:	op->NewMessage(kAoiInfo, text);		break;
				case kXtSevWarning:	op->NewMessage(kAoiWarning, text);	break;
				case kXtSevError:	op->NewMessage(kAoiError, text);	break;
				case kXtSevTrace:	op->AppendMessage(text);			break;
			}
		}
		if (severity == kXtSevWarning)
			warnings++;

		if (severity == kXtSevError)
			errors++;
	}

	virtual void Progress(float fraction)
	{
		if (op) op->SetProgress(fraction);
	}

	virtual bool IsCancelled()
	{
		return op ? op->IsCancelled() : false;
	}
};

bool BuildDirtyPacks(AsyncOp* op, C_PackExportParams* pExpParams)
{
	SorceryBuildParams params;

	params.db = C_Core::Instance().GetXtManager().GetDb();
	params.op = op;

	params.interactive = C_Core::Instance().GetConfig().appType == E_AppType_Interactive;
	params.doData			= true;
	params.doLocalisation	= false;
	params.doDeploy = params.interactive;

	if (op)
	{
		op->SetTitle("Exporting...");
		op->NewMessage(kAoiInfo, "Exporting:");
		op->AppendMessage("Database: " + params.db->path);
	}

	Btl::Vector<Xt::DomPack*> packsToDeploy;
	C_Core::Instance().GetXtManager().GetPacksPendingDeployment(packsToDeploy);
	Xt::DomPack* pLevelPack = C_Core::Instance().GetXtManager().FindPack(pExpParams->packId);
	bool foundLevelPack = false;
	for (S32 i = 0; i < packsToDeploy.Size(); ++i)
	{
		if (packsToDeploy[i] == pLevelPack)
		{
			foundLevelPack = true;
			break;
		}
	}
	//Ensure we always export our level pack.
	if (!foundLevelPack)
		packsToDeploy.PushBack(pLevelPack);

	Xt::DomPack** toCopy = XtUt::AppendExN(&params.packs, packsToDeploy.Size());
	memcpy(toCopy, &packsToDeploy[0], packsToDeploy.Size()*sizeof(Xt::DomPack*));

	params.overridePacks = true;
	XtBuild(&params);
	C_Core::Instance().GetXtManager().ClearPacksPendingDeployment();

	if ((params.warnings > 0) || (params.errors > 0))
	{
		XtString msg = Format("Export failed, %i warning(s) and %i error(s)", params.warnings, params.errors);

		if (op)
		{
			op->SetTitle("Export failed");
			op->NewMessage(kAoiWarning, msg);
		}
	}
	else
	{
		if (op) 
		{
			op->SetTitle("Export complete");
			op->NewMessage(kAoiInfo, "Export complete");
		}
	}

	return params.errors == 0;
}

bool Build(HWND hwnd, C_PackExportParams* pExpParams)
{
	return AsyncTask(hwnd, BuildDirtyPacks, pExpParams);
}



}
}
