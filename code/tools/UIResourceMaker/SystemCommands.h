#pragma once

#define MAX_SYSTEM_COMMANDS 1024

class SystemCommands
{
public:
	SystemCommands(){mNumCommands=0;};

	void AddCommand(char *comm);
	void Execute();
	void Flush(){mNumCommands=0;};

private:
	char mCommands[MAX_SYSTEM_COMMANDS][256];
	int mNumCommands;
};

extern SystemCommands gSystemCommands;
