// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdLobby/bdContentStreaming/bdContentStreaming.h>
#include <bdLobby/bdContentStreaming/bdDownloadInterceptor.h>
#include <bdLobby/bdContentStreaming/bdUploadInterceptor.h>
#include <bdLobby/bdContentStreaming/bdFileID.h>
#include <bdLobby/bdContentStreaming/bdQuotaUsage.h>
#include <bdLobby/bdHTTPWrapper/bdHTTPWrapper.h>
#include <bdLobby/bdStorage/bdGetFilesResult.h>
#include <bdLobby/bdRemoteTaskManager/bdRemoteTaskManager.h>
#include <bdLobby/bdRemoteTaskManager/bdRemoteTask.h>
#include <bdLobby/bdStorage/bdStorage.h>
#include <bdLobby/bdTags/bdTag.h>
#include <bdCore/bdUtilities/bdBase64.h>

#define BD_MAX_ITUNES_RECEIPT_SIZE 4 * 1024
#define BD_MAX_CONTENT_SERVER_FILE_SIZE 1024 * 1024 * 1024

#define BD_OPERATION_HEADER_SIZE (27u)
// thumb data + blob type + blob length
// num tags + tag type data + uint32 tags
#define BD_OPERATION_BUFFER_SIZE (BD_OPERATION_HEADER_SIZE+BD_MAX_FILE_NAME_LENGTH  +  (BD_MAX_THUMB_DATA_SIZE+sizeof(bdUByte8)*2+sizeof(bdUInt32))  +  (sizeof(bdUByte8)+sizeof(bdUInt32))  +  (BD_MAX_NUM_TAGS*sizeof(bdUByte8))  +  (BD_MAX_NUM_TAGS*sizeof(bdUInt32)))

// MULTIPHASE operations
#define BD_STREAMING_UPLOAD_FILE_VIA_HTTP	(101u)
#define BD_STREAMING_DOWNLOAD_FILE_VIA_HTTP	(102u)
#define BD_STREAMING_REMOVE_FILE_VIA_HTTP	(103u)

// MMP operations
#define BD_CONTENTSTREAMING_GET_FILE_DESCRIPTOR_BY_ID			(1u)
#define BD_CONTENTSTREAMING_GET_FILE_DESCRIPTORS_BY_OWNER		(2u)
#define BD_CONTENTSTREAMING_GET_PUBLISHER_FILE_DESCRIPTORS		(3u)
#define BD_CONTENTSTREAMING_SEARCH_BY_TAGS						(4u)
#define BD_CONTENTSTREAMING_PRE_UPLOAD							(5u)
#define BD_CONTENTSTREAMING_POST_UPLOAD							(6u)
#define BD_CONTENTSTREAMING_PRE_DOWNLOAD_BY_SLOT				(7u)
#define BD_CONTENTSTREAMING_PRE_DELETE_FILE						(8u)
#define BD_CONTENTSTREAMING_PRE_DOWNLOAD_BY_FILE_ID				(9u)
#define BD_CONTENTSTREAMING_PRE_DOWNLOAD_PUBLISHER_FILE			(10u)
#define BD_CONTENT_STREAMING_GET_USER_USAGE						(11u)
#define BD_CONTENT_STREAMING_REPORT_CONTENT						(12u)
#define BD_CONTENT_STREAMING_PRE_DOWNLOAD_ITUNES_PURCHASED_FILE (13u)

#define BD_LOG_LEVEL ("lobby/contentStreaming")
#define TYPE_CHECKED true

#define BD_NOT_USED 0

bdContentStreaming::bdContentStreaming(	bdRemoteTaskManager *const remoteTaskManager)
: m_currentTaskID(0),
  m_state(READY),
  m_uploadData(0),
  m_uploadFileID(0),
  m_thumbData(0),
  m_thumbDataSize(0),
  m_downloadMetaData(0),
  m_downloadData(0),
  m_downloadDataSize(0),
  m_startByte(0),
  m_endByte(0),
  m_remoteTaskManager(remoteTaskManager)
{
#if (defined BD_PLATFORM_WIN32) || (defined BD_PLATFORM_UNIX)
	m_testAddressCount = 0;
	m_useTestAddresses = false;
#endif
}

bdContentStreaming::~bdContentStreaming()
{
}

#ifdef BD_PLATFORM_XENON
void bdContentStreaming::init(DWORD lspServiceID)
{
	m_http.init(lspServiceID, true);
}
#endif	

bdRemoteTaskRef bdContentStreaming::getThumbNailData(const bdUInt numFileIDs,
													 const bdEntityID fileIDs[],
													 bdGetFilesResult filesResult[])
{
	bdStorage storage(m_remoteTaskManager);
	bdRemoteTaskRef remoteTask = storage.getFilesByID(numFileIDs, fileIDs, filesResult);
	return remoteTask;
}

bdRemoteTaskRef bdContentStreaming::getFileMetaDataByID(const bdUInt numFileIDs,
														const bdUInt64 fileIDs[],
														bdFileMetaData fileDescriptor[])
{
	bdRemoteTaskRef remoteTask = BD_NULL;
	bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(numFileIDs) +  (sizeof(bdUInt64)*numFileIDs);

	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_STREAMING_SERVICE2, BD_CONTENTSTREAMING_GET_FILE_DESCRIPTOR_BY_ID);


	buffer.writeUInt32(numFileIDs);
	for(bdUInt i=0; i<numFileIDs; i++)
	{
		buffer.writeUInt64(fileIDs[i]);
	}
	
	m_remoteTaskManager->startTask(remoteTask, buffer);
	remoteTask->setTaskResult(fileDescriptor, numFileIDs);
	return remoteTask;
}

