#pragma once

class CommandLineApp
{
	public:
		CommandLineApp() { 	m_bBigEndian = true; m_pApp = this; }
		CommandLineApp(const char* const strAppName) { Init(strAppName); m_bBigEndian = true; m_pApp = this; }
		~CommandLineApp() { }

		inline U16 DoEndian(U16 uValue)
		{
			if (m_bBigEndian)
			{
				U8 uNew[2];
				uNew[0] = uValue >> 8;
				uNew[1] = uValue & 0xff;
				return *(U16*)uNew;
			}
			else
				return uValue;
		}

		S16 DoEndian(S16 sValue) { return (S16)DoEndian((U16)sValue); }

		U32 EndianSwap32Bit(U32 uValue)
		{
			U8 uNew[4];
			uNew[0] = U8(uValue >> 24);
			uNew[1] = U8((uValue & 0xff0000) >> 16);
			uNew[2] = U8((uValue & 0xff00) >> 8);
			uNew[3] = U8(uValue & 0xff);
			return *(U32*)uNew;
		}

		U32 DoEndian(U32 uValue)
		{
			if (m_bBigEndian)
			{
				return EndianSwap32Bit(uValue);
			}
			else
				return uValue;
		}

		S32 DoEndian(S32 sValue) { return (S32)DoEndian((U32)sValue); }

		F32 DoEndian(F32 fValue)
		{
			if (m_bBigEndian)
			{
				U32 uValue = *(U32*)&fValue;
				U32 uRet = EndianSwap32Bit(uValue);
				return *(F32*)&uRet;
			}
			else
				return fValue;
		}

		void GetOutputFilename(char* sDestString, char* sDestinationDirectory, const char* sSourceString, char* strExtension)
		{
			char* strModdedSource = (char*)sSourceString;

			if (sDestinationDirectory && strlen(sDestinationDirectory))
			{
				char* strTempString = strModdedSource;
				while (strTempString)
				{
					strTempString = strpbrk(strTempString, "/\\");
					if (strTempString)
					{
						strTempString++;
						strModdedSource = strTempString;
					}
				}

				sprintf(sDestString, "%s\\%s", sDestinationDirectory, strModdedSource);
			}
			else
				strcpy(sDestString, strModdedSource);

			char* sExtension = strpbrk(sDestString, ".") + 1;
			strcpy(sExtension, strExtension);
		}

		virtual void Run(char* sFileToParse, char* sComLineOutputFile, char* sDestinationDirectory, bool bStrict, bool bBigEndian, bool bPauseAtEnd, bool bWriteBankList) = 0;

		void DisplayHelpMessage()
		{
			printf("\n%s [source] [/strict] [/littleendian] [/noprompt] [/output=] [/destdir=] [/writebanklist]\n\n", m_strAppName);
			system("pause");
			exit(0);
		}

		int Begin(int argc, char* argv[])
		{
			char sFileToParse[128] = { 0 };
			char sOutputFile[128] =  { 0 };
			char sDestDir[256] =  { 0 };

			bool bStrict = false;
			bool bBigEndian = true;
			bool bNoPrompt = false;
			bool bWriteBankList = false;

			system("color 0a");

			if (argc == 1)
				DisplayHelpMessage();
			else
			{
				for (int i = 1; i < argc; i++)
				{
					if (strncmp((const char*)argv[i], "/?", strlen("/?")) == 0)
					{
						DisplayHelpMessage();
					}
					if (strncmp((const char*)argv[i], "/output=", strlen("/output=")) == 0)
					{
						strcpy(sOutputFile, (const char*)argv[i] + strlen("/output="));
						printf("Output file set to %s\n", sOutputFile);
					}
					else if (strncmp((const char*)argv[i], "/strict", strlen("/strict")) == 0)
					{
						bStrict = true;
					}
					else if (strncmp((const char*)argv[i], "/littleendian", strlen("/littleendian")) == 0)
					{
						bBigEndian = false;
					}
					else if (strncmp((const char*)argv[i], "/noprompt", strlen("/noprompt")) == 0)
					{
						bNoPrompt = true;
					}
					else if (strncmp((const char*)argv[i], "/destdir=", strlen("/destdir=")) == 0)
					{
						strcpy(sDestDir, (const char*)argv[i] + strlen("/destdir="));
						printf("Destination directory set to %s\n", sDestDir);
					}
					else if (strncmp((const char*)argv[i], "/writebanklist", strlen("/writebanklist")) == 0)
					{
						bWriteBankList = true;
					}
					else if (! strlen(sFileToParse))
						strcpy(sFileToParse, (const char*)argv[i]);
				}
			}

			if (bStrict)
				printf("STRICT mode enabled.\n");
			else
				printf("STRICT mode disabled\n");


			printf("current directory: ");
			system("cd");
			printf("Parsing file %s\n", sFileToParse);

			if (! bNoPrompt)
				system("pause 1");

			//_CrtSetBreakAlloc(5309);
			Run(sFileToParse, sOutputFile, sDestDir, bStrict, bBigEndian, (! bNoPrompt), bWriteBankList);

			//    _CrtDumpMemoryLeaks();

			//#ifdef _DEBUG
			//	assert(! _CrtDumpMemoryLeaks());
			//#endif

			return 0;
		}

	void WriteBytes(void* pData, U32 u32Size, FILE* fDest, bool bEndianSwapIfNecessary = true)
	{
		if (bEndianSwapIfNecessary)
		{
			U32 u32BytesWritten = 0;
			U32* pu32Data = (U32*)pData;

			if (! u32Size)
				return;
			
			while (1)
			{
				U32 u32ToWrite = DoEndian(*(pu32Data + (u32BytesWritten / sizeof(U32))));
				fwrite(&u32ToWrite, sizeof(U32), 1, fDest);
				u32BytesWritten += sizeof(U32);

				if (u32BytesWritten == u32Size)
					return;

				if (u32BytesWritten > u32Size)
				{
					assert (0 && "writing slab of data that isn't 4 byte aligned!!");
					return;
				}
			}
		}
		else
			fwrite(pData, u32Size, 1, fDest);
	}

	void								SetLittleEndian() { m_bBigEndian = false; }
	static CommandLineApp*				GetApp() { return m_pApp; }
	bool								IsBigEndian() { return m_bBigEndian; }

protected:
		void	Init(const char* const strAppName) { strncpy(m_strAppName, strAppName, 64); m_strAppName[63] = 0; }

		char	m_strAppName[64];
		bool	m_bBigEndian;
		static CommandLineApp*	m_pApp;
};
