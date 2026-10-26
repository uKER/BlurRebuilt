#ifndef __SYSTEM_STRINGUTIL_H__
#define __SYSTEM_STRINGUTIL_H__

#ifdef TOOL

namespace Util
{
	/// returns the contents of the string up to and including the last slash
	/// returns an empty string if there is no slash
	C_String ExtractDirectory(const C_String& inString);

	/// returns the contents of the string after the last slash
	/// returns the entire string if there is no slash
	C_String ExtractFilename(const C_String& inString);

	/// returns the contents of the string without the last '.' and any characters after it
	/// returns the entire string if there is no '.'
	C_String RemoveFileExtension(const C_String& inString);

	/// returns the contents of the string after the last '.'
	/// returns an empty string if there is no '.'
	C_String ExtractFileExtension(const C_String& inString);

	/// replace the contents of the string after but excluding the last '.' with the given extension
	/// appens to the end if there is no '.'
	C_String ReplaceFileExtension(const C_String& inString, const C_String& extension);

	/// concatenates the strings as follows: directory + filename + "." + extension
	C_String ConstructFileString( const C_String& directory, const C_String& filename, const C_String& extension);

	/// de-concatenates? the strings as follows: directory + filename + "." + extension
	void DeconstructFileString( const C_String& input, C_String& directory, C_String& filename, C_String& extension);

	/// flip slashes from windows ('\\') to mercury file system format ('/')
	C_String FormatFileString(const C_String &inString);

	/// flip slashes from windows ('\\') to unix format ('/')
	C_String FormatUnixFileString(const C_String &inString);

	/// flip slashes from unix format ('/') to windows ('\\')
	C_String FormatWindowsFileString(const C_String& inString);
	C_String FormatWindowsFileString256(const C_String256& inString);

	/// flip slashes to local format (format of target platform)
	/// note, the mercury file system always uses '/' regardless of the platform
	C_String FormatLocalFileString(const C_String& inString);

	/// Escape escape chars.
	C_String EscapeEscapeCharacters(const C_String& inString);

	C_String AddTrailingSlash(const C_String& inString, char slash);

	/// Remove trailing '\\' or '/' character.
	C_String RemoveTrailingSlash(const C_String& inString);

	/// Remove leading '\\' or '/' character.
	C_String RemoveLeadingSlash(const C_String& inString);

	/// Remove trailing ' ' characters.
	C_String RemoveTrailingSpaces(const C_String& inString);


	/// convert a given filename to the currrent directory
	const Util::C_String ConvertFilenameToCurrentDirectory(const Util::C_String &currentDirectory, const Util::C_String &filename);

	/// returns true if the given path is a relative path
	bool								IsRelativePath(const Util::C_String &path);
	#ifdef PLATFORM_PC

	C_String ConvertABNetworkPathToLocalPath(const C_String& assetPath);

	C_String ConvertLocalPathToABNetworkPath(
		const C_String& assetPath,
		const C_String& computerName);

	#endif
}

#endif

#endif
