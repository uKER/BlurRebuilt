// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdPlatform/bdPlatformLog/bdPlatformLog.h>
#include <bdPlatform/bdPlatformMemory/bdPlatformMemory.h>
#include <bdPlatform/bdPlatformString/bdPlatformString.h>

#define BD_LOG_LEVEL "bdPlatformLog"

#if defined BD_PLATFORM_UNIX
#ifndef _SYS_SYSLOG_H
#include <syslog.h>
#endif
#define BD_LOG_UNIXSYSLOG_FACILITY LOG_LOCAL0
#define BD_LOG_UNIXSYSLOG_FLAGS LOG_PERROR+LOG_PID
#endif

bdLogSubscriber *g_logSubscriberList = BD_NULL;

bdLogSubscriber::bdLogSubscriber()
{
	bdMemset(m_channels, BD_NULL, sizeof(m_channels));

#ifdef BD_PLATFORM_UNIX
	bdMemset(m_processName, BD_NULL, sizeof(m_processName));
#endif // BD_PLATFORM_UNIX
}

bdLogSubscriber::~bdLogSubscriber()
{
}

bdBool bdLogSubscriber::addChannel(const bdNChar8 *const channel)
{
	bdBool added = false;
	for(bdUInt i = 0; !added && (i < BD_LOG_SUBSCRIBER_MAX_NUM_CHANNELS); i++)
	{
		if(m_channels[i] == BD_NULL)
		{
			added = true;
			m_channels[i] = channel;
		}
	}

	if(added && (false == bdLinkable::find(g_logSubscriberList, this)))
	{
		link(reinterpret_cast<bdLinkable**>(&g_logSubscriberList));
	}

	return added;
}

bdBool bdLogSubscriber::removeChannel(const bdNChar8 *const channel)
{
	bdBool removed = false;
	bdUInt numChannels = 0;
	for(bdUInt i = 0; i < BD_LOG_SUBSCRIBER_MAX_NUM_CHANNELS; i++)
	{
		if(m_channels[i] != BD_NULL)
		{
			// TODO: use bdStrcmp
			if(!removed && (bdStrcasecmp(m_channels[i], channel) == 0))
			{
				removed = true;
				m_channels[i] = BD_NULL;
			}
			else
			{
				numChannels++;
			}
		}
	}

	if(numChannels == 0)
	{
		unlink();
	}

	return removed;
}

void bdLogSubscriber::removeAllChannels()
{
	bdMemset(m_channels, BD_NULL, sizeof(m_channels));
	unlink();
}

void bdLogSubscriber::publish(const bdLogMessageType type,
							  const bdNChar8 *const,
							  const bdNChar8 *const file,
							  const bdNChar8 *const,
							  const bdUInt line,
							  const bdNChar8 *const msg) const
{
	const bdNChar8 *prefix = " ";
	const bdNChar8 *const warningPrefix = "\nWARNING: ";
	const bdNChar8 *const errorPrefix = "\nERROR: ";

	switch(type)
	{
		case BD_LOG_WARNING:
		{
			prefix = warningPrefix;
		}
		break;
		case BD_LOG_ERROR:
		{
			prefix = errorPrefix;
		}
		break;
		case BD_LOG_INFO:
		break;
	}

	const bdNChar8 *filename = bdStrrchr(file, '\\');
	if(filename == BD_NULL)
	{
		filename = file;
	}
	else
	{
		// skip the "\"
		filename++;
	}
#if defined BD_PLATFORM_UNIX
	const bdNChar8 *logPrefix = "bdLog";
	const bdUWord lenProcName = bdStrnlen(m_processName, sizeof(m_processName));
	if (lenProcName > 0)
	{
		logPrefix = m_processName;
	}

	openlog(logPrefix, BD_LOG_UNIXSYSLOG_FLAGS, BD_LOG_UNIXSYSLOG_FACILITY);

	syslog(LOG_INFO, "%s(%u):%s%s\n", filename, line, prefix, msg);

	closelog();

#elif defined BD_PLATFORM_XBOX
	bdNChar8 string[2048];
	const bdUWord stringLength = bdSnprintf(string, sizeof(string), "%s(%u):%s%s\n", filename, line, prefix, msg);
	if(stringLength >= sizeof(string))
	{
		BD_WARN(BD_LOG_LEVEL, "Log message truncated.");
	}
	OutputDebugString(string);
#else
	bdPrintf("%s(%u):%s%s\n", filename, line, prefix, msg);
#endif
}

void bdLogSubscriber::logMessage(const bdLogMessageType type,
								 const bdNChar8 *const channelName,
								 const bdNChar8 *const file,
								 const bdNChar8 *const function,
								 const bdUInt line,
								 const bdNChar8 *const msg) const
{
	for(bdUInt i = 0; i < BD_LOG_SUBSCRIBER_MAX_NUM_CHANNELS; i++)
	{
		const bdNChar8 *subscribedChannel = m_channels[i];
		if(m_channels[i] != BD_NULL)
		{
			const bdBool matchedChannel = bdDelimSubstr(channelName, subscribedChannel, "\\/");
			if(matchedChannel)
			{
				publish(type, channelName, file, function, line, msg);
			}
		}
	}
}

#ifdef BD_PLATFORM_UNIX
void bdLogSubscriber::setProcessName(const bdNChar8 *const processName)
{
	const bdUWord processNameLength = bdStrlcpy(m_processName, processName, sizeof(m_processName));
	if (processNameLength >= sizeof(m_processName))
	{
		BD_WARN(BD_LOG_LEVEL, "Process name truncated (%s).", m_processName);
	}
}
#endif // BD_PLATFORM_UNIX


void BD_CALL bdLogMessage(const bdLogMessageType type,
						  const bdNChar8 *const baseChannel,
						  const bdNChar8 *const channel,
						  const bdNChar8 *const file,
						  const bdNChar8 *const function,
						  const bdUInt line,
						  const bdNChar8 *const format,
						  ...)
{
	if(g_logSubscriberList)
	{
		const bdUInt messageSize = 256;
		bdNChar8 message[messageSize];
		bdMemset(message, BD_NULL, messageSize);

		bdVaList args;
		bdVaStart(args, format);

		const bdUWord messageLength = bdVsnprintf(message, messageSize, format, args);
		if(messageLength >= messageSize)
		{
			BD_WARN(BD_LOG_LEVEL, "Message truncated.", 0);
		}

		bdVaEnd(args);

		const bdNChar8 *channelName = baseChannel;
		const bdUInt channelNameBufferSize = 256;
		bdNChar8 channelNameBuffer[channelNameBufferSize];

		if(baseChannel && channel)
		{
			const bdInt channelNameLength = bdSnprintf(channelNameBuffer, channelNameBufferSize, "%s%s", baseChannel, channel);
			if(channelNameLength >= static_cast<bdInt>(channelNameBufferSize))
			{
				BD_WARN(BD_LOG_LEVEL, "Channel name truncated: %s%s", baseChannel, channel);
			}

			channelName = channelNameBuffer;
		}

		bdLogSubscriber* subscriber = g_logSubscriberList;
		while(subscriber)
		{
			subscriber->logMessage(type, channelName, file, function, line, message);

			subscriber = reinterpret_cast<bdLogSubscriber*>(subscriber->getNext());
		}
	}
}
