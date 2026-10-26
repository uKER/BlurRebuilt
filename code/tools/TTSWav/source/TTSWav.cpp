#include "stdafx.h"

#include <system/xml/Xml.h>
#include <system/debug/log/logger.h>
#include <system/debug/log/subscribers/LogFileSubscriber.h>
#include <system/debug/log/subscribers/ConsoleSubscriber.h>
#include <system/misc/BtlStringUtil.h>
#include <fstream>

#include "Options.h"
#include "TTSWav.h"

namespace TTSWav
{

const Btl::String C_TTSWav::token = "placeholder";

C_TTSWav::C_TTSWav(
	const char *szName,
	const char *szSubtitle,
	const char *szLogger,
	const char *szDmpPath,
	bool bExecuter) 
	:
	Tools::C_ToolsApplication(
		szName,
		szSubtitle,
		szLogger,
		szDmpPath,
		bExecuter)
{
	gotVoice = false;
	CoInitialize( NULL );  //Start COM
	if ( SUCCEEDED(m_cpVoice.CoCreateInstance(CLSID_SpVoice)) )
	{
		gotVoice = true;
	}
}

C_TTSWav::~C_TTSWav()
{
	if(gotVoice)
	{
		m_cpVoice.Release();
	}
	CoUninitialize();  //Stop COM
}

bool C_TTSWav::BatchXmlToWav(const Btl::String& xmlFileName)
{
	Sys::C_XmlDom dom;

	if (!dom.Read(xmlFileName.Chars()))
	{
		LOGERROR(GetLogger(), "Unable to read config file %s", xmlFileName);
		return false;
	}

	const Sys::C_XmlDomBaseNode* pRoot = dom.GetRootNodePtr();
							
	if(pRoot && pRoot->GetChildren().Size() > 0)
	{ 
		const Sys::C_XmlDomBaseNode* pTTSWav = pRoot->GetChildren()[0];
		for (int i = 0; i < pTTSWav->GetChildren().Size(); i++)
		{
			const Sys::C_XmlDomBaseNode* pChild = pTTSWav->GetChildren()[i];
			const char* nnn = pChild->GetName();
			if ((pChild->GetName() == "speak") && (pChild->GetType() == Sys::E_XmlDomNodeType_Element))
			{
				Sys::C_XmlDomElement* pChildAsEl = (Sys::C_XmlDomElement*)pChild;
				const char* pText = pChildAsEl->GetAttributes().GetValueByName("text");
				const char* pVoiceName = pChildAsEl->GetAttributes().GetValueByName("voiceName");
				const char* pFile = pChildAsEl->GetAttributes().GetValueByName("file");
				if (pText && pFile) //pVoiceName not mandatory
					if(!ToWav(pText, pFile, pVoiceName == NULL ? "" : pVoiceName))
						return false;
			}
		}
		return true;
	} 
	else
	{
		LOGERROR(GetLogger(), "Incorrect XML format\n");
		return false;
	}
}

bool C_TTSWav::ToWav(const Btl::String& text, const Btl::String& wavFileName, const Btl::String& voiceName)
{	
	if(!gotVoice)
	{
		LOGERROR(GetLogger(), "Problem accessing TTS Engine\n");
		return false;
	}

	Btl::String dir = Misc::ExtractDirectory(wavFileName);
	if (!Sys::C_FileSystem::Instance().FileExists(dir.Chars()) &&
		!Sys::C_FileSystem::Instance().CreateDirectory(dir.Chars()))
	{
		LOGERROR(GetLogger(), "Failed to create output folder %s", dir.Chars());
		return false;
	}

	HRESULT							hr = S_OK;
	CComPtr <ISpStream>				cpStream;
	CSpStreamFormat					cAudioFmt;
	CComPtr <ISpObjectToken>		cpToken;
	CComPtr <IEnumSpObjectTokens>	cpEnum;

	hr = cAudioFmt.AssignFormat(SPSF_48kHz16BitMono);

	if(voiceName != "")	//change the voice type
	{
		//Enumerate voice tokens with attribute "Name=voiceName" 
		if(SUCCEEDED(hr))
		{
			Btl::String voiceQuery = "Name=";
			voiceQuery += voiceName;
			
			//convert to widechars
			const char* src = voiceQuery.Chars();
			int srcSize = voiceQuery.Length() * sizeof(char);
			//get the destination size by calling MultiByteToWideChar with a NULL destination buffer
			int destSize = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, src, -1, NULL, 0);
			wchar_t* dest = new wchar_t[destSize];
			MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, src, -1, dest, destSize);
			hr = SpEnumTokens(SPCAT_VOICES, dest, NULL, &cpEnum);
			delete[] dest;
		}
	    
		//Get the closest token
		if(SUCCEEDED(hr))
		{
			ULONG count; 
			hr = cpEnum->GetCount(&count);
			hr = cpEnum ->Next(1, &cpToken, NULL);
		}
		
