#include "Common/XtPch.h"
#include <Xt/XtTool.h>

namespace Xt
{

	bool OpenConfig(DomDb* db, const XtString& path, StatusCallback* status, int& version)
	{
		DomBlock dom;

		if (!Import2(&dom, path, 0, status))
			return false;

		if ((version = atoi(AsText(At(&dom, "__version")))) > XtGetDbVersion())
		{
			StatusCallback::Error(status, "Database has been edited with a newer version of sorcery, please grab the latest tools");
			return false;
		}

		if (version <= 16)
		{
			StatusCallback::Error(status, "Database must be converted before opening with this version of sorcery, please grab the latest tools");
			return false;
		}

		db->name = AsText(At(&dom, "name"));

		if (DomBlock* domLanguages = AsBlock(At(&dom, "languages")))
		{
			for(DomBlock::Iterator it(domLanguages); it; ++it)
			{
				if (DomBlock* domLang = AsBlock(it))
				{
					DomLanguage* lang = new DomLanguage;

					if (!lang)
						return false;

					lang->name = it.Key();
					lang->code = AsText(At(domLang, "code"));

					if (!AppendEx(&db->languages, lang))
						return false;
				}
			}
		}

		if (DomBlock* domBuild = AsBlock(At(&dom, "build")))
		{
			if (DomBlock* domStructure = AsBlock(At(domBuild, "structure")))
			{
				db->build.structure.packs			= AsText(At(domStructure, "packs"));
				db->build.structure.localisation	= AsText(At(domStructure, "localisation"));

				CleanUpPath(db->build.structure.packs, false, false);
				CleanUpPath(db->build.structure.localisation, false, false);
			}

			if (DomBlock* domPlatform = AsBlock(At(domBuild, "platforms")))
			{
				for(DomBlock::Iterator it(domPlatform); it; ++it)
				{
					if (DomBlock* domPlat = AsBlock(it))
					{
						BuildPlatform* plat = new BuildPlatform;

						if (!AppendEx(&db->build.platforms, plat))
							return false;

						plat->name = it.Key();

						XtString endian = AsText(At(domPlat, "endian"));

						if (_stricmp(endian, "big") == 0)
							plat->endian = kXtBigEndian;
						else if (_stricmp(endian, "little") == 0)
							plat->endian = kXtLittleEndian;
						else if (*endian)
						{
							StatusCallback::Error(status, "Bad endian for platform '%s'", (const char*)plat->name);
							return false;
						}
					}
				}
			}

			if (DomBlock* domStaging = AsBlock(At(domBuild, "staging")))
			{
				for(DomBlock::Iterator it(domStaging); it; ++it)
				{
					if (DomBlock* domStage = AsBlock(it))
					{
						BuildStage* stage = new BuildStage;

						if (!AppendEx(&db->build.staging, stage))
							return false;

						stage->name = it.Key();
						stage->root = AsText(At(domStage, "root"));

						CleanUpPath(stage->root, false, true);

						XtString platform = AsText(At(domStage, "platform"));

						for(XtUt::List<BuildPlatform>::Iterator itPlat(&db->build.platforms); itPlat; ++itPlat)
						{
							if (itPlat->name == platform)
								stage->platform = itPlat;
						}

						if (!stage->platform)
						{
							StatusCallback::Error(status, "Missing platform for stage '%s'", (const char*)stage->name);
							return false;
						}
					}
				}
			}

			if (DomBlock* domDeploy = AsBlock(At(domBuild, "deployment")))
			{
				for(DomBlock::Iterator it(domDeploy); it; ++it)
				{
					if (DomBlock* domDep = AsBlock(it))
					{
						BuildDeploy* deploy  = new BuildDeploy;

						if (!AppendEx(&db->build.deployment, deploy))
							return false;

						deploy->name	= it.Key();
						deploy->method	= AsText(At(domDep, "method"));
						deploy->root	= AsText(At(domDep, "root"));

						CleanUpPath(deploy->root, false, true);

						XtString stage = AsText(At(domDep, "stage"));

						for(XtUt::List<BuildStage>::Iterator itStage(&db->build.staging); itStage; ++itStage)
						{
							if (itStage->name == stage)
								deploy->stage = itStage;
						}

						if (!deploy->stage)
						{
							StatusCallback::Error(status, "Missing stage for deploy '%s'", (const char*)deploy->name);
							return false;
						}
					}
				}
			}
		}

		return true;
	}

	bool Flush_Config(DomDb* db)
	{
		DomBlock dom;

		char version[16];
		_snprintf_s(version, sizeof(version), _TRUNCATE, "%i", XtGetDbVersion());

		if (!Set(&dom, "__version", (XtString)version))
			return false;

		if (!Set(&dom, "name", db->name))
			return false;

		DomBlock* domLanguages = Set(&dom, "languages", new DomBlock);

		if (!domLanguages)
			return false;

		for(XtUt::List<DomLanguage>::Iterator it(&db->languages); it; ++it)
		{
			DomBlock* domLang = Set(domLanguages, it->name, new DomBlock);

			if (!domLang)
				return false;

			if (!Set(domLang, "code", it->code))
				return false;
		}

		DomBlock* domBuild = Set(&dom, "build", new DomBlock);

		if (!domBuild)
			return false;

		DomBlock* domStructure	= Set(domBuild, "structure", new DomBlock);
		DomBlock* domPlatforms	= Set(domBuild, "platforms", new DomBlock);
		DomBlock* domStaging	= Set(domBuild, "staging", new DomBlock);
		DomBlock* domDeployment	= Set(domBuild, "deployment", new DomBlock);

		if (!domStructure || !domPlatforms || !domStaging || !domDeployment)
			return false;

		if (!Set(domStructure, "packs", db->build.structure.packs))
				return false;

		if (!Set(domStructure, "localisation", db->build.structure.localisation))
				return false;

		for(XtUt::List<BuildPlatform>::Iterator it(&db->build.platforms); it; ++it)
		{
			DomBlock* domPlatform = Set(domPlatforms, it->name, new DomBlock);

			if (!domPlatform)
				return false;

			if (!Set(domPlatform, "endian", (XtString)((it->endian == kXtBigEndian) ? "big" : "little")))
				return false;
		}

		for(XtUt::List<BuildStage>::Iterator it(&db->build.staging); it; ++it)
		{
			DomBlock* domStage = Set(domStaging, it->name, new DomBlock);

			if (!domStage)
				return false;

			if (!Set(domStage, "platform", it->platform->name))
				return false;

			if (!Set(domStage, "root", it->root))
				return false;
		}

		for(XtUt::List<BuildDeploy>::Iterator it(&db->build.deployment); it; ++it)
		{
			DomBlock* domDeploy = Set(domDeployment, it->name, new DomBlock);

			if (!domDeploy)
				return false;

			if (!Set(domDeploy, "stage", it->stage->name))
				return false;

			if (!Set(domDeploy, "method", it->method))
				return false;

			if (!Set(domDeploy, "root", it->root))
				return false;
		}

		return Export2(&dom, db->path + "config.xtp");
	}

}