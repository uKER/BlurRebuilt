// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: 

#ifndef BD_PLATFORM_LOG_H
#define BD_PLATFORM_LOG_H

#include <bdPlatform/bdPlatformCoreTypes/bdPlatformCoreTypes.h>
#include <bdPlatform/bdPlatformPatterns/bdLinkable.h>

#define BD_LOG_SUBSCRIBER_MAX_NUM_CHANNELS (5u)

// Defines the available log levels.
// Each log level corresponds to a root log channel.
enum bdLogMessageType
{
	BD_LOG_INFO,		// corresponds to the "info" channel
	BD_LOG_WARNING,		// corresponds to the "warn" channel
	BD_LOG_ERROR		// corresponds to the "err" channel
};

// Publishes logs messages for up to BD_LOG_SUBSCRIBER_MAX_NUM_CHANNELS
// registered log channels.
// Inherit from this class and overload the publish virtual member function
// to control the destination and format of log messages.
// NOTE: This class will not be allocated through bdMemory, so it is recommended
// you allocate it a global scope, or create it on the stack hence avoiding
// heap allocation. Alternatively make it a data member of another memory
// managed class, or a base class of another memory managed class.
// In short bdLogSubscriber *myLogSubscriber = new bdLogSubscriber, will not use bdMemory.
class bdLogSubscriber : public bdLinkable
{
	public:

		/// Default constructor
		bdLogSubscriber();

		/// Virtual Destructor
		virtual ~bdLogSubscriber();

		/// Register a log channel with this subscriber.
		/// Any messages logged to \a channel or any channel of which \a channel is a
		///	sub channel will be published through this subscriber, via the virtual
		/// publish method.
		/// For example: adding the channel "info/socket" will cause all message logged to
		/// "info/socket/..." to published through this subscriber. However messages logged to
		/// "info" will not be published.
		/// There are no restrictions on what channels can be registered, in particular the same
		/// channel can be registered twice and will result in two calls to the publish method.
		/// \param channel	The full channel path to subscribe to, may be the empty string "".
		/// \return 
		/// - True if the channel was added.
		/// - False if the channel could not be added. This is usually an indication that 
		/// BD_LOG_SUBSCRIBER_MAX_NUM_CHANNELS have already be registered.
		bdBool addChannel(const bdNChar8 *const channel);

		/// Unregister all matching channels from this subscriber.
		/// It the subscriber is registered to the same channel only one
		/// instance of the channel will be removed. Hence calls to removeChannel should
		/// has equivalent calls to addChannel.
		/// \param channel	The full channel path to unsubscribe from.
		/// - True if the channel was removed.
		/// - False if the channel was not found.
		bdBool removeChannel(const bdNChar8 *const channel);

		/// Unregister all channels from this subscriber.
		void removeAllChannels();

		/// Output a log message.
		/// One call to publish will be made for each message logged to a subscribed channel.
		/// For example a message logged to a channel named "info/socket" will result in a 
		/// call to publish if the subscriber is subscribed to "info" or "info/socket". If
		/// the subscriber is subscribed to both "info" and "info/socket" two calls will be made.
		/// Overload this function to output message to a different destination or format eg the 
		/// network or a file.
		/// \param type The type of the message being logged, typically used to prefix the log 
		///				message with a fixed string eg "Error: ".
		/// \param channelName	The full channel path/name.
		/// \param file	The name of the file from which the message originated, may be NULL.
		/// \param function	The name of the function from which the message originated, may be NULL.
		/// \param line	The line number from which the message originated.
		/// \param msg The message to be published.
		virtual void publish(const bdLogMessageType type,
							 const bdNChar8 *const channelName,
							 const bdNChar8 *const file,
							 const bdNChar8 *const function,
							 const bdUInt line,
							 const bdNChar8 *const msg)const;

		/// Publish a log message to any relevant subscribed channels.
		/// Iterates all subscribed channels calling \a publish for any that match
		/// \a channelName. A matching channel is one that is a subchannel of \a channelName.
		/// "info" is a subchannel or "info\socket", but "info\socket" in not a sub channel of "info".
		/// The empty string "" is a subchannel of all channels. Channel delimiters are '\' and '/'.
		/// \param type The type of the message being logged, typically used to prefix the log 
		///				message with a fixed string eg "Error: ".
		/// \param channelName	The full channel path/name.
		/// \param file	The name of the file from which the message originated, may be NULL.
		/// \param function	The name of the function from which the message originated, may be NULL.
		/// \param line	The line number from which the message originated.
		/// \param msg The message to be published.
		void logMessage(const bdLogMessageType type,
						const bdNChar8 *const channelName,
						const bdNChar8 *const file,
						const bdNChar8 *const function,
						const bdUInt line,
						const bdNChar8 *const msg)const;

#ifdef BD_PLATFORM_UNIX
		/// Unix only: sets a string the the \a publish method can optionally 
		/// write when outputting a message.
		/// The string is copied into \a m_processName.
		/// \processName A string to be stored in \a m_processName.
		void setProcessName(const bdNChar8 *const processName);
#endif // BD_PLATFORM_UNIX

