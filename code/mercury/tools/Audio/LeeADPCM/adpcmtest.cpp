// adpcmtest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "WAV_File.h"
#include "adpcm64.h"
#include "playback.h"

extern void test();

void encodeWav( char *inFileName, char *outFileName )
{
	C_WAVFile	*inFile;
	C_WAVFile	*outFile;

	inFile = new C_WAVFile();
	outFile = new C_WAVFile();

	inFile->Load( inFileName );
	
	unsigned char *outData = new unsigned char [ inFile->GetSampleNrChannels() * inFile->GetSampleLengthInFrames() ];

	#define READ_SIZE_IN_SAMPLES 8192
	short buffer[ READ_SIZE_IN_SAMPLES  ];
	short *pIn = (short *)inFile->GetDataPtr();
	unsigned char *pOut = (unsigned char *)outData;
	unsigned int *pHeader = (unsigned int *)pOut;
	pHeader[0] = 8; // Index of first sample data
	pHeader[1] = (inFile->GetSampleLengthInFrames()) * inFile->GetSampleNrChannels(); // Number of Samples
	pHeader[1] = (pHeader[1] + 1) & ~1;
	pOut = pOut + pHeader[0];
	int nrSamplesRemaining = inFile->GetSampleLengthInFrames() * inFile->GetSampleNrChannels();
	int nrSamplesBatchAlign = (nrSamplesRemaining + 63) & ~63;
	int nrWritten = 0;
	int totalWritten = 0;
	adpcm64 adpcm;

	adpcm._StartPack( inFile->GetSampleLengthInFrames(), inFile->GetSampleNrChannels() );
	
	while( nrSamplesRemaining > 0)
	{

		int readSize = 0;
		if (nrSamplesRemaining > READ_SIZE_IN_SAMPLES)
		{
			readSize = READ_SIZE_IN_SAMPLES;
			memcpy( buffer, pIn, readSize * sizeof(short));
		}
		else
		{ 
			memset( buffer, 0x00, sizeof(buffer));
			memcpy( buffer , pIn , nrSamplesRemaining * sizeof(short) );
			readSize = nrSamplesRemaining;
			if (inFile->GetSampleNrChannels() == 1)
			{
				readSize = (readSize + 63) &~ 63; 
			}
			else
			{
				readSize = (readSize + 127) &~ 127; 
			}
		}
		nrWritten = adpcm._PackWrite( buffer, readSize, pOut, READ_SIZE_IN_SAMPLES );
		totalWritten += nrWritten;
		pIn = pIn + readSize;
		pOut = pOut + nrWritten;
		nrSamplesRemaining -= readSize;
		nrSamplesBatchAlign -= readSize;
	}
	
	adpcm._PackClose();

	outFile->SetDataPtr( (char *)outData );
	outFile->SetChannels( adpcm.GetNrChannels() );
	outFile->SetSampleLengthInBytes( totalWritten );
	outFile->SetFormatTag( 0xaaab ); 
	outFile->SetSamplesPerSecond( inFile->GetSamplesPerSecond() );
	outFile->SetAvgBytesPerSec( inFile->GetSamplesPerSecond() * adpcm.GetNrChannels() * 8 / 4 );
	outFile->SetBlockAlign( 1 );
	outFile->SetBitsPerSample( 4 );

	outFile->Save( outFileName );

}

void decodeWav( char *inFileName, char *outFileName )
{
	C_WAVFile	*inFile;
	C_WAVFile	*outFile;

	inFile = new C_WAVFile();
	outFile = new C_WAVFile();

	inFile->Load( inFileName );

	short *outData = new short[ inFile->GetSampleNrChannels() * inFile->GetSampleLengthInFrames() ];
	#define READ_SIZE_IN_SAMPLES 8192
	unsigned char buffer[ READ_SIZE_IN_SAMPLES  ];
	unsigned char *pIn = (unsigned char *)inFile->GetDataPtr();
	short *pOut = (short *)outData;
	unsigned int *pHeader = (unsigned int *)pIn;
	printf("Header says We Have %d bytes\n",pHeader[0]);
	printf("Sample Length is %d bytes\n",pHeader[1]);
	pIn = pIn + pHeader[0];

	int nrBytesRemaining = inFile->GetSampleLengthInBytes();
	int nrWritten = 0;
	int totalWritten = 0;
	adpcm64 adpcm;

	adpcm._StartPack( inFile->GetSampleLengthInFrames(), inFile->GetSampleNrChannels() );
	
	while( nrBytesRemaining > 0 )
	{

		int readSize = 0;
		if (nrBytesRemaining > READ_SIZE_IN_SAMPLES)
		{
			readSize = READ_SIZE_IN_SAMPLES;
		}
		else
		{ 
			readSize = nrBytesRemaining;
			memset(buffer,0x00,sizeof(buffer));
		}
		memcpy( buffer, pIn, readSize );
		nrWritten = adpcm._PackRead( buffer, readSize, pOut, READ_SIZE_IN_SAMPLES  );
		totalWritten += nrWritten << 1;
		pIn = pIn + readSize;
		pOut = pOut + (nrWritten);
		nrBytesRemaining -= readSize; 
	}
	adpcm._PackClose();

	outFile->SetDataPtr( (char *)outData );
	outFile->SetChannels( adpcm.GetNrChannels() );
	outFile->SetSampleLengthInBytes( pHeader[1] << 1 );
	outFile->SetFormatTag( 1 ); 
	outFile->SetSamplesPerSecond( inFile->GetSamplesPerSecond() );
	outFile->SetAvgBytesPerSec( inFile->GetSamplesPerSecond() * adpcm.GetNrChannels() * 2 );
	outFile->SetBlockAlign( 4 );
	outFile->SetBitsPerSample( 16 );

	outFile->Save( outFileName );

}

