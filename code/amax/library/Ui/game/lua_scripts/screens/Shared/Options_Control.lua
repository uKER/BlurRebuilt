-- [Blur2PC] Retail Blur PC keyboard options screen (Options > Keyboard), ported from the bytecode embedded in
-- Blur.exe 1.2 (Screens\Shared\Options_Control.lua, decompiled with unluac). The screen layout (SDR_MpCurrentConfig,
-- KeyNodeList and the SDR_* key rows) comes from the retail UI data. The screen shows the keys of the two
-- keyboard layouts; Accept stores the chosen layout, which Options then applies (Amax.UpdateCustomKeyboardControls,
-- Amax.Options). Differences from retail:
--   * no mouse handling (retail also clicks the arrows and the key rows: getMouseButton, UIButtons.GetParent,
--     UIButtons.SetCurrentItemByID, SetPreviousItem/SetNextItem), which this source's UI does not have;
--   * retail's key capture for a third, custom layout is left out: retail offers only the two layouts and nothing
--     in its scripts or its UI data ever sets GUI.WaitForInput, so the capture code (UIButtons.GetKeyInput,
--     SetBlockInput, the OptionsInvalidKeys popup) never runs.
-- See docs/analysis.md "Keyboard layouts (T103)".

GUI =
{
	finished = false,
	carousel_branch = "Options",
	OutputSelection = -1,
	UnblockDontChange = false,

	-- Copies the custom layout (the third) from the options table
	SetSelections = function()
		if UIGlobals.OptionsTable.IsCustom == 2 then
			GUI.KEYCONFIG[3][1]		= UIGlobals.OptionsTable.Throttle
			GUI.KEYCONFIG[3][2]		= UIGlobals.OptionsTable.Brake
			GUI.KEYCONFIG[3][3]		= UIGlobals.OptionsTable.SteerLeft
			GUI.KEYCONFIG[3][4]		= UIGlobals.OptionsTable.SteerRight
			GUI.KEYCONFIG[3][5]		= UIGlobals.OptionsTable.EBrake
			GUI.KEYCONFIG[3][6]		= UIGlobals.OptionsTable.Fire
			GUI.KEYCONFIG[3][7]		= UIGlobals.OptionsTable.SwitchPerk
			GUI.KEYCONFIG[3][8]		= UIGlobals.OptionsTable.ChangeView
			GUI.KEYCONFIG[3][9]		= UIGlobals.OptionsTable.Pause
			GUI.KEYCONFIG[3][10]	= UIGlobals.OptionsTable.RearView
			GUI.KEYCONFIG[3][11]	= UIGlobals.OptionsTable.PowerUpFwd
			GUI.KEYCONFIG[3][12]	= UIGlobals.OptionsTable.PowerUpBwd
			GUI.KEYCONFIG[3][13]	= UIGlobals.OptionsTable.Drop
		end
	end,

	WaitForInput = false,
	CurrentWaitIndex = -1,

	CanExit = function( _GUI )
		return false
	end,

	key_menu = {},
	map_id = {},
	KEYCONFIG = { {}, {}, {} },

	-- Shows the keys of a layout (1-based); on a French keyboard the Q/A and Z/W labels swap
	UpdateKeySet = function( set )
		for i, v in ipairs( GUI.key_menu ) do
			UIButtons.ClearItems( v )

			local text = UIText[ GUI.KEYCONFIG[ set ][ i ] ]
			if Amax.GetPCKeyboard() == "0000040C" then
				if GUI.KEYCONFIG[ set ][ i ] == "CMN_Q" then
					text = UIText.CMN_A
				elseif GUI.KEYCONFIG[ set ][ i ] == "CMN_A" then
					text = UIText.CMN_Q
				elseif GUI.KEYCONFIG[ set ][ i ] == "CMN_Z" then
					text = UIText.CMN_W
				elseif GUI.KEYCONFIG[ set ][ i ] == "CMN_W" then
					text = UIText.CMN_Z
				end
			end

			UIButtons.AddItem( v, 0, text, false )
		end
	end,

	FadeArrows = function()
		if GUI.current_config == 0 then
			UIButtons.TimeLineActive( "left_fade", false )
			UIButtons.TimeLineActive( "right_fade", true )
		elseif GUI.current_config == 1 then
			UIButtons.TimeLineActive( "left_fade", true )
			UIButtons.TimeLineActive( "right_fade", false )
		end
	end,

	Animatearrows = function( new_config )
		if new_config > GUI.current_config then
			UIButtons.TimeLineActive( "move_right", true, 0 )
		else
			UIButtons.TimeLineActive( "move_left", true, 0 )
		end
	end,
}

