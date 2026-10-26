#ifndef __MERCURY_TOOLS_APPLICATION_H__
#define __MERCURY_TOOLS_APPLICATION_H__

// forward declarations
namespace Debug
{
	class Logger;
	class ExecuterFeedback;
};
namespace Progress
{
	class I_ProgressBar;
};

namespace Tools
{
	/// base class for tools applications
	class C_ToolsApplication
	{
	public:
		/// constructor
		C_ToolsApplication(
			const char *szName,			// the name of the application
			const char *szSubtitle,		// subtitle e.g. "bizarre creations lighting tool"
			const char *szLogger,		// name of the base logger for this application
			const char *szDmpPath,		// path where .dmp files are to be written out to
			bool bExecuter = false		// true if the application needs to send executer feedback
			);

		/// destructor
		virtual ~C_ToolsApplication();

		/// run the application
		S32	Run(U32 argc, char **argv, bool bAutoConfigDebugLog = true);

		/// main startup function of the tool (lite version which starts more quickly)
		S32 RunLite(U32 argc, char **argv);

		Debug::ExecuterFeedback* GetExecuterFeedbackPtr();

		bool IsRunLocally() const;

		bool IsUsingExecuter() const;

		const char* GetName() const;

		const char* GetDumpPath() const;

		const char* GetSubtitle() const;

		Debug::Logger& GetLogger();

	protected:
		/// main callback function to be overloaded by derived class
		virtual	S32 Main(S32 argc, char **argv, Progress::I_ProgressBar *pProgress) = 0;

		const char* m_pszApplicationName;
		const char* m_pszDmpPath;
		const char* m_pszApplicationSubtitle;
		const bool	m_bExecuter;
		bool		m_bRunLocally;	// false if the application should exit silently when a fatal exception occurs

		Debug::Logger&				m_log;
		Debug::ExecuterFeedback*	m_pExecuter;
	};

	extern C_ToolsApplication* g_pMyApplication;
};

#endif
