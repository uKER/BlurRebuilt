#include "stdafx.h"
#include <nexus_core/core.h>

namespace Nexus
{
	void ParseLayerBackgroundColours(
		Ut::UtVector<Ut::DynamicVector<LiteMaths::Vec4U8> >& colourList,
		const std::string& colourString)
	{
		char delimiter[] = "|";
		char* token;
		
		//Get the first token:
		char* buffer = new char[colourString.length()+1];
		strcpy(buffer, colourString.c_str());
		token = strtok(buffer, delimiter);
		U32 layerIndex = 0;
		while( token != NULL )
		{
			size_t tokenLength = strlen(token);
			if (tokenLength > 1)
			{
				std::string tokenStd(token);
				colourList.PushBack(LiteMaths::Vec4U8(255,0,0,0));

				if (tokenStd.size() != 0)
				{
					U32 componentIndex = 0;
					std::size_t componentStringStartIndex = 0;
					std::size_t componentStringEndIndex = tokenStd.find(' ');
					if (componentStringEndIndex == std::string::npos)
						componentStringEndIndex = tokenStd.size();
					while((componentIndex < 4) && (componentStringStartIndex < tokenStd.size()))
					{
						std::string temp = 
							tokenStd.substr(componentStringStartIndex, componentStringEndIndex - componentStringStartIndex);
						std::stringstream ss;
						ss << temp;
						U32 val;
						ss >> val;
						U8* p = ((U8*)&(colourList[layerIndex]));
						p[componentIndex] = (U8)val;
						
						//Skip to next colour component.
						componentStringStartIndex = componentStringEndIndex+1;
						componentStringEndIndex = tokenStd.find(' ', componentStringStartIndex);
						if (componentStringEndIndex == std::string::npos)
							componentStringEndIndex = tokenStd.size();
						componentIndex++;
					}
				}
			}

			layerIndex++;
			//Get next token:
			token = strtok(NULL, delimiter);
		}
		delete[] buffer;
	}

	Tu::Status CSGenTasks::BuildScaleCropSheetFromTextureList(void* pProgress, const StringArray& arguments)
	{
		/*
		Nexus::g_pCore->Lock();
		if (arguments.Size() > 0)
			if (arguments.Size() != 14)
				return Tu::Status(Tu::Status::FAILURE, NEXUS_CORE_INCORRENT_TASK_ARG_COUNT);

		Progress* pProgressSystem = (Progress*)pProgress;
		
		const std::string& textureListFile = arguments[0];
		bool gnerateNormalMaps;
		{
			std::stringstream ss;
			ss << arguments[1];
			ss >> gnerateNormalMaps;
		}
		const std::string& dbFileName = arguments[2];
		const std::string& csBaseName = arguments[3];
		U32 csWidth;
		{
			std::stringstream ss;
			ss << arguments[4];
			ss >> csWidth;
		}
		U32 csHeight;
		{
			std::stringstream ss;
			ss << arguments[5];
			ss >> csHeight;
		}
		U32 borderSize;
		{
			std::stringstream ss;
			ss << arguments[6];
			ss >> borderSize;
		}
		F32 minimumScale;
		{
			std::stringstream ss;
			ss << arguments[7];
			ss >> minimumScale;
		}
		U32 minImageSize;
		{
			std::stringstream ss;
			ss << arguments[8];
			ss >> minImageSize;
		}
		U32 maxImageSize;
		{
			std::stringstream ss;
			ss << arguments[9];
			ss >> maxImageSize;
		}
		bool allowRotation;
		{
			std::stringstream ss;
			ss << arguments[10];
			ss >> allowRotation;
		}
		bool clampSmallestSide;
		{
			std::stringstream ss;
			ss << arguments[11];
			ss >> clampSmallestSide;
		}
		bool addBorderPixel;
		{
			std::stringstream ss;
			ss << arguments[12];
			ss >> addBorderPixel;
		}
		Ut::UtVector< Ut::DynamicVector<LiteMaths::Vec4U8> > layerBackgroundColourList;
		ParseLayerBackgroundColours(layerBackgroundColourList, arguments[13]);

		Tu::Status status;

		U32 minimumLayerCount = max(1, layerBackgroundColourList.Size());

		TexAtlas::TextureKeyArray textureList;
		if (!TexAtlas::CropSheet::BuildTextureList(
			textureList,
			textureListFile,
			addBorderPixel,
			minimumLayerCount,
			Nexus::g_pCore->GetPrintMessageCallback(),
			Nexus::g_pCore->GetPrintWarningCallback(),
			Nexus::g_pCore->GetPrintErrorCallback(),
			(TexAtlas::Progress*)pProgress))
		{
			status = Tu::Status::FAILURE;
			Nexus::g_pCore->Unlock();
			return status;
		}
			
		if (!TexAtlas::ScaleCropSheet::Build(
			textureList,
			gnerateNormalMaps,
			dbFileName,
			csBaseName,
			csWidth,
			csHeight,
			minimumScale,
			borderSize,
			minImageSize,
			maxImageSize,
			allowRotation,
			clampSmallestSide,
			layerBackgroundColourList,
			Nexus::g_pCore->GetPrintMessageCallback(),
			Nexus::g_pCore->GetPrintWarningCallback(),
			Nexus::g_pCore->GetPrintErrorCallback(),
			(TexAtlas::Progress*)pProgress))
		{
			status = Tu::Status::FAILURE;
			Nexus::g_pCore->Unlock();
			return status;
		}

		Nexus::g_pCore->OnSceneCropSheetChanged();
		Nexus::g_pCore->Unlock();
		return Tu::Status::SUCCESS;
		*/
		return Tu::Status::NOT_IMPLEMENTED;
	}

