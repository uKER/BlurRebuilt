#include "stdafx.h"
#include "WorldShaderConfig.h"

//#define USE_OLD_SHADER_COLLECTOR

#ifndef USE_OLD_SHADER_COLLECTOR

static Sys::C_StreamPos 	g_WhereShaderOffsetsAreInFile	= 0;
static Sys::C_StreamPos 	g_WhereShaderSizesAreInFile		= 0;
static Sys::C_StreamPos 	g_WhereBufferSizeIsInFile		= 0;
static Sys::C_StreamPos 	g_WhereBufferDataIsInFile		= 0;
static S32 					g_BufferSize					= 0;

void processShader(Sys::I_File* pShaderFile, Sys::I_File* pOutputFile, const S32 shaderIndex, S32* pShaderOffsets, S32* pShaderSizes)
{
	char padding = 0;
	
	Sys::BinaryDeserialiser<Ut::BigEndianToNative> shaderFile(*pShaderFile);
	Sys::BinarySerialiser<Ut::NativeToBigEndian> outputFile(*pOutputFile);
	
	S32 fileSize = (S32)pShaderFile->Length();
	U8* pFileData = new U8[fileSize];
	pShaderFile->Read(pFileData, fileSize);

	S32 alignStart = (g_BufferSize+32+127) & ~(127);

	S32 headerStart = alignStart-32;

	pShaderOffsets[shaderIndex] = headerStart;
	pShaderSizes[shaderIndex] = fileSize;

	for(S32 i=g_BufferSize; i<headerStart; i++)
	{
		outputFile << padding;
		g_BufferSize++;
	}

	assert((U32)g_BufferSize+32 == ((U32)(g_BufferSize+32+127) & ~127));

	for(S32 i=0; i<fileSize; i++)
	{
		outputFile << pFileData[i];
		g_BufferSize++;
	}
	
	SAFE_DELETE_ARY(pFileData);
}

