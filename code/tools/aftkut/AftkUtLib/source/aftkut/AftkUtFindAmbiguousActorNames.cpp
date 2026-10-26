#include <stdafx.h>
#include <system/serialise/C_FileSystem.h>
#include <system/serialise/TextSerialise.h>
using namespace AftkUt;


bool C_FindAmbiguousActorNames::C_FindClashingActorNames(
	const Aftk::C_Scene *pScene)
{
	//Collect scene nodes to find those with common names
	typedef Ut::UtMap<std::string, Ut::UtVector< Ut::DynamicVector< const Aftk::C_SceneNode* > > > SceneNodeNameMap;

	SceneNodeNameMap nameMap;		// node name -> array of scene nodes with name

	for (Aftk::C_SceneNodeConstIterator it(pScene); it; ++it)
	{
		if (C_NodeFlagParser::IsFlagged(*it, false))
		{
			// Check if it is attached to a mesh/mesh instance
			Aftk::C_SceneNode *pActorNode = it;
			std::string name(pActorNode->GetName());

			nameMap[name].PushBack(it);
		}
	}

	bool bFoundNameClash = false;
	for (SceneNodeNameMap::Iterator it=nameMap.Begin(); it!=nameMap.End(); ++it)
	{
		if (it->GetValue().Size() > 1)
		{
			bFoundNameClash = true;
			Aftk::E_ObjType type = it->GetValue()[0]->GetType();
			LOGDEBUG(g_Log, "Found actors with clashing names: %s\n", it->GetKey().c_str());
			for (U32 i=0; i<(U32)it->GetValue().Size(); ++i)
				LOGDEBUG(g_Log, "  [%d] %s\n", i, it->GetValue()[i]->GetPath().c_str());
		}
	}

	return bFoundNameClash;
}

bool C_FindAmbiguousActorNames::FindAmbiguousActorNames(
	const Aftk::C_Scene *pScene,
	const ActorAttributeList& actorAttrList)
{
	//Collect scene nodes to find those with common names
	typedef Ut::UtMap<std::string, Ut::UtVector< Ut::DynamicVector< const Aftk::C_SceneNode* > > > SceneNodeNameMap;
	
	SceneNodeNameMap nameMap;		// node name -> array of scene nodes with name
	SceneNodeNameMap referenceMap;	// node name -> array of scene nodes that reference the node

	if (actorAttrList.Size() == 0)
		return false;

	for (Aftk::C_SceneNodeConstIterator it(pScene); it; ++it)
	{
		if (C_NodeFlagParser::IsFlagged(*it, true))
		{
			std::string name(it->GetName());
			nameMap[name].PushBack(it);

			Aftk::C_DynamicPlug* pActorPlug;
			pActorPlug = (Aftk::C_DynamicPlug*)it->GetPlugPtr("actorString2");
			if (pActorPlug)
			{
				for (U32 attr=0; attr<(U32)actorAttrList.Size(); ++attr)
				{
					std::string refNodeName;
					std::string actorString;
					pActorPlug->GetValue(actorString);

					if (GetAttribute(actorString, actorAttrList[attr], refNodeName))
					{
						// node reference attribute could actually be a list of node names (delimited by spaces)
						size_t p0 = 0, p1 = std::string::npos;
						std::string delim(" ");
						while(p0 != std::string::npos)
						{
							p1 = refNodeName.find_first_of(delim, p0);
							if(p1 != p0)
							{
								std::string token = refNodeName.substr(p0, p1 - p0);
								referenceMap[token].PushBack(it);
							}
							p0 = refNodeName.find_first_not_of(delim, p1);
						}
					}
				}
			}
		}
	}

	bool bFoundAmbiguousNames = false;
	for (SceneNodeNameMap::Iterator refIt=referenceMap.Begin(); refIt!=referenceMap.End(); ++refIt)
	{
		SceneNodeNameMap::Iterator nameIt = nameMap.Find(refIt->GetKey());
		if (nameIt!=nameMap.End() && nameIt->GetValue().Size() > 1)
		{
			bFoundAmbiguousNames = true;
			LOGWARN(g_Log, "Ambiguous node name found: %s\n", refIt->GetKey().c_str());
			LOGWARN(g_Log, "Referenced in actor string on ...\n", refIt->GetKey().c_str());
			for (U32 i=0; i<(U32)nameIt->GetValue().Size(); ++i)
				LOGWARN(g_Log, "[%d] %s", i, nameIt->GetValue()[i]->GetPath().c_str());
		}
	}

	return bFoundAmbiguousNames;

}

bool C_FindAmbiguousActorNames::ReadActorAttributeListFromFile(
	const char *pActorAttrFile,
	ActorAttributeList& actorAttrList)
{
	std::ifstream fileStream;
	fileStream.open(pActorAttrFile);
	if (!fileStream.is_open())
		return false;

	while (fileStream.good())
	{
		actorAttrList.PushBack();
		fileStream >> actorAttrList[actorAttrList.Size()-1];
	}
	fileStream.close();

	return true;
}

bool C_FindAmbiguousActorNames::GetAttribute(
	const std::string& actorString,
	const std::string& attribute,
	std::string& retval)
{
	std::stringstream search;
	search << "<actor name=\"";

	size_t attrNameBegin, attrNameEnd, actorStringPos;
	{
		size_t categoryIndex(attribute.find("."));
		if (categoryIndex == std::string::npos)
			return false;

		size_t actorIndex(attribute.find(".",categoryIndex+1));
		if (actorIndex == std::string::npos)
			return false;

		attrNameBegin = actorIndex;
		attrNameEnd = attribute.find(".", attrNameBegin+1);
		search << attribute.substr(categoryIndex+1,actorIndex-categoryIndex-1) << "\" category=\"" << attribute.substr(0,categoryIndex) << "\">";
	}

	actorStringPos = actorString.find(search.str());
	if (actorStringPos == std::string::npos)
		return false;

	actorStringPos += search.str().length();

	while (attrNameBegin != std::string::npos)
	{
		search.str("");
		search.seekp(0);
		search.clear();
		attrNameEnd = attribute.find(".", attrNameBegin+1);
		if (attrNameEnd == std::string::npos)
			search << "name=\"" << attribute.substr(attrNameBegin+1) << "\"";
		else
			search << "name=\"" << attribute.substr(attrNameBegin+1, attrNameEnd-attrNameBegin-1) <<  "\"";

		attrNameBegin = attrNameEnd;
		actorStringPos = actorString.find(search.str(), actorStringPos);
		if (actorStringPos == std::string::npos)
			return false;

		actorStringPos += search.str().length();
	}

	search.str("");
	search.seekp(0);
	search.clear();
	search << "default=\"";
	actorStringPos = actorString.find(search.str(), actorStringPos+1);
	if (actorStringPos != std::string::npos)
	{
		actorStringPos += search.str().length();
		size_t attrEnd = actorString.find("\"", actorStringPos);
		if (attrEnd != std::string::npos && attrEnd!=actorStringPos)
		{
			retval = actorString.substr(actorStringPos, attrEnd-actorStringPos);
			return true;
		}
	}

	return false;
}