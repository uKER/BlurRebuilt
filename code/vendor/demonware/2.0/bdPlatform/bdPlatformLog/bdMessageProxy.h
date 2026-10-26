// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE:

#ifndef BD_MESSAGE_PROXY_H
#define BD_MESSAGE_PROXY_H

#include <bdPlatform/bdPlatformCoreTypes/bdPlatformCoreTypes.h>
#include <bdPlatform/bdPlatformLog/bdPlatformLog.h>

/// A helper class allowing macros to have variable length parameter
/// lists.
/// This class is only used on compilers that don't support variadic macros.
/// NOTE: This class will not be allocated through bdMemory, so it is recommended
/// you allocate it a global scope, or create it on the stack hence avoiding
/// heap allocation. Alternatively make it a data member of another memory
/// managed class, or a base class of another memory managed class.
/// In short bdMessageProxy *myProxy = new bdMessageProxy, will not use bdMemory.
class bdMessageProxy
{
	public:

		/// Constructor
		/// \param type	The type of message to be logged.
		/// \param file	The path of the file containing the log message, may be NULL.
		/// \param function	The name of the function producing the log message, may be NULL.
		/// \param line	The line number of the logging call.
		/// \param baseChannel	The root channel to which the log message is to be published.
		bdMessageProxy(const bdLogMessageType type,
					   const bdNChar8 *const file, 
					   const bdNChar8* const function, 
					   const bdUInt line, 
					   const bdNChar8 *const baseChannel);

		/// Publishes a message to the specified \a channel.
		/// \param channel	The name to the channel, not including the root channel name (see constructor).
		///					The concatenation of the root channel and /a channel make up the full channel name.
		/// \param format	The message to be logged in printf style.
		/// \param ...	Optional printf parameters.
		void log(const bdNChar8 *const channel, 
				 const bdNChar8 *const format, 
				 ...) const;

		/// Tests an assert expression. 
		/// Publishes a message and stop the program if the expression evaluates to false.
		/// \param expr	The expression to evaluate.
		/// \param format	The message to be logged in printf style, if \a expr evaluates to false.
		/// \param ...	Optional printf parameters.
		void handleAssert(const bdBool expr, 
						  const bdNChar8 *const format, 
						  ...) const;

	public:

		bdLogMessageType m_logMessageType;
		const bdNChar8 *m_file;
		const bdNChar8 *m_function;
		bdUInt m_line;
		const bdNChar8 *m_baseChannel;
};

#endif // BD_PLATFORM_ERROR_H