S32 _tmain(S32 argc, _TCHAR* argv[])
{
	Util::C_String inputFileName = "";
	Util::C_String outputFileName = "";
	Util::C_String searchPath = "";
	
	for(S32 i=1; i<argc; i++)
	{
		switch(i)
		{
		case 1:
			inputFileName.Printf("%S", argv[i]);
			break;
		case 2:
			outputFileName.Printf("%S", argv[i]);
			break;
		case 3:
			searchPath.Printf("%S", argv[i]);
			break;
		}
	}

	if(inputFileName == "")
	{
		printf("Input file name not valid '%s'\n", inputFileName.Chars());
		return 1;
	}

	if(outputFileName == "")
	{
		printf("Input file name not valid '%s'\n", outputFileName.Chars());
		return 1;
	}

	if(searchPath == "")
	{
		printf("Search path not valid '%s'\n", searchPath.Chars());
		return 1;
	}
	
	S32 errorCode = 0;

	// Open the shader registry

	Sys::I_File* pInputFile = Sys::C_FileSystem::Instance().Open(inputFileName, Sys::E_FileAccess_Read);
	if(pInputFile)
	{
		// Load the shader registry

		ShaderRegistry::C_Registry shaderRegistry;
		shaderRegistry.Load(*pInputFile);

		// This is how many shaders are in the shader registry

		S32 shaderCount = shaderRegistry.GetVertexShaderList().Size()+shaderRegistry.GetPixelShaderList().Size();

		// Write out the data

		Sys::I_File* pOutputFile = Sys::C_FileSystem::Instance().Open(outputFileName, Sys::E_FileAccess_Write);
		
		Sys::BinarySerialiser<Ut::NativeToBigEndian> outputFile(*pOutputFile);

		// Write out shader count
		
		outputFile << shaderCount;

		// Write out shader offsets

		g_WhereShaderOffsetsAreInFile = pOutputFile->GetPosition();
		S32* pShaderOffsets = new S32[shaderCount];
		for(S32 i=0; i<shaderCount; i++)
			outputFile << pShaderOffsets[i];

		// Write out shader sizes

		g_WhereShaderSizesAreInFile = pOutputFile->GetPosition();
		S32* pShaderSizes = new S32[shaderCount];
		for(S32 i=0; i<shaderCount; i++)
			outputFile << pShaderSizes[i];

		// Write out buffer size and data

		g_WhereBufferSizeIsInFile = pOutputFile->GetPosition();
		outputFile << g_BufferSize;
		g_WhereBufferDataIsInFile = pOutputFile->GetPosition();

		Util::C_String shaderName;

		S32 shaderIndex = 0;

		// Process all the vertex shaders

		for(S32 i=0; i<shaderRegistry.GetVertexShaderList().Size(); i++)
		{
			shaderName.Printf("%s\\%s.p3v",
				searchPath.Chars(),
				shaderRegistry.GetStringTable()[ shaderRegistry.GetVertexShaderList()[i].m_NameId ]);

			Sys::I_File* pShaderFile = Sys::C_FileSystem::Instance().Open(shaderName, Sys::E_FileAccess_Read);
			if(pShaderFile)
			{	
				processShader(pShaderFile, pOutputFile, shaderIndex, pShaderOffsets, pShaderSizes);
				SAFE_DELETE_PTR(pShaderFile);
			}
			else
			{
				printf("Failed to open file '%s'\n", shaderName.Chars());
			}

			shaderIndex++;
		}

		// Process all the pixel shaders

		for(S32 i=0; i< shaderRegistry.GetPixelShaderList().Size(); i++)
		{
			shaderName.Printf("%s\\%s.p3p",
				searchPath.Chars(),
				shaderRegistry.GetStringTable()[ shaderRegistry.GetPixelShaderList()[i].m_NameId ]);

			Sys::I_File* pShaderFile = Sys::C_FileSystem::Instance().Open(shaderName, Sys::E_FileAccess_Read);
			if(pShaderFile)
			{	
				processShader(pShaderFile, pOutputFile, shaderIndex, pShaderOffsets, pShaderSizes);
				SAFE_DELETE_PTR(pShaderFile);
			}
			else
			{
				printf("Failed to open file '%s'\n", shaderName.Chars());
			}

			shaderIndex++;
		}

		// Fill in shader offsets

		pOutputFile->SetPosition(g_WhereShaderOffsetsAreInFile);
		for(S32 i=0; i<shaderCount; i++)
			outputFile << pShaderOffsets[i];

		// Fill in shader sizes

		pOutputFile->SetPosition(g_WhereShaderSizesAreInFile);
		for(S32 i=0; i<shaderCount; i++)
			outputFile << pShaderSizes[i];

		// Fill in buffer size

		pOutputFile->SetPosition(g_WhereBufferSizeIsInFile);
		outputFile << g_BufferSize;

		// Clean up
		
		SAFE_DELETE_ARY(pShaderOffsets);
		SAFE_DELETE_ARY(pShaderSizes);
		SAFE_DELETE_PTR(pOutputFile);
		SAFE_DELETE_PTR(pInputFile);

		printf("File size %i\n", g_BufferSize);
	}
	else
	{
		printf("Failed to open file '%s'\n", inputFileName.Chars());
		errorCode = 1;
	}

	printf("\nPress any key to continue\n");
	getchar();

	return errorCode;
}

#else // USE_OLD_SHADER_COLLECTOR

typedef struct CellCgbHeader
{
	unsigned int fourcc;
	unsigned short format_version;
	unsigned short compiler_version;
	unsigned short ucode_size;
	unsigned char profile;
	unsigned char content;
} CellCgbHeader;

typedef struct CellCgbVertexProgramConfiguration
{
#if 1
	U8				data[20];
#else
	unsigned short	instructionSlot;
	unsigned short	instructionCount;
	unsigned short	attributeInputMask;
	unsigned char	registerCount;
#endif
}CellCgbVertexProgramConfiguration;

