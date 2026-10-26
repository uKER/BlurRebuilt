#pragma once

typedef unsigned int U32;

class cWavFileChunkHeader
{
	char	mId[4];
	int		mLength;
public:
	cWavFileChunkHeader( char *id );
	bool Read( FILE *fp );
	bool Write( FILE *fp );
	int GetLength();
	void SetLength( int length );
};

class cWavFileWavHeader
{
	cWavFileChunkHeader		mHeader;
	char					mRiff[4];
public:
	cWavFileWavHeader();
	bool Read( FILE *fp );
	bool Write( FILE *fp );
	void SetLength( int length ) { mHeader.SetLength( length ); }
};



class cWavFileFormatChunk
{
	cWavFileChunkHeader		mHeader;

public:
	short          			mFormatTag;
	unsigned short 			mChannels;
	unsigned long  			mSamplesPerSec;
	unsigned long  			mAvgBytesPerSec;
	unsigned short 			mBlockAlign;
	unsigned short 			mBitsPerSample;
	unsigned short			cbSize; // Loop = 1
public:
	cWavFileFormatChunk();
	bool Read( FILE *fp );
	bool Write( FILE *fp );
	DWORD GetNrChannels();
	DWORD GetSampleRate();
	DWORD GetBitsPerSample();
	DWORD GetSamplesPerSecond();

	void SetFormatTag( short tag ) { mFormatTag = tag; }
	void SetChannels( unsigned short channels ) { mChannels = channels; }
	void SetSamplesPerSecond( unsigned long samplesPerSecond ) { mSamplesPerSec = samplesPerSecond; }
	void SetAvgBytesPerSec( unsigned long avgBytesPerSecond ) { mAvgBytesPerSec = avgBytesPerSecond; }
	void SetBlockAlign( unsigned short blockAlign ) { mBlockAlign = blockAlign; }
	void SetBitsPerSample( unsigned short bitsPerSample ) { mBitsPerSample = bitsPerSample; }
	int GetStructureSize() { return sizeof(*this) - sizeof(mHeader); }

};

class cWavFileData
{
public:
	cWavFileChunkHeader	mDataHeader;
	unsigned char		*mData;
public:
	cWavFileData();
    ~cWavFileData();
	bool Read( FILE *fp );
	bool Write( FILE *fp );
	DWORD GetSampleLength();
	void SetSampleLength( DWORD sampleLength );
	int GetStructureSize() { return sizeof(mDataHeader); }
};

class cWavFileDesignation
{
	friend	class C_WAVFile;
public:
	cWavFileWavHeader	mHeader;
	cWavFileFormatChunk	mFormat;
	cWavFileData		mData;

public:
	cWavFileDesignation();
	bool Read( FILE *fp );
	bool Write ( FILE *fp );
};

class C_WAVFile
{
	char*		mName;
	char*		mDisplayName;
public:
	cWavFileDesignation		mWavFileDesignation;
	
public:
	C_WAVFile();
	~C_WAVFile();
	bool Load( char *filename );
	bool Save( char *filename );
	DWORD GetSampleLengthInFrames();
	DWORD GetSamplesPerSecond();
	DWORD GetSampleNrChannels();
	void *GetDataPtr();
	void SetName( char *name );
	char *GetName();
	char *GetDisplayName();

	void SetDataPtr( char *data );
	void SetSampleLengthInBytes( int nrSamples );
	int  GetSampleLengthInBytes();
	

	void SetFormatTag( short tag ); 
	void SetChannels( unsigned short channels );
	void SetSamplesPerSecond( unsigned long samplesPerSecond );
	void SetAvgBytesPerSec( unsigned long avgBytesPerSecond );
	void SetBlockAlign( unsigned short blockAlign );
	void SetBitsPerSample( unsigned short bitsPerSample );

};