bdRemoteTaskRef bdContentStreaming::listFilesByOwner(const bdOnlineUserID ownerID,
													 const bdUInt32 startDate,
													 const bdUInt16 category,
													 bdFileMetaData fileDescriptors[],
													 const bdUInt16 maxNumResults,
													 const bdUInt16 offset,
													 const bdNChar8 *fileName)
{
	bdRemoteTaskRef remoteTask = BD_NULL;

	bdUInt taskSize = BD_TASK_OVERHEAD;
	taskSize += sizeof(startDate);
	taskSize += sizeof(category);
	taskSize += sizeof(maxNumResults);
	taskSize += sizeof(offset);
	if (fileName)
	{
		taskSize += (bdUInt)bdStrnlen(fileName, BD_MAX_FILE_NAME_LENGTH);
	}
	
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_STREAMING_SERVICE2, BD_CONTENTSTREAMING_GET_FILE_DESCRIPTORS_BY_OWNER);

	buffer.writeUInt64(ownerID);
	buffer.writeUInt32(startDate);
	buffer.writeUInt16(maxNumResults);
	buffer.writeUInt16(offset);
	buffer.writeUInt16(category);

	if (fileName)
	{
		buffer.writeString(fileName, bdStrnlen(fileName, BD_MAX_FILE_NAME_LENGTH)+1);
	}


	m_remoteTaskManager->startTask(remoteTask, buffer);
	remoteTask->setTaskResult(fileDescriptors, maxNumResults);
	return remoteTask;
}


bdRemoteTaskRef bdContentStreaming::listAllPublisherFiles(const bdUInt32 startDate,
														  const bdUInt16 category,
														  bdFileMetaData fileDescriptors[],
														  const bdUInt16 maxNumResults,
														  const bdUInt16 offset,
														  const bdNChar8 *fileName)
{
	bdRemoteTaskRef remoteTask = BD_NULL;

	bdUInt taskSize = BD_TASK_OVERHEAD;
	taskSize += sizeof(startDate);
	taskSize += sizeof(category);
	taskSize += sizeof(maxNumResults);
	taskSize += sizeof(offset);
	if (fileName)
	{
		taskSize += (bdUInt)bdStrnlen(fileName, BD_MAX_FILE_NAME_LENGTH);
	}

	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_STREAMING_SERVICE2, BD_CONTENTSTREAMING_GET_PUBLISHER_FILE_DESCRIPTORS);

	buffer.writeUInt32(startDate);
	buffer.writeUInt16(maxNumResults);
	buffer.writeUInt16(offset);
	buffer.writeUInt16(category);

	if (fileName)
	{
		buffer.writeString(fileName, bdStrnlen(fileName, BD_MAX_FILE_NAME_LENGTH)+1);
	}

	m_remoteTaskManager->startTask(remoteTask, buffer);
	remoteTask->setTaskResult(fileDescriptors, maxNumResults);
	return remoteTask;
}

void bdContentStreaming::pump()
{
	switch (m_currentTaskID)
	{
		case BD_STREAMING_UPLOAD_FILE_VIA_HTTP:
		{
			pumpUploadViaHTTP();
		}
		break;
		case BD_STREAMING_DOWNLOAD_FILE_VIA_HTTP:
		{
			pumpDownloadViaHTTP();
		}
		break;
		case BD_STREAMING_REMOVE_FILE_VIA_HTTP:
		{
			pumpDeleteViaHTTP();
		}
		break;
	}
}

void bdContentStreaming::initUpload(const bdUInt16 fileSlot,
									const void* fileData,
									const bdUInt32 fileSize,
									bdUploadInterceptor* uploadHandler,
									const bdNChar8 *const fileName,
									const bdUInt16 category,
									const void* thumbData,
									const bdUInt32 thumbDataSize,
									const bdUInt32 numTags,
									const bdTag tags[],
									bdFileID *fileID)
{
	m_uploadData = fileData;
	m_uploadHandler = uploadHandler;
	m_uploadFileID = fileID;

	m_thumbData = thumbData;
	m_thumbDataSize = thumbDataSize;

	m_taskData.m_fileSlot = fileSlot;
	m_taskData.m_fileSize = fileSize;
	m_taskData.m_category = category;
	m_taskData.m_numTags = numTags;

	// copy the filename and tags so they are not required to persist
	bdStrlcpy(m_taskData.m_fileName, fileName, bdStrnlen(fileName,BD_MAX_FILE_NAME_LENGTH)+1);

	for (bdUInt32 i(0); i<numTags; ++i)
	{
		m_taskData.m_tags[i] = tags[i];
	}

	m_httpSite = 0;
}

void bdContentStreaming::initDownload(void *fileData, bdUInt32 fileSize, bdDownloadInterceptor *downloadHandler, bdFileMetaData *fileMetaData, bdUInt32 startByte, bdUInt32 endByte)
{
	m_downloadData = fileData;
	m_downloadDataSize = fileSize;
	m_downloadMetaData = fileMetaData;
	m_downloadHandler = downloadHandler;
	m_startByte = startByte;
	m_endByte = endByte;
}

bdRemoteTaskRef bdContentStreaming::upload(const bdUInt16 fileSlot,
											const void* fileData,
											const bdUInt32 fileSize,
											const bdNChar8 *const fileName,
											const bdUInt16 category,
											const void* thumbData,
											const bdUInt32 thumbDataSize,
											const bdUInt32 numTags,
											const bdTag tags[],
											bdFileID *fileID)
{
	return _upload(fileSlot, fileData, BD_NOT_USED, fileSize, fileName, category, thumbData, thumbDataSize, numTags, tags, fileID);
}

bdRemoteTaskRef bdContentStreaming::upload(const bdUInt16 fileSlot,
											bdUploadInterceptor* uploadHandler,
											const bdUInt32 fileSize,
											const bdNChar8 *const fileName,
											const bdUInt16 category,
											const void* thumbData,
											const bdUInt32 thumbDataSize,
											const bdUInt32 numTags,
											const bdTag tags[],
											bdFileID *fileID)
{
	return _upload(fileSlot, BD_NOT_USED, uploadHandler, fileSize, fileName, category, thumbData, thumbDataSize, numTags, tags, fileID);
}

