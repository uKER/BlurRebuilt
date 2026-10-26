#ifndef NEXUS_CORE_CS_GEN_TASKS_H
#define NEXUS_CORE_CS_GEN_TASKS_H

namespace Nexus
{
	class CSGenTasks
	{
	public:
		static Tu::Status BuildScaleCropSheetFromTextureList(void* pProgress, const StringArray& arguments);

		static Tu::Status BuildGridCropSheet(void* pProgress, const StringArray& arguments);

		static Tu::Status BuildFixedSizeCropSheet(void* pProgress, const StringArray& arguments);

		static Tu::Status BuildAutoScaleCropSheetFromScene(void* pProgress, const StringArray& arguments);

		static Tu::Status BuildAutoScaleCropSheetFromTextureList(void* pProgress, const StringArray& arguments);

	private:
		static void Register(lua_State* pLuaState);

		static int BuildScaleCropSheetFromTextureListLua(lua_State* pLuaState);

		static int BuildGridCropSheetLua(lua_State* pLuaState);

		static int BuildFixedSizeCropSheetLua(lua_State* pLuaState);

		static int BuildAutoScaleCropSheetFromSceneLua(lua_State* pLuaState);

		static int BuildAutoScaleCropSheetFromTextureListLua(lua_State* pLuaState);

		friend class Core;
	};
}

#endif
