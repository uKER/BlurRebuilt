#include "stdafx.h"

#include <iostream>
#include <core/core.h>
#include <file_tag_db/FileTagDB.h>


int main(int argc, char* argv[])
{
	FileTag::C_MetaManager metaManager;

	// Open xml file.
	metaManager.OpenDatabaseFile( "E:\\toolsdev\\code\\tools\\file_tag_db\\file_tag_manager.data" );
	
	// Read in a file.
	metaManager.ReadMetaData("Level_1.a3d");

	// Output data.
	Util::C_String buildVersion = "Build Version V";
	Util::C_String version = "Version V";
	std::cout << buildVersion + metaManager.GetBuildVersion() << std::endl;
	std::cout << version << metaManager.GetVersion() << std::endl;

	// Update current tag and write back to file.
	metaManager.SetBuildVersion("1.00");
	metaManager.SetVersion("1.10");
	metaManager.WriteMetaData("Test 3.a3d");

	// Updata xml Database
	metaManager.SaveToDisk();
	std::system("pause");

	return 0;
/*
	const FileTag::C_Tag* pTag = FileTag::C_TagDBManager::Instance().GetTagPtr(
		"Z:\\PGR4\\Art\\Circuits\\Tokyo\\Textures\\CityBlocks\\block_01\\b01_concrete_bump.tga");

	assert(pTag);
	for (U32 i = 0; i < pTag->GetNumAttributes(); i++)
	{
		std::cout << pTag->GetFileName().Chars() << " " << std::endl;
		{
			const FileTag::C_GenericAttribute& gAttr = pTag->GetAttributes()[i];
			std::cout << "   " << gAttr.GetName().Chars() << " type [";
			switch (gAttr.GetDataType())
			{
				case FileTag::C_GenericAttribute::E_DataType_S32:
				{
					std::cout << "S32] value [";
					S32 value;
					gAttr.GetValue(value);
					std::cout << value << "]" << std::endl;
					break;
				}

				case FileTag::C_GenericAttribute::E_DataType_Boolean:
				{
					std::cout << "bool] value [";
					bool value;
					gAttr.GetValue(value);
					std::cout << value << "]" << std::endl;
					break;
				}

				case FileTag::C_GenericAttribute::E_DataType_String:
				{
					std::cout << "string] value [";
					Util::C_String value;
					gAttr.GetValue(value);
					std::cout << value << "]" << std::endl;
					break;
				}

				default:
				{
					std::cout << "null]" << std::endl;
					break;
				}
			}
		}
	}

	system("pause");
	return 0;
*/
}