typedef struct CellCgbFragmentProgramConfiguration
{
#if 1
	U8				data[20];
#else
	unsigned int	offset;
	unsigned int	attributeInputMask;
	unsigned short	texCoordsInputMask;
	unsigned short	texCoords2D;
	unsigned short	texCoordsCentroid;
	unsigned int	fragmentControl;
	unsigned char	registerCount;
#endif
}CellCgbFragmentProgramConfiguration;

static S32 totalPaddingWaste = 0;

static Sys::C_StreamPos 	g_HeaderOffsetsInFile		= 0;
static Sys::C_StreamPos 	g_HeaderSizesInFile			= 0;
static Sys::C_StreamPos		g_HeaderDataSizeInFile		= 0;
static S32					g_HeaderDataSize			= 0;
static Sys::C_StreamPos		g_WhereTheHeaderDataStarts	= 0;

static Sys::C_StreamPos 	g_CodeOffsetsInFile			= 0;
static Sys::C_StreamPos 	g_CodeSizesInFile			= 0;
static Sys::C_StreamPos 	g_CodeDataSizeInFile		= 0;
static S32					g_CodeDataSize				= 0;
static Sys::C_StreamPos		g_WhereTheCodeDataStarts	= 0;

void outputData(Sys::BinarySerialiser<Ut::NativeToBigEndian>& outputFile, S32 dataCount, S32* pData)
{
	for(S32 i=0; i<dataCount; i++)
		outputFile << pData[i];
}

S32 processShader(Sys::I_File* pShaderFile, Sys::I_File* pHeaderFile, Sys::I_File* pCodeFile, const Util::C_String& shaderName)
{
#if 0
	Sys::BinarySerialiser<Ut::NativeToBigEndian> headerFile(*pHeaderFile);
	Sys::BinarySerialiser<Ut::NativeToBigEndian> codeFile(*pCodeFile);
	
	Sys::BinaryDeserialiser<Ut::BigEndianToNative> shaderFile(*pShaderFile);

	S32 fileSize = (S32)pShaderFile->Length();
	U8* pFileData = new U8[fileSize];
	pShaderFile->Read(pFileData, fileSize);

	for(S32 i=0; i<fileSize; i++)
		headerFile << pFileData[i];

	U8* pTemp = pFileData;

	CellCgbHeader* pHeader = (CellCgbHeader*)pTemp;

	S32 codeSize = ((pHeader->ucode_size & 0xff00) >> 8) | ((pHeader->ucode_size & 0x00ff) << 8);
	
	pTemp += 12; // Header.
	pTemp += 20; // Hardware configuration.

	U8* pCodeData = pTemp;

	for(S32 i=0; i<codeSize; i++)
		codeFile << pCodeData[i];

	S32 alignSize = codeSize + 127;
	alignSize &= ~127;
	S32 paddingSize = alignSize-codeSize;
	char* pPadingData = new char[paddingSize];
	memset(pPadingData, 0x00, paddingSize);
	for(S32 i=0; i<paddingSize; i++)
		codeFile << pPadingData[i];
	SAFE_DELETE_ARY(pPadingData);

	SAFE_DELETE_ARY(pFileData);
#endif

	return 0;
}

