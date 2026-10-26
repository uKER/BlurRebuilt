#include "stdafx.h"

#include "SpeedTree.h"

#include <AftkSpeedTree/AftkSpeedTree.h>
#include <data/ConversionOptions.h>
#include <system/misc/stringutil.h>

bool IsSpeedTreeNode( const Aftk::C_SceneNode* pNode )
{
	if (pNode->GetBlindDataResourcePtr() != 0)
	{
		const Aftk::C_BlindDataNode& bd = pNode->GetBlindDataResourcePtr()->GetContent();

		for (U32 i = 0; i<(U32)bd.GetNumAttributes(); ++i)
		{
			if (bd.GetAttribute(i).GetName() == "SPTData")
			{
				return true;
			}
		}
	}

	return false;
}

bool HasSpeedTreeNodes( Aftk::C_Scene& aftk )
{
	Aftk::C_SceneNodeConstIterator it(&aftk);

	//Search for speedtree nodes
	bool foundSpeedTree = false;
	for (; it && !foundSpeedTree; ++it)
	{
		if( IsSpeedTreeNode( it ) )
			return true;
	}

	return false;
}

bool WriteSpeedTreeSetupFile( const C_DataConversionOptions& conversionOptions )
{
	//Write setup file for generating speedtrees
	{
		AftkSpeedTree::SpeedTreeSetup speedTree;

		//copy flag overrides from toolconfig
		const SpeedTreeFlagArray& flags = conversionOptions.GetToolConfig().GetSpeedTreeFlags();
		for (S32 i=0; i<flags.Size(); ++i)
		{
			AftkSpeedTree::Flag flag = {flags[i].name, flags[i].value};
			speedTree.m_flagOverrides.PushBack(flag);
		};

		if (!AftkSpeedTree::WriteSetupXmlFile( conversionOptions.GetTreeSetupFilename().Chars(), speedTree))
		{
			LOGERROR(Tools::g_log, "Failed to write setup file for speedtree generation\n");
			return false;	
		}
	}

	return true;
}

bool ExecuteSpeedTreeGen(const C_DataConversionOptions& conversionOptions)
{
	Util::C_String parameters;
	parameters << "--config " << conversionOptions.GetTreeSetupFilename().Chars() << " --in " << conversionOptions.GetTreeInputFilename().Chars() << " --out " << conversionOptions.GetTreeOutputFilename().Chars();

	S32 exitCode =  Sys::ShellExec::ExecuteCommandAndWait(
		conversionOptions.GetToolConfig().GetSpeedTreeGenFilenameWin32().Chars(),
		parameters.Chars(),
		Sys::RemoveTrailingSlash(conversionOptions.GetTreeTempDirectory()).Chars(),
		Sys::ShellExec::E_LaunchState_Minimise,
		&Tools::g_log);

	if (exitCode)
	{
		LOGERROR(Tools::g_log, "Failed to generate speedtrees\n");
		return false;	//abort export
	}

//#ifndef BUILD_CONFIG_DEBUG
	//DeleteFile(speedtreeInputScene.c_str());
	//DeleteFile(speedtreeOutputScene.c_str());
//#endif

	return true;
}

bool ExecuteSpeedTree(Aftk::C_Scene& aftk, const C_DataConversionOptions& conversionOptions )
{
	bool success = true;
	if (!HasSpeedTreeNodes( aftk ))
	{
		LOGINFO(Tools::g_log, "No trees were found in the scene");
		return true; // dont abort export
	}

	Sys::C_FileSystem::Instance().CreateDirectory( conversionOptions.GetTreeTempDirectory() );

	success = WriteSpeedTreeSetupFile( conversionOptions ) & success;
	if(!success)
		return true; // dont abort export

	//Write out scene
	Util::C_String speedtreeInputScene = conversionOptions.GetTreeInputFilename();
	if (!aftk.Write(speedtreeInputScene.Chars()))
	{
		LOGERROR(Tools::g_log, "Failed to write temp file for speedtree generation\n");
		return true;	//don't abort export
	}

	success = ExecuteSpeedTreeGen( conversionOptions ) & success;
	if (!success)
		return false; //abort export

	aftk.Clear();
	Util::C_String speedtreeOutputScene = conversionOptions.GetTreeOutputFilename();
	if (!aftk.Read(speedtreeOutputScene.Chars()))
	{
		LOGERROR(Tools::g_log, "Failed to read aftk scene after speedtree generation\n");
		return false; //abort export
	}
	
	return true;
}