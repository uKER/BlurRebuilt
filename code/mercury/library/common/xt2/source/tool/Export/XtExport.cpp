#include "Common/XtPch.h"
#include "Xt/XtTool.h"
#include <xbdm.h>

bool XtLocExport(XtTargetProfile* profile, XtUt::Array<XtString>* expLangs, Xt::DomRecord* record, XtUt::Block* data);

XtBuildParams::XtBuildParams() :
	db(0),
	doData(true),
	doLocalisation(true),
	doDeploy(true),
	overrideStage(false),
	overridePacks(false),
	overridePlatforms(false)
{
}

XtBuildParams::~XtBuildParams()
{
}

void XtBuildParams::Output(XtSeverity severity, const char* text)
{
	switch(severity)
	{
		case kXtSevInfo:	DebugLn("XtBuild: [INF] %s", text); break;
		case kXtSevWarning:	DebugLn("XtBuild: [WRN] %s", text); break;
		case kXtSevError:	DebugLn("XtBuild: [ERR] %s", text); break;
		case kXtSevTrace:	DebugLn("XtBuild:       %s", text); break;
	}
}

void XtBuildParams::Progress(float fraction)
{
}

bool XtBuildParams::IsCancelled()
{
	return false;
}

namespace
{
	void OutputMsg(XtBuildParams* params, XtSeverity severity, const char* text, va_list vl)
	{
		char buffer[512];

		_vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, text, vl);

