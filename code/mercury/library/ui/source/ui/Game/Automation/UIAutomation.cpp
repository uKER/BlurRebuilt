
#include "stdafx.h"

#include "system/automation/Automation.h"

#include "ui/Game/Automation/UIAutomation.h"
#include "ui/Game/Gadgets/ButtonController.h"
#include "ui/Game/Gadgets/ButtonMessages.h"
#include "ui/Game/Gadgets/BaseUIVariableSelect.h"
#include <UI/Game/UIScreenControl.h>

namespace UIAutomation
{

bool Callback(Automation::CommandContext& commandContext)
{
#ifndef CFG_GOLD
	if (!strcmp(commandContext.Command(), "CurrentUIScreen"))
	{
		commandContext.SingleReply(UI::gUIScreenControl.GetNextSreen());
	}
	else if (!strcmp(commandContext.Command(), "ExecuteLuaCommand"))
	{
		const char* luaCommand(NULL);

		if (commandContext.MessageReader().Read(luaCommand))
		{
			UI::C_UiLuaCommandMessage message(luaCommand); // e.g. "UIGlobals.DevMode = true"
			UI::gButtons.BroadcastMessageNow(0xffffffff, &message);
			commandContext.SingleReply("ExecuteLuaCommand");
		}
	}
	//Loop through the screen control instances to find the currently active one
	else if (!strcmp(commandContext.Command(), "CurrentUISubscreen"))
	{
		//Start from 1 as 0 is returned by the CurrentUIScreen (ie. is not a subscreen)
		for(int i = 1; i < 5; i++)
		{
			const char* screen = UI::C_ScreenControlManager::Instance().Get( i ).GetCurrentScreen();
			
			if ((screen != NULL) && (strlen(screen) > 0))
			{
				commandContext.SingleReply(screen);
				return true;
			}
		}
		commandContext.SingleReply(UI::gUIScreenControl.GetNextSreen());
	}
#ifdef PLATFORM_PS3
	else if (!strcmp(commandContext.Command(), "PS3GetScreenShotInfo"))
	{
		U32 width, height, pitch, colour;
		const void *bpLocation, *ptrToPtrToMemory;

		Display::Device()->GetScreenshotInfo(&width, &height, &pitch, &colour, &bpLocation, &ptrToPtrToMemory);

		Btl::String64 reply;
		reply.Printf("%d:%d:%d:%d:%d:%d", width, height, pitch, colour, bpLocation, ptrToPtrToMemory);

		commandContext.SingleReply(reply.Chars());

		//CORE_PRINT("w(%d) h(%d) p(%d) c(%d) bp(0x%x) ptr(0x%x)\n", width, height, pitch, colour, bpLocation, ptrToPtrToMemory);
	}
#endif
	//else if (!strcmp(commandContext.Command(), "CurrentUIButtons"))
	//{
	//	UI::C_GadgetBase *button = UI::gButtons.UIObjectList.First();

	//	while (button)
	//	{
	//		bool foundButton(false);

	//		switch (button->GetType())
	//		{
	//		case UI::E_UIButtonType_HorizontalMenu:
	//		case UI::E_UIButtonType_VerticalMenu:
	//			if (button->m_Activated || button->m_Selected)
	//			{
	//				//iRes = ((UI::BaseUIVariableSelect *) b)->GetCurrentItemValue();
	//				((UI::BaseUIVariableSelect*)button)->mNumberofItems;
	//				((UI::BaseUIVariableSelect*)button)->GetCurrentItemIndex();
	//				
	//				foundButton = true;
	//			}
	//			break;

	//		//case UI::E_UIButtonType_VsSpinner:
	//		//case E_UIButtonType_SlideMenu:
	//		//case E_UIButtonType_VariableSpinner:

	//		default:
	//			break;
	//		}

	//		if (foundButton)
	//			break;

	//		button = UI::gButtons.UIObjectList.Next();
	//	}
	//}
	else
	{
		return false;
	}
#endif
	return true;
}

}
