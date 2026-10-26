#include "stdafx.h"
#include "AppHeaders.h"

int IsDir(const char* pchStr);

using namespace Audio;

//-----------------------------------------------------------------------------
// Name: CreateXmlFromFiles
// Desc: Given a filename, will strip the path from it and look for all known
//		 wave files in the directory and make a bank xml file containing those
//		 waves.
//-----------------------------------------------------------------------------
void CreateXmlFromFiles(char* pchFname)
{
	//std::string strFname;

	char pchDrive[_MAX_DRIVE]={0};
	char pchDir[_MAX_DIR]={0};
	std::string strBankName = "bank";
		
	std::string strPath;
	
	if (IsDir(pchFname))
	{
		strPath = pchFname;
		// add backslash if one not present
		if (pchFname[strlen(pchFname)-1] != '\\' || pchFname[strlen(pchFname)-1] != '/')
			strPath += "\\";
	}
	else
	{
		_splitpath(pchFname, pchDrive, pchDir, NULL, NULL);
		strPath = std::string(pchDrive)+std::string(pchDir);

		// strip name of directory for naming the bank
		size_t end = strPath.find_last_of ( "\\");
		size_t start = strPath.rfind("\\", end-1);
		if (start != -1 && end != -1)
			strBankName = strPath.substr(start+1, end-start-1);	

		if (!IsDir(strPath.c_str()))
		{
			PrintError("could not strip directory from '%s'.\n", pchFname, strPath.c_str());
			return;
		}
	}
		
	//strPath += std::string("*");
	Audio::Print(0, "Trying to generate bank xml file...\n");
	Audio::Print(0, "Dir is '%s'...\n", strPath.c_str());
	Audio::Print(0, "Looking for .wav, .adpcm & .xma files \n\n");

	// got path. now look at all known filetypes in that path and build a list of them

	HANDLE find=NULL;
	WIN32_FIND_DATA wfd;

	memset(&wfd, 0, sizeof(wfd));

	std::string strPathWithWildcard = strPath + "*";
	find = FindFirstFile(strPathWithWildcard.c_str(), &wfd);

	if (find==INVALID_HANDLE_VALUE) 
	{
		return;
	}

	File file;
	std::string strXmlName=strBankName+".xml";
	std::string strXmlFname = strPath+strBankName+".xml";

	if (file.OpenWrite(strXmlFname.data())==false)
	{
		U32 error = GetLastError();
		LPVOID lpMsgBuf;
		FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL );
		PrintError("could not open '%s' for writing. %s\n", strXmlFname.c_str(), (char*)lpMsgBuf);
		if (error == 5)
			Print(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY, "[If overwriting an existing xml bank file make sure its not read only]\n\n");
		return;
	}

	//<wavebank name="bank" endian="big">
	char* pchHeaderTag1 = "<wavebank name=\"";
	char* pchHeaderTag2 = "\" endian=\"big\" streaming=\"no\" bitDepth=\"16\">\n";
	char* pchEndTag = "</wavebank>";
	char* pchWaveTag ="\t<wave\n";
	char* pchFilename = "\t\tfilename = \"";	
	char* pchInternalName = "\t\tinternalName = \"";	
	char* pchWaveFormat =  "\t\tformat = \"";
	char* pchWav = "pcm_wav_s16\"";
	char* pchXma = "xma\"/>\n";
	char* pchAdpcm = "adpcm\"/>\n";
	char* pchLoop = "\n\t\tloop = \"yes\"/>\n";

	file.Write(pchHeaderTag1, (U32)strlen(pchHeaderTag1));
	file.Write((void*)strBankName.c_str(), (U32)strBankName.length());
	file.Write(pchHeaderTag2, (U32)strlen(pchHeaderTag2));

	int numFilesAdded=0;

	for (;;)
	{
		if ( (!strnicmp(".wav", wfd.cFileName+strlen(wfd.cFileName)-4, 4)) ||
			 (!strnicmp(".adpcm", wfd.cFileName+strlen(wfd.cFileName)-6, 6)) ||
			 (!strnicmp(".xma", wfd.cFileName+strlen(wfd.cFileName)-4, 4)) )
		{
			Audio::Print(0, "adding %s\n",  wfd.cFileName);

			file.Write(pchWaveTag, (U32)strlen(pchWaveTag));

			file.Write(pchFilename, (U32)strlen(pchFilename));
			file.Write(wfd.cFileName, (U32)strlen(wfd.cFileName));
			file.Write("\"\n", 2);

			file.Write(pchInternalName, (U32)strlen(pchInternalName));
			if (!strnicmp(".adpcm", wfd.cFileName+strlen(wfd.cFileName)-6, 6))
				file.Write(wfd.cFileName, (U32)strlen(wfd.cFileName)-6);
			else
				file.Write(wfd.cFileName, (U32)strlen(wfd.cFileName)-4);
			file.Write("\"\n", 2);

			file.Write(pchWaveFormat, (U32)strlen(pchWaveFormat));
			if (!strnicmp(".wav", wfd.cFileName+strlen(wfd.cFileName)-4, 4))
			{
				file.Write(pchWav, (U32)strlen(pchWav));

				// see if file is looping and if so add looping flag
				std::string strWavFullpath = strPath+wfd.cFileName;
				Audio::CWaveFile waveFile;
				if(FAILED(waveFile.OpenForRead(strWavFullpath.c_str())))
				{
					PrintError("Wave file '%s' not found\n", strWavFullpath.c_str());
					break;
				}

				bool bLoop=false;
				if (SUCCEEDED(waveFile.ReadLoopInfo()))
				{
					bLoop = waveFile.IsLoop();
					
					if (bLoop)
						file.Write(pchLoop, (U32)strlen(pchLoop));
				}
				else
					PrintWarning("Could not read loop info for '%s'\n", wfd.cFileName);
				
				if (bLoop == false)
					file.Write("/>\n", (U32)strlen("/>\n"));
			}
			else
			if (!strnicmp(".adpcm", wfd.cFileName+strlen(wfd.cFileName)-6, 6))
				file.Write(pchAdpcm, (U32)strlen(pchAdpcm));
			else
			if (!strnicmp(".xma", wfd.cFileName+strlen(wfd.cFileName)-4, 4))
				file.Write(pchXma, (U32)strlen(pchXma));

			numFilesAdded++; 
		}

		if (FindNextFile(find, &wfd)==0) 
		{
			DWORD dwError;
			dwError = GetLastError();
			if (dwError == ERROR_NO_MORE_FILES) 
				break;
			else 
			{
				PrintError ("FindNextFile error. Error is %u\n", dwError);
				break;
			}
		}
	}

	FindClose(find);

	file.Write(pchEndTag, (U32)strlen(pchEndTag));

	file.Close();

	if (numFilesAdded==0)
		PrintError("\nCould not find any wave files.\n"); 

	Audio::Print(0, "\nAdded %d file(s) to the xml file %s\n", numFilesAdded, strXmlName.c_str()); 
	if (numFilesAdded)
		Audio::Print(0, "\nDone.\n"); 

	return;
}

//-----------------------------------------------------------------------------
// Name: IsDir
// Desc: Given a string determines if it is a directory
//-----------------------------------------------------------------------------
int IsDir(const char* pchStr)
{
	DWORD dwAttribs = GetFileAttributes(pchStr);
	if (dwAttribs == 0xffffffff)
		return 0;
	if (dwAttribs & FILE_ATTRIBUTE_DIRECTORY)
		return 1;
	return 0;
}