		params->Output(severity, buffer);
	}

	void MsgInfo(XtBuildParams* params, const char* text, ...)
	{
		va_list vl;

		va_start(vl, text);
		XtLogV("STAT", text, vl);
		va_end(vl);

		va_start(vl, text);
		OutputMsg(params, kXtSevInfo, text, vl);
		va_end(vl);
	}

	void MsgWarning(XtBuildParams* params, const char* text, ...)
	{
		va_list vl;

		va_start(vl, text);
		XtLogV("STAT", text, vl);
		va_end(vl);

		va_start(vl, text);
		OutputMsg(params, kXtSevWarning, text, vl);
		va_end(vl);
	}

	void MsgError(XtBuildParams* params, const char* text, ...)
	{
		va_list vl;

		va_start(vl, text);
		XtLogV("STAT", text, vl);
		va_end(vl);

		va_start(vl, text);
		OutputMsg(params, kXtSevError, text, vl);
		va_end(vl);
	}

	void MsgTrace(XtBuildParams* params, const char* text, ...)
	{
		va_list vl;

		va_start(vl, text);
		XtLogV("STAT", text, vl);
		va_end(vl);

		va_start(vl, text);
		OutputMsg(params, kXtSevTrace, text, vl);
		va_end(vl);
	}

	XtString GetWindowsErrorMessage(int errorCode)
	{
		char* message;

		XtString result;

		if (FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				0, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&message, 0, 0
			) >= 0)
		{
			for(char* ptr = message; *ptr != '\0'; ptr++)
			{
				if (!isprint(*ptr))
					*ptr = ' ';
			}

			result = Format("%s", message);

			LocalFree(message);
		}
		else
		{
			result = Format("Error code %i", errorCode);
		}

		return result;
	}

	XtString GetDefaultX360()
	{
		XtString result;

		const int kMaxChars = 256;

		if (char* p = result.Capture(kMaxChars, false))
		{
			DWORD length = kMaxChars;

			if (FAILED(DmGetXboxName(p, &length)))
			{
				_snprintf_s(p, kMaxChars, _TRUNCATE, "(unknown)");
				length = 9;
			}

			result.Truncate(length);
		}

		return result;
	}

	bool CreateDirectories(XtBuildParams* params, const XtString& path)
	{
		const char* start = path;
		const char* drive = start;

		if ((drive[0] == '\\') && (drive[1] == '\\'))
		{
			while((*drive == '\\') || (*drive == '/'))
				drive++;

			while((*drive != '\\') && (*drive != '/') && (*drive != '\0'))
				drive++;
		}
		else
		{
			for(const char* p = path; *p; p++)
			{
				if (*p == ':')
				{
					drive = p + 1;
					break;
				}
			}
		}

		do
		{
			while((*drive == '\\') || (*drive == '/'))
				drive++;

			while((*drive != '\\') && (*drive != '/') && (*drive != '\0'))
				drive++;

			if (*drive == '\0')
				break;

			XtString subPath(start, (int)(drive - start));

			if (CreateDirectory(subPath, 0))
			{
				MsgInfo(params, "Created directory: %s", (const char*)subPath);
			}
			else
			{
				if (GetLastError() != ERROR_ALREADY_EXISTS)
				{
					XtString error = GetWindowsErrorMessage(GetLastError());

					MsgWarning(params, "Failed to create directory: %s", (const char*)subPath);
					MsgTrace(params, "%s", (const char*)error);

					return false;
				}
			}
		}
		while(*drive != '\0');

		return true;
	}

	bool CreateDirectoriesX360(XtBuildParams* params, const XtString& path)
	{
		const char* start = path;
		const char* drive = start;

		for(const char* p = path; *p; p++)
		{
			if (*p == ':')
			{
				drive = p + 1;
				break;
			}
		}

		do
		{
			while((*drive == '\\') || (*drive == '/'))
				drive++;

			while((*drive != '\\') && (*drive != '/') && (*drive != '\0'))
				drive++;

			if (*drive == '\0')
				break;

			XtString subPath(start, (int)(drive - start));

			HRESULT result = DmMkdir(subPath);

			if (SUCCEEDED(result))
			{
				MsgInfo(params, "Created directory on x360: %s", (const char*)subPath);
			}
			else
			{
				if (result != XBDM_ALREADYEXISTS)
				{
					XtString xbox = GetDefaultX360();

					MsgWarning(params, "Failed to create directory on x360: %s", (const char*)subPath);
					MsgTrace(params, "Devkit: %s", (const char*)xbox);

					return false;
				}
			}
		}
		while(*drive != '\0');

		return true;
	}

	bool AppendFormatString(XtString& output, const XtString& format, XtUt::Dictionary<XtString, XtString>* substitutions)
	{
		const char* start	= format;
		const char* ptr		= format;

		for(;;)
		{
			while((*ptr != '%') && (*ptr != '\0'))
				ptr++;

			if (!XtsAppend(output, start, ptr - start))
				return false;

			if (*ptr == '\0')
				break;

			start = ptr + 1;

			for(ptr++; (*ptr != '%') && (*ptr != '\0'); )
				ptr++;

			if (*ptr == '\0')
				return false;

			if (start != ptr)
			{
				XtString code(start, ptr - start);

				if (XtString* translated = At(substitutions, code))
				{
					if (!XtsAppend(output, *translated, translated->Length()))
						return false;
				}
			}
			else
			{
				if (!XtsAppend(output, "%", 1))
					return false;
			}

			start	= ptr + 1;
			ptr		= start;
		}

		return true;
	}

	struct XtExportPbp : public XtPackBuildParams
	{
		Xt::DomPack*	pack;
		XtBuildParams*	params;
		bool			firstMsg;

	public:
		XtExportPbp(Xt::DomPack* pack_, XtBuildParams* params_) :
			pack(pack_),
			params(params_),
			firstMsg(false)
		{
		}

	public:
		virtual void Output(XtSeverity severity, const char* text)
		{
			if (!firstMsg)
			{
				MsgInfo(params, "Compiling Pack: %s", pack->name.Text());
				firstMsg = true;
			}

			params->Output(severity, text);
		}
	};

	void BuildPack(XtBuildParams* params, XtTargetProfile* profile, Xt::DomPack* pack, const XtString& filename)
	{
		XtUt::Block data;
		XtExportPbp pbp(pack, params);

		DWORD start = GetTickCount();

		if (XtPackBuild(profile, params->db, pack, &data, &pbp))
		{
			XtSimpleFile f;

			if (!f.Open(filename, true) || !f.Write(data.start, (int)Capacity(&data)))
			{
				XtString error = GetWindowsErrorMessage(GetLastError());

				MsgError(params, "Failed to save pack: %s", (const char*)pack->name);
				MsgTrace(params, "%s", (const char*)error);
				MsgTrace(params, "%s", (const char*)filename);
			}
			else
			{
				MsgInfo(params, "Written: %s", (const char*)filename);
			}
		}

		DWORD end = GetTickCount();

		DebugLn("%s: %.2f", filename.Text(), (end - start) / 1000.0f);
	}

	void BuildLoc(XtBuildParams* params, XtTargetProfile* profile, XtUt::Array<XtString>* langs, Xt::DomRecord* record, const XtString& filename)
	{
		XtUt::Block data;

		if (XtLocExport(profile, langs, record, &data))
		{
			XtSimpleFile f;

			if (!f.Open(filename, true) || !f.Write(data.start, (int)Capacity(&data)))
			{
				XtString error = GetWindowsErrorMessage(GetLastError());

				MsgError(params, "Failed to save pack: %s", (const char*)record->name);
				MsgTrace(params, "%s", (const char*)error);
				MsgTrace(params, "%s", (const char*)filename);
			}
			else
			{
				MsgInfo(params, "Written: %s", (const char*)filename);
			}
		}
		else
		{
			XtString languages;

			for(XtUt::Array<XtString>::Iterator it(langs); it; ++it)
				languages += " " + *it;

			MsgError(params, "Failed to compile localisation: %s", (const char*)record->name);
			MsgTrace(params, "Languages:%s", (const char*)languages);
		}
	}

	struct DeployFile
	{
		XtString filename;
	};

	struct DeployStage
	{
		XtString				name;
		XtString				path;
		XtUt::List<DeployFile>	files;
	};

	struct DeployContext
	{
		XtUt::List<DeployStage> staging;
	};

	DeployStage* FindDeployStage(DeployContext* dc, const XtString& name)
	{
		for(XtUt::List<DeployStage>::Iterator itStage(&dc->staging); itStage; ++itStage)
			if (itStage->name == name)
				return itStage;

		return 0;
	}

	void DeployX360(XtBuildParams* params, Xt::BuildDeploy* deploy, DeployStage* stage, int& estimateDeployProgress, int estimateDeployCount)
	{
		for(XtUt::List<DeployFile>::Iterator itFile(&stage->files); itFile; ++itFile)
		{
			if (params->IsCancelled())
				break;

			params->Progress(0.5f + 0.5f * ((float)estimateDeployProgress++ / (float)estimateDeployCount));

			XtString localFile	= stage->path + itFile->filename;
			XtString remoteFile	= deploy->root + itFile->filename;

			if (FAILED(DmSendFile(localFile, remoteFile)))
			{
				if (params->IsCancelled())
					break;

				CreateDirectoriesX360(params, remoteFile);

				if (params->IsCancelled())
					break;

				HRESULT hr = DmSendFile(localFile, remoteFile);

				if (FAILED(hr))
				{
					XtString xbox = GetDefaultX360();

					char buf[512];

					MsgError(params, "Copy to x360 failed: %s", (const char*)remoteFile);
					MsgTrace(params, "Source: %s", (const char*)localFile);
					MsgTrace(params, "Devkit: %s", (const char*)xbox);

					if (FAILED(DmTranslateError(hr, buf, 512)))
						MsgTrace(params, "Error: %08X", hr);
					else
						MsgTrace(params, "Error: %s", buf);
				}
			}
		}
	}

	void DeployPc(XtBuildParams* params, Xt::BuildDeploy* deploy, DeployStage* stage, int& estimateDeployProgress, int estimateDeployCount)
	{
		for(XtUt::List<DeployFile>::Iterator itFile(&stage->files); itFile; ++itFile)
		{
			if (params->IsCancelled())
				break;

			params->Progress(0.5f + 0.5f * ((float)estimateDeployProgress++ / (float)estimateDeployCount));

			XtString localFile	= stage->path + itFile->filename;
			XtString remoteFile	= deploy->root + itFile->filename;

			if (CopyFile(localFile, remoteFile, FALSE) == FALSE)
			{
				if (params->IsCancelled())
					break;

				CreateDirectories(params, remoteFile);

				if (params->IsCancelled())
					break;

				if (CopyFile(localFile, remoteFile, FALSE) == FALSE)
				{
					XtString error = GetWindowsErrorMessage(GetLastError());

					MsgError(params, "Copy failed: %s", (const char*)remoteFile);
					MsgTrace(params, "Error:  %s", (const char*)error);
					MsgTrace(params, "Source: %s", (const char*)localFile);
				}
			}
		}
	}

	bool IsPlatformActive(XtBuildParams* params, const XtString& platform)
	{
		if (!params->overridePlatforms)
			return true;

		for(XtUt::Array<XtString>::Iterator itPlatform(&params->platforms); itPlatform; ++itPlatform)
		{
			if (itPlatform == platform)
				return true;
		}

		return false;
	}
}

