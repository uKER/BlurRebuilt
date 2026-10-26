#include "StdAfx.h"
#include "Main.h"
#include "Export.h"
#include "XtoMSH.h"

int main(int argc, char* argv[])
{
	int errorCode = 0;

	if(D3DInitialise())
	{
		C_ExportList exportList;

		exportList.AddFile("x", "C:\\Program Files\\Microsoft DirectX SDK (March 2009)\\Samples\\Media\\misc\\", "sphere",	"sphere");

		//exportList.AddFile("x", "C:\\Program Files\\Microsoft DirectX SDK (March 2009)\\Samples\\Media\\Dwarf\\", "Dwarf",	"Dwarf");

		//exportList.AddFile("x", "C:\\Program Files\\Microsoft Xbox 360 SDK\\Source\\Samples\\Media\\Meshes\\", "ChessBoard",	"Chess");
		//exportList.AddFile("x", "C:\\Program Files\\Microsoft Xbox 360 SDK\\Source\\Samples\\Media\\Meshes\\", "ChessKnight",	"Chess");
		//exportList.AddFile("x", "C:\\Program Files\\Microsoft Xbox 360 SDK\\Source\\Samples\\Media\\Meshes\\", "ChessKing",		"Chess");
		//exportList.AddFile("x", "C:\\Program Files\\Microsoft Xbox 360 SDK\\Source\\Samples\\Media\\Meshes\\", "ChessPawn",		"Chess");
		//exportList.AddFile("x", "C:\\Program Files\\Microsoft Xbox 360 SDK\\Source\\Samples\\Media\\Meshes\\", "ChessQueen",	"Chess");
		//exportList.AddFile("x", "C:\\Program Files\\Microsoft Xbox 360 SDK\\Source\\Samples\\Media\\Meshes\\", "ChessRook",		"Chess");

		for(S32 i=0; i<exportList.m_Files.Size(); i++)
		{
			C_ExportFile& exportFile = exportList.m_Files[i];

			LOGINFO(g_Log, "Exporting %s\n", exportFile.m_SourceFilePath.Chars());

			C_XMesh sourceMesh;
			if(sourceMesh.Load(exportFile.m_SourceFilePath))
			{
				C_Mesh targetMesh;
				XtoMSH("", "", &sourceMesh, &targetMesh);

				for(U32 i=0; i<Render::E_Mesh2FilePlatform_Count; i++)
				{
					const Render::E_Mesh2FilePlatform filePlatform = (Render::E_Mesh2FilePlatform)i;

					Util::C_String targetFileExt = "";
					Util::C_String targetFileDir = "";

					switch(filePlatform)
					{
					case Render::E_Mesh2FilePlatform_PCD3D9:
						targetFileExt += "pcmsh";
						targetFileDir = "PC";
						break;
					case Render::E_Mesh2FilePlatform_XB2:
						targetFileExt += "x2msh";
						targetFileDir = "360";
						break;
					case Render::E_Mesh2FilePlatform_PS3GCM:
						targetFileExt += "p3msh";
						targetFileDir = "PS3";
						break;
					}

					Util::C_String targetRootDir = "Z:\\MercuryGame\\image\\" + targetFileDir + "\\meshes\\" + exportFile.m_TargetFileDir;

					Sys::C_FileSystem::Instance().CreateDirectory(targetRootDir);

					Util::C_String targetFilePath = targetRootDir + "\\" + exportFile.m_SourceFileName + "." + targetFileExt;

					if(targetMesh.Save(targetFilePath, filePlatform))
					{
						LOGINFO(g_Log, "Successfully exported mesh %s\n", targetFilePath.Chars());
					}
					else
					{
						errorCode = 1;
						LOGERROR(g_Log, "Failed save mesh %s\n", exportFile.m_SourceFilePath.Chars());
					}
				}
			}
			else
			{
				errorCode = 1;
				LOGERROR(g_Log, "Failed load mesh %s\n", exportFile.m_SourceFilePath.Chars());
			}
		}

	}
	else
	{
		errorCode = 1;
		LOGERROR(g_Log, "Failed to initialise d3d\n");
	}

	D3DShutdown();

	return errorCode;
}
