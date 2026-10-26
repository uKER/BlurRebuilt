#pragma once

#include "ProgressTimeToComplete.h"

class TProgressPercentComplete : public TProgressTimeToComplete
{
	CString GetRemainingText(double lfPercent, double lfSecsRemaining)
	{
		CString str;

		str.Format("%5.0f%%",lfPercent*100);

		return str;
	};
};