bdRemoteTaskRef bdContentStreaming::_upload(const bdUInt16 fileSlot,
										   const void* fileData,
										   bdUploadInterceptor* uploadHandler,
										   const bdUInt32 fileSize,
										   const bdNChar8 *const fileName,
										   const bdUInt16 category,
										   const void* thumbData,
										   const bdUInt32 thumbDataSize,
										   const bdUInt32 numTags,
										   const bdTag tags[],
										   bdFileID *fileID)
{
	// in case we are currently involved in an active or even aborted operation
	if (!m_http.isOkayToStart())
	{
		m_overallTask = new bdRemoteTask();
		setState(FAILED, BD_CONTENTSTREAMING_FAILED_TO_START_HTTP);

		return m_overallTask;
	}

	m_http.resetTransferStats();

	initUpload(fileSlot, 
			   fileData, 
			   fileSize, 
			   uploadHandler,
			   fileName, 
			   category,
			   thumbData,
			   thumbDataSize,
			   numTags,
			   tags,
			   fileID);

	if (thumbDataSize > BD_MAX_THUMB_DATA_SIZE)
	{
		BD_ERR("err", "bdMaxThumbDataSizeExceeded");
		return BD_NULL;
	}

	bdRemoteTaskRef remoteTask = _preUpload(fileName,fileSlot,fileSize,category,m_URLs);
	
	if (remoteTask == BD_NULL)
	{
		m_currentTaskID = 0;
		return BD_NULL;
	}
	else
	{
		bdRemoteTaskRef overallTask = new bdRemoteTask();
		overallTask->m_status = bdRemoteTask::BD_PENDING;

		m_remoteTask = remoteTask;
		m_overallTask = overallTask;

		setState(PRE_HTTP_OPERATION);
		m_currentTaskID = BD_STREAMING_UPLOAD_FILE_VIA_HTTP;
		return overallTask;
	}
}

void bdContentStreaming::pumpUploadViaHTTP()
{
	switch (m_state)
	{
		case PRE_HTTP_OPERATION:
		{
			bdRemoteTask::bdStatus preState = m_remoteTask->getStatus();
			
			if ( preState != bdRemoteTask::BD_PENDING )
			{
				if ( preState == bdRemoteTask::BD_DONE )
				{
#if (defined BD_PLATFORM_WIN32) || (defined BD_PLATFORM_UNIX)
					if (m_useTestAddresses)
					{
						swapURLInfo(m_URLs[m_httpSite].m_url);
					}
#endif
					bdHTTPWrapper::bdStatus httpState;
					BD_INFO("sim","Starting HTTP PUT using URL %s", m_URLs[m_httpSite].m_url);
					
					if (m_uploadHandler != BD_NOT_USED)
					{
						httpState = m_http.startUpload(m_URLs[m_httpSite].m_url, 
													   m_uploadHandler,
													   m_taskData.m_fileSize,
													   m_remoteTask->getTransactionID());
					}
					else
					{
						
						httpState = m_http.startUpload(m_URLs[m_httpSite].m_url, 
													   m_uploadData,
													   m_taskData.m_fileSize,
													   m_remoteTask->getTransactionID());
					}

					if (httpState == bdHTTPWrapper::BD_PENDING || httpState == bdHTTPWrapper::BD_DONE)
					{
						setState(HTTP_OPERATION);
					}
					else
					{
						setState(FAILED, BD_CONTENTSTREAMING_FAILED_TO_START_HTTP);
					}
				}
				else
				{
					setState(FAILED);
				}
			}
		}
		break;

		case HTTP_OPERATION:
		{
			// check for done or failure
			bdHTTPWrapper::bdStatus httpStatus = m_http.getStatus(); 
			if (httpStatus != bdHTTPWrapper::BD_PENDING)
			{
				if (httpStatus == bdHTTPWrapper::BD_DONE)
				{
					bdFloat64 speed = m_http.getTransferSpeed();
					BD_INFO("sim","Uploaded: \"%s\" @ %.2f Kbytes/sec", m_taskData.m_fileName, speed/1000.0f);
				
					m_remoteTask = _postUpload(); 

					if (m_remoteTask != BD_NULL)
					{
						setState(POST_HTTP_OPERATION);
					}
					else
					{
						setState(FAILED);
					}
				}
				else // bdHTTPWrapper::BD_FAILED
				{
					// try next http site
					m_httpSite++;
					if (m_httpSite >= m_remoteTask->getNumResults() || m_httpSite >= BD_MAX_URL_RESULTS)
					{
						setState(FAILED, BD_CONTENTSTREAMING_HTTP_ERROR);	
					}
					else
					{
						BD_INFO("sim","Starting HTTP PUT using URL %s", m_URLs[m_httpSite].m_url);
						bdHTTPWrapper::bdStatus httpState = m_http.startUpload(m_URLs[m_httpSite].m_url, 
																				m_uploadData,
																				m_taskData.m_fileSize,
																				m_remoteTask->getTransactionID());
				
						if (httpState != bdHTTPWrapper::BD_PENDING)
						{
							setState(FAILED, BD_CONTENTSTREAMING_FAILED_TO_START_HTTP);
						}
					}
				}
			}
		}
		break;

		case POST_HTTP_OPERATION:
		{
			bdRemoteTask::bdStatus postState = m_remoteTask->getStatus();

			if ( postState != bdRemoteTask::BD_PENDING )
			{
				if ( postState == bdRemoteTask::BD_DONE )
				{
					setState(DONE);
				}
				else
				{
					setState(FAILED);
				}
			}
		}
		break;
		default:
		{

		}
		break;
	}

	if (m_state == FAILED ||
		m_state == DONE)
	{
		m_currentTaskID = 0;
	}
}

