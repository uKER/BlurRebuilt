// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdLobby/bdLSGServices/bdBandwidthTest.h>
#include <bdLobby/bdLobbyService.h>
#include <bdLobby/bdRemoteTaskManager/bdRemoteTaskManager.h>
#include <bdLobby/bdLSGServices/bdLSGServices.h>
#include <bdLobby/bdRemoteTaskManager/bdRemoteTask.h>

#define BD_LOG_LEVEL "bdBandwidthTestClient"

enum bdBandwidthRequest
{
	BD_BANDWIDTH_TEST_REQUEST = 0,
	BD_BANDWIDTH_TEST_FINALIZE = 1
};

bdBandwidthTestClient::bdBandwidthTestClient()
: m_type(BD_UPLOAD_TEST),
  m_initStatus(BD_BANDWIDTH_TEST_UNINITIALIZED),
  m_testStatus(BD_BANDWIDTH_TEST_IDLE),
  m_error(BD_NO_ERROR),
  m_remoteTask(BD_NULL),
  m_packetBuffer(BD_NULL),
  m_nextPacketNum(0),
  m_actualNumSent(0)
{
}

bdBandwidthTestClient::~bdBandwidthTestClient()
{
	stop();
	quit();
}

bdBool bdBandwidthTestClient::init(bdLobbyService *const lobbyService)
{
#ifdef BD_PLATFORM_WII
	// Running a bandwidth test using the regular server settings may cause
	// the code to freeze, this is a bug in the Nintendo sockets library 
	// and is currently being investigated.. 
	BD_ERR(BD_LOG_LEVEL, "Bandwidth testing may freeze Demonware code on the Wii platform!");
#endif
	// fail safe
	bdBool ok = false;
	m_error = BD_BANDWIDTH_TEST_SOCKET_ERROR;

	if(m_initStatus != BD_BANDWIDTH_TEST_UNINITIALIZED)
	{
		BD_WARN(BD_LOG_LEVEL, "bdBandwidthTestClient already initialized.");
	}
	else
	{
		// fail safe
		const bdBool blocking = false;
		const bdBool broadcast = false;
		ok = m_socket.create(blocking, broadcast);
		if(ok)
		{
			m_error = BD_NO_ERROR;
			m_initStatus = BD_BANDWIDTH_TEST_INITIALIZED;
			m_lobbyService = lobbyService;
			BD_INFO(BD_LOG_LEVEL, "Initialised client.");
		}
		else
		{
			BD_ERR(BD_LOG_LEVEL, "bdBandwidthTestClient failed to create UDP socket.");
		}
	}

	return ok;
}

void bdBandwidthTestClient::quit()
{
	if(m_initStatus == BD_BANDWIDTH_TEST_UNINITIALIZED)
	{
		BD_INFO(BD_LOG_LEVEL, "quit called on uninitialized bdBandwidthTestClient instance.");
	}
	else
	{
		const bdBool ok = m_socket.close();
		if(!ok)
		{
			BD_ERR(BD_LOG_LEVEL, "bdBandwidthTestClient failed to close UDP socket.");
		}
	
		m_initStatus = BD_BANDWIDTH_TEST_UNINITIALIZED;
		BD_INFO(BD_LOG_LEVEL, "Quit client.");

	}
}