		//set the voice 
		if(SUCCEEDED(hr))
		{
			hr = m_cpVoice->SetVoice(cpToken); //If no voice is found, it will use the windows default
		}
	}
	
	//Call SPBindToFile, a SAPI helper method,  to bind the audio stream to the file
	if(SUCCEEDED(hr))
	{
		hr = SPBindToFile( wavFileName.Chars(),  SPFM_CREATE_ALWAYS, 
			&cpStream, & cAudioFmt.FormatId(),cAudioFmt.WaveFormatExPtr() );
	}
	
	//set the output to cpStream so that the output audio data will be stored in cpStream
	if(SUCCEEDED(hr))
	{
		hr = m_cpVoice->SetOutput( cpStream, TRUE );
	}

	//Speak the text asynchronously
	if(SUCCEEDED(hr))
	{
		//convert to widechars
		const char* src = text.Chars();
		int srcSize = text.Length() * sizeof(char);
		//get the destination size by calling MultiByteToWideChar with a NULL destination buffer
		int destSize = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, src, -1, NULL, 0);
		wchar_t* dest = new wchar_t[destSize];
		MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, src, -1, dest, destSize);
		hr = m_cpVoice->Speak(dest,  SPF_DEFAULT, NULL );
		delete[] dest;
	}
	
	//close the stream
	if(SUCCEEDED(hr))
	{
		hr = cpStream->Close();
	}

	//Release the stream object
	cpStream.Release();

	bool tokenAdded = PutPlaceholderToken(wavFileName);

	if(SUCCEEDED(hr) && tokenAdded)
	{
		return true;
	} 
	else
	{
		LOGERROR(GetLogger(), "Failed to convert to WAV file");
		return false;
	}
}

bool C_TTSWav::PutPlaceholderToken(const Btl::String& wavFileName)
{
	//Open file for read and write
	std::fstream wavFile(
		wavFileName.Chars(),
		std::ios::in | std::ios::out | std::ios::binary );

	if (wavFile.is_open())
	{
		wavFile.seekg(0, std::ios::end);		
		std::fstream::pos_type prevSize;
		prevSize = wavFile.tellg();
		
		U32 tokenLen = token.Length();
		U32 newSize = (U32)prevSize + tokenLen - 8; //length chunk doesnt include RIFF(4) and its own size(4)
					
		//Fix length chunk (since we will add token at the end)
		wavFile.seekp(4, std::ios::beg); //length chunk starts at 4th byte 
		char* pNewSize = (char*)&newSize;
		wavFile << pNewSize[0] << pNewSize[1] << pNewSize[2] << pNewSize[3];

		//Append custom data
		wavFile.seekp(0, std::ios::end);
		wavFile << token.Chars();

		//Close File
		wavFile.close();

		//assert(IsPlaceholder(wavFileName));

		return true;
	}
	LOGERROR(GetLogger(), "Failed to add custom data to WAV file\n");
	return false;
}

bool C_TTSWav::IsPlaceholder(const Btl::String& wavFileName)
{
	 std::ifstream wavFile(
		wavFileName.Chars(),
		std::ios::in | std::ios::binary | std::ios::ate ); //open at the end

	if (wavFile.is_open())
	{
		
		char* ph = new char[token.Length() + 1];
		int size = wavFile.tellg();
		wavFile.seekg(size - token.Length(), std::ios::beg);
		wavFile.read(ph, token.Length());
		ph[token.Length()] = '\0';
		bool equal = !strcmp(ph, token.Chars());
		delete[] ph;
		wavFile.close();
		return equal;
	}
	LOGERROR(GetLogger(), "Failed to get custom placeholder data\n");
	return false;
}



S32 C_TTSWav::Main(S32 argc, char **argv, Progress::I_ProgressBar *pProgress)
{
	C_Options options;
	bool success = true;

	Debug::LogFileSubscriber* pLogFile = new Debug::LogFileSubscriber("TTSWav.log");
	GetLogger().AddSubscriber( pLogFile );

	Debug::ConsoleSubscriber* pConsole = new Debug::ConsoleSubscriber();
	GetLogger().AddSubscriber( pConsole );


	//Build options struct
	{
		//Parse our command-line options.
		success = ParseCommandLineOptions(argc, argv, options);
		if (!success) goto exit;
	}

	//This is where you would do some stuff.
	{
		success = BatchXmlToWav(options.xmlFileName);
		if (!success) goto exit;
	}

exit:
	if (success)
	{
		LOGINFO(GetLogger(), "Processing completed succesfully!\n");
	}
	else
	{
		LOGINFO(GetLogger(), "Processing failed!\n");
	}

	GetLogger().RemoveSubscriber(pConsole);
	delete pConsole;
	GetLogger().RemoveSubscriber(pLogFile);
	delete pLogFile;

	return success ? 0 : 1;
}

bool C_TTSWav::ParseCommandLineOptions(
	int argc, 
	char* argv[], 
	C_Options& options)
{
	// Wrap everything in a try block.  Do this every time, 
	// because exceptions will be thrown for problems. 
	try
	{  
		//Define the command line object.
		TCLAP::CmdLine cmd("TTSWav Command-line Tool.", ' ', "1.00");

		//Define a value argument and add it to the command line.
		TCLAP::ValueArg<std::string> xmlFileName(
			"f","filename","The XML Filename",true,"","String containing filename");
		cmd.add(xmlFileName);

		//Parse the args.
		cmd.parse(argc, argv);

		//Get the values
		options.xmlFileName = xmlFileName.getValue().c_str();
	} 
	catch (TCLAP::CmdLineParseException &e)
	{
		LOGERROR(GetLogger(), "Failed to parse arguments: %s\n", e.argId().c_str());
		return false;
	}
	catch (TCLAP::ArgException &e)  // catch any exceptions
	{ 
		LOGERROR(GetLogger(), "error: %s for arg: %s\n", e.error(), e.argId().c_str());
		return false;
	}

	return true;
}

} //namespace TTSWav
