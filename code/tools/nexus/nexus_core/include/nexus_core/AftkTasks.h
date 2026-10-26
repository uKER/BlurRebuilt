#ifndef NEXUS_CORE_AFTK_TASKS_H
#define NEXUS_CORE_AFTK_TASKS_H

namespace Nexus
{
	class AftkTasks
	{
	public:
		static void NewScene();

		static Tu::Status OpenScene(void* pProgress, const StringArray& arguments);

		static Tu::Status ImportScene(void* pProgress, const StringArray& arguments);

		static Tu::Status ConvertScene(void* pProgress, const StringArray& arguments);

		static Tu::Status RepairScene(void* pProgress, const StringArray& arguments);

		static Tu::Status WeldMesh(void* pProgress, const StringArray& arguments);

		static Tu::Status WeldMeshComponents(void* pProgress, const StringArray& arguments);

		static Tu::Status WeldMeshPolygonVertices(void* pProgress, const StringArray& arguments);

		static Tu::Status MergeMeshes(void* pProgress, const StringArray& arguments);

		static Tu::Status SaveScene(void* pProgress, const StringArray& arguments);

		static Tu::Status OpenExternalScene(void* pProgress, const StringArray& arguments);

		static Tu::Status SaveExternalScene(void* pProgress, const StringArray& arguments);

		static Tu::Status ValidateScene(void* pProgress, const StringArray& arguments);

		static Tu::Status CalculateTangentsAndBinormals(void* pProgress, const StringArray& arguments);

		static Tu::Status TriangulateMesh(void* pProgress, const StringArray& arguments);

		static void PrintSceneGraph();

		static void PrintSceneBounds();

		static void PrintSelectedNodeInfo();

		static void PrintSelectedNodeInfo(const C_SelectedNodeInfo& selectedNodeInfo);

		static void PrintSelectedFaceInfo();

		static void PrintSelectedFaceInfo(const C_SelectedNodeInfo& selectedNodeInfo);

		static void PrintSceneInfo();

	private:
		static void Register(lua_State* pLuaState);

		static int NewSceneLua(lua_State* pLuaState);

		static int OpenSceneLua(lua_State* pLuaState);

		static int ImportSceneLua(lua_State* pLuaState);

		static int ConvertSceneLua(lua_State* pLuaState);

		static int SaveSceneLua(lua_State* pLuaState);

		static int ValidateSceneLua(lua_State* pLuaState);

		static int RepairSceneLua(lua_State* pLuaState);

		static int OpenExternalSceneLua(lua_State* pLuaState);

		static int SaveExternalSceneLua(lua_State* pLuaState);

		static int WeldMeshLua(lua_State* pLuaState);

		static int WeldMeshComponentsLua(lua_State* pLuaState);

		static int WeldMeshPolygonVerticesLua(lua_State* pLuaState);

		static int TriangulateMeshLua(lua_State* pLuaState);

		static int CalculateTangentsAndBinormalsLua(lua_State* pLuaState);

		static int MergeMeshesLua(lua_State* pLuaState);

		static int PrintSceneBoundsLua(lua_State* pLuaState);

		static int PrintSceneGraphLua(lua_State* pLuaState);

		static int PrintSelectedNodeInfoLua(lua_State* pLuaState);

		static int PrintSelectedFaceInfoLua(lua_State* pLuaState);

		static int PrintSceneInfoLua(lua_State* pLuaState);

		static int IsUpToDateLua(lua_State* pLuaState);

		static void PrintMaterial(
			std::stringstream& ss,
			const Aftk::C_MaterialNode& mtl);

		static void PrintBlindData(
			std::stringstream& ss,
			const Aftk::C_BlindDataNode& bd);

		static void PrintGenericAttribute(
			std::stringstream& ss,
			const Aftk::C_GenericAttribute& attr);

		static void PrintPlug(
			std::stringstream& ss,
			const Aftk::I_Plug& plug,
			U32 depth);

		template<class T>
		static void PrintGenericAttributeOrPlug(
			std::stringstream& ss,
			const T& attr);