bdRemoteTaskRef bdContentStreaming::downloadFileBySlot(const bdOnlineUserID ownerID,
													   const bdUInt16 fileSlot,
													   void *fileData, 
													   const bdUInt32 fileSize,
													   bdFileMetaData *fileMetaData,
													   bdUInt32 startByte,
													   bdUInt32 endByte)
{
	return _download(BD_NOT_USED, ownerID, fileSlot, fileData, fileSize, BD_NOT_USED, fileMetaData, startByte, endByte);
}

bdRemoteTaskRef bdContentStreaming::downloadFileBySlot(const bdOnlineUserID ownerID,
													   const bdUInt16 fileSlot,
													   bdDownloadInterceptor *downloadHandler,
													   bdFileMetaData *fileMetaData,
													   bdUInt32 startByte,
													   bdUInt32 endByte)
{
	return _download(BD_NOT_USED, ownerID, fileSlot, BD_NOT_USED, BD_NOT_USED, downloadHandler, fileMetaData, startByte, endByte);
}

bdRemoteTaskRef bdContentStreaming::download(const bdUInt64 fileID,
											 bdDownloadInterceptor *downloadHandler,
											 bdFileMetaData *fileMetaData,
											 bdUInt32 startByte,
											 bdUInt32 endByte)
{
	return _download(fileID, BD_NOT_USED, BD_NOT_USED, BD_NOT_USED, BD_NOT_USED, downloadHandler, fileMetaData, startByte, endByte);
}

bdRemoteTaskRef bdContentStreaming::download(const bdUInt64 fileID,
											 void *fileData, 
											 const bdUInt32 fileSize,
											 bdFileMetaData *fileMetaData,
											 bdUInt32 startByte,
											 bdUInt32 endByte)
{
	return _download(fileID, BD_NOT_USED, BD_NOT_USED, fileData, fileSize, BD_NOT_USED, fileMetaData, startByte, endByte);
}

bdRemoteTaskRef bdContentStreaming::_download(const bdUInt64 fileID,
											  const bdOnlineUserID ownerID,
											  const bdUInt16 fileSlot,
											  void *fileData, 
											  bdUInt32 fileSize,
											  bdDownloadInterceptor *downloadHandler,
											  bdFileMetaData *fileMetaData,
											  bdUInt32 startByte,
											  bdUInt32 endByte)
{
	// in case we are currently involved in an active or even aborted operation
	if (!m_http.isOkayToStart())
	{
		m_overallTask = new bdRemoteTask();
		setState(FAILED, BD_CONTENTSTREAMING_FAILED_TO_START_HTTP);
		return m_overallTask;
	}

	// hack: we set fileSize to a max value to effectively bypass the buffer size check on the server
	if (fileSize == BD_NOT_USED)
	{
		fileSize = BD_MAX_CONTENT_SERVER_FILE_SIZE;
	}

	m_http.resetTransferStats();
	initDownload(fileData, fileSize, downloadHandler, fileMetaData, startByte, endByte);

	bdRemoteTaskRef remoteTask;

	if (ownerID != BD_NOT_USED)
	{
		remoteTask = _preDownloadFileBySlot(ownerID, fileSlot, fileSize, m_downloadMetaData);
	}
	else
	{
		remoteTask = _preDownloadByFileID(fileID, fileSize, m_downloadMetaData);
	}
	
	if (remoteTask == BD_NULL)
	{
		m_currentTaskID = 0;
		return BD_NULL;
	}
	else
	{
		bdRemoteTaskRef overallTask = new bdRemoteTask();
		overallTask->m_status = bdRemoteTask::BD_PENDING;

		m_remoteTask = remoteTask;
		m_overallTask = overallTask;

		setState(PRE_HTTP_OPERATION);
		m_currentTaskID = BD_STREAMING_DOWNLOAD_FILE_VIA_HTTP;
		return overallTask;
	}
}

bdRemoteTaskRef bdContentStreaming::downloadPublisherFile(const bdUInt64 fileID,
														  void *fileData, 
														  const bdUInt32 fileSize,
														  bdFileMetaData *fileMetaData,
														  bdUInt32 startByte,
														  bdUInt32 endByte)
{
	return _downloadPublisherFile(fileID, fileData, fileSize, BD_NOT_USED, fileMetaData, startByte, endByte);
}

bdRemoteTaskRef bdContentStreaming::downloadPublisherFile(const bdUInt64 fileID,
														  bdDownloadInterceptor *downloadHandler,
														  bdFileMetaData *fileMetaData,
														  bdUInt32 startByte,
														  bdUInt32 endByte)
{
	return _downloadPublisherFile(fileID, BD_NOT_USED, BD_NOT_USED, downloadHandler, fileMetaData, startByte, endByte);
}

bdRemoteTaskRef bdContentStreaming::_downloadPublisherFile(const bdUInt64 fileID,
														  void *fileData, 
														  bdUInt32 fileSize,
														  bdDownloadInterceptor *downloadHandler,
														  bdFileMetaData *fileMetaData,
														  bdUInt32 startByte,
														  bdUInt32 endByte)
{
	// in case we are currently involved in an active or even aborted operation
	if (!m_http.isOkayToStart())
	{
		m_overallTask = new bdRemoteTask();
		setState(FAILED, BD_CONTENTSTREAMING_FAILED_TO_START_HTTP);
		return m_overallTask;
	}

	// hack: we set fileSize to a max value to effectively bypass the buffer size check on the server
	if (fileSize == BD_NOT_USED)
	{
		fileSize = BD_MAX_CONTENT_SERVER_FILE_SIZE;
	}

	initDownload(fileData, fileSize, downloadHandler, fileMetaData, startByte, endByte);

	bdRemoteTaskRef remoteTask = _preDownloadPublisherFile(fileID, fileSize, m_downloadMetaData);

	if (remoteTask == BD_NULL)
	{
		m_currentTaskID = 0;
		return BD_NULL;
	}
	else
	{
		bdRemoteTaskRef overallTask = new bdRemoteTask();
		overallTask->m_status = bdRemoteTask::BD_PENDING;

		m_remoteTask = remoteTask;
		m_overallTask = overallTask;

		setState(PRE_HTTP_OPERATION);
		m_currentTaskID = BD_STREAMING_DOWNLOAD_FILE_VIA_HTTP;
		return overallTask;
	}
}