S32 processShader( Sys::I_File* pShaderFile, Sys::I_File* pHeaderFile, Sys::I_File* pCodeFile )
{
	Sys::BinarySerialiser<Ut::NativeToBigEndian> headerFile(*pHeaderFile);
	Sys::BinarySerialiser<Ut::NativeToBigEndian> codeFile(*pCodeFile);

	CellCgbHeader header;
	CellCgbVertexProgramConfiguration vertexConfig;
	//CellCgbFragmentProgramConfiguration fragmentConfig;
	char* uCode;
	char* restOfData;
	S32 fileSize;
	
	fileSize = (S32)pShaderFile->Length();

	// read header
	pShaderFile->Read( &header, sizeof(header) );
	S32 ucodeSize = ((header.ucode_size & 0xff00) >> 8) | ((header.ucode_size & 0x00ff) << 8);
	// Skip padding
	pShaderFile->Read( &vertexConfig, sizeof(vertexConfig) );
	// read ucode
	uCode = new char [ ucodeSize ];
	pShaderFile->Read( uCode, ucodeSize );
	// Read The Rest of the file
	S32 restOfDataSize = fileSize - (S32)pShaderFile->GetPosition();
	restOfData = new char [ restOfDataSize ];
	pShaderFile->Read( restOfData, restOfDataSize );

	// Write Out Header Info
	header.ucode_size = 0;

	headerFile << header.fourcc;
	headerFile << header.format_version;
	headerFile << header.compiler_version;
	headerFile << header.ucode_size;
	headerFile << header.profile;
	headerFile << header.content;

	for(U32 i=0; i<20; i++)
		headerFile << vertexConfig.data[i];

	for(S32 i=0; i<restOfDataSize; i++)
		headerFile << restOfData[i];

	// Write out Ucode
	for(S32 i=0; i<ucodeSize; i++)
		codeFile << uCode[i];

	// Align to 128
	S32 alignSize = ucodeSize + 127;
	alignSize &= ~127;
	S32 paddingSize = alignSize - ucodeSize;
	totalPaddingWaste += paddingSize;
	char *padding = new char[ paddingSize ];
	memset(padding, 0x00, paddingSize );

	for(S32 i=0; i<paddingSize; i++)
		codeFile << padding[i];

	// Free Up Heap Resources
	delete [] padding;
	delete [] restOfData;
	delete [] uCode;

	return 0;
}

