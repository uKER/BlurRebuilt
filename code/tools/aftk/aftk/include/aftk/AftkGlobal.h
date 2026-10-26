#ifndef AFTK_GLOBAL_H
#define AFTK_GLOBAL_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_Global
	{
	public:
		static bool Create();

		static bool Created();

		static void Destroy();

		///\brief Returns the AFTK library version number.
		///
		///@return Returns the version number.
		static U32 GetVersion();

		///\brief Returns the time-stamp indicating when this version of the AFTK was built.
		///
		///@return The build stamp.
		static std::string GetBuildStamp();

		///\brief Returns the name of the PC on which this version of the AFTK was built.
		///
		///@return The PC name.
		static std::string GetBuildPc();

		///\brief Returns the SVN revision numbers associated with this version of the AFTK.
		///
		///@return The SVN revision numbers.
		static std::string GetSvnRevision();

		///\brief Queries the highest supported version of a specified file format.
		///
		///@param format The file type to query.
		///@param version The version number of the file.
		///@return C_Status class, indicating the result of the operation.
		static C_Status GetHighestSupportedFileVersion(E_FileType format, U32& version);

		///\brief Queries the lowest supported version of a specified file format.
		///
		///@param format The file type to query.
		///@param version The version number of the file.
		///@return C_Status class, indicating the result of the operation.
		static C_Status GetLowestSupportedFileVersion(E_FileType, U32& version);

		///\brief Returns the type of the specified file.
		///
		///If the type could not be determined then AFTK_FILE_TYPE_UNKNOWN is returned.
		///
		///@param fileName The name of the file to query.
		///@param format The format of the queried file.
		///@return C_Status class, indicating the result of the operation.
		static C_Status GetFileType(const std::string& fileName, E_FileType& format);

		///\brief Queries the version of the specified file.
		///
		///@param fileName The name of the file to query.
		///@param version The version number of the file.
		///@return C_Status class, indicating the result of the operation.
		static C_Status GetFileVersion(const std::string& fileName, U32& version);

		///\brief Returns the file extension used by a given file format enum.
		///
		///@param format File format enum.
		///@return file extension.
		static std::string GetFileExtension(E_FileType format);

		///\brief Returns an array of strings describing the file formats that the AFTK reads.
		///
		///@return String array.
		static StringArray GetReadSupportedFileDescriptions();

		///\brief Returns an array of strings describing the file formats that the AFTK writes.
		///
		///@return String array.
		static StringArray GetWriteSupportedFileDescriptions();

		///\brief Returns an array of the file extensions read by the AFTK.
		///
		///@return String array.
		static StringArray GetReadSupportedFileExtensions();

		///\brief Returns an array of the file extensions written by the AFTK.
		///
		///@return String array.
		static StringArray GetWriteSupportedFileExtensions();

		static void OutputText(
			E_OutputChannel channel, 
			const std::string& text);

	private:
		static bool m_created;
	};
}

#endif
