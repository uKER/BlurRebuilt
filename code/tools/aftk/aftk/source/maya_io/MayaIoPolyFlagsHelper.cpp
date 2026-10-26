#include "stdafx.h"
#include "MayaIoPolyFlagsHelper.h"
#include "system/serialise/C_FileSystem.h"
#include "system/serialise/TextSerialise.h"

using namespace MayaIo;
/*
bool PolyFlagsHelper::Validate(
	const char* pConfigFile)
{
	PolyFlags polyFlags;
	PolyFlagEnumTable polyFlagEnums;

	if (!ParseConfigFile(pConfigFile, polyFlags, polyFlagEnums))
		return false;

	MStatus mStatus;
	for (MItDependencyNodes it(MFn::kInvalid); !it.isDone(); it.next())
	{
		//Ignore everything but sets.
		if (!it.item().hasFn(MFn::kSet))
			continue;

		MFnSet fnSet(it.thisNode(), &mStatus);
		MCHECK_BEGIN << "Could not acquire function set from graph node." MCHECK_ERROR_RETURN_END_1;

		//Only interested in sets which are restricted to operating on faces only.
		if (fnSet.restriction() == MFnSet::kFacetsOnly)
		{
			//Get PolySetName;
			std::string polySetName(fnSet.name().asChar());
			//Remove namespacing for references.
			std::string::size_type pos = polySetName.find("polySet");
			if (pos != std::string::npos)
				polySetName = polySetName.substr(pos, polySetName.size()-pos);
			else
				continue;

			MPlug plug = fnSet.findPlug("enumIndex", &mStatus);
			if (mStatus == MStatus::kSuccess)
			{
				S32 value;
				plug.getValue(value);

				//In the case where we're dealing with an enum, strip the last fragment from the set name.
				std::string::size_type pos = polySetName.rfind("_");

				if (pos == std::string::npos)
				{
					AFTK_DO_BEGIN << "Failed to parse polyflag value: " << polySetName.c_str() << "\n" AFTK_DO_WARNING_END
					continue;
				}

				std::string enumValue = polySetName.substr(pos+1);
				polySetName = polySetName.substr(0, pos);

				pos = polySetName.rfind("_");
				if (pos != std::string::npos)
					polySetName = polySetName.substr(pos+1);

				if (polySetName == "lightmapResolution")
				{
					std::string::size_type replacePos = enumValue.find("xpointx");
					if (replacePos != std::string::npos)
						enumValue.replace(replacePos, 7, ".", 1);
				}

				S32 actualEnum = FindPolyFlagEnum(polySetName, enumValue, polyFlagEnums);
				if (actualEnum != -1)
				{
					if (actualEnum != value)
					{
						AFTK_DO_BEGIN << "Fixing polyflag value: " << polySetName.c_str() << "." << enumValue.c_str() << "[actual " << actualEnum
							<< ", found " << value << "]\n" AFTK_DO_WARNING_END
						plug.setValue(actualEnum);
					}
				}
				else
					AFTK_DO_BEGIN << "Unknown polyflag found: " << polySetName.c_str() << "." << enumValue.c_str() << "\n" AFTK_DO_WARNING_END

			}
			else
			{
				std::string::size_type pos = polySetName.rfind("_");
				if (pos != std::string::npos)
					polySetName = polySetName.substr(pos+1);
				
				if (FindInPolyFlagList(polySetName, polyFlags) == -1)
				{
					AFTK_DO_BEGIN << "Unknown polyflag found: " << polySetName.c_str() << "\n" AFTK_DO_WARNING_END
					//fnSet.clear();
				}
			}
		}
	}

	return true;
}

S32 PolyFlagsHelper::FindInPolyFlagList(
	const std::string &flag,
	const MayaIo::PolyFlagsHelper::PolyFlags polyFlags)
{
	for (U32 i=0; i<(U32)polyFlags.Size(); ++i)
		if (flag == polyFlags[i])
			return i;

	return -1;
}

S32 PolyFlagsHelper::FindPolyFlagEnum(
	const std::string flag,
	const std::string enumString,
	const PolyFlagEnumTable& polyFlagEnums)
{
	PolyFlagEnumTable::ConstIterator it;

	for (it = polyFlagEnums.Begin(); it!=polyFlagEnums.End(); ++it)
		if (it->GetKey() == flag)
			break;

	if (it != polyFlagEnums.End())
	{
		const Ut::UtVector< Ut::DynamicVector<std::string> >& enumList = it->GetValue();
		for (U32 i=0; i<(U32)enumList.Size(); ++i)
			if (enumString == enumList[i])
				return i;
	}

	return -1;
}

bool PolyFlagsHelper::ParseConfigFile(
	const char* pConfigFile,
	PolyFlags& flags,
	PolyFlagEnumTable& polyFlagEnums)
{
	Sys::I_File* pFile = Sys::C_FileSystem::Instance().Open(pConfigFile);
	if (!pFile)
		return false;

	Sys::TextDeserialiser deserialiser(*pFile);

	while (pFile->GetPosition() < pFile->Length())
	{
		Util::C_String fileLine;
		deserialiser >> fileLine;

		if (fileLine(0,2) == "//" || fileLine[0] == '\n')
			continue;

		S32 pos = fileLine.Search("AddCheckBox");
		if (pos != -1)
		{
			S32 start = fileLine.Search("\"", 0);
			S32 finish = fileLine.Search("\"", start+1);
			
			if (start != -1 && finish != -1)
				flags.PushBack(std::string(fileLine(start+1, finish).Chars()));

			continue;
		}
		
		pos = fileLine.Search("AddDropDown");
		if (pos != -1)
		{
			S32 start = fileLine.Search("\"", 0);
			S32 finish = fileLine.Search("\"", start+1);
			
			std::string polyFlagName = std::string(fileLine(start+1, finish).Chars());
			if (start != -1 && finish != -1)
				flags.PushBack(polyFlagName);

			polyFlagEnums[polyFlagName].Clear();

			start = fileLine.Search("{", finish);
			finish = fileLine.Search("}", start);

			if (start != -1 && finish != -1)
			{
				S32 enumStart = fileLine.Search("\"", start);
				S32 enumFinish = fileLine.Search("\"", enumStart+1);

				while (enumStart != -1 && enumFinish != -1 && enumStart < finish)
				{
					polyFlagEnums[polyFlagName].PushBack(std::string(fileLine(enumStart+1, enumFinish)));
					enumStart = fileLine.Search("\"", enumFinish+1);
					enumFinish = fileLine.Search("\"", enumStart+1);
				}
			}

			continue;
		}
	}

	delete pFile;

	return true;
}
*/