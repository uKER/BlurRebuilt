#include "SystemPch.h"

#include <btl/utility/string.h>
#include <system/misc/BtlStringUtil.h>

namespace Misc
{

/// returns the contents of the string up to and including the last slash
/// returns the empty string if there is no slash
Btl::String ExtractDirectory(const Btl::String& inString)
{
    Btl::String testString = inString.Replace("/", "\\");

    S32 slashPos = testString.RevSearch('\\');
    if(slashPos < 0)
    {
        return Btl::String("");
    }
    else
    {
        return inString(0, slashPos + 1);
    }
}

/// returns the contents of the string after the last slash
/// returns the entire string if there is no slash
Btl::String ExtractFilename(const Btl::String& inString)
{
    Btl::String testString = inString.Replace("/", "\\");

    S32 startPos;

    S32 slashPos = testString.RevSearch('\\');
    if(slashPos < 0)
    {
        startPos = 0;
    }
    else
    {
        startPos = slashPos + 1;
    }

	return inString(startPos, testString.Length());
}

/// returns the contents of the string without the last '.' and any characters after it
/// returns the entire string if there is no '.'
Btl::String RemoveFileExtension(const Btl::String& inString)
{
    Btl::String testString = inString.Replace("/", "\\");

    // extension may have more than one dot
    // reverse search for slash, this is where the filename starts
    S32 slashPos = testString.RevSearch('\\');
    if(slashPos < 0)
    {
        slashPos = 0;
    }

    // then forward search for dot from start of filename
    S32 dotPos = testString.RevSearch('.');
    if(dotPos <= 0 || slashPos > dotPos)
        return Btl::String("");
    else
		return inString(0, dotPos);
}

/// returns the contents of the string after the last '.'
/// returns an empty string if there is no '.'
Btl::String ExtractFileExtension(const Btl::String& inString)
{
    Btl::String testString = inString.Replace("/", "\\");

    // extension may have more than one dot
    // reverse search for slash, this is where the filename starts
    S32 slashPos = testString.RevSearch('\\');
    if(slashPos < 0)
    {
        slashPos = 0;
    }

    // then forward search for dot from start of filename
    S32 dotPos = testString.RevSearch('.');
    if(dotPos < 0 || slashPos > dotPos)
        return Btl::String("");
    else
        return inString(dotPos + 1, inString.Length());
}

/// replace the contents of the string after but excluding the last '.' with the given extension
/// appens to the end if there is no '.'
Btl::String ReplaceFileExtension(const Btl::String& inString, const Btl::String& extension)
{
	return RemoveFileExtension(inString) + Btl::String(".") + extension;
}

/// concatenates the strings as follows: directory + filename + "." + extension
Btl::String ConstructFileString( const Btl::String& dir, const Btl::String& filename, const Btl::String& extension)
{
	Btl::String result;
    if(extension == Btl::String(""))
    {
        result = dir + filename;
    }
    else
    {
        result = dir + filename + Btl::String(".") + extension;
    }

	return result;
}
/// de-concatenates? the strings as follows: directory + filename + "." + extension
void DeconstructFileString( const Btl::String& input, Btl::String& path, Btl::String& filename, Btl::String& extension)
{
	path = ExtractDirectory(input);
	filename = RemoveFileExtension(	ExtractFilename(input) );
	extension = ExtractFileExtension(input);
}


#ifdef TOOL

/// flip slashes from windows ('\\') to mercury file system format ('/')
Btl::String FormatFileString(const Btl::String &inString)
{
   Btl::String rtn = inString.Replace("\\", "/");
   return rtn;
}

/// flip slashes from windows ('\\') to unix format ('/')
Btl::String FormatUnixFileString(const Btl::String &inString)
{
   Btl::String rtn = inString.Replace("\\", "/");
#ifdef PLATFORM_PS3
	char searchStr[] = "/USRDIR/";
	char *lower = (char *)strstr(rtn.Chars(), searchStr);
	if (lower)
	{
		U32 start = strlen(searchStr);
		U32 end = strlen(lower);
		for (U32 i = start ; i < end ; i++)
		{
			if (lower[i] >= 'A' && lower[i] <= 'Z') lower[i] += 0x20;
		}
	}
#else
   rtn.ConvertToLower();
#endif

   return rtn;
}

/// flip slashes from unix format ('/') to windows ('\\')
//lint -save -e111
Btl::String FormatWindowsFileString(const Btl::String& inString)
{
   Btl::String rtn = inString;//.Replace("/", "\\");
   for (U32 i = 0 ; i < (U32)inString.Length() ; i++)
   {
		if ( rtn[i] == '/') rtn[i] = '\\';
   }
   rtn.ConvertToLower();
   return rtn;
}
//lint -restore

Btl::String FormatWindowsFileString256(const Btl::String256& inString)
{
   Btl::String256 rtn = inString;//.Replace("/", "\\");
   for (U32 i = 0 ; i < (U32)inString.Length() ; i++)
   {
		if ( rtn.NonConstChars()[i] == '/') rtn.NonConstChars()[i] = '\\';
		if ( rtn.NonConstChars()[i] >= 'A' && rtn.NonConstChars()[i] <= 'Z') rtn.NonConstChars()[i] += 0x20;
   }
   return rtn;
}

Btl::String AddTrailingSlash(const Btl::String& inString, char slash)
{
	if (inString.Length() > 0)
	{
		char c = inString[inString.Length()-1];
		if (c == '\\' || c == '/')
			return inString;
	}

	return inString + Btl::String(slash);
}

Btl::String RemoveTrailingSlash(const Btl::String& inString)
{
	if (inString.Length() > 0)
	{
		char c = inString[inString.Length()-1];
		if (c == '\\' || c == '/')
			return inString(0, inString.Length()-1);
	}

	return inString;
}

Btl::String RemoveLeadingSlash(const Btl::String& inString)
{
	if (inString.Length() > 0)
	{
		char c = inString[0];
		if (c == '\\' || c == '/')
			return inString(1, inString.Length());
	}

	return inString;
}


/// flip slashes to local format (format for target platform)
/// note, the mercury file system always uses '/' regardless of the platform
Btl::String FormatLocalFileString(const Btl::String& inString)
{
	#if defined(PLATFORM_PC)
	return FormatWindowsFileString(inString);
	#elif defined(PLATFORM_PS3)
	return FormatUnixFileString(inString);
	#elif defined(PLATFORM_LINUX)
	return FormatUnixFileString(inString);
	#else
	#error ("*** ERROR: Unsupported platform")
	#endif
}

/// Escape escape chars.
Btl::String EscapeEscapeCharacters(const Btl::String& inString)
{
	return inString.Replace("\\", "\\\\");
}

bool IsRelativePath(const Btl::String &path)
{
	if (path.Length() == 0)
		return false;

	if (path.Search(':') != -1)
		return false;

	const char *p = path.Chars();
	if (p[0] == '\\' || p[0] == '/')
		return false;

	return true;
}


const Btl::String ConvertFilenameToCurrentDirectory(const Btl::String &currentDirectory, const Btl::String &filename)
{
	// if the filename isn't a relative path then return it as is
	if (!IsRelativePath(filename))
		return filename;

	if (currentDirectory.Length() == 0)
		return filename;

	// Make sure that the join between the current working directory and the filename are
	// going to marry up correctly i.e. Remove "\" ".\" 
	int stripPos = 0;
	char *p = (char *)filename.Chars();
	if ((p[0] == '\\') || (p[0] == '\\')) 
		stripPos = 1;
	if (filename.Length() == 1 && p[0] == '.')
		stripPos = 1;
	else if ((p[0] == '.') && ((p[1] == '\\') || (p[1] == '/'))) 
		stripPos = 2;
//	Btl::String fname = &p[stripPos];
	// prepend with the current directory and return that
//	return Btl::String(currentDirectory + "\\" + fname);
	Btl::String fname ( currentDirectory + "\\" + filename( stripPos, filename.Length() ) );
	return fname;
}


#ifdef PLATFORM_PC

// [Blur2PC] The Alienbrain workspace drive (retail hard-coded z:) comes from BIZARRE_ROOT, the
// variable the build scripts use for that share; unset, paths pass through unchanged.
static Btl::String AlienBrainDrive()
{
	const char* root = getenv("BIZARRE_ROOT");
	return (root && *root) ? Btl::String(root) + "\\" : Btl::String("");
}
#define ALIEN_BRAIN_DRIVE	AlienBrainDrive()
#define ALIEN_BRAIN_FOLDER	"alienbrainwork\\"

Btl::String ConvertABNetworkPathToLocalPath(const Btl::String& assetPath)
{
	Btl::String result(assetPath);
	result = FormatWindowsFileString(result);
	// Look for /<machine_name>/alienBrainWork and convert back to Z:
	S32 pos = result.ISearch(ALIEN_BRAIN_FOLDER);
	if (pos == -1)
		return result;

	result = ALIEN_BRAIN_DRIVE + result(pos + (S32)strlen(ALIEN_BRAIN_FOLDER), result.Length());
	return result;
}

Btl::String ConvertLocalPathToABNetworkPath(
	const Btl::String& assetPath,
	const Btl::String& computerName)
{
	Btl::String result(assetPath);
	result = FormatWindowsFileString(result);

	S32 pos = ALIEN_BRAIN_DRIVE.Length() ? result.ISearch(ALIEN_BRAIN_DRIVE) : -1;
	if (pos != 0)
	{
		return result;
	}

	result = Btl::String("\\\\") + computerName + Btl::String("\\") + Btl::String(ALIEN_BRAIN_FOLDER) + result(pos + ALIEN_BRAIN_DRIVE.Length(), result.Length());
	return result;
}

#endif

#endif

}
