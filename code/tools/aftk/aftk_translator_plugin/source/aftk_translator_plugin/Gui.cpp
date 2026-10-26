#include "stdafx.h"

F32 g_workToDo = 0;
F32 g_workDone = 0;

U32 g_warningCount = 0;
U32 g_errorCount = 0;

void GuiResetCounts()
{
	g_warningCount = 0;
	g_errorCount = 0;
}

void GuiDisplayOutput(DebugLevel::Level channel, const std::string& text)
{
	switch (channel)
	{
	case DebugLevel::Info:
		MGlobal::displayInfo(MString(text.c_str()));
		break;

	case DebugLevel::Warn:
		g_warningCount++;
		MGlobal::displayWarning(MString(text.c_str()));
		break;

	case DebugLevel::Error:
		g_errorCount++;
		MGlobal::displayError(MString(text.c_str()));
		break;

	default:
		break;
	};
}

void GuiDisplayMessageBox(const std::string& title, const std::string& message)
{
	std::string command;
	command += "confirmDialog ";
	command += "-title \"" + title + "\" ";
	command += "-message \"" + message + "\" ";
	command += "-button \"OK\" ";
	command += "-defaultButton \"OK\" "; 

	MGlobal::executeCommand(MString(command.c_str()));
}

void GuiDisplayEmailPrompt()
{
	std::string command;
	command += "confirmDialog ";
	command += "-title \"Email Export Log to the Tools Team\" ";
	command += "-message \"Important warnings/errors were encountered. Please through the next dialog to transmit an error report.\" ";
	command += "-button \"OK\" ";
	command += "-defaultButton \"OK\" "; 

	MGlobal::executeCommand(MString(command.c_str()));
}

void GuiDisplayExportSummary()
{
	if ((g_warningCount + g_errorCount) > 0)
	{
		std::string warningCountString;
		std::string errorCountString;
		{
			char buffer[255];
			sprintf(buffer, "There were %i warnings and ", g_warningCount);
			warningCountString = buffer;
		}
		{
			U32 errorCount = g_errorCount;
			char buffer[255];
			sprintf(buffer, "%i errors. ", errorCount);
			errorCountString = buffer;
		}
		std::string message = warningCountString + errorCountString;
		if (g_errorCount != 0)
		{
			message += "Scene has NOT been EXPORTED. ";
		}
		message += "See the script window for further information.";
		GuiDisplayMessageBox("Warnings/Errors Encountered", message);
	}
}