bdRemoteTaskRef bdContentStreaming::downloadApplePurchasedFile(const bdUInt64 fileID,
															   const void *receipt, 
															   const bdUInt32 receiptSize,
															   bdDownloadInterceptor *downloadHandler,
															   bdFileMetaData *fileMetaData,
															   bdUInt32 startByte,
															   bdUInt32 endByte)
{
	return _downloadApplePurchasedFile(fileID, receipt, receiptSize, BD_NOT_USED, BD_NOT_USED, downloadHandler, fileMetaData, startByte, endByte);
}

bdRemoteTaskRef bdContentStreaming::downloadApplePurchasedFile(const bdUInt64 fileID,
															   const void *receipt, 
															   const bdUInt32 receiptSize,
															   void *fileData,
															   const bdUInt32 fileSize,
															   bdFileMetaData *fileMetaData,
															   bdUInt32 startByte,
															   bdUInt32 endByte)
{
	return _downloadApplePurchasedFile(fileID, receipt, receiptSize, fileData, fileSize, BD_NOT_USED, fileMetaData, startByte, endByte);
}

bdRemoteTaskRef bdContentStreaming::_downloadApplePurchasedFile(const bdUInt64 fileID,
															   const void *receipt, 
															   const bdUInt32 receiptSize,
															   void *fileData,
															   bdUInt32 fileSize,
															   bdDownloadInterceptor *downloadHandler,
															   bdFileMetaData *fileMetaData,
															   bdUInt32 startByte,
															   bdUInt32 endByte)
{
	// in case we are currently involved in an active or even aborted operation
	if (!m_http.isOkayToStart())
	{
		m_overallTask = new bdRemoteTask();
		setState(FAILED, BD_CONTENTSTREAMING_FAILED_TO_START_HTTP);
		return m_overallTask;
	}

	if (receiptSize > BD_MAX_ITUNES_RECEIPT_SIZE)
	{
		BD_ERR(BD_LOG_LEVEL, "receiptSize exceeds BD_MAX_ITUNES_RECEIPT_SIZE");
		m_overallTask = new bdRemoteTask();
		setState(FAILED, BD_CONTENTSTREAMING_FAILED_TO_START_HTTP);
		return m_overallTask;
	}

	// hack: we set fileSize to a max value to effectively bypass the buffer size check on the server
	if (fileSize == BD_NOT_USED)
	{
		fileSize = BD_MAX_CONTENT_SERVER_FILE_SIZE;
	}

	initDownload(fileData, fileSize, downloadHandler, fileMetaData, startByte, endByte);

	bdRemoteTaskRef remoteTask = _preDownloadITunesPurchasedFile(fileID, fileSize, receipt, receiptSize, m_downloadMetaData);

	if (remoteTask == BD_NULL)
	{
		m_currentTaskID = 0;
		return BD_NULL;
	}
	else
	{
		bdRemoteTaskRef overallTask = new bdRemoteTask();
		overallTask->m_status = bdRemoteTask::BD_PENDING;

		m_remoteTask = remoteTask;
		m_overallTask = overallTask;

		setState(PRE_HTTP_OPERATION);
		m_currentTaskID = BD_STREAMING_DOWNLOAD_FILE_VIA_HTTP;
		return overallTask;
	}
}

void bdContentStreaming::pumpDownloadViaHTTP()
{
	switch (m_state)
	{
		case PRE_HTTP_OPERATION:
		{
			bdRemoteTask::bdStatus preState = m_remoteTask->getStatus();

			if ( preState != bdRemoteTask::BD_PENDING )
			{
				if ( preState == bdRemoteTask::BD_DONE )
				{
#if (defined BD_PLATFORM_WIN32) || (defined BD_PLATFORM_UNIX)
					if (m_useTestAddresses)
					{
						swapURLInfo( m_downloadMetaData->m_url);
					}
#endif
					// move to http state
					BD_INFO("sim","Got URL for HTTP GET ", m_downloadMetaData->m_url);
					
					bdHTTPWrapper::bdStatus httpState;
					if (m_downloadHandler != BD_NOT_USED)
					{
						httpState = m_http.startDownload(m_downloadMetaData->m_url,
														 m_downloadHandler,
														 m_remoteTask->getTransactionID(),
														 m_startByte,
														 m_endByte);
					}
					else
					{
						httpState = m_http.startDownload(m_downloadMetaData->m_url,
														 m_downloadData, 
														 m_downloadDataSize,
														 m_remoteTask->getTransactionID(),
														 m_startByte,
														 m_endByte);

					}

					if (httpState == bdHTTPWrapper::BD_PENDING || httpState == bdHTTPWrapper::BD_DONE)
					{
						setState(HTTP_OPERATION);
					}
					else
					{
						setState(FAILED, BD_CONTENTSTREAMING_FAILED_TO_START_HTTP);
					}
				}
				else
				{
					setState(FAILED);

				}
			}
		}
		break;

		case HTTP_OPERATION:
		{
			// check for done or failure
			bdHTTPWrapper::bdStatus httpStatus = m_http.getStatus(); 
			if (httpStatus != bdHTTPWrapper::BD_PENDING)
			{
				if (httpStatus == bdHTTPWrapper::BD_DONE)
				{

					bdUInt32 progress = m_http.getTransferProgress();
					bdFloat64 speed = m_http.getTransferSpeed();
					BD_INFO("sim","Transfered: %d KB (%.0f%%) @ %0.2f KBytes/sec", 
							progress/1024,
							(static_cast<bdFloat64>(progress) * 100.0f)/(m_taskData.m_fileSize - m_startByte), 
							speed/1024.0f);

					BD_INFO("sim","Downloaded file: \n%s",m_downloadMetaData->m_url);
					setState(DONE);
					m_overallTask->m_numResults = 1;
				}
				else // bdHTTPWrapper::BD_FAILED
				{
					BD_INFO("sim","Failed to download: \n%s",m_downloadMetaData->m_url);
					setState(FAILED, BD_CONTENTSTREAMING_HTTP_ERROR);	
					m_overallTask->m_numResults = 0;
				}
			}
		}
		break;
		default:
		{

		}
		break;
	}

	if (m_state == FAILED ||
		m_state == DONE)
	{
		m_currentTaskID = 0;
	}
}

