#ifndef MERCURY_EXECUTER_HELPERS_H
#define MERCURY_EXECUTER_HELPERS_H

#ifdef _MSC_VER
#pragma once
#endif

#define EXECUTER_FATAL( message, ... ) {  Tools::C_ExecuterHelpers::FeedbackFatal(*Tools::g_pMyApplication->GetExecuterFeedbackPtr(), message, __VA_ARGS__); }
#define EXECUTER_ERROR( message, ... ) {  Tools::C_ExecuterHelpers::FeedbackError(*Tools::g_pMyApplication->GetExecuterFeedbackPtr(), message, __VA_ARGS__); }
#define EXECUTER_WARN( message, ... ) {  Tools::C_ExecuterHelpers::FeedbackWarn(*Tools::g_pMyApplication->GetExecuterFeedbackPtr(), message, __VA_ARGS__); }
#define EXECUTER_INFO( message, ... ) {  Tools::C_ExecuterHelpers::FeedbackInfo(*Tools::g_pMyApplication->GetExecuterFeedbackPtr(), message, __VA_ARGS__); }

namespace Debug
{
	class ExecuterFeedback;
	class Logger;
}

namespace Tools
{
	class C_ExecuterHelpers
	{
	public:
		static void FeedbackFatal(
			Debug::ExecuterFeedback& executerFeedback,
			const char *format, ...);

		static void FeedbackError(
			Debug::ExecuterFeedback& executerFeedback,
			const char *format, ...);

		static void FeedbackWarn(
			Debug::ExecuterFeedback& executerFeedback,
			const char *format, ...);

		static void FeedbackInfo(
			Debug::ExecuterFeedback& executerFeedback,
			const char *format, ...);
	};
}

#endif

