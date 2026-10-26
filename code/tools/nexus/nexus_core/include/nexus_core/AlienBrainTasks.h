#ifndef NEXUS_CORE_ALIENBRAIN_TASKS_H
#define NEXUS_CORE_ALIENBRAIN_TASKS_H

#ifdef NEXUS_ALIENBRAIN

#pragma warning(push)
	#pragma warning(disable: 4267)
	#include <NxNIntegratorSDK.h>
	#include <NxNSmartIntegrator.h>
#pragma warning(pop)

namespace Nexus
{
	class AlienBrainTasks
	{
	public:
		AlienBrainTasks();

	   ~AlienBrainTasks();

	   bool Init();
	   void Term();

		bool GetLatest(const char *name, U32 nRetries=0);
		bool SyncLatest(const char *name, U32 nRetries=0, bool forceOnFailure=true);

		void RecurseDeleteFilesNotInAB(CNxNString folderName);

		CNxNSmartIntegrator	*m_pIntegrator;

		static Tu::Status ABGetLatest(const char* pCommand, U32 nRetries=0);
		static Tu::Status ABSyncLatest(const char* pCommand, U32 nRetries=0, bool forceOnFailure=true);

		static AlienBrainTasks& Instance(void)
		{
			return s_instance;
		}

	private:
		static void Register(lua_State* pLuaState);

		static int ABGetLatestLua(lua_State* pLuaState);
		static int ABSyncLatestLua(lua_State* pLuaState);

		friend class Core;

		static AlienBrainTasks	s_instance;
	};
}

#endif

#endif
