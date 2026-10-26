#ifndef __SYSTEM_SERIALISE_FILEPATH_H__
#define __SYSTEM_SERIALISE_FILEPATH_H__

#ifdef _MSC_VER
#pragma once
#endif

#include <Btl/Utility/String.h>
#include <Core/Containers/String.h>
#include <System/Misc/BtlStringUtil.h>

#ifdef PLATFORM_PS3
namespace PlatformPS3
{
	extern char g_ps3_titleId[];
}
#endif

#define FILEPATH_DEFER_FORMAT

namespace Sys
{
	class C_FilePath
	{
	public:
		//MAX_PATH under XP and what we've elected to use across all platforms.
		static const U32 kMaxPath = 260;

		INLINE C_FilePath();
		INLINE C_FilePath(const C_FilePath& rhs);
		INLINE C_FilePath(const Util::C_String& filePath);
		INLINE C_FilePath(const Btl::String& filePath);
		INLINE C_FilePath(const Btl::String260& filePath);
		INLINE C_FilePath(const char* filePath);
		INLINE C_FilePath(const char filePath);

		///String length
		FORCE_INLINE S32 Length() const;

		INLINE const C_FilePath& operator=(const C_FilePath& rhs);
		INLINE const C_FilePath& operator=(const Util::C_String& rhs);
		INLINE const C_FilePath& operator=(const Btl::String& rhs);
		INLINE const C_FilePath& operator=(const Btl::String260& rhs);
		INLINE const C_FilePath& operator=(const char* rhs);
		INLINE const C_FilePath& operator=(const char rhs);

		//Store a pre-formatted path. DO NOT USE UNLESS ABSOLUTELY REQUIRED.
		INLINE void SetFormatted(const Util::C_String& filePath);
		INLINE void SetFormatted(const Btl::String& filePath);
		INLINE void SetFormatted(const Btl::String260& filePath);
		INLINE void SetFormatted(const char* filePath);

		///Concatenation operator
		INLINE C_FilePath operator + (const C_FilePath& rhs) const;

		///Concatenation operator
		INLINE const C_FilePath& operator += (const C_FilePath& rhs);

		///Equality
		INLINE const bool operator == (const C_FilePath& rhs) const;

		///Inequality
		INLINE const bool operator != (const C_FilePath& rhs) const;

		///Less than
		INLINE const bool operator < (const C_FilePath& rhs) const;

		///Greater than
		INLINE const bool operator > (const C_FilePath& rhs) const;

		/// Sub-string extraction operator.
		/// NOTE: lo = inclusive, hi = exclusive, range ( 0, 0 ) denotes an empty string!
		INLINE C_FilePath operator () (S32 lo, S32 hi) const;

		INLINE const char* Chars() const;
		INLINE const Btl::String260& String() const;

		///Forward Search
		///returns -1 if the substring was not found
		INLINE S32 Search(char c, S32 offset = 0) const;

		/// Reverse search
		/// returns -1 if the substring was not found
		INLINE S32 RevSearch(char c, S32 offset = 0) const;

		///Forward Search
		///returns -1 if the substring was not found
		INLINE S32 Search(const C_FilePath& rhs, S32 offset = 0) const;

		INLINE C_FilePath Replace(const C_FilePath& src, const C_FilePath& dst) const;

		INLINE const char& operator [] (S32 i) const;
		INLINE char& operator [] (S32 i);

		void TokeniseByDir(Btl::Vector<C_FilePath>& filePaths) const;

		INLINE void Clear();

	private:
		INLINE void FormatChar(char* pCursor);

#ifdef FILEPATH_DEFER_FORMAT
		INLINE void FormatFilePath() const;
		INLINE void StoreFilePathAndDirty(const char toFormat);
		INLINE void StoreFilePathAndDirty(const char* toFormat);

		mutable U8				m_isDirty;
		mutable Btl::String260	m_formattedFilePath;
#else
		INLINE void FormatFilePath();
		INLINE void FormatAndStoreFilePath(const char toFormat);
		INLINE void FormatAndStoreFilePath(const char* toFormat);
		Btl::String260	m_formattedFilePath;
#endif
	};

	INLINE char GetLocalDirToken();
	INLINE char GetNotLocalDirToken();

	bool IsRelativePath(const C_FilePath& path);

	C_FilePath ConvertToCurrentDirectory(
		const C_FilePath& currentDirectory, 
		const C_FilePath& filename);

	/// returns the contents of the string up to and including the last slash
	/// returns an empty string if there is no slash
	C_FilePath ExtractDirectory(const C_FilePath& filePath);

	/// returns the contents of the string after the last slash
	/// returns the entire string if there is no slash
	C_FilePath ExtractFilename(const C_FilePath& filePath);

	/// returns the contents of the string without the last '.' and any characters after it
	/// returns the entire string if there is no '.'
	C_FilePath RemoveFileExtension(const C_FilePath& filePath);

	/// returns the contents of the string after the last '.'
	/// returns an empty string if there is no '.'
	C_FilePath ExtractFileExtension(const C_FilePath& filePath);

	/// replace the contents of the string after but excluding the last '.' with the given extension
	/// appens to the end if there is no '.'
	C_FilePath ReplaceFileExtension(const C_FilePath& filePath, const C_FilePath& extension);

	C_FilePath AddTrailingSlash(const C_FilePath& filePath);

	/// Remove trailing '\\' or '/' character.
	C_FilePath RemoveTrailingSlash(const C_FilePath& filePath);

	/// Remove leading '\\' or '/' character.
	C_FilePath RemoveLeadingSlash(const C_FilePath& filePath);

	/// concatenates the strings as follows: directory + filename + "." + extension
	C_FilePath ConstructFilePath(const C_FilePath& dir, const C_FilePath& filename, const C_FilePath& extension);

	/// de-concatenates? the strings as follows: directory + filename + "." + extension
	void DeconstructFilePath(const C_FilePath& input, C_FilePath& path, C_FilePath& filename, C_FilePath& extension);

	#include "FilePath.inl"
};

#endif
