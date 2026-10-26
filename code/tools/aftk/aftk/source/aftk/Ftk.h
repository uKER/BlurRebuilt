#ifndef AFTK_FTK_H
#define AFTK_FTK_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_Scene;
	class C_Progress;

	///
	///This abstract class defines the interface for all AFTK compatable FTK's.
	///
	class C_Ftk
	{
	public:
		///\brief Queries the lowest supported file version of the file read/writer.
		///
		///If not defined in a derived class, this returns C_Status::NOT_IMPLEMENTED.
		///
		///@param version The version number of the file.
		///@return C_Status class, indicating the result of the operation.
		static U32 GetLowestSupportedFileVersion()
		{
			return 100;
		}

		///\brief Queries the highest supported file version of the file read/writer.
		///
		///If not defined in a derived class, this returns C_Status::NOT_IMPLEMENTED.
		///
		///@param version The version number of the file.
		///@return C_Status class, indicating the result of the operation.
		static U32 GetHighestSupportedFileVersion()
		{
			return 100;
		}
	    
		///\brief Queries the version number of the given file.
		///
		///If not defined in a derived class, this returns C_Status::NOT_IMPLEMENTED.
		///
		///@param fileName The name of the file to read.
		///@param version The version number of the file.
		///@return C_Status class, indicating the result of the operation.
		static C_Status GetFileVersion(
			const std::string& fileName, 
			U32& version)
		{
			return C_Status::E_Status_NotImplemented;
		}
		
		///\brief Read file from disk and store within AFTK.
		///
		///If not defined in a derived class, this returns C_Status::NOT_IMPLEMENTED.
		///
		///@param fileName The name of the file to read.
		///@param pScene The scene which is to be filled using the given file.
		///@param importParameters Any parameters which are to be passed to the importer.
		///@param pProgressCallback Callback structure for file loading progress.
		///@return C_Status class, indicating the result of the operation.
		virtual C_Status Read(
			const std::string& fileName, 
			C_Scene *pScene, 
			bool& nonFatalErrorsEncountered,
			const char* pOptions,
			C_Progress* pProgressCallback = 0)
		{
			return C_Status::E_Status_NotImplemented;
		}
		
		///\brief Write AFTK contents to disk.
		///
		///If not defined in a derived class, this returns C_Status::NOT_IMPLEMENTED.
		///
		///@param fileName The name of the file to create/overwrite.
		///@param version The version of the file to create. Setting this to 0 always writes out the latest version of the file format.
		///@param pScene The scene which is to be written out.
		///@param exportParameters Any parameters which are to be passed to the exporter.
		///@param pProgressCallback Callback structure for file saving progress.
		///@return C_Status class, indicating the result of the operation.
		virtual C_Status Write(
			const std::string &fileName, 
			F32 version, 
			const C_Scene *pScene,
			bool& nonFatalErrorsEncountered,
			const char* pOptions,
			C_Progress* pProgressCallback = 0)
		{
			return C_Status::E_Status_NotImplemented;
		}

		virtual C_Status ReadExternalScene(
			C_Scene *pScene,
			const char* pOptions,
			C_Progress* pProgressCallback = 0)
		{
			return C_Status::E_Status_NotImplemented;
		}

		virtual C_Status WriteExternalScene(
			const C_Scene *pScene,
			const char* pOptions,
			C_Progress* pProgressCallback = 0)
		{
			return C_Status::E_Status_NotImplemented;
		}
	};
}

#endif
