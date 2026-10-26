#include "stdafx.h"

#include <DialogueMan/Xt/XtTypeDatabase.h>
#include <DialogueMan/Xt/Types/IXtType.h>
#include <DialogueMan/Xt/Types/XtSxlRecord.h>
#include <DialogueMan/Xt/Types/XtSxlRow.h>
#include <DialogueMan/App.h>

namespace DMan
{
	bool IsSxlRecord(const XtString& type)
	{
		return wxGetApp().GetXtTypeDatabase().FindType(type.Text()) != 0;
	}
}