void bdBandwidthTestClient::start(const bdBandwidthTestType type)
{	
	m_error = BD_START_TASK_FAILED;

	if(m_initStatus == BD_BANDWIDTH_TEST_UNINITIALIZED)
	{
		BD_WARN(BD_LOG_LEVEL, "bdBandwidthTestClient not initialized.");
	}
	else if((m_testStatus != BD_BANDWIDTH_TEST_IDLE) && (m_testStatus != BD_BANDWIDTH_TEST_DONE))
	{
		BD_WARN(BD_LOG_LEVEL, "bdBandwidthTestClient already in use.");
	}
	else if (BD_NULL == m_lobbyService)
	{
		BD_WARN(BD_LOG_LEVEL, "Cannot start test as lobby service pointer is invalid.");
	}
	else
	{
		bdRemoteTaskManager *const taskManager = m_lobbyService->getTaskManager();
		if(taskManager)
		{
			const bdUInt bufferSize = 16;
			bdUByte8 buffer[bufferSize];
			bdUInt offset = 0;

			const bdUByte8 requestType8 = BD_BANDWIDTH_TEST_REQUEST;
			const bdUByte8 testType8 = bdUByte8(type);

			bdBool ok = bdBytePacker::appendBasicType(buffer, bufferSize, offset, offset, requestType8);
			ok = ok && bdBytePacker::appendBasicType(buffer, bufferSize, offset, offset, testType8);

			if(ok)
			{
				m_error = taskManager->startLSGTask(m_remoteTask,
													BD_LSG_SERVICE, 
													BD_BANDWIDTH_TEST, 
													buffer, 
													sizeof(buffer));
				if(m_error == BD_NO_ERROR)
				{						
					m_testStatus = BD_BANDWIDTH_TEST_REQUESTING_TEST;
					m_type = type;

					// clear the results
					m_uploadResults.clear();
					m_downloadResults.clear();
					BD_INFO(BD_LOG_LEVEL, "Started bandwidth test of type: %u." , m_type);
				}
				else
				{
					BD_WARN(BD_LOG_LEVEL, "Failed to start task for bandwidth test of type: %u." , m_type);
				}
			}
			else
			{
				BD_WARN(BD_LOG_LEVEL, "Failed to pack request buffer." );
			}
		}
		else
		{
			BD_WARN(BD_LOG_LEVEL, "Failed to retreive task manager from lobby service.");
		}
	}
}

void bdBandwidthTestClient::stop()
{
	BD_INFO(BD_LOG_LEVEL, "Stopping bandwidth test of type: %u." , m_type);

	if(m_remoteTask)
	{
		m_remoteTask = BD_NULL;
	}

	if(m_packetBuffer)
	{
		bdDeallocate<bdUByte8>(m_packetBuffer);
		m_packetBuffer = BD_NULL;
	}

	m_testStatus = BD_BANDWIDTH_TEST_IDLE;
	m_error = BD_NO_ERROR;
}

bdBandwidthTestStatus bdBandwidthTestClient::getStatus() const
{
	return m_testStatus;
}

void bdBandwidthTestClient::pump()
{
	if(m_initStatus != BD_BANDWIDTH_TEST_INITIALIZED)
	{
		BD_WARN(BD_LOG_LEVEL, "Pumping uninitialized bdBandwidthTestClient.");
	}

	switch(m_testStatus)
	{
		case BD_BANDWIDTH_TEST_IDLE:
		{
			BD_WARN(BD_LOG_LEVEL, "Pumping idle bdBandwidthTestClient.");
		}
		break;
		case BD_BANDWIDTH_TEST_REQUESTING_TEST:
		{
			pumpRequest();
		}
		break;
		case BD_BANDWIDTH_TEST_UPLOAD_WAITING:
		{
			pumpUploadWait();
		}
		break;		
		case BD_BANDWIDTH_TEST_UPLOAD_SENDING:
		{
			pumpUploadSend();
		}
		break;
		case BD_BANDWIDTH_TEST_DOWNLOAD_WAITING:
		case BD_BANDWIDTH_TEST_DOWNLOAD_RECEIVING:
		{
			pumpDownloadReceive();
		}
		break;
		case BD_BANDWIDTH_TEST_FINALIZING:
		{
			pumpFinalize();
		}
		break;
		case BD_BANDWIDTH_TEST_DONE:
		{
			// Nothing more to do.
		}
		break;
	}
}

bdBandwidthTestType bdBandwidthTestClient::getTestType() const
{
	return m_type;
}

bdLobbyErrorCode bdBandwidthTestClient::getErrorCode() const
{
	return m_error;
}