bdRemoteTaskRef bdContentStreaming::removeFile(const bdUInt16 fileSlot)
{
	// in case we are currently involved in an active or even aborted operation
	if (!m_http.isOkayToStart())
	{
		m_overallTask = new bdRemoteTask();
		setState(FAILED, BD_CONTENTSTREAMING_FAILED_TO_START_HTTP);
		return m_overallTask;
	}

	bdRemoteTaskRef remoteTask = _preDeleteFile(fileSlot, m_URLs);

	if (remoteTask == BD_NULL)
	{
		m_currentTaskID = 0;
		return BD_NULL;
	}
	else
	{
		bdRemoteTaskRef overallTask = new bdRemoteTask();
		overallTask->m_status = bdRemoteTask::BD_PENDING;

		m_remoteTask = remoteTask;
		m_overallTask = overallTask;

		// progress to pre upload state
		setState(PRE_HTTP_OPERATION);
		m_currentTaskID = BD_STREAMING_REMOVE_FILE_VIA_HTTP;
		return overallTask;
	}
}

void bdContentStreaming::pumpDeleteViaHTTP()
{
	switch (m_state)
	{
		case PRE_HTTP_OPERATION:
		{
			bdRemoteTask::bdStatus preState = m_remoteTask->getStatus();

			if ( preState != bdRemoteTask::BD_PENDING )
			{
				if ( preState == bdRemoteTask::BD_DONE )
				{
#if (defined BD_PLATFORM_WIN32) || (defined BD_PLATFORM_UNIX)
					if (m_useTestAddresses)
					{
						swapURLInfo( m_URLs[0].m_url);
					}
#endif
					BD_INFO("sim","Got URL for HTTP DELETE", m_URLs[0].m_url);
					bdHTTPWrapper::bdStatus httpState = m_http.startDelete(m_URLs[0].m_url, m_remoteTask->getTransactionID());

					if (httpState == bdHTTPWrapper::BD_PENDING || httpState == bdHTTPWrapper::BD_DONE)
					{
						// progress to download state
						setState(HTTP_OPERATION);
					}
					else
					{
						setState(FAILED, BD_CONTENTSTREAMING_FAILED_TO_START_HTTP);
					}
				}
				else // bdHTTPWrapper::BD_FAILED
				{
					setState(FAILED);
				}
			}
		}
		break;

		case HTTP_OPERATION:
		{
			// check for done or failure
			bdHTTPWrapper::bdStatus httpStatus = m_http.getStatus(); 
			if (httpStatus != bdHTTPWrapper::BD_PENDING)
			{
				if (httpStatus == bdHTTPWrapper::BD_DONE)
				{
					setState(DONE);
				}
				else
				{
					setState(FAILED, BD_CONTENTSTREAMING_HTTP_ERROR);
				}
			}
		}
		break;
		default:
		{

		}
		break;
	}

	if (m_state == FAILED ||
		m_state == DONE)
	{
		m_currentTaskID = 0;
	}
}

bdRemoteTaskRef bdContentStreaming::_preUpload(const bdNChar8 *const filename,
											   const bdUInt16 fileSlot,
											   const bdUInt32 fileSize,
											   const bdUInt16 category,
											   bdURL uploadURLs[])
{
	bdRemoteTaskRef remoteTask = BD_NULL;
	bdUInt taskSize = BD_TASK_OVERHEAD + (bdUInt)bdStrnlen(filename, BD_MAX_FILE_NAME_LENGTH) + sizeof(fileSlot) + sizeof(fileSize) + sizeof(category);
	
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_STREAMING_SERVICE2, BD_CONTENTSTREAMING_PRE_UPLOAD);

	buffer.writeString(filename, bdStrnlen(filename, BD_MAX_FILE_NAME_LENGTH)+1);
	buffer.writeUInt16(fileSlot);
	buffer.writeUInt32(fileSize);
	buffer.writeUInt16(category);
	
	m_remoteTaskManager->startTask(remoteTask, buffer);
	remoteTask->setTaskResult(uploadURLs, BD_MAX_URL_RESULTS);

	return remoteTask;
}