	Tu::Status CSGenTasks::BuildGridCropSheet(void* pProgress, const StringArray& arguments)
	{
		/*
		if (arguments.Size() > 0)
			if (arguments.Size() != 4)
				return Tu::Status(Tu::Status::FAILURE, NEXUS_CORE_INCORRENT_TASK_ARG_COUNT);

		Progress* pProgressSystem = (Progress*)pProgress;
		const std::string& dbFileName = arguments[0];
		const std::string& csBaseName = arguments[1];
		U32 csDimensions;
		U32 borderSize;
		{
			std::stringstream ss;
			ss << arguments[2];
			ss >> csDimensions;
		}
		{
			std::stringstream ss;
			ss << arguments[3];
			ss >> borderSize;
		}

		Tu::Status status;
		TexAtlas::Core core;
		//Register output callbacks.
		core.SetPrintMessageCallback(Nexus::g_pCore->PrintMessageCallback());
		core.SetPrintWarningCallback(Nexus::g_pCore->PrintWarningCallback());
		core.SetPrintErrorCallback(Nexus::g_pCore->PrintErrorCallback());
		core.SetProgressCallback((TexAtlas::Progress*)pProgress);
		status = core.Create();
		if (!status) return status;

		status = core.BuildGridCropSheet(
				Nexus::g_pCore->AftkScenePtr(),
				dbFileName,
				csBaseName,
				csDimensions,
				borderSize);
		if (!status) 
		{
			core.Destroy();
			return status;
		}

		core.Destroy();
		Nexus::g_pCore->OnSceneCropSheetChanged();
		return Tu::Status::SUCCESS;
		*/
		return Tu::Status::NOT_IMPLEMENTED;
	}

	Tu::Status CSGenTasks::BuildFixedSizeCropSheet(void* pProgress, const StringArray& arguments)
	{
/*		if (arguments.Size() > 0)
			if (arguments.Size() != 5)
				return Tu::Status(Tu::Status::FAILURE, NEXUS_CORE_INCORRENT_TASK_ARG_COUNT);

		Progress* pProgressSystem = (Progress*)pProgress;
		const std::string& dbFileName = arguments[0];
		const std::string& csBaseName = arguments[1];
		U32 csDimensions;
		U32 borderSize;
		U32 subImageSize;
		{
			std::stringstream ss;
			ss << arguments[2];
			ss >> csDimensions;
		}
		{
			std::stringstream ss;
			ss << arguments[3];
			ss >> subImageSize;
		}
		{
			std::stringstream ss;
			ss << arguments[4];
			ss >> borderSize;
		}

		Tu::Status status;
		TexAtlas::Core core;
		//Register output callbacks.
		core.SetPrintMessageCallback(Nexus::g_pCore->PrintMessageCallback());
		core.SetPrintWarningCallback(Nexus::g_pCore->PrintWarningCallback());
		core.SetPrintErrorCallback(Nexus::g_pCore->PrintErrorCallback());
		core.SetProgressCallback((TexAtlas::Progress*)pProgress);
		status = core.Create();
		if (!status) return status;

		status = core.BuildFixedSizeCropSheet(
				Nexus::g_pCore->AftkScenePtr(),
				dbFileName,
				csBaseName,
				csDimensions,
				subImageSize,
				borderSize);
		if (!status) 
		{
			core.Destroy();
			return status;
		}

		core.Destroy();
		Nexus::g_pCore->OnSceneCropSheetChanged();
		return Tu::Status::SUCCESS;
		*/
		return Tu::Status::NOT_IMPLEMENTED;
	}

