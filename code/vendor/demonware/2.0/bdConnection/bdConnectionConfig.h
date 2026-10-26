// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#ifndef BD_CONNECTION_CONFIG_H
#define BD_CONNECTION_CONFIG_H

/// Defines the maximum number of messages that can be queued
/// in the connection at any one time.
#define BD_MAX_WINDOW_SIZE bdSeqNumber(128u)

/// The default window receive credit.
#define BD_DEFAULT_RECEIVE_WINDOW_CREDIT	(15000u)

/// The minimum retransmission timeout for connections.
#define BD_UC_RTO_MIN	(.02f)
/// The maximum retransmission timeout for connections.
#define BD_UC_RTO_MAX	(2.f)

/// Retransmission timeout is calculated as follows:
/// RTO <- SRTT + (MULTIPLIER * RTTVAR).
/// Where RTO is Retransmission timeout, SRTT is smoothed round
/// trip time multiplier is defined below and RTTVAR is round
/// trip time variation.
#define BD_RT0_MULTIPLIER (2.f)

/// When a chunk is indicated as missing (from a gap ACK in a
/// SACK chunk) it's missing count is incremented. When the
/// missing count hits a threshold it is retransmitted.
#define BD_FAST_RETRANSMIT_THRESH	(3)

/// Defines the default start values for timeout periods.
/// The timeout period is the amount of time (in milliseconds)
/// spent waiting for an ACK before deciding the packet is lost and resending.
#define BD_RTT_START_VALUE  (.3f)

/// Defines the alive period timeout. The alive period specifies how long (in seconds)
/// a connection should stay open after last received data.
#ifdef BD_DEBUG
#	define BD_ALIVE_PERIOD		   (30*60.f)
#else
#	define BD_ALIVE_PERIOD		   (9.f)
#endif //BD_DEBUG

#endif //BD_CONNECTION_CONFIG_H