void XtBuild(XtBuildParams* params)
{
	Xt::DomDb*		db		= params->db;
	Xt::DomBuild*	build	= &db->build;

	DeployContext dc;

	//
	// //
	// // Build
	// //
	//

	// Select list of pack files to process

	XtUt::Array<Xt::DomPack*>	dbPacks;
	XtUt::Array<Xt::DomPack*>*	packs = &dbPacks;

	if (!params->overridePacks)
	{
		for(Xt::DomDb::Iterator itPack(db); itPack; ++itPack)
		{
			if (atoi(AsText(At(&itPack->properties, "noDefaultExport"))) != 0)
				continue;

			if (!XtUt::AppendEx(&dbPacks, &*itPack))
			{
				MsgWarning(params, "Out of memory");
				return;
			}
		}
	}
	else
		packs = &params->packs;

	// Estimate number of staging items for progress bar

	int estimatedStageCount = 0;

	for(XtUt::Array<Xt::DomPack*>::Iterator itPack(packs); itPack; ++itPack)
	{
		for(Xt::DomPack::Iterator itRecord(itPack); itRecord; ++itRecord)
		{
			if (itRecord->root.type != "XtLocTable")
				continue;

			estimatedStageCount++;
		}
	}

	estimatedStageCount = 1 + (((estimatedStageCount * db->languages.count) + db->packs.pairs.elements.count) * build->staging.count);

	int estimatedStageProgress = 0;

	float progressFraction = params->doDeploy ? 0.5f : 1.0f;

	// Create substitutions for stage root formatting

	XtUt::Dictionary<XtString, XtString> stageSubstitutions;

	if (!XtUt::Insert(&stageSubstitutions, "db", new XtString(db->path)))
		return;

	if (build->staging.count <= 0)
	{
		MsgWarning(params, "There's nothing to build");
		MsgTrace(params, "No stages defined");
	}

	for(XtUt::List<Xt::BuildStage>::Iterator itStage(&build->staging); itStage; ++itStage)
	{
		if (params->IsCancelled())
		{
			MsgError(params, "User cancelled");
			return;
		}

		if (params->overrideStage && (_stricmp(itStage->name, params->stageName) != 0))
			continue;

		if (!IsPlatformActive(params, itStage->platform->name))
			continue;

		MsgInfo(params, "Compiling stage: %s", (const char*)itStage->name);

		DeployStage* deployStage = XtUt::AppendEx(&dc.staging, new DeployStage);

		if (!deployStage)
		{
			MsgError(params, "Out of memory");
			MsgTrace(params, "new DeployStage");
			continue;
		}

		deployStage->name = itStage->name;

		// Generate the full path to the stage root, and make sure the directory exists

		if (!params->overrideStage || !*params->stagePath)
		{
			if (!AppendFormatString(deployStage->path, itStage->root, &stageSubstitutions))
			{
				MsgWarning(params, "Syntax error in stage path format: %s", (const char*)itStage->root);
				continue;
			}
		}
		else
			deployStage->path = params->stagePath;

		if (!CleanUpPath(deployStage->path, false, true))
		{
			MsgError(params, "Out of memory");
			MsgTrace(params, "CleanUpPath deployStage");
			continue;
		}

		MsgTrace(params, "Path: %s", (const char*)deployStage->path);

		CreateDirectories(params, deployStage->path);

		// Create target profile based on platform

		XtTargetProfile profile;

		profile.endian = itStage->platform->endian;

		//
		// Export all pack files
		//
	
		if (params->doData)
		{
			for(XtUt::Array<Xt::DomPack*>::Iterator itPack(packs); itPack; ++itPack)
			{
				params->Progress(progressFraction * ((float)estimatedStageProgress++ / (float)estimatedStageCount));

				if (params->IsCancelled())
				{
					MsgError(params, "User cancelled");
					return;
				}

				DeployFile* deployFile = XtUt::AppendEx(&deployStage->files, new DeployFile);

				if (!deployFile)
				{
					MsgError(params, "Out of memory");
					MsgTrace(params, "new DeployFile");
					continue;
				}

				// Create substitutions for pack filename formatting

				XtUt::Dictionary<XtString, XtString> packSubstitutions;

				if (!XtUt::Insert(&packSubstitutions, "pack", new XtString((*itPack)->name)))
				{
					MsgError(params, "Out of memory");
					MsgTrace(params, "Insert packSubstitutions");
					continue;
				}

				// Generate pack filename

				if (!AppendFormatString(deployFile->filename, build->structure.packs, &packSubstitutions))
				{
					MsgWarning(params, "Syntax error in structure pack path format: %s", (const char*)build->structure.packs);
					continue;
				}

				if (Xt::DomBlock* asProps = AsBlock(At(&(*itPack)->properties, "__ASProperties")))
				{
					XtString outputFolder = AsText(At(asProps, "outputFolder"));

					if (*outputFolder)
					{
						if (!CleanUpPath(outputFolder, false, true))
						{
							MsgError(params, "Out of memory");
							MsgTrace(params, "CleanUpPath actor studio output folder");
							continue;
						}

						deployFile->filename = outputFolder + "actorpack.xt";
					}
				}

				if (!CleanUpPath(deployFile->filename, false, false))
				{
					MsgError(params, "Out of memory");
					MsgTrace(params, "CleanUpPath deployFile");
					continue;
				}

				XtString fullPath = deployStage->path + deployFile->filename;

				CreateDirectories(params, fullPath);

				BuildPack(params, &profile, itPack, fullPath);
			}
		}

		//
		// Export all localisation data
		//

		if (params->doLocalisation)
		{
			for(XtUt::List<Xt::DomLanguage>::Iterator itLang(&db->languages); itLang; ++itLang)
			{
				// Create array which identifies which languages to export this pass

				XtUt::Array<XtString> langs;

				if (!AppendEx(&langs, itLang->name))
					continue;

				// For each XtLocTable

				for(XtUt::Array<Xt::DomPack*>::Iterator itPack(packs); itPack; ++itPack)
				{
					for(Xt::DomPack::Iterator itRecord(itPack); itRecord; ++itRecord)
					{
						if (itRecord->root.type != "XtLocTable")
							continue;

						if (params->IsCancelled())
						{
							MsgError(params, "User cancelled");
							return;
						}

						params->Progress(progressFraction * ((float)estimatedStageProgress++ / (float)estimatedStageCount));

						DeployFile* deployFile = XtUt::AppendEx(&deployStage->files, new DeployFile);

						if (!deployFile)
							continue;

						// Create substitutions for localisation filename formatting

						XtUt::Dictionary<XtString, XtString> locSubstitutions;

						XtString recordPath = Replace(itRecord->name, '.', '\\');

						if (
								!XtUt::Insert(&locSubstitutions, "code", new XtString(itLang->code)) ||
								!XtUt::Insert(&locSubstitutions, "lang", new XtString(itLang->name)) ||
								!XtUt::Insert(&locSubstitutions, "record", new XtString(recordPath))
							)
						{
							continue;
						}

						// Generate pack filename

						if (!AppendFormatString(deployFile->filename, build->structure.localisation, &locSubstitutions))
						{
							MsgWarning(params, "Syntax error in structure localisation path format: %s", (const char*)build->structure.localisation);
							continue;
						}

						if (!CleanUpPath(deployFile->filename, false, false))
							continue;

						XtString fullPath = deployStage->path + deployFile->filename;

						CreateDirectories(params, fullPath);

						BuildLoc(params, &profile, &langs, itRecord, fullPath);
					}
				}
			}
		}
	}

	if (!params->doDeploy)
		return;

	//
	// //
	// // Deploy
	// // 
	//

	// Estimate number of deploy items

	int estimateDeployCount = 1;

	for(XtUt::List<Xt::BuildDeploy>::Iterator itDeploy(&build->deployment); itDeploy; ++itDeploy)
	{
		DeployStage* deployStage = FindDeployStage(&dc, itDeploy->stage->name);

		if (!deployStage)
			continue;

		estimateDeployCount += deployStage->files.count;
	}

	int estimateDeployProgress = 0;

	// For each deployment

	for(XtUt::List<Xt::BuildDeploy>::Iterator itDeploy(&build->deployment); itDeploy; ++itDeploy)
	{
		if (params->IsCancelled())
		{
			MsgError(params, "User cancelled");
			return;
		}

		DeployStage* deployStage = FindDeployStage(&dc, itDeploy->stage->name);

		if (!deployStage)
			continue;

		MsgInfo(params, "Deploying to: %s", (const char*)itDeploy->name);
		MsgTrace(params, "Method: %s", (const char*)itDeploy->method);
		MsgTrace(params, "Stage:  %s", (const char*)deployStage->path);
		MsgTrace(params, "Dest:   %s", (const char*)itDeploy->root);

		if (_stricmp(itDeploy->method, "copy-x360") == 0)
			DeployX360(params, itDeploy, deployStage, estimateDeployProgress, estimateDeployCount);
		else if (_stricmp(itDeploy->method, "copy-pc") == 0)
			DeployPc(params, itDeploy, deployStage, estimateDeployProgress, estimateDeployCount);
		else
		{
			MsgError(params, "Unknown deployment method: %s", (const char*)itDeploy->method);
		}
	}

	if (params->IsCancelled())
	{
		MsgError(params, "User cancelled");
		return;
	}
}