S32 _tmain(S32 argc, _TCHAR* argv[])
{
	// Params
	// 1.Input PC Shader Registry
	// 2.Output 'filename'
	//	(a) Will Write out 'filename'.hdr = header info
	//	(b) And 'filename'.bin  = ucode
	// 3.Alignment (default = 128)
	// Purpose to Separate Header from Microcode in Shaders, align microcode data to [Alignment]

	// Open the shader registry

	Util::C_String shaderRegistryFileName = "Z:\\shitstorm\\Working\\PC\\Data\\Image\\Shaders\\World\\Speed\\shaderRegistry.pcb";
	Sys::I_File* pFile = Sys::C_FileSystem::Instance().Open(shaderRegistryFileName.Chars(), Sys::E_FileAccess_Read);
	if(pFile)
	{
		Sys::I_File* pHeaderFile = Sys::C_FileSystem::Instance().Open( "c:\\temp\\header.bin", Sys::E_FileAccess_Write );
		Sys::I_File* pCodeFile = Sys::C_FileSystem::Instance().Open( "c:\\temp\\code.bin", Sys::E_FileAccess_Write );

		Sys::BinarySerialiser<Ut::NativeToBigEndian> headerFile(*pHeaderFile);
		Sys::BinarySerialiser<Ut::NativeToBigEndian> codeFile(*pCodeFile);

		// Load the shader registry

		ShaderRegistry::C_Registry shaderRegistry;
		shaderRegistry.Load( *pFile );

		// This is how many shaders we are processing

		S32 shaderCount = shaderRegistry.GetVertexShaderList().Size()+shaderRegistry.GetPixelShaderList().Size();
		
		// Reserve space for header offsets and sizes
		
		outputData(headerFile, 1, &shaderCount);
		
		g_HeaderOffsetsInFile = pHeaderFile->GetPosition();
		S32* pHeaderOffsets = new S32[shaderCount];
		outputData(headerFile, shaderCount, pHeaderOffsets);

		g_HeaderSizesInFile = pHeaderFile->GetPosition();
		S32* pHeaderSizes = new S32[shaderCount];
		outputData(headerFile, shaderCount, pHeaderSizes);

		g_HeaderDataSizeInFile = pHeaderFile->GetPosition();
		outputData(headerFile, 1, &g_HeaderDataSize);

		g_WhereTheHeaderDataStarts = pHeaderFile->GetPosition();

		// Reserve space for code offsets and sizes
		
		outputData(codeFile, 1, &shaderCount);
		
		g_CodeOffsetsInFile = pCodeFile->GetPosition();
		S32* pCodeOffsets = new S32[shaderCount];
		outputData(codeFile, shaderCount, pCodeOffsets);

		g_CodeSizesInFile = pCodeFile->GetPosition();
		S32* pCodeSizes = new S32[shaderCount];
		outputData(codeFile, shaderCount, pCodeSizes);

		g_CodeDataSizeInFile = pCodeFile->GetPosition();
		outputData(codeFile, 1, &g_CodeDataSize);

		g_WhereTheCodeDataStarts = pCodeFile->GetPosition();
		
		// Process all the shaders
		
		Util::C_String searchPath;
		searchPath.Printf("%s","Z:\\shitstorm\\Working\\PS3\\Data\\Image\\Shaders\\World\\Debug" );

		Util::C_String shaderName;

		S32 shaderIndex = 0;
		
		// Process all the vertex shaders

		for(S32 i=0; i<shaderRegistry.GetVertexShaderList().Size(); i++)
		{
			pHeaderOffsets[shaderIndex] = (S32)pHeaderFile->GetPosition()-(S32)g_WhereTheHeaderDataStarts;
			pCodeOffsets[shaderIndex] = (S32)pCodeFile->GetPosition()-(S32)g_WhereTheCodeDataStarts;

			assert((U32)pCodeOffsets[shaderIndex] == ((U32)(pCodeOffsets[shaderIndex]+127) & ~127));

			if(shaderIndex > 0)
			{
				pHeaderSizes[shaderIndex-1] = pHeaderOffsets[shaderIndex]-pHeaderOffsets[shaderIndex-1];
				pCodeSizes[shaderIndex-1] = pCodeOffsets[shaderIndex]-pCodeOffsets[shaderIndex-1];
			}
			
			shaderName.Printf("%s\\%s.p3v",
				searchPath.Chars(),
				shaderRegistry.GetStringTable()[ shaderRegistry.GetVertexShaderList()[i].m_NameId ]);

			Sys::I_File* pInputFile = Sys::C_FileSystem::Instance().Open(shaderName, Sys::E_FileAccess_Read);
			if(pInputFile)
			{	
				processShader(pInputFile, pHeaderFile, pCodeFile);
				SAFE_DELETE_PTR(pInputFile);
			}
			else
			{
				printf("Failed to open file '%s'\n", shaderName.Chars());
			}

			shaderIndex++;
		}

		if(shaderIndex > 1)
		{
			pHeaderSizes[shaderIndex-1] = (S32)pHeaderFile->GetPosition()-pHeaderOffsets[shaderIndex-1];
			pCodeSizes[shaderIndex-1] = (S32)pCodeFile->GetPosition()-pCodeOffsets[shaderIndex-1];
		}

		{
			U32 oldOffset = (S32)pCodeFile->GetPosition()-(S32)g_WhereTheCodeDataStarts;
			U32 newOffset = (oldOffset+127) & ~127;
			U32 offset = newOffset+oldOffset;
			for(U32 i=0; i<offset; i++)
			{
				U8 padding = 0;
				codeFile << padding;
			}
		}
		
		// Process all the pixel shaders

		for(S32 i=0; i< shaderRegistry.GetPixelShaderList().Size(); i++)
		{
			pHeaderOffsets[shaderIndex] = (S32)pHeaderFile->GetPosition()-(S32)g_WhereTheHeaderDataStarts;
			pCodeOffsets[shaderIndex] = (S32)pCodeFile->GetPosition()-(S32)g_WhereTheCodeDataStarts;

			assert((U32)pCodeOffsets[shaderIndex] == ((U32)(pCodeOffsets[shaderIndex]+127) & ~127));

			if(shaderIndex > 0)
			{
				pHeaderSizes[shaderIndex-1] = pHeaderOffsets[shaderIndex]-pHeaderOffsets[shaderIndex-1];
				pCodeSizes[shaderIndex-1] = pCodeOffsets[shaderIndex]-pCodeOffsets[shaderIndex-1];
			}
			
			shaderName.Printf("%s\\%s.p3p",
				searchPath.Chars(),
				shaderRegistry.GetStringTable()[ shaderRegistry.GetPixelShaderList()[i].m_NameId ]);

			Sys::I_File* pInputFile = Sys::C_FileSystem::Instance().Open(shaderName, Sys::E_FileAccess_Read);
			if(pInputFile)
			{	
				processShader(pInputFile, pHeaderFile, pCodeFile);
				SAFE_DELETE_PTR(pInputFile);
			}
			else
			{
				printf("Failed to open file '%s'\n", shaderName.Chars());
			}

			shaderIndex++;
		}

		if(shaderIndex > 1)
		{
			pHeaderSizes[shaderIndex-1] = (S32)pHeaderFile->GetPosition()-pHeaderOffsets[shaderIndex-1];
			pCodeSizes[shaderIndex-1] = (S32)pCodeFile->GetPosition()-pCodeOffsets[shaderIndex-1];
		}

		// Calculate the header size

		{
			Sys::C_StreamPos posA = pHeaderFile->GetPosition();
			pHeaderFile->SetPosition(g_HeaderSizesInFile);
			Sys::C_StreamPos posB = pHeaderFile->GetPosition();
			g_HeaderDataSize = ((S32)(posA-posB))-sizeof(S32);
		}

		// Calculate the code size

		{
			Sys::C_StreamPos posA = pCodeFile->GetPosition();
			pCodeFile->SetPosition(g_CodeSizesInFile);
			Sys::C_StreamPos posB = pCodeFile->GetPosition();
			g_CodeDataSize = ((S32)(posA-posB))-sizeof(S32);
		}

		// File in header offsets and sizes

		pHeaderFile->SetPosition(g_HeaderOffsetsInFile);
		outputData(headerFile, shaderCount, pHeaderOffsets);

		pHeaderFile->SetPosition(g_HeaderSizesInFile);
		outputData(headerFile, shaderCount, pHeaderSizes);
		
		pHeaderFile->SetPosition(g_HeaderDataSizeInFile);
		outputData(headerFile, 1, &g_HeaderDataSize);

		// Fill in code offsets and sizes

		pCodeFile->SetPosition(g_CodeOffsetsInFile);
		outputData(codeFile, shaderCount, pCodeOffsets);

		pCodeFile->SetPosition(g_CodeSizesInFile);
		outputData(codeFile, shaderCount, pCodeSizes);

		pCodeFile->SetPosition(g_CodeDataSizeInFile);
		outputData(codeFile, 1, &g_CodeDataSize);

		SAFE_DELETE_ARY(pHeaderOffsets);
		SAFE_DELETE_ARY(pHeaderSizes);
		SAFE_DELETE_ARY(pCodeOffsets);
		SAFE_DELETE_ARY(pCodeSizes);
		SAFE_DELETE_PTR(pHeaderFile);
		SAFE_DELETE_PTR(pCodeFile);
		SAFE_DELETE_PTR(pFile);

		printf("totalPaddingWaste %d\n",totalPaddingWaste);
	}
	else
	{
		printf("Failed to open file '%s'\n", shaderRegistryFileName.Chars());
	}

	//printf("\nPress any key to continue\n");
	//getchar();

	return 0;
}

#endif // USE_OLD_SHADER_COLLECTOR
