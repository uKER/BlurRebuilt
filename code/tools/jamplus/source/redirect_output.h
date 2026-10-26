#ifndef __REDIRECT_OUTPUT_H__
#define __REDIRECT_OUTPUT_H__

#include <sstream>

typedef unsigned __int64 U64;

// class for spawning a process with redirected output, then parsing the output line by line and exiting cleanly
class RedirectedProcess
{
public:
	RedirectedProcess();

	std::string GetOutput();

	// run the commandline with redirected output
	void RunCommandlineRedirect(const std::string &commandline, bool bConvertErrors);

	// handle incoming redirected output. returns false if the process exited
	bool ReadAndHandleOutput();

	// returns the handle to this process
	HANDLE GetHandle();

	// close the process and return the exit code
	DWORD Close();

	// stop timing
	void StopTiming();

private:
	void PrepAndLaunchRedirectedChild(HANDLE hChildStdOut, HANDLE hChildStdIn, HANDLE hChildStdErr, char *buffer);
	void DisplayError(char *pszAPI);

	// handles to various things
	HANDLE m_hChildProcess;
    HANDLE m_hOutputRead, m_hOutputWrite;
    HANDLE m_hInputRead, m_hInputWrite;
    HANDLE m_hErrorWrite;
	int m_iLine;
	U64 m_iStartTime;
	std::string m_sFirstLine;

	// input buffer
	CHAR m_line_buf[256*16];
	int m_iBufPos;
	bool m_bConvertErrors;

	std::stringstream m_output;
};

#endif