	protected:

		const bdNChar8* m_channels[BD_LOG_SUBSCRIBER_MAX_NUM_CHANNELS];	

#ifdef BD_PLATFORM_UNIX
		bdNChar8 m_processName[256];
#endif // BD_PLATFORM_UNIX

};

/// Output a message via any subscribed listeners.
/// The message will be published to any bdLogSubscriber instances that are subscribed to 
/// subchannels of baseChannel\channel. For example "info" is a subchannel or "info\socket", 
/// but "info\socket" in not a sub channel of "info". The empty string "" is a subchannel 
/// of all channels.
/// \param type The type of the message being logged, typically used to prefix the log 
///				message with a fixed string eg "Error: ".
/// \param baseChannel	The first element in the channel name, typically "info", "warn" or "err".
/// \param channel	The remaining elements of the full log channel name. \a baseChannel is prepended 
///					to channel to create the full channel name.
/// \param file	The name of the file from which the message originated, may be NULL.
/// \param function	The name of the function from which the message originated, may be NULL.
/// \param line	The line number from which the message originated.
/// \param format The message to be published in printf format.
/// \param ...	Optional printf parameters.
void BD_CALL bdLogMessage(const bdLogMessageType type,
						  const bdNChar8 *const baseChannel,
						  const bdNChar8 *const channel,
						  const bdNChar8 *const file,
						  const bdNChar8 *const function,
						  const bdUInt line,
						  const bdNChar8 *const format,
						  ...);

#if defined BD_COMPILER_MSVC7 || defined BD_COMPILER_MSVC_XBOX

// Compiler does not support variadic macros

#include <bdPlatform/bdPlatformLog/bdMessageProxy.h>

/// Marco to log a printf style message to the "info" channel.
/// Usage BD_INFO(infoChannelName, format, ...);
#define BD_INFO \
	bdMessageProxy(BD_LOG_INFO, __FILE__, __FUNCTION__, __LINE__, "info/").log 

/// Marco to log a printf style message to the "warn" channel.
/// Usage BD_WARN(warningChannelName, format, ...);
#define BD_WARN \
	bdMessageProxy(BD_LOG_WARNING, __FILE__, __FUNCTION__, __LINE__, "warn/").log 

/// Marco to log a printf style message to the "err" channel.
/// Usage BD_ERR(errorChannelName, format, ...);
#define BD_ERR \
	bdMessageProxy(BD_LOG_ERROR, __FILE__, __FUNCTION__, __LINE__, "err/").log 

#elif defined BD_COMPILER_CODEWARRIOR

/// Marco to log a printf style message to the "info" channel.
/// Usage BD_INFO(infoChannelName, format, ...);
#define BD_INFO(channelName, ...) \
	bdLogMessage( BD_LOG_INFO, "info/", channelName, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

/// Marco to log a printf style message to the "warn" channel.
/// Usage BD_WARN(warningChannelName, format, ...);
#define BD_WARN(channelName, ...) \
		bdLogMessage( BD_LOG_WARNING, "warn/", channelName, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

/// Marco to log a printf style message to the "err" channel.
/// Usage BD_ERR(errorChannelName, format, ...);
#define BD_ERR(channelName, ...) \
	bdLogMessage( BD_LOG_ERROR, "err/", channelName, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

#else // BD_COMPILER_CODEWARRIOR

// Compiler does support variadic macros
// Note ## is not required on win32 or Xbox platforms

/// Marco to log a printf style message to the "info" channel.
/// Usage BD_INFO(infoChannelName, format, ...);
#define BD_INFO(channelName, msg, ...) \
	bdLogMessage( BD_LOG_INFO, "info/", channelName, __FILE__, __FUNCTION__, __LINE__, msg, ## __VA_ARGS__)

/// Marco to log a printf style message to the "warn" channel.
/// Usage BD_WARN(warningChannelName, format, ...);
#define BD_WARN(channelName, msg, ...) \
	bdLogMessage( BD_LOG_WARNING, "warn/", channelName, __FILE__, __FUNCTION__, __LINE__, msg, ## __VA_ARGS__)

/// Marco to log a printf style message to the "err" channel.
/// Usage BD_ERR(errorChannelName, format, ...);
#define BD_ERR(channelName, msg, ...) \
	bdLogMessage( BD_LOG_ERROR, "err/", channelName, __FILE__, __FUNCTION__, __LINE__, msg, ## __VA_ARGS__)

#endif // BD_COMPILER_CODEWARRIOR

#endif // BD_PLATFORM_LOG_H
