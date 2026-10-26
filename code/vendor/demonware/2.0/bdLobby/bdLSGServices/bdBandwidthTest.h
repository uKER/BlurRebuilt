// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS
// PURPOSE:

#ifndef BD_BANDWIDTH_TEST_H
#define BD_BANDWIDTH_TEST_H

#include <bdCore/bdCore.h>

#include <bdLobby/bdLobbyCommon/bdLobbyServicesCommon.h>
#include <bdLobby/bdLSGServices/bdBandwidthTestResults.h>
#include <bdLobby/bdLobbyService.h>

BD_REFERENCE(bdRemoteTask);

/// The types of bandwidth test that can be performed.
/// - BD_UPLOAD_TEST, test only the client upload bandwidth.
/// - BD_UPLOAD_DOWNLOAD_TEST, test both the clients upload and download bandwidth.
enum bdBandwidthTestType
{
	BD_UPLOAD_TEST = 0,
	BD_UPLOAD_DOWNLOAD_TEST = 1
};

/// Internal status codes.
/// User to record and check that the bdBandwidthTestClient has been 
/// initialized. Once initialized (bdBandwidthTestClient::init) any number of
/// bandwidth tests can be run sequentially.
enum bdBandwidthTestInitStatus
{
	BD_BANDWIDTH_TEST_UNINITIALIZED,
	BD_BANDWIDTH_TEST_INITIALIZED
};

/// Codes representing the various stages of bandwidth testing.
/// The client game need only test for !BD_BANDWIDTH_TEST_DONE to check if a 
/// test is still in progress. There is no internal timeout in bdBandwidthTestClient.
enum bdBandwidthTestStatus 
{
	BD_BANDWIDTH_TEST_IDLE,
	BD_BANDWIDTH_TEST_REQUESTING_TEST,
	BD_BANDWIDTH_TEST_UPLOAD_WAITING,
	BD_BANDWIDTH_TEST_UPLOAD_SENDING,
	BD_BANDWIDTH_TEST_DOWNLOAD_WAITING,
	BD_BANDWIDTH_TEST_DOWNLOAD_RECEIVING,
	BD_BANDWIDTH_TEST_FINALIZING,
	BD_BANDWIDTH_TEST_DONE
};

