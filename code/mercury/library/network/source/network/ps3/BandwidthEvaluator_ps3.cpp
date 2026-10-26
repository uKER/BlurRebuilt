#include <StdAfx.h>
#include <Network\Core.h>
#include <Network\BandwidthEvaluator.h>


namespace NetPlay
{
	// constructor
	C_BandwidthEvaluator::C_BandwidthEvaluator(const U32 minUpstream, const U32 minDownstream) :m_UpStream(minUpstream), m_DownStream(minDownstream), 
		m_MinUpStream(minUpstream), m_MinDownStream(minDownstream),
		m_pClient(NULL), m_Delaying(false), m_RetryCount(0)

	{
	}


	// destructor
	C_BandwidthEvaluator::~C_BandwidthEvaluator()
	{
		Stop();
	}

	bool C_BandwidthEvaluator::Start(void)
	{
		Stop();
		m_pClient = new bdBandwidthTestClient;
		m_pTimer = new bdStopwatch;

		if (m_pClient && m_pTimer)
		{
			bdLobbyService* lobby = NetPlay::C_Core::Instance().DemonwareLobbyService();
			if (lobby && lobby->getStatus() == bdLobbyConnection::BD_CONNECTED && m_pClient->init(lobby))
			{
				m_pClient->start(BD_UPLOAD_DOWNLOAD_TEST);
				if (BD_NO_ERROR == m_pClient->getErrorCode())
					return true;
				else
					Stop();
			}
			else
				Stop();
		}
		return false;
	}

	void C_BandwidthEvaluator::Update(void)
	{
		if (!IsActive())
			return;

		if (m_Delaying)
		{
			if (m_pTimer->getElapsedTimeInSeconds() >= 3.0f)
			{
				m_pClient->start(BD_UPLOAD_DOWNLOAD_TEST);
				if (BD_NO_ERROR == m_pClient->getErrorCode())
					m_Delaying = false;
				else
					Stop(); // unable to  restart scan so just stop it.
			}
		}
		else
		{
			m_pClient->pump();

			const bdLobbyErrorCode ec = m_pClient->getErrorCode();
			if (BD_BANDWIDTH_TEST_STILL_IN_PROGRESS == ec)
				return;

			if (BD_NO_ERROR == ec)
			{
				if (BD_BANDWIDTH_TEST_DONE == m_pClient->getStatus())
				{
					const U32 up =  ((U32)m_pClient->getUploadResults().getBandwidth())*1024;
					const U32 down = ((U32)m_pClient->getDownloadResults().getBandwidth())*1024;

					m_UpStream		= up > m_MinUpStream ? up : m_MinUpStream;
					m_DownStream	= down > m_MinDownStream ? down : m_MinDownStream;

					Stop();
				}
			}
			else if (BD_BANDWIDTH_TEST_TRY_AGAIN == ec)
			{
				// delay for a few seconds and rey again.
				m_RetryCount += 1;
				if (m_RetryCount > 3)
					Stop();
				else
				{
					m_pClient->stop();
					m_pTimer->start();
					m_Delaying = true;
				}
			}
			else
				Stop();
		}
	}

	bool C_BandwidthEvaluator::IsActive(void)
	{
		return NULL == m_pClient ? false : true; 
	}

	void C_BandwidthEvaluator::Stop(void)
	{
		if (m_pClient)
		{
			delete m_pTimer;
			delete m_pClient;
			m_pClient = NULL;
			m_Delaying = false;
			m_RetryCount = 0;
		}
	}
}