	Tu::Status CSGenTasks::BuildAutoScaleCropSheetFromScene(void* pProgress, const StringArray& arguments)
	{
		/*
		Nexus::g_pCore->Lock();
 		if (arguments.Size() > 0)
			if (arguments.Size() != 14)
			{
				Nexus::g_pCore->Unlock();
				return Tu::Status(Tu::Status::FAILURE, NEXUS_CORE_INCORRENT_TASK_ARG_COUNT);
			}

		Progress* pProgressSystem = (Progress*)pProgress;
		
		bool gnerateNormalMaps;
		{
			std::stringstream ss;
			ss << arguments[0];
			ss >> gnerateNormalMaps;
		}
		const std::string& dbFileName = arguments[1];
		const std::string& csBaseName = arguments[2];
		U32 csWidth;
		{
			std::stringstream ss;
			ss << arguments[3];
			ss >> csWidth;
		}
		U32 csHeight;
		{
			std::stringstream ss;
			ss << arguments[4];
			ss >> csHeight;
		}
		U32 borderSize;
		{
			std::stringstream ss;
			ss << arguments[5];
			ss >> borderSize;
		}
		F32 minimumScale;
		{
			std::stringstream ss;
			ss << arguments[6];
			ss >> minimumScale;
		}
		U32 minImageSize;
		{
			std::stringstream ss;
			ss << arguments[7];
			ss >> minImageSize;
		}
		U32 maxImageSize;
		{
			std::stringstream ss;
			ss << arguments[8];
			ss >> maxImageSize;
		}
		bool allowRotation;
		{
			std::stringstream ss;
			ss << arguments[9];
			ss >> allowRotation;
		}
		bool clampSmallestSide;
		{
			std::stringstream ss;
			ss << arguments[10];
			ss >> clampSmallestSide;
		}
		bool addBorderPixel;
		{
			std::stringstream ss;
			ss << arguments[11];
			ss >> addBorderPixel;
		}
		Ut::UtVector< Ut::DynamicVector<LiteMaths::Vec4U8> > layerBackgroundColourList;
		ParseLayerBackgroundColours(layerBackgroundColourList, arguments[12]);

		TexAtlas::AutoScaleCropSheet::Mode mode;
		{
			std::string modeStr = arguments[13];
			if (modeStr == "autofindcssize")
				mode = TexAtlas::AutoScaleCropSheet::AUTO_FIND_CROP_SHEET_SIZE;
			else if (modeStr == "normal")
				mode = TexAtlas::AutoScaleCropSheet::NORMAL;
			else
			{
				Nexus::g_pCore->Unlock();
                return Tu::Status(Tu::Status::FAILURE, NEXUS_CORE_INVALID_TASK_ARG);
			}
		}

		Tu::Status status;

		TexAtlas::TextureKeyArray textureList;
		if (!TexAtlas::CropSheet::BuildTextureList(
			textureList,
			Nexus::g_pCore->GetAftkScenePtr(), 
			addBorderPixel,
			Nexus::g_pCore->GetPrintMessageCallback(),
			Nexus::g_pCore->GetPrintWarningCallback(),
			Nexus::g_pCore->GetPrintErrorCallback(),
			(TexAtlas::Progress*)pProgress))
		{
			status = Tu::Status::FAILURE;
			Nexus::g_pCore->Unlock();
			return status;
		}
			
		if( !TexAtlas::AutoScaleCropSheet::Build(
					textureList,
					gnerateNormalMaps,
					dbFileName,
					csBaseName,
					csWidth,
					csHeight,
					borderSize,
					minimumScale,
					minImageSize,
					maxImageSize,
					allowRotation,
					clampSmallestSide,
					layerBackgroundColourList,
					mode,
					Nexus::g_pCore->GetPrintMessageCallback(),
					Nexus::g_pCore->GetPrintWarningCallback(),
					Nexus::g_pCore->GetPrintErrorCallback(),
					(TexAtlas::Progress*)pProgress))
		{
			status = Tu::Status::FAILURE;
			Nexus::g_pCore->Unlock();
			return status;
		}

		Nexus::g_pCore->OnSceneCropSheetChanged();
		Nexus::g_pCore->Unlock();
		return Tu::Status::SUCCESS;
		*/
		return Tu::Status::NOT_IMPLEMENTED;
	}