bdBandwidthTestResults& bdBandwidthTestClient::getUploadResults()
{
	return m_uploadResults;
}

bdBandwidthTestResults& bdBandwidthTestClient::getDownloadResults()
{
	return m_downloadResults;
}

void bdBandwidthTestClient::pumpRequest()
{	
	BD_ASSERT(m_remoteTask != BD_NULL, "bdBandwidthTestClient status is BD_REQUESTING_TEST with null task handle.");
	BD_ASSERT(m_testStatus == BD_BANDWIDTH_TEST_REQUESTING_TEST, "bdBandwidthTestClient::pumpRequest called in incorrect status.");

	// fail safe
	m_error = BD_HANDLE_TASK_FAILED;

	if(m_remoteTask && (m_testStatus == BD_BANDWIDTH_TEST_REQUESTING_TEST))
	{
		const bdRemoteTask::bdStatus taskStatus = m_remoteTask->getStatus();
		switch (taskStatus)
		{
			case bdRemoteTask::BD_EMPTY:
			{
				BD_ERR(BD_LOG_LEVEL, "bdBandwidthTestClient status is BD_BANDWIDTH_TEST_REQUESTING_TEST but task is bdRemoteTask::BD_EMPTY.");				
			}
			break;
			case bdRemoteTask::BD_PENDING:
			{
				// still requesting
				m_error = BD_NO_ERROR;
			}
			break;
			case bdRemoteTask::BD_DONE:
			{
				// handle the reply
				m_error = BD_NO_ERROR;
				const bdByteBufferRef replyBuffer = m_remoteTask->getByteResults();
				m_remoteTask = BD_NULL;

				handleRequestReply(replyBuffer);
			}
			break;
			case bdRemoteTask::BD_TIMED_OUT:
			{
				BD_WARN(BD_LOG_LEVEL, "bdBandwidthTestClient test request timed out.");
			}
			break;
			case bdRemoteTask::BD_MAX_STATUS:
			{
				BD_ERR(BD_LOG_LEVEL, "bdBandwidthTestClient test request invalid remote task state (bdRemoteTask::BD_MAX_STATUS).");
			}
			break;
			default:
			break;
		}
	}
}

void bdBandwidthTestClient::handleRequestReply(const bdByteBufferRef replyBuffer)
{
	// fail safe
	m_error = BD_HANDLE_TASK_FAILED;

	if(replyBuffer.notNull())
	{
		bdUByte8 testRejected = 1;
		
		// parse the reply buffer
		bdBool ok = replyBuffer->read(testRejected);
		if(ok && (testRejected == 1))
		{
			// test not allowed, read the error code	
			bdUInt16 errorCode(0);
			if(replyBuffer->read(errorCode))
			{
				m_error = static_cast<bdLobbyErrorCode>(errorCode);	
			}
			else
			{
				BD_ERR(BD_LOG_LEVEL, "Failed to parse bandwidth test error code.");
			}

			BD_INFO(BD_LOG_LEVEL, "Request for bandwidth test rejected (%u).", m_error);
		}

		if(ok && (testRejected == 0))
		{
			bdUInt32 lsgAddr = 0;

			// test allowed
			ok = replyBuffer->read(m_packetSize);
			ok = ok && replyBuffer->read(m_numPackets);
			ok = ok && replyBuffer->read(m_senderInitialWait);
			ok = ok && replyBuffer->read(m_sendDuration);
			ok = ok && replyBuffer->read(m_receiverInitialWait);
			ok = ok && replyBuffer->read(m_receiveDuration);
			ok = ok && replyBuffer->read(m_lingerDuration);
			ok = ok && replyBuffer->read(m_lsgPort);
			ok = ok && replyBuffer->read(lsgAddr);
			bdBitBuffer::endianSwap(lsgAddr, m_lsgAddr);		

			bdMemset(m_cookie, BD_NULL, sizeof(m_cookie));

			for(bdUInt i = 0; ok && (i < sizeof(m_cookie)); i++)
			{
				ok = replyBuffer->read(m_cookie[i]);
			}

			// allocate a buffer for the packet
			m_packetBuffer = bdReallocate<bdUByte8>(m_packetBuffer, m_packetSize);

			if(ok && m_packetBuffer)
			{
				m_error = BD_NO_ERROR;
				startUploadTest();				
			}
			else
			{
				BD_ERR(BD_LOG_LEVEL, "Failed to parse bandwidth test reply.");
			}
		}
	}
}

