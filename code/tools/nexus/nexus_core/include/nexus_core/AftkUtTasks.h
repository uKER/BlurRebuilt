#ifndef NEXUS_CORE_AFTKUT_TASKS_H
#define NEXUS_CORE_AFTKUT_TASKS_H

namespace Nexus
{
	class AftkUtTasks
	{
	public: 

		static Tu::Status RemoveDuplicateMeshes(void* pProgress, const StringArray& arguments);

		static Tu::Status CreateInstances(void* pProgress, const StringArray& arguments);
		
		static Tu::Status SmoothTangentsAndBinormals(void* pProgress, const StringArray& arguments);

		static Tu::Status CalculateSmoothTangentsAndBinormals(void* pProgress, const StringArray& arguments);

		static Tu::Status CalculateNormals(void* pProgress, const StringArray& arguments);

		static Tu::Status SmoothNormals(void* pProgress, const StringArray& arguments);

		static Tu::Status CalculateSmoothNormals(void* pProgress, const StringArray& arguments);

		static Tu::Status ExpandUniqueRemappedInstances(void* pProgress, const StringArray& arguments);

		//Performs the following operations:
		//	AftkUt::RadialEdgeMesh::Mesh* pRadialMesh = new AftkUt::RadialEdgeMesh::Mesh();
		//	pRadialMesh->CreateLinkedFromAftkMesh(pAftkSceneMeshNode, status);
		//	pRadialMesh->CreateUVIntegerOffsets(status);
		//	pRadialMesh->RemoveCoPlanarPolys(status);
		//	pRadialMesh->CreateAftkMeshFromLinked(status);
		static Tu::Status OptimiseMesh(void* pProgress, const StringArray& arguments);


	private:

		static void Register(lua_State* pLuaState);
		
		static int RemoveDuplicateMeshesLua(lua_State* pLuaState);

		static int CreateInstancesLua(lua_State* pLuaState);

		static int CalculateNormalsLua(lua_State* pLuaState);

		static int SmoothNormalsLua(lua_State* pLuaState);

		static int CalculateSmoothNormalsLua(lua_State* pLuaState);

		static int SmoothTangentsAndBinormalsLua(lua_State* pLuaState);

		static int CalculateSmoothTangentsAndBinormalsLua(lua_State* pLuaState);

		static int ExpandUniqueRemappedInstancesLua(lua_State* pLuaState);

		static int OptimiseMeshLua(lua_State* pLuaState);


		friend class Core;
	};

}

#endif