	Tu::Status CSGenTasks::BuildAutoScaleCropSheetFromTextureList(void* pProgress, const StringArray& arguments)
	{
		Nexus::g_pCore->Lock();
 		if (arguments.Size() > 0)
			if (arguments.Size() != 15)
				return Tu::Status(Tu::Status::FAILURE, NEXUS_CORE_INCORRENT_TASK_ARG_COUNT);

		Progress* pProgressSystem = (Progress*)pProgress;
		
		const std::string& textureListFile = arguments[0];
		bool gnerateNormalMaps;
		{
			std::stringstream ss;
			ss << arguments[1];
			ss >> gnerateNormalMaps;
		}
		const std::string& dbFileName = arguments[2];
		const std::string& csBaseName = arguments[3];
		U32 csWidth;
		{
			std::stringstream ss;
			ss << arguments[4];
			ss >> csWidth;
		}
		U32 csHeight;
		{
			std::stringstream ss;
			ss << arguments[5];
			ss >> csHeight;
		}
		U32 borderSize;
		{
			std::stringstream ss;
			ss << arguments[6];
			ss >> borderSize;
		}
		F32 minimumScale;
		{
			std::stringstream ss;
			ss << arguments[7];
			ss >> minimumScale;
		}
		U32 minImageSize;
		{
			std::stringstream ss;
			ss << arguments[8];
			ss >> minImageSize;
		}
		U32 maxImageSize;
		{
			std::stringstream ss;
			ss << arguments[9];
			ss >> maxImageSize;
		}
		bool allowRotation;
		{
			std::stringstream ss;
			ss << arguments[10];
			ss >> allowRotation;
		}
		bool clampSmallestSide;
		{
			std::stringstream ss;
			ss << arguments[11];
			ss >> clampSmallestSide;
		}
		bool addBorderPixel;
		{
			std::stringstream ss;
			ss << arguments[12];
			ss >> addBorderPixel;
		}
		Ut::UtVector< Ut::DynamicVector<LiteMaths::Vec4U8> > layerBackgroundColourList;
		ParseLayerBackgroundColours(layerBackgroundColourList, arguments[13]);

		TexAtlas::AutoScaleCropSheet::Mode mode;
		{
			std::string modeStr = arguments[14];
			if (modeStr == "autofindcssizemulti")
				mode = TexAtlas::AutoScaleCropSheet::AUTO_FIND_CROP_SHEET_SIZE_MULTI;
			else if (modeStr == "autofindcssize")
				mode = TexAtlas::AutoScaleCropSheet::AUTO_FIND_CROP_SHEET_SIZE;
			else if (modeStr == "normal")
				mode = TexAtlas::AutoScaleCropSheet::NORMAL;
			else
			{
				Nexus::g_pCore->Unlock();
                return Tu::Status(Tu::Status::FAILURE, NEXUS_CORE_INVALID_TASK_ARG);
			}
		}

		Tu::Status status;

		U32 minimumLayerCount = max(1, layerBackgroundColourList.Size());

		TexAtlas::TextureKeyArray textureList;
		if (!TexAtlas::CropSheet::BuildTextureList(
			textureList,
			textureListFile,
			addBorderPixel,
			minimumLayerCount,
			Nexus::g_pCore->GetPrintMessageCallback(),
			Nexus::g_pCore->GetPrintWarningCallback(),
			Nexus::g_pCore->GetPrintErrorCallback(),
			(TexAtlas::Progress*)pProgress))
		{
			status = Tu::Status::FAILURE;
			Nexus::g_pCore->Unlock();
			return status;
		}
			
		if (!TexAtlas::AutoScaleCropSheet::Build(
			textureList,
			gnerateNormalMaps,
			dbFileName,
			csBaseName,
			csWidth,
			csHeight,
			borderSize,
			minimumScale,
			minImageSize,
			maxImageSize,
			allowRotation,
			clampSmallestSide,
			layerBackgroundColourList,
			mode,
			Nexus::g_pCore->GetPrintMessageCallback(),
			Nexus::g_pCore->GetPrintWarningCallback(),
			Nexus::g_pCore->GetPrintErrorCallback(),
			(TexAtlas::Progress*)pProgress))
		{
			status = Tu::Status::FAILURE;
			Nexus::g_pCore->Unlock();
			return status;
		}

		Nexus::g_pCore->OnSceneCropSheetChanged();
		Nexus::g_pCore->Unlock();
		return Tu::Status::SUCCESS;
	}