void bdBandwidthTestClient::startUploadTest()
{
	BD_INFO(BD_LOG_LEVEL, "Starting upload test: m_packetSize:%u m_numPackets:%u m_senderInitialWait:%u " \
						"m_sendDuration:%u m_receiverInitialWait:%u m_receiveDuration:%u m_lingerDuration:%u " \
						"m_lsgPort:%u m_lsgAddr:%u " , m_packetSize, m_numPackets, m_senderInitialWait, m_sendDuration, 
						m_receiverInitialWait, m_receiveDuration, m_lingerDuration, m_lsgPort, m_lsgAddr );

	m_timer.start();
	m_testStatus = BD_BANDWIDTH_TEST_UPLOAD_WAITING;
}

void bdBandwidthTestClient::pumpUploadWait()
{
	if(m_timer.getElapsedTimeInSeconds() >= (m_senderInitialWait / 1000.0f))
	{
		m_testStatus = BD_BANDWIDTH_TEST_UPLOAD_SENDING;
		// reset the sequence number
		m_nextPacketNum = 0;
		m_timer.start();
	}
}

void bdBandwidthTestClient::pumpUploadSend()
{
	// the proportion of the send time that has elapsed
	const bdFloat32 sendDurationSecs = m_sendDuration / 1000.0f;
	bdFloat32 sendTimeProportion = m_timer.getElapsedTimeInSeconds() / sendDurationSecs;
	sendTimeProportion = BD_CLAMP(sendTimeProportion, 0, 1);

	// the total number of packets that should have been sent by the time this function is finished.
	const bdFloat32 lastPacket = sendTimeProportion * m_numPackets;
	const bdUInt32 firstPacket = m_nextPacketNum;

	bdBool sent = true;
	while(sent && (m_nextPacketNum < lastPacket))
	{
		sent = false;

		// write the packet
		bdUInt offset = 0;
		bdBool packetOk = bdBytePacker::appendBasicType(m_packetBuffer, m_packetSize, offset, offset, m_nextPacketNum);
		packetOk = packetOk && bdBytePacker::appendBuffer(m_packetBuffer, m_packetSize, offset, offset, m_cookie, sizeof(m_cookie));
		if(packetOk && (offset < m_packetSize))
		{
			bdRandom rand;
			const bdUInt noiseSize = m_packetSize - offset;
			rand.nextUBytes(m_packetBuffer + offset, noiseSize);
		}

		if(!packetOk)
		{
			BD_ERR(BD_LOG_LEVEL, "Failed to create UDP packet.");
		}
		else
		{	
			// send the packet
			const bdAddr destAddr(m_lsgAddr, m_lsgPort);
			const bdInt bytesSent = m_socket.sendTo(destAddr, m_packetBuffer, m_packetSize);
			if(bytesSent > 0 && m_packetSize == static_cast<bdUInt>(bytesSent))
			{
				// the packet was successfully sent, so keep going
				sent = true;
				m_nextPacketNum++;
				m_actualNumSent++;
			}
		}
	}

	// by the time the above packets were sent, more time has elapsed.
	// how many packets should have really been sent by now?
	const bdUInt32 lastPacketByNow = bdUInt(BD_CLAMP(m_timer.getElapsedTimeInSeconds() / sendDurationSecs, 0, 1) * m_numPackets);

	// packets =~ time
	// (m_nextPacketNum - firstPacket) = # how many packets we sent in this procedure call
	// (lastPacketByNow - m_nextPacketNum) = # how many more packets should have been sent during that time.
	if((m_nextPacketNum != firstPacket) && (m_nextPacketNum < lastPacketByNow) && m_nextPacketNum - firstPacket < lastPacketByNow - m_nextPacketNum)
	{
		//BD_INFO(BD_LOG_LEVEL, "Took %u packets worth of time to send %u packets - skipping some.", lastPacketByNow - m_nextPacketNum, m_nextPacketNum - firstPacket);
		// it took > X ms to send X ms worth of packets.
		// we just can't sent packets fast enough.. so skip some.
		m_nextPacketNum = lastPacketByNow;
	}

	if(m_nextPacketNum == m_numPackets)
	{
		// finished sending
		BD_INFO(BD_LOG_LEVEL, "Upload bandwidth test took %.2fs to send %u/%u packets of %u bytes each.", m_timer.getElapsedTimeInSeconds(), m_actualNumSent, m_numPackets, m_packetSize);

		if(m_type == BD_UPLOAD_TEST)
		{
			// not doing a download test so finish
			finalizeTest();
		}

		if(m_type == BD_UPLOAD_DOWNLOAD_TEST)
		{
			// start download test
			startDownloadTest();			
		}
	}	
}

