#pragma once
#include <stdafx.h>
#include "WAV_File.h"

template<class T> bool ReadItem( FILE *fp, T* a )
{
	if (fread(a,1,sizeof(*a),fp) == sizeof(*a))
	{
		return true;
	}
	else return false;
}

template<class T> bool WriteItem( FILE *fp, T *a )
{
	if (fwrite(a,1,sizeof(*a),fp) == sizeof(*a)) return true;
	else return false;
}

template<class T> bool ReadBlock( FILE *fp, T a, DWORD byteCount )
{
	if (fread(a,1,byteCount,fp) == byteCount) return true;
	else return false;
}

template<class T> bool WriteBlock( FILE *fp, T a, DWORD byteCount )
{
	if (fwrite(a,1,byteCount,fp) == byteCount) return true;
	else return false;
}


cWavFileChunkHeader::cWavFileChunkHeader( char* id )
{
	memcpy(&mId,id,sizeof(mId));
	mLength = 0;
}
bool cWavFileChunkHeader::Read( FILE *fp )
{
        if ( !ReadItem(fp, &mId) ) return false;
	if ( !ReadItem(fp, &mLength) ) return false;
	return true;
}
bool cWavFileChunkHeader::Write( FILE *fp )
{
        if ( !WriteItem(fp, &mId) ) return false;
	if ( !WriteItem(fp, &mLength) ) return false;
	return true;
}

int cWavFileChunkHeader::GetLength()
{
        return mLength;
}

void cWavFileChunkHeader::SetLength( int length )
{
	mLength = length;
}


cWavFileWavHeader::cWavFileWavHeader() : mHeader( "RIFF" )
{
        memcpy(&mRiff,"WAVE",sizeof(mRiff));
}
bool cWavFileWavHeader::Read( FILE *fp )
{
        if ( !mHeader.Read( fp ) ) return false;
	if ( !ReadItem( fp, &mRiff ) ) return false;
	return true;
}
bool cWavFileWavHeader::Write( FILE *fp )
{
        if ( !mHeader.Write( fp ) ) return false;
	if ( !WriteItem( fp, &mRiff ) ) return false;
	return true;
}


cWavFileFormatChunk::cWavFileFormatChunk() : mHeader( "fmt " )
{
        mFormatTag      = 0xaaab;
        mChannels       = 0;
        mSamplesPerSec  = 0;
        mAvgBytesPerSec	= 0;
        mBlockAlign		= 0;
        mBitsPerSample	= 0;
		cbSize			= 2;
}

bool cWavFileFormatChunk::Read( FILE *fp )
{
	if ( !mHeader.Read( fp ) ) return false;
	if ( !ReadItem( fp, &mFormatTag		) ) return false;
	if ( !ReadItem( fp, &mChannels		) ) return false;
	if ( !ReadItem( fp, &mSamplesPerSec	) ) return false;
	if ( !ReadItem( fp, &mAvgBytesPerSec) ) return false;
	if ( !ReadItem( fp, &mBlockAlign	) ) return false;
	if ( !ReadItem( fp, &mBitsPerSample	) ) return false;
	if ( mFormatTag == (short)0xaaab )
	{
		short loop = 0;
		if ( !ReadItem( fp, &cbSize			) ) return false;
		if ( !ReadItem( fp, &loop			) ) return false;
		
	}
	
	return true;
}
bool cWavFileFormatChunk::Write( FILE *fp )
{
	mHeader.SetLength( GetStructureSize() );
	if ( !mHeader.Write( fp) ) return false;
	if ( !WriteItem( fp, &mFormatTag        ) ) return false;
	if ( !WriteItem( fp, &mChannels			) ) return false;
	if ( !WriteItem( fp, &mSamplesPerSec	) ) return false;
	if ( !WriteItem( fp, &mAvgBytesPerSec	) ) return false;
	if ( !WriteItem( fp, &mBlockAlign		) ) return false;
	if ( !WriteItem( fp, &mBitsPerSample	) ) return false;
	if ( !WriteItem( fp, &cbSize			) ) return false;
// Loop
	unsigned short loop = 0;
	if ( !WriteItem( fp, &loop ) ) return false;
	return true;
}

DWORD cWavFileFormatChunk::GetNrChannels()
{
        return mChannels;
}

DWORD cWavFileFormatChunk::GetSampleRate()
{
        return mSamplesPerSec;
}

DWORD cWavFileFormatChunk::GetBitsPerSample()
{
        return mBitsPerSample;
}

DWORD cWavFileFormatChunk::GetSamplesPerSecond()
{
        return mSamplesPerSec;
}


cWavFileData::cWavFileData() : mDataHeader( "data" )
{
	mData = NULL;
}
cWavFileData::~cWavFileData()
{
        delete [] mData;
}

bool cWavFileData::Read( FILE *fp )
{
	if ( !mDataHeader.Read( fp ) ) return false;
	mData = new unsigned char [ mDataHeader.GetLength() + ( sizeof(short) * 16 ) ];
	if ( !mData ) return false;
	int fdatapos = ftell(fp);
	if ( !ReadBlock( fp, mData, mDataHeader.GetLength() ) ) return false;
	int fdataendpos = ftell(fp);
	fseek(fp, fdatapos, SEEK_SET);
	if ( !ReadBlock( fp, mData + mDataHeader.GetLength(), sizeof(short) * 16 ) ) return false;
	fseek( fp, fdataendpos, SEEK_SET );
	return true;
}
bool cWavFileData::Write( FILE *fp )
{
	if ( !mDataHeader.Write( fp ) ) return false;
	if ( !WriteBlock( fp, mData, mDataHeader.GetLength() ) ) return false;
	return true;
}
DWORD cWavFileData::GetSampleLength()
{
        return mDataHeader.GetLength();
}