	int CSGenTasks::BuildScaleCropSheetFromTextureListLua(lua_State* pLuaState)
	{
		Tu::Status status;
		StringArray arguments;
		U32 argIndex = 1;
		while(true)
		{
			const char* pStr = lua_tostring(pLuaState, argIndex);
			if (pStr == 0)
				break;
			arguments.PushBack(pStr);
			argIndex++;
		}
		if (g_pProgressCallback)
			status = g_pProgressCallback->ExecuteTask(BuildScaleCropSheetFromTextureList, arguments);

		//Return result
		return GlobalTasks::ProcessResult(pLuaState, status);
	}

	int CSGenTasks::BuildAutoScaleCropSheetFromSceneLua(lua_State* pLuaState)
	{
		Tu::Status status;
		StringArray arguments;
		U32 argIndex = 1;
		while(true)
		{
			const char* pStr = lua_tostring(pLuaState, argIndex);
			if (pStr == 0)
				break;
			arguments.PushBack(pStr);
			argIndex++;
		}
		if (g_pProgressCallback)
			status = g_pProgressCallback->ExecuteTask(BuildAutoScaleCropSheetFromScene, arguments);

		//Return result
		return GlobalTasks::ProcessResult(pLuaState, status);
	}

	int CSGenTasks::BuildAutoScaleCropSheetFromTextureListLua(lua_State* pLuaState)
	{
		Tu::Status status;
		StringArray arguments;
		U32 argIndex = 1;
		while(true)
		{
			const char* pStr = lua_tostring(pLuaState, argIndex);
			if (pStr == 0)
				break;
			arguments.PushBack(pStr);
			argIndex++;
		}
		if (g_pProgressCallback)
			status = g_pProgressCallback->ExecuteTask(BuildAutoScaleCropSheetFromTextureList, arguments);

		//Return result
		return GlobalTasks::ProcessResult(pLuaState, status);
	}

	int CSGenTasks::BuildGridCropSheetLua(lua_State* pLuaState)
	{
		Tu::Status status;
		StringArray arguments;
		U32 argIndex = 1;
		while(true)
		{
			const char* pStr = lua_tostring(pLuaState, argIndex);
			if (pStr == 0)
				break;
			arguments.PushBack(pStr);
			argIndex++;
		}
		if (g_pProgressCallback)
			status = g_pProgressCallback->ExecuteTask(BuildGridCropSheet, arguments);

		//Return result
		return GlobalTasks::ProcessResult(pLuaState, status);
	}

	int CSGenTasks::BuildFixedSizeCropSheetLua(lua_State* pLuaState)
	{
		Tu::Status status;
		StringArray arguments;
		U32 argIndex = 1;
		while(true)
		{
			const char* pStr = lua_tostring(pLuaState, argIndex);
			if (pStr == 0)
				break;
			arguments.PushBack(pStr);
			argIndex++;
		}
		if (g_pProgressCallback)
			status = g_pProgressCallback->ExecuteTask(BuildFixedSizeCropSheet, arguments);

		//Return result
		return GlobalTasks::ProcessResult(pLuaState, status);
	}

	void CSGenTasks::Register(lua_State* pLuaState)
	{
		//Register Lua functions
		lua_register( pLuaState, "BuildScaleCropSheetFromTextureList",		&CSGenTasks::BuildScaleCropSheetFromTextureListLua);
		lua_register( pLuaState, "BuildAutoScaleCropSheetFromScene",		&CSGenTasks::BuildAutoScaleCropSheetFromSceneLua);
		lua_register( pLuaState, "BuildAutoScaleCropSheetFromTextureList",		&CSGenTasks::BuildAutoScaleCropSheetFromTextureListLua);
		lua_register( pLuaState, "BuildGridCropSheet",		&CSGenTasks::BuildGridCropSheetLua);
		lua_register( pLuaState, "BuildFixedSizeCropSheet",	&CSGenTasks::BuildFixedSizeCropSheetLua);
	}
}