void bdBandwidthTestClient::startDownloadTest()
{
	BD_INFO(BD_LOG_LEVEL, "Starting download test: m_packetSize:%u m_numPackets:%u m_senderInitialWait:%u "\
						"m_sendDuration:%u m_receiverInitialWait:%u m_receiveDuration:%u m_lingerDuration:%u "\
						"m_lsgPort:%u m_lsgAddr:%u " , m_packetSize, m_numPackets, m_senderInitialWait, m_sendDuration, 
						m_receiverInitialWait, m_receiveDuration, m_lingerDuration, m_lsgPort, m_lsgAddr );

	m_testStatus = BD_BANDWIDTH_TEST_DOWNLOAD_WAITING;
	m_timer.start();
	m_seqNumberTotal = 0;
}

void bdBandwidthTestClient::pumpDownloadReceive()
{
	bdInt received = 0;

	do
	{
		bdAddr fromAddr;
		received = m_socket.receiveFrom(fromAddr, m_packetBuffer, m_packetSize);
		
		if(received > 0)
		{
			const bdAddr lsgAddr(m_lsgAddr, m_lsgPort);
			const bdUInt32 receivedSize = static_cast<bdUInt32>(received);

			bdBool ok = (receivedSize == m_packetSize);
			ok = ok && (fromAddr == lsgAddr);

			// read the sequence number
			bdUInt32 sequenceNumber = m_numPackets;
			bdUInt offset = 0;

			ok = ok && bdBytePacker::removeBasicType(m_packetBuffer, receivedSize, offset, offset, sequenceNumber);

			// validate the sequence number
			ok = ok && (sequenceNumber < m_numPackets);

			// read the cookie
			bdUInt cookieBytesRead = 0;
			while (ok && cookieBytesRead < sizeof(m_cookie))
			{
				bdUByte8 cookieByte = 0;
				ok = bdBytePacker::removeBasicType(m_packetBuffer, receivedSize, offset, offset, cookieByte);

				// validate the cookie byte
				ok = ok && (cookieByte == m_cookie[cookieBytesRead]);

				if(ok)
				{
					cookieBytesRead++;
				}
			}
			
			// was the packet correctly formed
			if(ok)
			{
				if(m_downloadResults.m_bytesReceived == 0)
				{
					m_testStatus = BD_BANDWIDTH_TEST_DOWNLOAD_RECEIVING;

					// first packet so start timer
					m_timer.start();

					// check download results are clear
					BD_ASSERT(m_downloadResults.m_bytesReceived == 0, "Bandwidth test download results not cleared.");
					BD_ASSERT(m_downloadResults.m_receivePeriodMs == 1, "Bandwidth test download results not cleared.");
					BD_ASSERT(m_seqNumberTotal == 0, "Bandwidth test download results not cleared.");

					// init download results
					m_downloadResults.m_avgSeqNumber = sequenceNumber;
					m_downloadResults.m_minSeqNumber = sequenceNumber;
					m_downloadResults.m_maxSeqNumber = sequenceNumber;
				}

				// +8 for UDP header
				m_downloadResults.m_bytesReceived += (received + 8);
				m_downloadResults.m_receivePeriodMs = static_cast<bdUInt32>(m_timer.getElapsedTimeInSeconds() * 1000);				
				m_downloadResults.m_minSeqNumber = BD_MIN(m_downloadResults.m_minSeqNumber, sequenceNumber);
				m_downloadResults.m_maxSeqNumber = BD_MAX(m_downloadResults.m_maxSeqNumber, sequenceNumber);

				// +8 for UDP header
				const bdUInt numPacketsReceived = m_downloadResults.m_bytesReceived / (m_packetSize + 8);
				m_seqNumberTotal += sequenceNumber;

				m_downloadResults.m_avgSeqNumber =  m_seqNumberTotal / numPacketsReceived;

				m_recvTimeoutTimer.start();
			}
			else
			{
				BD_WARN(BD_LOG_LEVEL, "Received malformed bandwidth test packet.");
			}
		}

	}
	while( received > 0 );

	// are we still waiting for the first packet
	if(m_testStatus == BD_BANDWIDTH_TEST_DOWNLOAD_WAITING)
	{
		// ensure we wait long enough for the server to start sending
		bdUInt serverReceiveTimeout = 0;
		if(m_receiveDuration > m_sendDuration)
		{	
			serverReceiveTimeout = m_receiveDuration - m_sendDuration;
		}

		const bdUInt firstPacketTimeout = m_receiverInitialWait + serverReceiveTimeout;
		if(m_timer.getElapsedTimeInSeconds() >= (firstPacketTimeout / 1000.0f))
		{
			BD_WARN(BD_LOG_LEVEL, "Download test timed out. Failed to receive any download packets in %.2fs.", firstPacketTimeout / 1000.0f);
			finalizeTest();
		}
	}

	// stop the test if the receive duration has elapsed and we have not received a packet
	// within the linger duration
	if(m_testStatus ==  BD_BANDWIDTH_TEST_DOWNLOAD_RECEIVING &&
	   m_timer.getElapsedTimeInSeconds() >= (m_receiveDuration / 1000.0f) &&
	   m_recvTimeoutTimer.getElapsedTimeInSeconds() >= ((m_downloadResults.m_receivePeriodMs + m_lingerDuration) / 1000.0f))
	{
		BD_INFO(BD_LOG_LEVEL, "Download test complete. Received %u bytes in %.2fs.", m_downloadResults.m_bytesReceived, m_downloadResults.m_receivePeriodMs / 1000.0f);
		finalizeTest();
	}
}


