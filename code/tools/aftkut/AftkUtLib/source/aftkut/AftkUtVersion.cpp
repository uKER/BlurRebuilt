//version info

#include <stdafx.h>
#include "aftkut/aftkUt.h"

using namespace AftkUt;

std::string C_Version::GetBuildStamp() 
{
	const static char* version = "Version: ABCD.EF.GH (HH:MM)";
	return std::string(version);
}

std::string C_Version::GetBuildPc() 
{
	const static char* buildPC = "BuildPC:                                                                                                                                                                                                                                                               ";
	return std::string(buildPC);
}


std::string C_Version::GetSvnRevision() 
{
	const static char* svnRevision = "SVN Rev:                                                                                                                                                                                                                                                               ";
	std::string svnRevisionStr(svnRevision);
	std::string::size_type pos = svnRevisionStr.find("\n");
	if (pos != std::string::npos)
	{
		svnRevisionStr = svnRevisionStr.substr(0, pos);	
	}
	
	return svnRevisionStr;
}
