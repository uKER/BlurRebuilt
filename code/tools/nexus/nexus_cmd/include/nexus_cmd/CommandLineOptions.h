#ifndef NEXUS_CMD_COMMAND_LINE_OPTIONS_H
#define NEXUS_CMD_COMMAND_LINE_OPTIONS_H

namespace NexusCmd
{
struct CommandLineOptions
{
	std::string commandString;
	std::string commandFile;
	std::string logFile;
	bool overwriteLog;
};
}

#endif