void bdBandwidthTestClient::finalizeTest()
{
	// fail safe
	m_error = BD_START_TASK_FAILED;

	// set the new task status
	m_testStatus = BD_BANDWIDTH_TEST_FINALIZING;

	if(BD_NULL == m_lobbyService)
	{
		BD_WARN(BD_LOG_LEVEL, "Finalize bandwidth test failed, lobby service invalid.");
	}
	else
	{
		bdRemoteTaskManager *const taskManager = m_lobbyService->getTaskManager();
		if(taskManager)
		{
			bdUByte8 buffer[sizeof(m_downloadResults)+1];

			bdUInt offset = 0;
			const bdUByte8 requestType = BD_BANDWIDTH_TEST_FINALIZE;
			bdBool ok = bdBytePacker::appendBasicType(buffer, sizeof(buffer), offset, offset, requestType);
			ok = ok && m_downloadResults.serialize(buffer+offset, sizeof(buffer)-offset);
			if(ok)
			{
				m_error = taskManager->startLSGTask(m_remoteTask,
													BD_LSG_SERVICE, 
													BD_BANDWIDTH_TEST, 
													buffer, 
													sizeof(buffer));
				if(m_error == BD_NO_ERROR)
				{						
					BD_INFO(BD_LOG_LEVEL, "Finalize bandwidth test task started.");
				}
				else
				{
					BD_WARN(BD_LOG_LEVEL, "Failed to start task for finalize bandwidth test.");
				}
			}
			else
			{
				BD_WARN(BD_LOG_LEVEL, "Failed to pack request buffer for finalize bandwidth test.");
			}
		}
		else
		{
			BD_WARN(BD_LOG_LEVEL, "Failed to retreive task manager from lobby service.");
		}
	}
}

