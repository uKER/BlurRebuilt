#include "stdafx.h"
#include <tools_common/SendEmail.h>

void EmailReport(const Aftk::C_Status& status)
{
	if (status.GetSeverity() == 0)
	{
		//I don't need to see this error, it's not serious and can be fixed by the artist.
		return;
	}
	Tu::Status tuStatus;
	Tu::SendEmail mail(tuStatus);

	mail.AddRecipient(AFTK_MAYA_SUPPORT_EMAIL);
	std::string subject = "Maya->Aftk export problems for plug-in version ";
	subject += std::string(g_versionString);
	subject += ".";
	mail.SetSubject(subject);
	mail.SetBody(status.GetErrorString());
	std::string err;
	err = "Email sending failed : " + tuStatus.ErrorString();
	tuStatus = mail.Send();

	if (!tuStatus)
	{
		std::string err;
		err = "Email sending failed : " + tuStatus.ErrorString();
		S_INFO(AftkMaya::g_mainLogger, err);
		GuiDisplayOutput(DebugLevel::Info, err);
	}
}