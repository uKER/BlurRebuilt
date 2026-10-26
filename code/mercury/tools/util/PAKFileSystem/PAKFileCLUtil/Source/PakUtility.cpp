
#include "stdafx.h"
#include "PakUtility.h"
#include "MakePAK.h"

#include <tools_shared\VersionStrings.inl>
#include <CmdLine.h>

C_PAKUtility::C_PAKUtility()
: C_ToolsApplication(	"PakUtility",			
						"Utility Tool",			
						"tools.PakUtility",			
						"",				
						true )
{
}


Tu::Status C_PAKUtility::ParseCommandLineOptions(
	int argc, 
	char* argv[],
	CommandLineOptions &options)
{
	// Wrap everything in a try block.  Do this every time, 
	// because exceptions will be thrown for problems. 
	try
	{  
		// Define the command line object.
		TCLAP::CmdLine cmd("This program creates PAK files.", ' ', "");

		TCLAP::ValueArg<std::string> xmlFile("x","xml","Xml File", true, "", "Xml File");

		TCLAP::ValueArg<std::string> folder("f","folder","Folder name", true, "", "Folder name");
		cmd.xorAdd( xmlFile, folder );

		TCLAP::ValueArg<std::string> mount("m","mount","Mount point", false, "", "Mount point");
		cmd.add( mount );

		TCLAP::ValueArg<std::string> endianSwap("e","endian","Endian Swap", false, "", "Endian Swap Yes or no");
		cmd.add( endianSwap );
		
		TCLAP::ValueArg<std::string> compress("c","compress","Compress File",false, "", "Compress true or false");
		cmd.add( compress );

		TCLAP::ValueArg<U32> sectorSize("s","sectorsize","Sector Size",false, 0, "Sector Size (bytes)");
		cmd.add( sectorSize );

		// parse the args.
		cmd.parse( argc, argv );

		// Get the values passed by each arg.
		if ( xmlFile.isSet() )
		{
			options.xmlFile = xmlFile.getValue().c_str();
			options.parseXml = true;
			options.sectorSize = 0;
		}
		else
		{
			options.parseXml = false;
			
			if ( folder.isSet() )
				options.pak = folder.getValue().c_str();
			else
			{
				LOGINFO(GetLogger(), "Folder\\File not set, parsing failed!");
				return Tu::Status::FAILURE;
			}
			
			if ( folder.isSet() )
				options.mountPoint = mount.getValue().c_str();
				
			if ( endianSwap.getValue() == "yes" )
				options.endianSwap = true;
			else
				options.endianSwap = false;
			
			if ( compress.getValue() == "true" )
				options.compress = true;
			else
				options.compress = false;

			if (sectorSize.isSet())
				options.sectorSize = sectorSize.getValue();
			else
				options.sectorSize = 0;
		}
	}
	catch (TCLAP::ArgException &e)  // catch any exceptions
	{ 
		std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl; 
	}
	return Tu::Status::SUCCESS;	
}


int C_PAKUtility::Main(S32 argc, char **argv, Progress::I_ProgressBar *pProgress)
{
	C_PAKFileSystem PAKFileSystem;
	Sys::C_FileSystem& fileSystem = Sys::C_FileSystem::Instance();
 
	LOGINFO(GetLogger(), "App Started\n");

	C_MakePAK	makePak(GetLogger());
	CommandLineOptions options;

	if( ParseCommandLineOptions( argc, argv, options ) == Tu::Status::SUCCESS )
	{
		if ( options.parseXml )
		{
			if(!makePak.XML_Read( options.xmlFile, options.sectorSize ))
				return 1;
		}
		else
		{
			// Create pak file from file/folder args.
			makePak.SetEndianSwap( options.endianSwap );
			makePak.SetPakRoot( options.root );
			makePak.CreatePAKFileList();

			// Check to see if file or folder.
			Util::C_String excludeList;
			Util::C_String check = Util::ExtractFileExtension( options.pak );

			// Folder has no extension so check will be empty.
			if (check == "")
			{
				// Folder
				if( !makePak.CreatePAKFromFolder( 
					options.pak, 
					options.mountPoint,
					options.compress,
					options.sectorSize,
					excludeList ) )
				{
					LOGERROR(GetLogger(), "Unable to create pak file from folder ", options.pak);
				}
			}
			else
			{
				LOGERROR(GetLogger(), "Only PAKing of folders is supported.");
				return 1;
			}
		}
		LOGINFO(GetLogger(), "Pak Utility Complete");

		#ifdef USER_LEEW
			getchar();
		#endif
		#ifdef USER_WILLB
			#if defined _DEBUG
			system("pause");
			#endif
		#endif
		return 0;
	}

	LOGERROR(GetLogger(), "Pak file not created!");
	return 1;
}



int main(int argc, char **argv, LPTSTR lpCmdLine)
{
	C_PAKUtility mainApp;

	S32 result = mainApp.Run(argc, argv);
	LOGDEBUG(mainApp.GetLogger(), "PakUtility Main app returning with code %i\n", result);

	return result;
}