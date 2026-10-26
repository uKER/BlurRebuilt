// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#define BD_ARG_NOT_USED BD_NULL

inline void* bdCallocate(size_t nmemb, size_t size)
{
	bdUWord bytes = static_cast<bdUWord>(nmemb * size);
	void *mem = bdMemory::allocate(bytes);
	bdMemset(mem,0,bytes);

	return mem;
}

inline char* bdStrdup(const char* string)
{
	char *memory;
	size_t size = 0;

	if (!string)
		return(NULL);

	size = bdStrlen(string) + 1;
	memory = static_cast<char*>(bdMemory::allocate(size));
	if (memory)
	{
		bdStrlcpy(memory, string, size);
		return memory;
	}

	return(NULL);
}

inline bdHTTPWrapper::bdStatus bdHTTPWrapper::startUpload(const bdNChar8 *const uploadURL, const void *const fileData, bdUInt32 uploadSize, bdUInt64 transactionID)
{
	return _startUpload(uploadURL, fileData, BD_ARG_NOT_USED, uploadSize, transactionID);
}

inline bdHTTPWrapper::bdStatus bdHTTPWrapper::startUpload(const bdNChar8 *const uploadURL, bdUploadInterceptor* uploadHandler, bdUInt32 uploadSize, bdUInt64 transactionID)
{
	return _startUpload(uploadURL, BD_ARG_NOT_USED, uploadHandler, uploadSize, transactionID);
}

inline bdHTTPWrapper::bdStatus bdHTTPWrapper::startDownload(const bdNChar8 *const downloadURL, void *const downloadBuffer, bdUInt32 bufferSize, bdUInt64 transactionID, bdUInt32 startByte, bdUInt32 endByte)
{
	return _startDownload(downloadURL, downloadBuffer, bufferSize, BD_ARG_NOT_USED, transactionID, startByte, endByte);
}

inline bdHTTPWrapper::bdStatus bdHTTPWrapper::startDownload(const bdNChar8 *const downloadURL, bdDownloadInterceptor* downloadHandler, bdUInt64 transactionID, bdUInt32 startByte, bdUInt32 endByte)
{
	return _startDownload(downloadURL, BD_ARG_NOT_USED, BD_ARG_NOT_USED, downloadHandler, transactionID, startByte, endByte);
}




