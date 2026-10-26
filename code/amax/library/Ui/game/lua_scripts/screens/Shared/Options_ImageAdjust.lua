GUI =
{
	finished = false,
	CanExit = function( _GUI )
		return false
	end,
}

function Init()
	AddSCUI_Elements()
	
	GUI.MessageListId = SCUI.name_to_id[ "OptionsList" ]
	
	GUI.GammaId			= SCUI.name_to_id[ "Sdr_Gamma" ]
	-- Add ten items for gamma
	for i = 1, 10, 1 do
		UIButtons.AddItem( GUI.GammaId, i, i, false )
	end
		
	UIButtons.SetSelection( GUI.GammaId, UIGlobals.OptionsTable.gamma )	
		
	GUI.gamma = UIGlobals.OptionsTable.gamma
	GUI.initialGamma = GUI.gamma
	
	StoreInfoLine()
	SetupInfoLine(UIText.INFO_A_CONFIRM, UIText.INFO_B_CANCEL, UIText.INFO_RESET_TO_DEFAULT)
end

function PostInit()
	if UIGlobals.IsIngame == true then
		UIButtons.TimeLineActive( "DarkenScreen", false )
		UIButtons.SetActive( SCUI.name_to_id[ "Gfx_Image" ], false )
		UIButtons.ChangePanel( SCUI.name_to_id["Dummy"], 1, true )
	end
end

function StartLoop( time )
	--
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end

	if message == UIEnums.Message.MenuNext and arg2 == true then
		PlaySfxNext()
		
		local gamma = UIButtons.GetSelection( GUI.GammaId )
		if UIGlobals.OptionsTable.gamma ~= gamma then
			Amax.GameDataLogBrightnessChanged(UIGlobals.OptionsTable.gamma, gamma)
			UIGlobals.OptionsTable.gamma = gamma
			UIGlobals.SaveOptions = true
		end

		GoScreen( ImageAdjust_ReturnScreen() )
	end
	
	if message == UIEnums.Message.MenuBack and arg2 == true then
		PlaySfxBack()
		local gamma = UIButtons.GetSelection( GUI.GammaId )
		if GUI.initialGamma ~= gamma then
			Amax.GammaCorrection( GUI.initialGamma )
		end
		GoScreen( ImageAdjust_ReturnScreen() )
	end
	
	if message == UIEnums.Message.ButtonX and arg2 == true then
		PlaySfxError()
		
		local gamma = 5 -- default gamma
		
		Amax.GammaCorrection( gamma )
		UIButtons.SetSelection( GUI.GammaId, gamma )	
		
	end
end

function FrameUpdate( time )
	--
	local gamma = UIButtons.GetSelection( GUI.GammaId )
	if GUI.gamma ~= gamma then
		GUI.gamma = gamma
		Amax.GammaCorrection( gamma )
	end
end

function Render()
	--
end

function EnterEnd()
	RestoreInfoLine()

	if UIGlobals.IsIngame == true then
		UIButtons.TimeLineActive( "DarkenScreen", true )
	end
end

function EndLoop( time )
	--
end

function End()
	-- safety, we could leave the screen by other means e.g. accepting an xbox live invite
	Amax.GammaCorrection( UIGlobals.OptionsTable.gamma )
end

-- [Blur2PC] retail Blur PC reaches image adjust from the graphics options screen and returns there
function ImageAdjust_ReturnScreen()
	if UIEnums.CurrentPlatform == UIEnums.Platform.PC then
		return "Shared\\Options_Graphics.lua"
	end
	return "Shared\\Options.lua"
end
