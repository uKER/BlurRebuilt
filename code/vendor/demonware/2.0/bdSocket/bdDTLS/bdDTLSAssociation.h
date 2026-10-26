// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 PS2 UNIX OSX IPHONE PSP-INFRA PS3 WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_DTLS_ASSOCIATION_H
#define BD_DTLS_ASSOCIATION_H

#include <bdCore/bdCore.h>

#include <bdCore/bdContainers/bdNonce.h>
#include <bdCore/bdCrypto/bdCypher3Des.h>
#include <bdSocket/bdDTLS/bdPacket/bdDTLSError.h>
#include <bdSocket/bdAddrHandle.h>
#include <bdSocket/bdDTLS/bdPacket/bdDTLSInitAck.h>

class bdDTLSInit;
class bdDTLSCookieEcho;
class bdDTLSData;
class bdSecurityKeyMap;
class bdECCKey;
class bdAddressMap;


// Size of the cookie key in bytes.
#define BD_SOCKET_COOKIE_KEY_SIZE (16)

enum bdDTLSAssociationStatus
{
	BD_SOCKET_IDLE,
	BD_SOCKET_PENDING,
	BD_SOCKET_CONNECTED,
	BD_SOCKET_LOST
};

/// A secure and NAT traversed association between 2 endpoints.
/// The setup and management of these associations is performed by the 
/// bdSocketRouter class.
class bdDTLSAssociation
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS;

		bdDTLSAssociation(	bdDefaultSocket *socket,
							bdSecurityKeyMap *keyMap,
							bdECCKey *key,
							const bdAddr &addr,
							bdAddrHandleRef addrHandle,
							bdCommonAddrRef localCommonAddr,
							bdAddressMap *addrMap,
							bdFloat32 receiveTimeout);

		/// Virtual Destructor
		virtual ~bdDTLSAssociation();

		void connect();

		bdDTLSAssociationStatus getStatus();

		/// Send data to addr.
		/// Data to send needs to be in a particular format. Data consists of
		/// encryptable and plain parts. Plain part is sent in the clear. It should
		/// follow the encrypted data part in the buffer. First two bytes of the
		/// buffer specify the size of the encrypted part (these two bytes are not
		/// counted).
		/// \param addr[in]		The address and port to send the data to.
		/// \param data[in]		A pointer to the data.
		/// \param length[in]	The length of the data.
		/// \return An signed integer representing:
		///  - If positive, the amount of amount of data sent.
		///  - If negative then an error from b	dSocketStatusCode.
		virtual bdInt	sendTo(const bdAddr &addr,
								const void *data,
								const bdUInt length);

		/// Receive data.
		/// Data is received in a particular format. See \a sendTo() for description.
		/// Make sure size is greater than the max packet size or data will be
		/// lost.
		/// \param addr[out]	The address of the sender.
		/// \param data[out]	A pointer to a buffer into which the data read
		///						from the socket will be read.
		/// \param size[out]
		/// \param addrHandle[out]
		/// \param buffer[out]
		/// \param bufferSize[out]
		/// \return An signed integer representing:
		///  - If positive, the amount of amount of data read into buffer.
		///  - If negative then an error from bdSocketStatusCode.
		virtual bdInt	receiveFrom(bdAddr &addr,
									const void *data,
									const bdUInt size,
									bdAddrHandleRef &addrHandle,
									void *buffer,
									const bdUInt bufferSize);


		void pump();

		bdAddrHandleRef getAddrHandle();

		const bdSecurityID* getLocalSecurityId();

		bdUInt prepareData(const void *data, const bdUInt length, void *outData, const bdUInt outDataSize);

	protected:

		enum bdDTLSStatus
		{
			BD_DTLS_CLOSED = 0,
			BD_DTLS_COOKIE_WAIT,
			BD_DTLS_COOKIE_ECHOED,
			BD_DTLS_ESTABLISHED
		};

	protected:

		bdInt handleInit(const bdAddr &addr, const void *data, const bdUInt size);

		bdInt handleInitAck(const bdAddr &addr, const void *data, const bdUInt size);

		bdInt handleCookieEcho(const bdAddr &addr, const void *data, const bdUInt size);

		bdInt handleCookieAck(const void *data, const bdUInt size);

		bdInt handleError(const bdAddr &addr, const void *data, const bdUInt size);

		bdInt handleData(bdAddr &addr,
						const bdUByte8 *data,
						const bdUInt size,
						bdAddrHandleRef &addrHandle,
						bdUByte8 *buffer,
						const bdUInt bufferSize);

		void sendInit();

		void sendInitAck(const bdAddr &addr, const bdDTLSInit &init);

		void sendCookieEcho(const bdAddr &addr);

		void sendCookieAck(const bdAddr &addr, const bdDTLSCookieEcho &cookieEcho);

		void sendError(const bdAddr &addr, const bdSecurityID &secID, const bdDTLSError::bdDTLSErrorType &type);

		bdInt sendData(const bdAddr &addr, const void *data, const bdUInt length);

		void reset();

		bdBool calculateSharedKey(	const bdUByte8 *const pubKey,
									const bdUInt keylen,
									const bdSecurityID &secID);

		bdBool verify(const bdDTLSData& dataHeader);

		bdBool checkCookieValidity(const bdAddr &peerAddr, const bdDTLSCookieEcho &cookiePacket) const;

	protected:

		bdDefaultSocket *m_socket;

		bdSecurityKeyMap *m_keyMap;

		bdECCKey *m_ECCKey;

		bdCypher3Des m_cypher;

		bdUByte8 m_sharedKey[BD_SHARED_KEY_SIZE];

		// the real address.
		bdAddr m_addr;

		// the address handle.
		bdAddrHandleRef m_addrHandle;

		bdDTLSStatus m_state;

		bdStopwatch m_lastReceived;

		bdStopwatch m_initTimer;
		bdUByte8 m_initResends;

		bdStopwatch m_cookieTimer;
		bdUByte8 m_cookieResends;

		bdUInt16 m_localTag;
		bdUInt16 m_peerTag;

		bdSequenceNumber m_seqNum;

		bdSequenceNumberStore m_incomingSeqNums;

		bdDTLSInitAck m_initAck;

		static bdNonce<BD_SOCKET_COOKIE_KEY_SIZE> m_cookieKey;

		bdSecurityID m_localId;

		bdCommonAddrRef m_localCommonAddr;

		bdAddressMap *m_addrMap;

		bdFloat32 m_receiveTimeout;
};

#endif // BD_DTLS_ASSOCIATION_H
