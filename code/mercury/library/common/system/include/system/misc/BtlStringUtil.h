#ifndef __SYSTEM_BTLSTRINGUTIL_H__
#define __SYSTEM_BTLSTRINGUTIL_H__

//TODO: The Btl and Util string classes need to be unified via typedef. Having duplicate string utility
//functions is horrific.

namespace Misc
{
	/// returns the contents of the string up to and including the last slash
	/// returns an empty string if there is no slash
	Btl::String ExtractDirectory(const Btl::String& inString);

	/// returns the contents of the string after the last slash
	/// returns the entire string if there is no slash
	Btl::String ExtractFilename(const Btl::String& inString);

	/// returns the contents of the string without the last '.' and any characters after it
	/// returns the entire string if there is no '.'
	Btl::String RemoveFileExtension(const Btl::String& inString);

	/// returns the contents of the string after the last '.'
	/// returns an empty string if there is no '.'
	Btl::String ExtractFileExtension(const Btl::String& inString);

	/// replace the contents of the string after but excluding the last '.' with the given extension
	/// appens to the end if there is no '.'
	Btl::String ReplaceFileExtension(const Btl::String& inString, const Btl::String& extension);

	/// concatenates the strings as follows: directory + filename + "." + extension
	Btl::String ConstructFileString( const Btl::String& directory, const Btl::String& filename, const Btl::String& extension);

	/// de-concatenates? the strings as follows: directory + filename + "." + extension
	void DeconstructFileString( const Btl::String& input, Btl::String& directory, Btl::String& filename, Btl::String& extension);

#ifdef TOOL
	/// flip slashes from windows ('\\') to mercury file system format ('/')
	Btl::String FormatFileString(const Btl::String &inString);

	/// flip slashes from windows ('\\') to unix format ('/')
	Btl::String FormatUnixFileString(const Btl::String &inString);

	/// flip slashes from unix format ('/') to windows ('\\')
	Btl::String FormatWindowsFileString(const Btl::String& inString);
	Btl::String FormatWindowsFileString256(const Btl::String256& inString);

	/// flip slashes to local format (format of target platform)
	/// note, the mercury file system always uses '/' regardless of the platform
	Btl::String FormatLocalFileString(const Btl::String& inString);

	/// Escape escape chars.
	Btl::String EscapeEscapeCharacters(const Btl::String& inString);

	Btl::String AddTrailingSlash(const Btl::String& inString, char slash);

	/// Remove trailing '\\' or '/' character.
	Btl::String RemoveTrailingSlash(const Btl::String& inString);

	/// Remove leading '\\' or '/' character.
	Btl::String RemoveLeadingSlash(const Btl::String& inString);


	/// convert a given filename to the currrent directory
	const Btl::String ConvertFilenameToCurrentDirectory(const Btl::String &currentDirectory, const Btl::String &filename);

	/// returns true if the given path is a relative path
	bool								IsRelativePath(const Btl::String &path);
	#ifdef PLATFORM_PC

	Btl::String ConvertABNetworkPathToLocalPath(const Btl::String& assetPath);

	Btl::String ConvertLocalPathToABNetworkPath(
		const Btl::String& assetPath,
		const Btl::String& computerName);

	#endif
#endif
}

#endif