bdRemoteTaskRef bdContentStreaming::_postUpload()
{
	bdRemoteTaskRef remoteTask = BD_NULL;

	bdUInt taskSize = BD_TASK_OVERHEAD;
	bdUInt fileNameSize = (bdUInt)bdStrnlen(m_taskData.m_fileName, BD_MAX_FILE_NAME_LENGTH) + 1;
	bdUInt URLSize = (bdUInt)bdStrnlen(m_URLs[m_httpSite].m_serverIndex, BD_MAX_SERVER_URL_SIZE) + 1;
	taskSize += fileNameSize;
	taskSize += sizeof(m_taskData.m_fileSlot);
	taskSize += sizeof(m_URLs[m_httpSite].m_serverType);
	taskSize += URLSize;
	taskSize += sizeof(m_taskData.m_fileSize);
	taskSize += sizeof(m_taskData.m_category);
	taskSize += sizeof(m_thumbDataSize);
	taskSize += m_thumbDataSize;
	taskSize += BD_TYPED_ARRAY_HEADER_SIZE + sizeof(bdUInt32)*m_taskData.m_numTags*2;

	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_STREAMING_SERVICE2, BD_CONTENTSTREAMING_POST_UPLOAD);

	buffer.writeString(m_taskData.m_fileName, fileNameSize);
	buffer.writeUInt16(m_taskData.m_fileSlot);
	buffer.writeUInt16(m_URLs[m_httpSite].m_serverType);
	buffer.writeString(m_URLs[m_httpSite].m_serverIndex, URLSize);
	buffer.writeUInt32(m_taskData.m_fileSize);
	buffer.writeUInt16(m_taskData.m_category);
	buffer.writeBlob(m_thumbData, m_thumbDataSize);

	buffer.writeArrayStart(BD_BB_UNSIGNED_INTEGER32_TYPE, m_taskData.m_numTags *2, sizeof(bdUInt32));
	for (bdUInt i(0); i < m_taskData.m_numTags; i++)
	{
		//m_taskData.m_tags[i].serialize(buffer);
		buffer.writeUInt32(m_taskData.m_tags[i].m_priTag);
		buffer.writeUInt32(m_taskData.m_tags[i].m_secTag);
	}
	buffer.writeArrayEnd();

	m_remoteTaskManager->startTask(remoteTask, buffer);
	remoteTask->setTaskResult(m_uploadFileID, 1);
	return remoteTask;
}

bdRemoteTaskRef bdContentStreaming::_preDownloadFileBySlot(bdOnlineUserID ownerUID,
														   bdUInt16 fileSlot,
														   bdUInt32 fileSize,
														   bdFileMetaData *fileMetaData)
{
	bdRemoteTaskRef remoteTask = BD_NULL;
	bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(ownerUID) + sizeof(fileSlot) + sizeof(fileSize);

	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_STREAMING_SERVICE2, BD_CONTENTSTREAMING_PRE_DOWNLOAD_BY_SLOT);

	buffer.writeUInt64(ownerUID);
	buffer.writeUInt16(fileSlot);
	buffer.writeUInt32(fileSize);

	m_remoteTaskManager->startTask(remoteTask, buffer);
	remoteTask->setTaskResult(fileMetaData,1);

	return remoteTask;
}

bdRemoteTaskRef bdContentStreaming::_preDownloadByFileID(bdUInt64 fileID,
														 bdUInt32 fileSize,
														 bdFileMetaData *fileMetaData)
{
	bdRemoteTaskRef remoteTask = BD_NULL;
	bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(fileID) + sizeof(fileSize);

	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_STREAMING_SERVICE2, BD_CONTENTSTREAMING_PRE_DOWNLOAD_BY_FILE_ID);

	buffer.writeUInt64(fileID);
	buffer.writeUInt32(fileSize);

	m_remoteTaskManager->startTask(remoteTask, buffer);
	remoteTask->setTaskResult(fileMetaData,1);

	return remoteTask;
}

bdRemoteTaskRef bdContentStreaming::_preDownloadPublisherFile(bdUInt64 fileID,
															  bdUInt32 fileSize,
															  bdFileMetaData *fileMetaData)
{
	bdRemoteTaskRef remoteTask = BD_NULL;
	bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(fileID) + sizeof(fileSize);
	
	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_STREAMING_SERVICE2, BD_CONTENTSTREAMING_PRE_DOWNLOAD_PUBLISHER_FILE);

	buffer.writeUInt64(fileID);
	buffer.writeUInt32(fileSize);

	m_remoteTaskManager->startTask(remoteTask, buffer);
	remoteTask->setTaskResult(fileMetaData,1);

	return remoteTask;
}

bdRemoteTaskRef bdContentStreaming::_preDownloadITunesPurchasedFile(bdUInt64 fileID,
																	bdUInt32 fileSize,
																	const void *receipt, 
																	const bdUInt32 receiptSize,
																	bdFileMetaData *fileMetaData)
{
	bdRemoteTaskRef remoteTask = BD_NULL;
	bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(fileID) + sizeof(fileSize) + receiptSize;

	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_STREAMING_SERVICE2, BD_CONTENT_STREAMING_PRE_DOWNLOAD_ITUNES_PURCHASED_FILE);

	buffer.writeUInt64(fileID);
	buffer.writeUInt32(fileSize);
	buffer.writeBlob(receipt, receiptSize);

	m_remoteTaskManager->startTask(remoteTask, buffer);
	remoteTask->setTaskResult(fileMetaData,1);

	return remoteTask;
}

bdRemoteTaskRef bdContentStreaming::_preDeleteFile(bdUInt16 fileSlot,
												  bdURL deleteURLs[])
{
	bdRemoteTaskRef remoteTask = BD_NULL;
	bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(fileSlot);

	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_STREAMING_SERVICE2, BD_CONTENTSTREAMING_PRE_DELETE_FILE);

	buffer.writeUInt16(fileSlot);
	
	m_remoteTaskManager->startTask(remoteTask, buffer);
	remoteTask->setTaskResult(deleteURLs, 1);

	return remoteTask;
}

bdRemoteTaskRef bdContentStreaming::getQuotaUsage(bdQuotaUsage *quotaUsage)
{
	bdRemoteTaskRef remoteTask = BD_NULL;
	bdUInt taskSize = BD_TASK_OVERHEAD;

	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_STREAMING_SERVICE2, BD_CONTENT_STREAMING_GET_USER_USAGE);

	m_remoteTaskManager->startTask(remoteTask, buffer);
	remoteTask->setTaskResult(quotaUsage, 1);

	return remoteTask;
}

