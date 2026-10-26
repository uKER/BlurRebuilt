#ifndef AFTK_UT_FIND_AMBIGUOUS_ACTOR_NAMES_H
#define AFTK_UT_FIND_AMBIGUOUS_ACTOR_NAMES_H

#ifdef _MSCVER
#pragma once
#endif

namespace Debug
{
	class Logger;
}

namespace AftkUt
{
	class C_FindAmbiguousActorNames
	{
	public:
		typedef Ut::UtVector< Ut::DynamicVector<std::string> > ActorAttributeList;

		// Find ambiguous actor names
		static bool C_FindClashingActorNames(
			const Aftk::C_Scene *pScene);

		// Find ambiguous actor names referenced in actor strings( e.g link attributes)
		static bool FindAmbiguousActorNames(
			const Aftk::C_Scene *pScene,
			const ActorAttributeList& actorAttrList);

		// Read a list of attributes to check for from an external file.
		// File format: one parameter per line.
		static bool ReadActorAttributeListFromFile(
			const char* pActorAttrFile,
			ActorAttributeList& actorAttrList);

		// Gets actor attribute from actor string.
		// The parameter takes account of the hierarchy and must contain the actor category and type
		// e.g System.SpawnPoint.Waypoint
		static bool GetAttribute(
			const std::string& actorString,
			const std::string& attribute,
			std::string& retval);
	};

}
#endif