		bool IsUpToDate(const std::string& file1, const std::string& fileList);

        friend class Core;
	};

	template <class T, int N>
	std::ostream& operator <<(std::ostream& os, const UtMaths::BaseVector<T, N>& value)
	{
		for (U32 i = 0; i < value.SIZE; i++)
		{
			os << value[i];
			if (i < value.SIZE-1)
			{
				os << " ";
			}
		}

		return os;
	}

	template <class T, int N>
	std::ostream& operator <<(std::ostream& os, const UtMaths::BasicMatrix<T, N>& value) 
	{
		for (U32 i = 0; i < value.SIZE; i++)
		{
			os << value[i];
			if (i < value.SIZE-1)
			{
				os << "\n";
			}
		}
		os << "\n";

		return	os;
	}

	template<class T>
	void AftkTasks::PrintGenericAttributeOrPlug(
		std::stringstream& ss,
		const T& attr)
	{
		switch (attr.GetDataType())
		{
			case Aftk::E_GenericAttributeType_String:
				{
					std::string val;
					attr.GetValue(val);
					ss << "Name [" << attr.GetName() << "] Type  [string], Value [" << val << "].\n";
				}
				break;

			case Aftk::E_GenericAttributeType_Boolean:
				{
					bool val;
					attr.GetValue(val);
					ss << "Name [" << attr.GetName() << "] Type  [boolean], Value [" << val << "].\n";
				}
				break;

			case Aftk::E_GenericAttributeType_F32:
				{
					F32 val;
					attr.GetValue(val);
					ss << "Name [" << attr.GetName() << "] Type  [F32], Value [" << val << "].\n";
				}
				break;

			case Aftk::E_GenericAttributeType_F64:
				{
					F64 val;
					attr.GetValue(val);
					ss << "Name [" << attr.GetName() << "] Type  [F64], Value [" << val << "].\n";
				}
				break;

			case Aftk::E_GenericAttributeType_Matrix44F32:
				{
					Aftk::Matrix44 val;
					attr.GetValue(val);
					ss << "Name [" << attr.GetName() << "] Type  [Matrix44], Value [" << val << "].\n";
				}
				break;

			case Aftk::E_GenericAttributeType_Null:
				{
					ss << "Name [" << attr.GetName() << "] Type  [Null].\n";
				}
				break;

			case Aftk::E_GenericAttributeType_S16:
				{
					S16 val;
					attr.GetValue(val);
					ss << "Name [" << attr.GetName() << "] Type  [S16], Value [" << val << "].\n";
				}
				break;

			case Aftk::E_GenericAttributeType_S32:
				{
					S32 val;
					attr.GetValue(val);
					ss << "Name [" << attr.GetName() << "] Type  [S32], Value [" << val << "].\n";
				}
				break;

			case Aftk::E_GenericAttributeType_S64:
				{
					S64 val;
					attr.GetValue(val);
					ss << "Name [" << attr.GetName() << "] Type  [S64], Value [" << val << "].\n";
				}
				break;

			case Aftk::E_GenericAttributeType_U8:
				{
					U8 val;
					attr.GetValue(val);
					ss << "Name [" << attr.GetName() << "] Type  [U8], Value [" << val << "].\n";
				}
				break;

			case Aftk::E_GenericAttributeType_U16:
				{
					U16 val;
					attr.GetValue(val);
					ss << "Name [" << attr.GetName() << "] Type  [U16], Value [" << val << "].\n";
				}
				break;

			case Aftk::E_GenericAttributeType_U32:
				{
					U32 val;
					attr.GetValue(val);
					ss << "Name [" << attr.GetName() << "] Type  [U32], Value [" << val << "].\n";
				}
				break;

			case Aftk::E_GenericAttributeType_U64:
				{
					U64 val;
					attr.GetValue(val);
					ss << "Name [" << attr.GetName() << "] Type  [U64], Value [" << val << "].\n";
				}
				break;

			default:
				break;
		}
	}
}

#endif
