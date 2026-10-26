#include "stdafx.h"

//#include "XmaFile.h"
#include "riff.h"

namespace Audio
{
//--------------------------------------------------------------------------------------
// FourCC definitions
//--------------------------------------------------------------------------------------
const DWORD AUDIO_FOURCC_RIFF   = 'FFIR';	//"RIFF"
const DWORD AUDIO_FOURCC_WAVE   = 'EVAW'; //"WAVE"
const DWORD AUDIO_FOURCC_FORMAT = ' tmf'; //"fmt "
const DWORD AUDIO_FOURCC_DATA   = 'atad'; //"data"
const DWORD AUDIO_FOURCC_WSMP   = 'pmsw'; //"wsmp"
const DWORD AUDIO_FOURCC_SMPL   = 'pmsl'; //"lsmp"
const DWORD AUDIO_FOURCC_SEEK   = 'kees'; //"seek"

// overriden mem functions
RiffChunk::AllocFuncPtr RiffChunk::AllocPtr = NULL;
RiffChunk::FreeFuncPtr RiffChunk::FreePtr = NULL;
	
//--------------------------------------------------------------------------------------
// Name: Alloc()
// Desc: 
//--------------------------------------------------------------------------------------
void* RiffChunk::Alloc(size_t size)
{ 
	if (m_bOverrideMemFuncs && AllocPtr) 
	{
		void* ptr = AllocPtr(size); 
		//Audio::Print(0, "\noveridden malloc(%d) returned %x\n", size, (U32)ptr);
		return ptr;
	}

	void* ptr=malloc(size);
	//Audio::Print(0, "\n%x = malloc(%d)\n", ptr, size);
	return ptr;
}

//--------------------------------------------------------------------------------------
// Name: Free()
// Desc: 
//--------------------------------------------------------------------------------------
void RiffChunk::Free(void* ptr) 
{ 
	if (m_bOverrideMemFuncs && FreePtr) 
	{
		//Audio::Print(0, "\noveridden free(%x)\n", (U32)ptr);
		return FreePtr(ptr); 
	}
	
	//Audio::Print(0, "\nfree(%x)\n", (U32)ptr);
	free(ptr); 
}

//--------------------------------------------------------------------------------------
// Name: RiffChunk()
// Desc: Constructor
//--------------------------------------------------------------------------------------
RiffChunk::RiffChunk()
{
    // Initialize defaults
    m_fccChunkId   		= 0;
    m_pParentChunk 		= NULL;
    m_hFile        		= INVALID_HANDLE_VALUE;
    m_dwDataOffset 		= 0;
    m_dwDataSize   		= 0;
    m_dwFlags      		= 0;
	m_pData		   		= 0;
	m_bOverrideMemFuncs	= false;
}

//--------------------------------------------------------------------------------------
// Name: RiffChunk()
// Desc: Destructor
//--------------------------------------------------------------------------------------
RiffChunk::~RiffChunk()
{
	if (m_pData)
	{
		Free(m_pData);
		m_pData = NULL;
	}
}

//--------------------------------------------------------------------------------------
// Name: Initialize()
// Desc: Initializes the Riff chunk for use
//--------------------------------------------------------------------------------------
VOID RiffChunk::Initialize( FOURCC fccChunkId, RiffChunk* pParentChunk, 
                             HANDLE hFile )
{
    m_fccChunkId   = fccChunkId;
    m_pParentChunk = pParentChunk;
    m_hFile        = hFile;
}


//--------------------------------------------------------------------------------------
// Name: Open()
// Desc: Opens an existing chunk
//--------------------------------------------------------------------------------------
HRESULT RiffChunk::Open()
{
    LONG lOffset = 0;

    // Seek to the first byte of the parent chunk's data section
    if( m_pParentChunk )
    {
        lOffset = m_pParentChunk->m_dwDataOffset;

        // Special case the RIFF chunk
        if( AUDIO_FOURCC_RIFF == m_pParentChunk->m_fccChunkId )
            lOffset += sizeof(FOURCC);
    }
    
    // Read each child chunk header until we find the one we're looking for
    for( ;; )
    {
        if( INVALID_SET_FILE_POINTER == SetFilePointer( m_hFile, lOffset, NULL, FILE_BEGIN ) )
            return HRESULT_FROM_WIN32( GetLastError() );

        RIFFHEADER rhRiffHeader;
        DWORD dwRead;
        if( 0 == ReadFile( m_hFile, &rhRiffHeader, sizeof(rhRiffHeader), &dwRead, NULL ) )
            return HRESULT_FROM_WIN32( GetLastError() );
        //rhRiffHeader.dwDataSize = __loadwordbytereverse( 0, &rhRiffHeader.dwDataSize);

        // Hit EOF without finding it
        if( 0 == dwRead )
            return E_FAIL;

        // Check if we found the one we're looking for
        if( m_fccChunkId == rhRiffHeader.fccChunkId )
        {
            // Save the chunk size and data offset
            m_dwDataOffset = lOffset + sizeof(rhRiffHeader);
            m_dwDataSize   = rhRiffHeader.dwDataSize;

            // Success
            m_dwFlags |= RIFFCHUNK_FLAGS_VALID;

            return S_OK;
        }

        lOffset += sizeof(rhRiffHeader) + rhRiffHeader.dwDataSize;
    }
}

//--------------------------------------------------------------------------------------
// Name: ReadData()
// Desc: Reads from the file into internal pointer
//--------------------------------------------------------------------------------------
HRESULT RiffChunk::ReadData()
{
    // Seek to the offset
    DWORD dwPosition = SetFilePointer( m_hFile, m_dwDataOffset, NULL, FILE_BEGIN );
    if( INVALID_SET_FILE_POINTER == dwPosition )
        return HRESULT_FROM_WIN32( GetLastError() );

	assert(m_pData==NULL);

	m_pData = Alloc(m_dwDataSize);

	if (m_pData==NULL)
	{
		Audio::Print(0, "RiffChunk::ReadData() : Could not allocate memory for data");
		return S_FALSE;
	}

    // Read from the file
    DWORD dwRead;
    if( 0 == ReadFile( m_hFile, m_pData, m_dwDataSize, &dwRead, NULL ) )
        return HRESULT_FROM_WIN32( GetLastError() );

    return S_OK;
}

//--------------------------------------------------------------------------------------
// Name: ReadData()
// Desc: Reads from the file into a given pointer
//--------------------------------------------------------------------------------------
HRESULT RiffChunk::ReadData( LONG lOffset, VOID* pData, DWORD dwDataSize ) const
{
    // Seek to the offset
    DWORD dwPosition = SetFilePointer( m_hFile, m_dwDataOffset+lOffset, NULL, FILE_BEGIN );
    if( INVALID_SET_FILE_POINTER == dwPosition )
        return HRESULT_FROM_WIN32( GetLastError() );

    // Read from the file
    DWORD dwRead;
    if( 0 == ReadFile( m_hFile, pData, dwDataSize, &dwRead, NULL ) )
        return HRESULT_FROM_WIN32( GetLastError() );

    return S_OK;
}

//--------------------------------------------------------------------------------------
// Name: SetData()
// Desc: Sets the data for this chunk
// Note: memory passed in to be set directly (ie bCopyData == false) must have been 
// allocated with the RiffChunk::Alloc function.
//--------------------------------------------------------------------------------------
HRESULT RiffChunk::FreeDataAndSetNew( VOID* pData, DWORD dwDataSize, bool bCopyData )
{
	if (m_pData)
	{
		Free(m_pData);
		m_pData = NULL;
	}

	VOID* pNewData = 0;
	if (bCopyData)
	{
		pNewData = (VOID*)Alloc(dwDataSize);
		memcpy( pNewData, pData, dwDataSize );
	}
	else
	{
		pNewData = pData;
	}

	SetData(pNewData, dwDataSize);

    return S_OK;
}

//--------------------------------------------------------------------------------------
// Name: SetData()
// Desc: Sets the data for this chunk
// Note: memory passed in must have been allocated with the RiffChunk::Alloc function
//--------------------------------------------------------------------------------------
HRESULT RiffChunk::SetData( VOID* pData, DWORD dwDataSize, bool bCopyData )
{
    DWORD dwParentDataSize = 0;
    
    // Get the current size of the parent chunk
    if( m_pParentChunk )
    {
        dwParentDataSize = m_pParentChunk->GetDataSize();
    }

	assert(m_pData==NULL);

    // Store the data
    if( pData )
    {
		m_pData = pData;
    }

    // Update the parent's chunk size
    if( m_pParentChunk )
    {
        m_pParentChunk->SetData( NULL, (dwParentDataSize + dwDataSize - m_dwDataSize) );
    }

    // Store the data size
    m_dwDataSize = dwDataSize;

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Name: Write()
// Desc: Writes the data for this chunk to it's file handle
//--------------------------------------------------------------------------------------
HRESULT RiffChunk::Write( BOOL bWriteChunkSize )
{
    if ( m_hFile == INVALID_HANDLE_VALUE )
        return E_FAIL;

	//if (m_pData == NULL)
	//	return E_FAIL;

    DWORD dwBytesWritten = 0;
    CHAR  strChunkName[4] = {0};
    DWORD dwChunkSize = m_dwDataSize;
	BOOL bOk;

    GetFourCCString( strChunkName );

    // Write the chunk name
	bOk = WriteFile( m_hFile, strChunkName, 4, &dwBytesWritten, NULL );	

	if (!bOk || !dwBytesWritten)
		return E_FAIL;

	if ( bWriteChunkSize )
	{
        // Special case for the RIFF chunk
        if( AUDIO_FOURCC_RIFF == m_fccChunkId )
        {
            // For the riff chunk we need to add the size of all it's child chunk
            // headers
            //dwChunkSize += 28;	// why?
        }

        // Write the chunk size
		bOk = WriteFile( m_hFile, &dwChunkSize, sizeof(DWORD), &dwBytesWritten, NULL );
		
		if (!bOk || !dwBytesWritten)
			return E_FAIL;
	}

    // Write the data for the chunk. not for the riff chunk as it has no data
	if( AUDIO_FOURCC_RIFF != m_fccChunkId )
	{
		bOk = WriteFile( m_hFile, m_pData, m_dwDataSize, &dwBytesWritten, NULL );

		if (!bOk || !dwBytesWritten)
			return E_FAIL;
	}
	
	// Special case for the RIFF chunk - write the "WAVE" chunk id
	if( AUDIO_FOURCC_RIFF == m_fccChunkId )
	{
		bOk = WriteFile( m_hFile, (void*)&AUDIO_FOURCC_WAVE, 4, &dwBytesWritten, NULL );
		if (!bOk || !dwBytesWritten)
			return E_FAIL;
	}

    return S_OK;
}

//--------------------------------------------------------------------------------------
// Name: GetFourCCString()
// Desc: Converts the FourCC code to an array of four chars
//--------------------------------------------------------------------------------------
VOID RiffChunk::GetFourCCString( CHAR* strFourCC )
{
    strFourCC[0] = (CHAR)(m_fccChunkId >> 0);
    strFourCC[1] = (CHAR)(m_fccChunkId >> 8);
    strFourCC[2] = (CHAR)(m_fccChunkId >> 16);
    strFourCC[3] = (CHAR)(m_fccChunkId >> 24);
}

} // namespace Audio