bdRemoteTaskRef bdContentStreaming::reportContent(bdEntityID entityID)
{
	bdRemoteTaskRef remoteTask = BD_NULL;
	bdUInt taskSize = BD_TASK_OVERHEAD + sizeof(entityID);

	bdByteBuffer buffer(taskSize, TYPE_CHECKED);
	bdRemoteTaskManager::initTaskBuffer(buffer, BD_REPORT_CONTENT_SERVICE, BD_CONTENT_STREAMING_REPORT_CONTENT);

	buffer.writeUInt64(entityID);
	
	m_remoteTaskManager->startTask(remoteTask, buffer);

	return remoteTask;
}


void bdContentStreaming::setState(bdStatus newState, bdLobbyErrorCode errorCode)
{
	// check to see if we're done, if so set 
	// m_overallTask->m_errorCode and
	// m_overallTask->m_status
	if (newState == DONE)
	{
		m_overallTask->m_status = bdRemoteTask::BD_DONE; 
		m_overallTask->m_errorCode = BD_NO_ERROR; 
	}
	else if (newState == FAILED)
	{
		m_overallTask->m_status = bdRemoteTask::BD_FAILED; 
		// if error code is specified, use it
		if (errorCode != BD_NO_ERROR)
		{
			m_overallTask->m_errorCode = static_cast<bdLobbyErrorCode>(errorCode);
			m_overallTask->m_status = bdRemoteTask::BD_FAILED; 
		}
		else
		{
			if (m_state == PRE_HTTP_OPERATION || m_state == POST_HTTP_OPERATION)
			{
				m_overallTask->m_errorCode = m_remoteTask->m_errorCode;
			}
		}
	}
	BD_INFO("sim","state %s -> %s", s_bdStatusString[m_state], s_bdStatusString[newState]);
	m_state = newState;
}

void bdContentStreaming::abortHTTPOperation()
{
	if (m_overallTask.notNull())
	{
		if (m_state != HTTP_OPERATION )
		{
			BD_WARN("sim","Aborting HTTP operation while in state %s", s_bdStatusString[m_state]);
			m_overallTask->m_status = bdRemoteTask::BD_FAILED;
			if (m_remoteTask.notNull())
			{
				m_remoteTask->cancelTask();
			}
		}
		else
		{
			m_http.abortOperation();
			BD_WARN("sim","Aborting HTTP operation");
			m_overallTask->m_status = bdRemoteTask::BD_FAILED;
		}
	}
}

void bdContentStreaming::checkProgress(bdUInt32 &bytesTransfered, bdFloat32 &dataRate)
{
	bytesTransfered = m_http.getTransferProgress();
	dataRate = m_http.getTransferSpeed();
}

bdInt32 bdContentStreaming::getLastHTTPError() const
{
	return m_http.getLastHTTPError();
}

#if (defined BD_PLATFORM_WIN32) || (defined BD_PLATFORM_UNIX)

void bdContentStreaming::enableVerboseOutput(bdBool enable)
{
	m_http.enableVerboseOutput(enable);
}

void bdContentStreaming::enableProgressMeter(bdBool enable)
{
	m_http.enableProgressMeter(enable);
}

void bdContentStreaming::setTestLSPServerAddresses(bdURL* URLs, bdUInt32 numURLs)
{
	m_testAddressCount = numURLs;
	m_useTestAddresses = true;
	for (bdUInt32 i=0;i<numURLs;i++)
	{
		bdMemcpy(m_testURLs[i].m_url,URLs[i].m_url,512);
	}
}

void bdContentStreaming::swapURLInfo(bdNChar8* URL)
{
	//Start of file info is delimited by a '/' character
	const bdNChar8* startFileInfo = bdStrchr(URL, '/');
						
	if (startFileInfo != BD_NULL)
	{
		//Extract the server name from the URL
		bdInt32 URLSize = startFileInfo - URL;
		bdNChar8 tempURL[BD_MAX_URL_SIZE];
		bdMemcpy(tempURL, URL, URLSize);
		tempURL[URLSize] = '\0';

		//Determine if this server name is to be swapped with a server name passed from the command line
		for (bdUInt32 i = 0; i < m_testAddressCount ; i++)
		{
			if (bdStrcmp(tempURL,m_testURLs[i].m_url) == 0)
			{
				//Found it..now append the file info from the current URL to the next server name in the array 
				//(the one we are swapping with)
				
				//Determine the size of the file info in the existing URL to be copied
				const bdNChar8* endFileInfo = bdStrchr(URL, '\0');
				bdInt32 fileInfoSize = endFileInfo - startFileInfo;

				//Determine the size of the server name in the URL to be copied
				bdNChar8* endURLInfo = bdStrchr(m_testURLs[i+1].m_url, '\0');
				bdInt32 serverNameToBeCopiedSize = endURLInfo - m_testURLs[i+1].m_url;

				bdNChar8 copyURL[BD_MAX_URL_SIZE];
				
				//Copy the new server name 
				bdMemcpy(copyURL, m_testURLs[i+1].m_url, serverNameToBeCopiedSize);		
				
				//Copy the file info from the current URL 
				bdMemcpy(copyURL+serverNameToBeCopiedSize, URL+URLSize,  fileInfoSize);
				
				//Add a terminator character to the string (otherwise file info won't be recognised on server!)
				copyURL[serverNameToBeCopiedSize + fileInfoSize] = '\0';	

				//Copy the new URL to the old URL
				bdMemcpy(URL, copyURL, BD_MAX_URL_SIZE);

				return;
			}
		}		
	}
}

#endif