function Init()
	AddSCUI_Elements()

	-- Key name to key number (this source's C_Keyboard enum)
	GUI.map_id.CMN_1 = 1
	GUI.map_id.CMN_2 = 2
	GUI.map_id.CMN_3 = 3
	GUI.map_id.CMN_4 = 4
	GUI.map_id.CMN_5 = 5
	GUI.map_id.CMN_6 = 6
	GUI.map_id.CMN_7 = 7
	GUI.map_id.CMN_8 = 8
	GUI.map_id.CMN_9 = 9
	GUI.map_id.CMN_0 = 10
	GUI.map_id.CMN_KEYS_MINUS = 11
	GUI.map_id.CMN_KEYS_PLUS = 12
	GUI.map_id.CMN_KEYS_BACKSPACE = 13
	GUI.map_id.CMN_KEYS_TAB = 14
	GUI.map_id.CMN_Q = 15
	GUI.map_id.CMN_W = 16
	GUI.map_id.CMN_E = 17
	GUI.map_id.CMN_R = 18
	GUI.map_id.CMN_T = 19
	GUI.map_id.CMN_Y = 20
	GUI.map_id.CMN_U = 21
	GUI.map_id.CMN_I = 22
	GUI.map_id.CMN_O = 23
	GUI.map_id.CMN_P = 24
	GUI.map_id.CMN_KEYS_LSQR_BRACKET = 25
	GUI.map_id.CMN_KEYS_RSQR_BRACKET = 26
	GUI.map_id.CMN_KEYS_RETURN = 27
	GUI.map_id.CMN_KEYS_LCTRL = 28
	GUI.map_id.CMN_A = 29
	GUI.map_id.CMN_S = 30
	GUI.map_id.CMN_D = 31
	GUI.map_id.CMN_F = 32
	GUI.map_id.CMN_G = 33
	GUI.map_id.CMN_H = 34
	GUI.map_id.CMN_J = 35
	GUI.map_id.CMN_K = 36
	GUI.map_id.CMN_L = 37
	GUI.map_id.CMN_KEYS_SEMI_COLON = 38
	GUI.map_id.CMN_KEYS_APOSTROPHE = 39
	GUI.map_id.CMN_KEYS_GRAVE = 40
	GUI.map_id.CMN_KEYS_LSHIFT = 41
	GUI.map_id.CMN_KEYS_BSLASH = 42
	GUI.map_id.CMN_Z = 43
	GUI.map_id.CMN_X = 44
	GUI.map_id.CMN_C = 45
	GUI.map_id.CMN_V = 46
	GUI.map_id.CMN_B = 47
	GUI.map_id.CMN_N = 48
	GUI.map_id.CMN_M = 49
	GUI.map_id.CMN_KEYS_COMMA = 50
	GUI.map_id.CMN_KEYS_FULLSTOP = 51
	GUI.map_id.CMN_KEYS_FSLASH = 52
	GUI.map_id.CMN_KEYS_RSHIFT = 53
	GUI.map_id.CMN_KEYS_NUMPAD_ASTERISK = 54
	GUI.map_id.CMN_KEYS_LALT = 55
	GUI.map_id.CMN_KEYS_SPACE = 56
	GUI.map_id.CMN_KEYS_F1 = 58
	GUI.map_id.CMN_KEYS_F2 = 59
	GUI.map_id.CMN_KEYS_F3 = 60
	GUI.map_id.CMN_KEYS_F4 = 61
	GUI.map_id.CMN_KEYS_F5 = 62
	GUI.map_id.CMN_KEYS_F6 = 63
	GUI.map_id.CMN_KEYS_F7 = 64
	GUI.map_id.CMN_KEYS_F8 = 65
	GUI.map_id.CMN_KEYS_F9 = 66
	GUI.map_id.CMN_KEYS_F10 = 67
	GUI.map_id.CMN_KEYS_NUMPAD_7 = 70
	GUI.map_id.CMN_KEYS_NUMPAD_8 = 71
	GUI.map_id.CMN_KEYS_NUMPAD_9 = 72
	GUI.map_id.CMN_KEYS_NUMPAD_MINUS = 73
	GUI.map_id.CMN_KEYS_NUMPAD_4 = 74
	GUI.map_id.CMN_KEYS_NUMPAD_5 = 75
	GUI.map_id.CMN_KEYS_NUMPAD_6 = 76
	GUI.map_id.CMN_KEYS_NUMPAD_PLUS = 77
	GUI.map_id.CMN_KEYS_NUMPAD_1 = 78
	GUI.map_id.CMN_KEYS_NUMPAD_2 = 79
	GUI.map_id.CMN_KEYS_NUMPAD_3 = 80
	GUI.map_id.CMN_KEYS_NUMPAD_0 = 81
	GUI.map_id.CMN_KEYS_NUMPAD_POINT = 82
	GUI.map_id.CMN_KEYS_F11 = 84
	GUI.map_id.CMN_KEYS_F12 = 85
	GUI.map_id.CMN_KEYS_NUMPAD_RETURN = 105
	GUI.map_id.CMN_KEYS_RCTRL = 106
	GUI.map_id.CMN_KEYS_NUMPAD_SLASH = 115
	GUI.map_id.CMN_KEYS_RALT = 117
	GUI.map_id.CMN_KEYS_HOME = 119
	GUI.map_id.CMN_KEYS_UP_ARROW = 120
	GUI.map_id.CMN_KEYS_PAGE_UP = 121
	GUI.map_id.CMN_KEYS_LEFT_ARROW = 122
	GUI.map_id.CMN_KEYS_RIGHT_ARROW = 123
	GUI.map_id.CMN_KEYS_END = 124
	GUI.map_id.CMN_KEYS_DOWN_ARROW = 125
	GUI.map_id.CMN_KEYS_PAGE_DOWN = 126
	GUI.map_id.CMN_KEYS_INSERT = 127
	GUI.map_id.CMN_KEYS_DELETE = 128

	GUI.ConfigNodeID = SCUI.name_to_id.SDR_MpCurrentConfig

	local keys = Amax.UpdateCustomKeyboardControls()
	GUI.current_config = keys.IsCustom

	UIButtons.SetActive( GUI.ConfigNodeID, true )

	GUI.MessageListId		= SCUI.name_to_id.KeyNodeList
	GUI.AdvancedNodeId		= SCUI.name_to_id.Nde_Controller
	GUI.LeftArrow			= SCUI.name_to_id.arrow_left
	GUI.RightArrow			= SCUI.name_to_id.arrow_right
	GUI.ThrottleNodeID		= SCUI.name_to_id.SDR_Throttle
	GUI.BrakeNodeID			= SCUI.name_to_id.SDR_Brake
	GUI.SteerLeftNodeID		= SCUI.name_to_id.SDR_SteerLeft
	GUI.SteerRightNodeID	= SCUI.name_to_id.SDR_SteerRight
	GUI.EBrakeNodeID		= SCUI.name_to_id.SDR_EBrake
	GUI.FireNodeID			= SCUI.name_to_id.SDR_Fire
	GUI.SwitchPerkNodeID	= SCUI.name_to_id.SDR_SwitchPerk
	GUI.ChangeViewNodeID	= SCUI.name_to_id.SDR_ChangeView
	GUI.PauseNodeID			= SCUI.name_to_id.SDR_Pause
	GUI.RearViewNodeID		= SCUI.name_to_id.SDR_LookBack
	GUI.FwdFireNodeID		= SCUI.name_to_id.SDR_FireFwd
	GUI.BwdFireNodeID		= SCUI.name_to_id.SDR_FireBwd
	GUI.DropNodeID			= SCUI.name_to_id.SDR_Drop

	GUI.key_menu[1]		= GUI.ThrottleNodeID
	GUI.key_menu[2]		= GUI.BrakeNodeID
	GUI.key_menu[3]		= GUI.SteerLeftNodeID
	GUI.key_menu[4]		= GUI.SteerRightNodeID
	GUI.key_menu[5]		= GUI.EBrakeNodeID
	GUI.key_menu[6]		= GUI.FireNodeID
	GUI.key_menu[7]		= GUI.SwitchPerkNodeID
	GUI.key_menu[8]		= GUI.ChangeViewNodeID
	GUI.key_menu[9]		= GUI.PauseNodeID
	GUI.key_menu[10]	= GUI.RearViewNodeID
	GUI.key_menu[11]	= GUI.FwdFireNodeID
	GUI.key_menu[12]	= GUI.BwdFireNodeID
	GUI.key_menu[13]	= GUI.DropNodeID

	UIButtons.AddItem( GUI.ConfigNodeID, 0, UIText.CMN_OPT_CNT_LAYOUT1, false )
	UIButtons.AddItem( GUI.ConfigNodeID, 1, UIText.CMN_OPT_CNT_LAYOUT2, false )

	-- The keys shown for each layout: throttle, brake, steer left, steer right, handbrake, fire, switch perk,
	-- change view, pause, rear view, fire forward, fire backward, drop
	GUI.KEYCONFIG[1] = { "CMN_Q", "CMN_A", "CMN_KEYS_LEFT_ARROW", "CMN_KEYS_RIGHT_ARROW", "CMN_KEYS_DOWN_ARROW", "CMN_KEYS_RCTRL",
						 "CMN_KEYS_UP_ARROW", "CMN_V", "CMN_P", "CMN_KEYS_TAB", "CMN_W", "CMN_S", "CMN_D" }
	GUI.KEYCONFIG[2] = { "CMN_W", "CMN_S", "CMN_A", "CMN_D", "CMN_KEYS_SPACE", "CMN_KEYS_RCTRL",
						 "CMN_KEYS_LEFT_ARROW", "CMN_Z", "CMN_P", "CMN_KEYS_PAGE_DOWN", "CMN_KEYS_UP_ARROW", "CMN_KEYS_DOWN_ARROW", "CMN_KEYS_RIGHT_ARROW" }
	GUI.KEYCONFIG[3] = { "CMN_Q", "CMN_A", "CMN_KEYS_LEFT_ARROW", "CMN_KEYS_RIGHT_ARROW", "CMN_KEYS_DOWN_ARROW", "CMN_KEYS_RCTRL",
						 "CMN_KEYS_UP_ARROW", "CMN_V", "CMN_P", "CMN_KEYS_TAB", "CMN_W", "CMN_S", "CMN_D" }

	GUI.SetSelections()

	for i, v in ipairs( GUI.key_menu ) do
		UIButtons.AddItem( v, 0, UIText[ GUI.KEYCONFIG[ GUI.current_config + 1 ][ i ] ], false )
	end

	UIButtons.SetSelected( GUI.ConfigNodeID, true )
	UIButtons.SetSelected( GUI.MessageListId, false )

	GUI.do_update_keys = true
	UIButtons.SetSelectionByIndex( GUI.ConfigNodeID, GUI.current_config )
	GUI.UpdateKeySet( GUI.current_config + 1 )

	StoreInfoLine()
	SetupMenuOptions()
end

function PostInit()
	--
end

function StartLoop( time )
	--
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end

	if GUI.WaitForInput == true and message == UIEnums.Message.MenuBack and arg2 == true then
		GUI.UnblockDontChange = true
	end

	if GUI.WaitForInput == false then
		if message == UIEnums.Message.MenuBack and arg2 == true then
			if UIButtons.IsSelected( GUI.MessageListId ) == true then
				UIButtons.SetSelected( GUI.MessageListId, false )
				UIButtons.SetSelected( GUI.ConfigNodeID, true )
			else
				UIGlobals.SaveOptions = false
				GoScreen( "Shared\\Options.lua" )
			end
		elseif message == UIEnums.Message.MenuNext and arg2 == true then
			PlaySfxNext()
			UIGlobals.SaveOptions = true
			GoScreen( "Shared\\Options.lua" )
		elseif message == UIEnums.Message.ButtonLeft or message == UIEnums.Message.ButtonRight then
			GUI.do_update_keys = true
		end
	end
end

function FrameUpdate( time )
	if GUI.current_config ~= 2 then
		UIButtons.SetSelected( GUI.MessageListId, false )
	end

	local selection = UIButtons.GetSelection( GUI.ConfigNodeID )
	if GUI.do_update_keys == true and selection ~= GUI.current_config then
		GUI.Animatearrows( selection )
		GUI.current_config = selection
		GUI.UpdateKeySet( selection + 1 )
		do_update_keys = false	-- retail sets a global here; GUI.do_update_keys stays true
		GUI.FadeArrows()
	end
end

function Render()
	--
end

function EnterEnd()
	RestoreInfoLine()
end

function EndLoop( time )
	--
end

function End()
	if UIGlobals.SaveOptions == true then
		GetScreenOtions().selection = UIButtons.GetSelectionIndex( GUI.MessageListId )

		UIGlobals.OptionsTable.IsCustom		= GUI.current_config
		UIGlobals.OptionsTable.Throttle		= GUI.map_id[ UIGlobals.OptionsTable.Throttle ]
		UIGlobals.OptionsTable.Brake		= GUI.map_id[ UIGlobals.OptionsTable.Brake ]
		UIGlobals.OptionsTable.SteerLeft	= GUI.map_id[ UIGlobals.OptionsTable.SteerLeft ]
		UIGlobals.OptionsTable.SteerRight	= GUI.map_id[ UIGlobals.OptionsTable.SteerRight ]
		UIGlobals.OptionsTable.EBrake		= GUI.map_id[ UIGlobals.OptionsTable.EBrake ]
		UIGlobals.OptionsTable.Fire			= GUI.map_id[ UIGlobals.OptionsTable.Fire ]
		UIGlobals.OptionsTable.SwitchPerk	= GUI.map_id[ UIGlobals.OptionsTable.SwitchPerk ]
		UIGlobals.OptionsTable.ChangeView	= GUI.map_id[ UIGlobals.OptionsTable.ChangeView ]
		UIGlobals.OptionsTable.Pause		= GUI.map_id[ UIGlobals.OptionsTable.Pause ]
		UIGlobals.OptionsTable.RearView		= GUI.map_id[ UIGlobals.OptionsTable.RearView ]
		UIGlobals.OptionsTable.PowerUpFwd	= GUI.map_id[ UIGlobals.OptionsTable.PowerUpFwd ]
		UIGlobals.OptionsTable.PowerUpBwd	= GUI.map_id[ UIGlobals.OptionsTable.PowerUpBwd ]
		UIGlobals.OptionsTable.Drop			= GUI.map_id[ UIGlobals.OptionsTable.Drop ]

		Amax.UpdateCustomKeyboardControls( UIGlobals.OptionsTable, GUI.current_config )
	end
end