/// The bdBandwidthTestClient API provides quality of service measurements for 
/// upstream bandwidth, or upstream and downstream bandwidth. Bandwidth tests are 
/// useful for selecting which client should act as the server for client 
/// server games, or ensuring all participants meet minimum bandwidth 
/// requirements. Many of the settings used by these tests are specified by 
/// the DemonWare lobby service gateway (LSG), to which you must be connected 
/// to run any test. These server side settings allow modifications of the 
/// test behavior after a game has shipped. Currently the only client 
/// side setting is the type of test performed, upstream or upstream and 
/// downstream. The results of bandwidth tests can also be recorded in the 
/// DemonData data warehouse for report generation.
class bdBandwidthTestClient
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Default constructor.
		/// Before use a bdBandwidthTestClient must be initialized with a call to
		/// bdBandwidthTestClient::init.
		bdBandwidthTestClient();

		/// Destructor.
		/// Will terminate any currently active test, and clean up by calling
		/// bdBandwidthTestClient::quit.
		~bdBandwidthTestClient();

		/// This function must be called before any test are run. After a successful call
		/// any number of sequential tests can be run. Running two or more tests in parallel is
		/// not supported. Multiple parallel tests with more that one bdBandwidthTestClient 
		/// are not recommended.
		/// \param lobbyService[in] A pointer to the bdLobbyService object.
		/// \return
		/// - true, if successful.
		/// - false otherwise, if initialization was unsuccessful calling 
		///   bdBandwidthTestClient::getErrorCode will return a detailed error code.
		///   Calling bdBandwidthTestClient::init on a already initialized bdBandwidthTestClient
		///   instance will return false.
		bdBool init(bdLobbyService *const lobbyService);

		/// Cleanup.
		/// Shutdown the bdBandwidthTestClient and release any system resources.
		/// bdBandwidthTestClient::init may be called after bdBandwidthTestClient::quit.
		void quit();

		/// Initiate a specified type of bandwidth test.
		/// Always check bdBandwidthTestClient::getErrorCode after calling 
		/// bdBandwidthTestClient::start to ensure the test is running, a error code of
		/// BD_NO_ERROR indicates the test has started. You can check the progress of the 
		/// test at any time by calling bdBandwidthTestClient::getStatus.
		/// The bdBandwidthTestClient does not run in a thread, and so must be pumped to
		/// completion or error. It is recommended that the test is pumped each frame or in a 
		/// blocking loop, using bdBandwidthTestClient::pump.
		/// \param type[in] The type of bandwidth test to be performed.
		void start(const bdBandwidthTestType type);

		/// Terminate the currently running test (client side only).
		/// Ends any test that is in progress and sets up the bdBandwidthTestClient
		/// ready for bdBandwidthTestClient::start to be called.
		/// Because bandwidth test require state to be stored on the LSG subsequent restarts of
		/// a bandwidth test may result in errors (BD_BANDWIDTH_TEST_STILL_IN_PROGRESS) until
		/// the test times out on the LSG.
		void stop();

		/// Return the progress/status of a test.
		/// The status of a test is progressed by calling bdBandwidthTestClient::pump.
		/// Completion of a test is indicated by the BD_BANDWIDTH_TEST_DONE status. 
		/// For every call to bdBandwidthTestClient::getStatus it is also wise to call 
		/// bdBandwidthTestClient::getErrorCode and check for BD_NO_ERROR.
		bdBandwidthTestStatus getStatus() const;

		/// Progress the currently running test.
		/// The bdBandwidthTestClient does not run in a thread, and so must be pumped to
		/// completion or error. It is recommended that the test is pumped each frame or in a 
		/// blocking loop.
		/// The progress of the test can be monitored with bdBandwidthTestClient::getStatus 
		/// and errors should be check after each pump using bdBandwidthTestClient::getErrorCode.
		void pump();

		/// Fetch the current error code.
		/// \return an error code, if no errors have occurred BD_NO_ERROR is returned.
		bdLobbyErrorCode getErrorCode() const;

		/// Return the type of test currently running.
		/// This is only valid after bdBandwidthTestClient::start has been called.
		/// The value returned by bdBandwidthTestClient::getTestType should not be used as
		/// and indicator that a test is running. Instead use bdBandwidthTestClient::getStatus 
		/// and bdBandwidthTestClient::getErrorCode.
		/// \return The type of test running, if no test is active the value is undefined.
		bdBandwidthTestType getTestType() const;

		/// Fetch the results of a completed upload test.
		/// This function should only be called once bdBandwidthTestClient::getStatus
		/// returns BD_BANDWIDTH_TEST_DONE and bdBandwidthTestClient::getErrorCode returns
		/// BD_NO_ERROR. Otherwise the results are undefined.
		/// \return	A instance of bdBandwidthTestResults containing the results of the
		//			most recent upload test.
		bdBandwidthTestResults& getUploadResults();

		/// Fetch the results of a completed download test.
		/// This function should only be called once bdBandwidthTestClient::getStatus
		/// returns BD_BANDWIDTH_TEST_DONE and bdBandwidthTestClient::getErrorCode returns
		/// BD_NO_ERROR. Otherwise the results are undefined.
		/// \return	A instance of bdBandwidthTestResults containing the results of the
		//			most recent download test.
		bdBandwidthTestResults& getDownloadResults();

	protected:

		void pumpRequest();

		void handleRequestReply(const bdByteBufferRef replyBuffer);

		void startUploadTest();

		void pumpUploadWait();

		void pumpUploadSend();

		void startDownloadTest();

		void pumpDownloadReceive();
		
		void finalizeTest();

		void pumpFinalize();

		void handleFinalizeReply(const bdByteBufferRef replyBuffer);

	protected:

		bdSocket m_socket;

		bdBandwidthTestType m_type;

		bdBandwidthTestInitStatus m_initStatus;
		bdBandwidthTestStatus m_testStatus;

		bdLobbyErrorCode m_error;

		bdRemoteTaskRef m_remoteTask;

		bdUInt32 m_packetSize;
		bdUInt32 m_numPackets;
		bdUInt32 m_senderInitialWait;
		bdUInt32 m_sendDuration;
		bdUInt32 m_receiverInitialWait;
		bdUInt32 m_receiveDuration;
		bdUInt32 m_lingerDuration;
		bdUInt16 m_lsgPort;
		bdUInt32 m_lsgAddr;
		bdUByte8 m_cookie[8];

		bdStopwatch m_timer;
		bdStopwatch m_recvTimeoutTimer;

		bdUByte8 *m_packetBuffer;
		bdUInt32 m_nextPacketNum;
		bdUInt32 m_actualNumSent;

		bdUInt32 m_seqNumberTotal;

		bdBandwidthTestResults m_uploadResults;
		bdBandwidthTestResults m_downloadResults;
		bdLobbyService *m_lobbyService;
		
};


#endif // BD_BANDWIDTH_TEST_H
