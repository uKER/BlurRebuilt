#ifndef APP_EXPORT_GAME_SCENE_H
#define APP_EXPORT_GAME_SCENE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <converter_shared/ProcessGraph.h>


class C_ProcessExportGameScene : public Tools::ProcessGraph::C_Process
{
public:
	C_ProcessExportGameScene();

	// inherited from C_Process
	const Util::C_String GetName() const;

	// inherited from C_Process
	bool Process( Progress::I_ProgressBar* pProgress );

private:
	bool ExportMainScene( const Util::C_String& outputFilename );
	bool ExportMainScene( Sys::I_File* pFile );

	bool ExportCollision( const Util::C_String& outputFilename );
	//bool ExportCollision( Sys::I_File* pFile );

	bool ExportRender( const Util::C_String& outputFilename );
	//bool ExportRender( Sys::I_File* pFile );

	Tools::FileCopy::C_FileCopyList& GetFileCopyList();
	Util::E_Platform GetCurrPlatform();

private:
	Util::E_Platform m_Platform;
	Tools::FileCopy::C_FileCopyList m_FileCopyList;
};

#endif