void cWavFileData::SetSampleLength( DWORD sampleLength )
{
	mDataHeader.SetLength( sampleLength );
}


cWavFileDesignation::cWavFileDesignation()
{
}

bool cWavFileDesignation::Read( FILE *fp )
{
	if ( !mHeader.Read( fp ) ) return false;
	if ( !mFormat.Read( fp ) ) return false;
	if ( !mData.Read( fp )   ) return false;
	return true;
}
bool cWavFileDesignation::Write( FILE *fp )
{
	mHeader.SetLength( mData.GetSampleLength() + sizeof( mHeader ) + sizeof( mFormat ) + mData.GetStructureSize() );
	if ( !mHeader.Write( fp ) ) return false;
	if ( !mFormat.Write( fp ) ) return false;
	if ( !mData.Write( fp )   ) return false;
	return true;
}






C_WAVFile::C_WAVFile()
{
}

C_WAVFile::~C_WAVFile()
{
	delete [] mName;
}

bool C_WAVFile::Load( char *filename )
{
	FILE *fp;
	fp = fopen( filename, "rb" );
	if (fp)
	{
		bool result = mWavFileDesignation.Read( fp );
		fclose(fp);

		char drive[_MAX_DRIVE];
		char dir[_MAX_DIR];
		char fname[_MAX_FNAME];
		char ext[_MAX_EXT];

		_splitpath( filename, drive, dir, fname, ext );
	
		SetName( filename );
		
		return result;
	}
	else
	{
		return false;
	}
}

bool C_WAVFile::Save( char *filename )
{
	FILE *fp;
	fp = fopen( filename, "wb" );
	if (fp)
	{
		bool result = mWavFileDesignation.Write( fp );
		fclose(fp);
		return result;
	}
	else
	{
		return false;
	}
}

DWORD C_WAVFile::GetSampleLengthInFrames()
{
	float sampleLength	= (float)mWavFileDesignation.mData.GetSampleLength();
	float bitsPerSample = (float)mWavFileDesignation.mFormat.GetBitsPerSample();
	float nrChannels	= (float)mWavFileDesignation.mFormat.GetNrChannels();
	if (bitsPerSample > 0)
	{
		return (DWORD)((sampleLength / (bitsPerSample / 8)) / nrChannels);
	}
	return 0;
}

DWORD C_WAVFile::GetSamplesPerSecond()
{
	return mWavFileDesignation.mFormat.GetSamplesPerSecond();
}

DWORD C_WAVFile::GetSampleNrChannels()
{
	return mWavFileDesignation.mFormat.GetNrChannels();
}

void *C_WAVFile::GetDataPtr()
{
	return mWavFileDesignation.mData.mData;
}

void C_WAVFile::SetName( char *name )
{
	int pos = (int)strlen(name)-1;
	mName = new char [strlen(name)+1];
	strcpy( mName, name );
	for (; pos > 0 ; pos--)
	{
		if ((name[pos] == '\\') || (name[pos] == '\0'))
		{
			pos = max(pos+1,0);
			break;
		}
	}
	mDisplayName = new char[ strlen(name)-pos + 1];
	strcpy(mDisplayName, &name[pos] );
}

char *C_WAVFile::GetName()
{ 
	return mName;
}

char *C_WAVFile::GetDisplayName()
{
	return mDisplayName;
}

void C_WAVFile::SetDataPtr( char *data )
{
	mWavFileDesignation.mData.mData = (unsigned char*)data;
}

void C_WAVFile::SetSampleLengthInBytes( int nrSamples )
{
	mWavFileDesignation.mData.SetSampleLength( nrSamples );
}

int C_WAVFile::GetSampleLengthInBytes()
{
	return mWavFileDesignation.mData.GetSampleLength();
}


void C_WAVFile::SetFormatTag( short tag ) 
{
	mWavFileDesignation.mFormat.SetFormatTag( tag );
}

void C_WAVFile::SetChannels( unsigned short channels ) 
{
	mWavFileDesignation.mFormat.SetChannels( channels );
}

void C_WAVFile::SetSamplesPerSecond( unsigned long samplesPerSecond )
{
	mWavFileDesignation.mFormat.SetSamplesPerSecond( samplesPerSecond );
}

void C_WAVFile::SetAvgBytesPerSec( unsigned long avgBytesPerSecond )
{
	mWavFileDesignation.mFormat.SetAvgBytesPerSec( avgBytesPerSecond );
}

void C_WAVFile::SetBlockAlign( unsigned short blockAlign )
{
	mWavFileDesignation.mFormat.SetBlockAlign( blockAlign );
}

void C_WAVFile::SetBitsPerSample( unsigned short bitsPerSample )
{
	mWavFileDesignation.mFormat.SetBitsPerSample( bitsPerSample );
}
