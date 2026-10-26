#ifndef FRAMEWORK_CORE_DIALOGS_H
#define FRAMEWORK_CORE_DIALOGS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Framework
{
	typedef S32 (*PrimaryPadQueryCallback)();

	//Display default dirty disc dialog and halt application.
	void CriticalReadErrorDialog(const char* fileName, bool writing, bool isCritical, void* pUserData = 0);

	void SetPrimaryPadQueryCallback(PrimaryPadQueryCallback cb);

	S32 GetPrimaryPadNo();
}; // namespace Framework

#endif // FRAMEWORK_CORE_DIALOGS_H