void bdBandwidthTestClient::pumpFinalize()
{
	BD_ASSERT(m_remoteTask != BD_NULL, "bdBandwidthTestClient status is BD_BANDWIDTH_TEST_FINALIZING with null task handle.");
	BD_ASSERT(m_testStatus == BD_BANDWIDTH_TEST_FINALIZING, "bdBandwidthTestClient::pumpFinalize called in incorrect state.");

	// fail safe
	m_error = BD_HANDLE_TASK_FAILED;

	if(m_remoteTask && (m_testStatus == BD_BANDWIDTH_TEST_FINALIZING))
	{
		const bdRemoteTask::bdStatus taskStatus = m_remoteTask->getStatus();
		switch (taskStatus)
		{
			case bdRemoteTask::BD_EMPTY:
			{
				BD_ERR(BD_LOG_LEVEL, "bdBandwidthTestClient status is BD_BANDWIDTH_TEST_FINALIZING but task is bdRemoteTask::BD_EMPTY.");				
			}
			break;
			case bdRemoteTask::BD_PENDING:
			{
				// still requesting
				m_error = BD_NO_ERROR;
			}
			break;
			case bdRemoteTask::BD_DONE:
			{
				// handle the reply
				m_error = BD_NO_ERROR;
				const bdByteBufferRef replyBuffer = m_remoteTask->getByteResults();
				m_remoteTask = BD_NULL;

				handleFinalizeReply(replyBuffer);
			}
			break;
			case bdRemoteTask::BD_TIMED_OUT:
			{
				BD_WARN(BD_LOG_LEVEL, "bdBandwidthTestClient finalize request timed out.");
			}
			break;
			case bdRemoteTask::BD_MAX_STATUS:
			{
				BD_ERR(BD_LOG_LEVEL, "bdBandwidthTestClient finalize request invalid remote task state (bdRemoteTask::BD_MAX_STATUS).");
			}
			break;
			default:
			break;
		}
	}
}

void bdBandwidthTestClient::handleFinalizeReply(const bdByteBufferRef replyBuffer)
{
	// fail safe
	m_error = BD_HANDLE_TASK_FAILED;

	if(replyBuffer.notNull())
	{
		bdUByte8 finalizeFailed = 1;
		
		// parse the reply buffer
		bdBool ok = replyBuffer->read(finalizeFailed);
		if(ok && (finalizeFailed == 1))
		{
			// failed, read the error code	
			bdUInt16 errorCode(0);
			if(replyBuffer->read(errorCode))
			{
				m_error = static_cast<bdLobbyErrorCode>(errorCode);	
			}
			else
			{
				BD_ERR(BD_LOG_LEVEL, "Failed to parse bandwidth test error code.");
			}

			BD_INFO(BD_LOG_LEVEL, "Request for bandwidth finalize rejected (%u).", m_error);
		}

		if(ok && (finalizeFailed == 0))
		{
			ok = m_uploadResults.deserialize(replyBuffer);
			if(ok)
			{
				BD_INFO(BD_LOG_LEVEL, "Bandwidth test finalize complete.", m_error);
				m_error = BD_NO_ERROR;
				m_testStatus = BD_BANDWIDTH_TEST_DONE;
			}
		}
	}
}
