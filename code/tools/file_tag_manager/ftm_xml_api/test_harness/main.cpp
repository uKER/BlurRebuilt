#include "stdafx.h"
#include <ftm_xml_api/Headers.h>
#include <tools_common/StringProc.h>

#define TEX_DATA_XML_FILE_NAME "TextureData.xml"

int main(int argc, char* argv[])
{
	FTM::TexDatabase db;
	
	std::string lastXMLPath;

	{
		std::string texturePath("Z:\\Gotham3\\Art\\Circuits\\Garages\\Garage_002_Lon\\Textures\\Textures\\G2_Window_Stone_Beam_BUMP.tga");
		std::string dir(Tu::DirFromString(texturePath));
		std::string xmlPath = dir + std::string("/") + TEX_DATA_XML_FILE_NAME;
		if (lastXMLPath != xmlPath)
		{
			Tu::Status status = db.Read(xmlPath.c_str());
			if (!status)
				return -1;
		}
		lastXMLPath = xmlPath;

		S32 recordIndex = db.RecordIndex(Tu::FileNameFromString(texturePath).c_str());
		if (recordIndex == -1)
			return -1;

		const FTM::TexDatabase::DBRecord& record = db.Record(recordIndex);
		int x = 5;
	}

	for (U32 i = 0; i < db.NumRecords(); i++)
	{
		const FTM::TexDatabase::DBRecord& r = db.Record(i);
		std::cout << r.FileName() << " " << std::endl;
		for (U32 j = 0; j < r.NumAttributes(); j++)
		{
			const GenericAttribute& gAttr = r.Attribute(j);
			std::cout << "   " << gAttr.Name() << " type [";
			switch (gAttr.DataType())
			{
				case GenericAttribute::ATTR_S32:
				{
					std::cout << "S32] value [";
					S32 value;
					gAttr.Value(value);
					std::cout << value << "]" << std::endl;
					break;
				}

				case GenericAttribute::ATTR_BOOLEAN:
				{
					std::cout << "bool] value [";
					bool value;
					gAttr.Value(value);
					std::cout << value << "]" << std::endl;
					break;
				}

				case GenericAttribute::ATTR_STRING:
				{
					std::cout << "string] value [";
					std::string value;
					gAttr.Value(value);
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

	return 0;
}