void ShowUsage( char *exeName )
{
	printf("Usage :\n");
	printf("%s e <inputfile> <outputfile>\n",exeName);
	printf("     Encodes WAV PCM 16 Bit to ADPCM\n");
	printf("%s d <inputfile> <outputfile>\n",exeName);
	printf("     Deodes ADPCM to WAV PCM 16 Bit\n");
	printf("%s p <inputfile>\n",exeName);
	printf("     Plays a File\n");
	exit(1);
}

int PlayBack( char *filename )
{
	LARGE_INTEGER perfStart;
	LARGE_INTEGER perfEnd;
	LARGE_INTEGER perfTotal;
	LARGE_INTEGER perfFreq;

	perfTotal.QuadPart = 0;
	QueryPerformanceFrequency( &perfFreq );

	C_WAVFile	*inFile;
	inFile = new C_WAVFile();
	inFile->Load( filename );
	if (inFile->mWavFileDesignation.mFormat.GetBitsPerSample() == 4)
	{
		#define READ_SIZE_IN_SAMPLES 8192
		unsigned char buffer[ READ_SIZE_IN_SAMPLES  ];
		unsigned char outbuffer[ READ_SIZE_IN_SAMPLES * 4 ];
		unsigned char *pIn = (unsigned char *)inFile->GetDataPtr();
		int nrBytesRemaining = inFile->GetSampleLengthInBytes();
		int nrWritten = 0;
		int totalWritten = 0;
		adpcm64 adpcm;

		adpcm._StartPack( inFile->GetSampleLengthInFrames(), inFile->GetSampleNrChannels() );
		InitStreamPlay( inFile->mWavFileDesignation.mFormat.GetSampleRate(), inFile->mWavFileDesignation.mFormat.GetNrChannels()  );
		while( nrBytesRemaining > 0 )
		{
			int readSize = 0;
			if (nrBytesRemaining > READ_SIZE_IN_SAMPLES)
			{
				readSize = READ_SIZE_IN_SAMPLES;
			}
			else
			{ 
				readSize = nrBytesRemaining;
				memset(buffer,0x00,sizeof(buffer));
			}
			memcpy( buffer, pIn, readSize );
			QueryPerformanceCounter( &perfStart );
			nrWritten = adpcm._PackRead( buffer, readSize, (short*)outbuffer, READ_SIZE_IN_SAMPLES  );
			QueryPerformanceCounter( &perfEnd );
			perfTotal.QuadPart += perfEnd.QuadPart - perfStart.QuadPart;
			PlayBlock( (char *)outbuffer, nrWritten << 1);
			totalWritten += nrWritten << 1;
			pIn = pIn + readSize;
			nrBytesRemaining -= readSize; 
			float sampleRate = inFile->GetSamplesPerSecond();
			float perfMs = ((float)perfTotal.QuadPart / (float)perfFreq.QuadPart) * 1000.0f;
			printf("%d bytes = %4.2f(ms) = %4.2f secs of %2.1fkhz 2ch (%4.4fX)\r",	totalWritten, 
																				perfMs, 
																				(float)totalWritten / (sampleRate * 2.0f * 2.0f), 
																				sampleRate / 1000.0f,
																				((float)totalWritten / (sampleRate * 2.0f * 2.0f)) / (perfMs / 1000.0f));



		}
		adpcm._PackClose();
		CloseStreamPlay();

	}
	else
	{
		#define READ_SIZE_IN_SAMPLES 8192
		unsigned char buffer[ READ_SIZE_IN_SAMPLES  ];
		unsigned char *pIn = (unsigned char *)inFile->GetDataPtr();
		int nrBytesRemaining = inFile->GetSampleLengthInBytes();
		int nrWritten = 0;
		int totalWritten = 0;
		InitStreamPlay( inFile->mWavFileDesignation.mFormat.GetSampleRate(), inFile->mWavFileDesignation.mFormat.GetNrChannels()  );
		while( nrBytesRemaining > 0 )
		{
			int readSize = 0;
			if (nrBytesRemaining > READ_SIZE_IN_SAMPLES)
			{
				readSize = READ_SIZE_IN_SAMPLES;
			}
			else
			{ 
				readSize = nrBytesRemaining;
				memset(buffer,0x00,sizeof(buffer));
			}
			PlayBlock( (char *)pIn, readSize );
			pIn += readSize;
			nrBytesRemaining -= readSize; 
		}
		CloseStreamPlay();
		
	}

	return 0;
}


extern int playBack( char *filename );
int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		ShowUsage(argv[0]);
	}
	switch ( argv[1][0] )
	{
		case 'e':
			encodeWav( argv[2], argv[3] );
			break;
		case 'd':
			decodeWav( argv[2], argv[3] );
			break;
		case 'p':
			if (argc != 3) ShowUsage(argv[2]);
			PlayBack( argv[2] );
			break;
		default:
			ShowUsage(argv[0]);
			break;
	}

//	encodeWav( "c:\\temp\\TEST.wav", "c:\\temp\\testCOMP.wav" );	
//	decodeWav( "c:\\temp\\testCOMP.wav", "c:\\temp\\testDEC.wav");

//	getchar();

//	test();
	return 0;
}

