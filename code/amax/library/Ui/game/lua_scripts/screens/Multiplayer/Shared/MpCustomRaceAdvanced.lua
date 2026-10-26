GUI =
{
	finished = false,

	options = 
	{
		damage		= 1,
		respawn		= 2,
		mods		= 3,
		upgrades	= 4,
		ai			= 5,
		ai_padding	= 6,
		timeout		= 7,
		handicap	= 8,
		powerups	= 9,
	},

	toggles = {},

	CanExit = function( _GUI )
		return false
	end,
}

function Init()
	AddSCUI_Elements()
	Amax.ChangeUiCamera( "Sp_3", UIGlobals.CameraLerpTime, 0 )
	
	StoreInfoLine()
	SetupInfoLine( UIText.INFO_A_SELECT, UIText.INFO_B_BACK_CONFIRM, UIText.INFO_LT_BASIC, UIText.INFO_RT_POWERUPS )

	GUI.helptext =
	{
		[GUI.options.damage]	= UIText.MP_CRHELP_DAMAGE,
		[GUI.options.respawn]	= UIText.MP_CRHELP_RESPAWN,
		[GUI.options.mods]		= UIText.MP_CRHELP_MODS,
		[GUI.options.upgrades]	= UIText.MP_CRHELP_UPGRADES,
		[GUI.options.ai]		= UIText.MP_CRHELP_AI,
		[GUI.options.ai_padding]= UIText.MP_CRHELP_AIPADDING,
		[GUI.options.timeout]	= UIText.MP_CRHELP_TIMEOUT,
		[GUI.options.handicap]	= UIText.MP_CRHELP_HANDICAP,
		[GUI.options.powerups]	= UIText.MP_CRHELP_POWERUPS,
	}

	GUI.table_id = SCUI.name_to_id["table"]

	Mp_ReadCustomRaceSettings() -- get latest settings

	CustomRace_CreateLeftColumn()
	CustomRace_CreateCentreColumn()
	CustomRace_CreateRightColumn()
end

function PostInit()
	UIButtons.ChangePanel( SetupScreenTitle( UIText.MP_CUSTOM_RACE_ADVANCED_TITLE, SCUI.name_to_id["centre"], "race_settings" ), UIEnums.Panel._3DAA_WORLD, true )
	GUI.helpline_id = SetupBottomHelpBar( UIText.MP_CRHELP_GAMEMODE )

	CustomRace_UpdateHelptext()
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )	

	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end

	if message == UIEnums.Message.MenuNext and arg2 == true then

		local selection = UIButtons.GetTableSelection( GUI.table_id )
		if selection == GUI.options.damage or selection == GUI.options.handicap or selection == GUI.options.ai or selection == GUI.options.ai_padding then
			GUI.table_toggle = not GUI.table_toggle
			UIButtons.ActivateTableElement( GUI.table_id, GUI.table_toggle )
			PlaySfxNext()
		else
			PlaySfxToggle()
			Amax.Toggle( GUI.toggles[selection] )
		end

	elseif message == UIEnums.Message.ButtonRightTrigger and arg2 == true then
		if UIButtons.TableElementActivated( GUI.table_id ) == false then
			CustomRace_AdvancedTable()
			
			UISystem.PlaySound( UIEnums.SoundEffect.StickersPage )
			GoScreen( "Multiplayer\\Shared\\MpCustomRacePowerups.lua" )
		end

	elseif message == UIEnums.Message.ButtonLeftTrigger and arg2 == true then
		if UIButtons.TableElementActivated( GUI.table_id ) == false then
			CustomRace_AdvancedTable()
			
			UISystem.PlaySound( UIEnums.SoundEffect.StickersPage )
			GoScreen( "Multiplayer\\Shared\\MpCustomRace.lua" )
		end

	elseif message == UIEnums.Message.MenuBack and arg2 == true then
		-- Splitscreen and Online lobbys are on the same camera
		PlaySfxBack()
		
		if UIButtons.TableElementActivated( GUI.table_id ) == true then
			GUI.table_toggle = false
			UIButtons.ActivateTableElement( GUI.table_id, false )
		else
			CustomRace_AdvancedTable()
			Amax.ChangeUiCamera( UIGlobals.CameraNames.MpLobby, UIGlobals.CameraLerpTime, 0 )
		
			if Amax.GetGameMode() == UIEnums.GameMode.Online or Amax.GetGameMode() == UIEnums.GameMode.SystemLink then
				NetRace.SendNewRaceSettings()
				PopScreen()

			elseif Amax.GetGameMode() == UIEnums.GameMode.SplitScreen then
				GoScreen( "Multiplayer\\MpSplitscreenLobby.lua" )
			end
			
			PlaySfxGraphicNext()
		end
	end
end

function FrameUpdate( time )
	CustomRace_UpdateHelptext()

	local ai_mode = UIButtons.GetSelection( GUI.ai_id )
	if ai_mode ~= GUI.ai_mode then
		GUI.ai_mode = ai_mode

		if Amax.GetGameMode() ~= UIEnums.GameMode.SplitScreen then
			UIButtons.SetActive( UIButtons.GetNodeID( GUI.table_id, GUI.options.ai_padding ), not ( ai_mode == UIEnums.MpAI.Off ) )
		end
	end
end

function EnterEnd()
	RestoreInfoLine()
end

function End()
end

function CustomRace_UpdateHelptext()
	local selection = UIButtons.GetTableSelection( GUI.table_id )
	if selection ~= GUI.selection then
		GUI.selection = selection

		UIButtons.ChangeText( GUI.helpline_id, GUI.helptext[selection] )
		UIButtons.TimeLineActive( "HelpFade", true, 0.5 )
	end
end

function CustomRace_CreateLeftColumn( )
	UIButtons.AddTableCol( GUI.table_id )

	-- damage modes
	GUI.damage_id = CustomRace_SetupSpinner( GUI.table_id, GUI.options.damage, UIText.MP_CUSTOM_RACE_DAMAGE, "damage", "fe_icons" )
	UIButtons.AddItem( GUI.damage_id, UIEnums.MpDamageMode.Off,		UIText.MP_CUSTOM_RACE_OFF, false )
	UIButtons.AddItem( GUI.damage_id, UIEnums.MpDamageMode.Low,		UIText.MP_CUSTOM_RACE_LOW, false )
	UIButtons.AddItem( GUI.damage_id, UIEnums.MpDamageMode.Medium,	UIText.MP_CUSTOM_RACE_MEDIUM, false )
	UIButtons.AddItem( GUI.damage_id, UIEnums.MpDamageMode.High,	UIText.MP_CUSTOM_RACE_HIGH, false )
	UIButtons.AddItem( GUI.damage_id, UIEnums.MpDamageMode.OneHit,	UIText.MP_DAMAGE_MODE_ONE_HIT, false )
	UIButtons.SetSelection( GUI.damage_id, UIGlobals.CustomRaceSettings.damage )

	-- timeout
	if IsSplitScreen() == false then
		GUI.toggles[GUI.options.timeout] = CustomRace_SetupToggle( GUI.table_id, GUI.options.timeout, UIText.MP_CUSTOM_RACE_TIMEOUT, "stopwatch", "common_icons" )
		UIButtons.SetSelection( GUI.toggles[GUI.options.timeout], UIGlobals.CustomRaceSettings.timeout )
	end

	-- ai
	if UIGlobals.CustomRaceSettings.game_mode == UIEnums.MpRaceType.Racing then
		GUI.ai_id = CustomRace_SetupSpinner( GUI.table_id, GUI.options.ai, UIText.MP_CUSTOM_RACE_AI, "add_ai", "common_icons" )
		UIButtons.AddItem( GUI.ai_id, UIEnums.MpAI.Off,		UIText.MP_CUSTOM_RACE_OFF, false )
		UIButtons.AddItem( GUI.ai_id, UIEnums.MpAI.Easy,	UIText.MP_CUSTOM_RACE_EASY, false )
		UIButtons.AddItem( GUI.ai_id, UIEnums.MpAI.Medium,	UIText.MP_CUSTOM_RACE_MEDIUM, false )
		UIButtons.AddItem( GUI.ai_id, UIEnums.MpAI.Hard,	UIText.MP_CUSTOM_RACE_HARD, false )
		UIButtons.SetSelection( GUI.ai_id, UIGlobals.CustomRaceSettings.ai )
	end
end

function CustomRace_CreateCentreColumn( )
	UIButtons.AddTableCol( GUI.table_id )

	-- respawn
	GUI.toggles[GUI.options.respawn] = CustomRace_SetupToggle( GUI.table_id, GUI.options.respawn, UIText.MP_CUSTOM_RACE_RESPAWN, "style_elimination", "common_icons" )
	UIButtons.SetSelection( GUI.toggles[GUI.options.respawn], UIGlobals.CustomRaceSettings.respawn )

	if IsSplitScreen() == false then
		-- mods
		GUI.toggles[GUI.options.mods] = CustomRace_SetupToggle( GUI.table_id, GUI.options.mods, UIText.MP_CUSTOM_RACE_MODS, "custom_shop", "common_icons" )
		UIButtons.SetSelection( GUI.toggles[GUI.options.mods], UIGlobals.CustomRaceSettings.mods )

		-- ai padding
		if UIGlobals.CustomRaceSettings.game_mode == UIEnums.MpRaceType.Racing then
			GUI.ai_padding_id = CustomRace_SetupSpinner( GUI.table_id, GUI.options.ai_padding, UIText.MP_CUSTOM_RACE_AIPADDING, "add_ai", "common_icons" )
			for i = 2, 18 do
				UIButtons.AddItem( GUI.ai_padding_id, i, "GAME_NUM_" .. i, true )
			end
			UIButtons.SetSelection( GUI.ai_padding_id, UIGlobals.CustomRaceSettings.ai_padding )
		end
	end
end

function CustomRace_CreateRightColumn( )
	UIButtons.AddTableCol( GUI.table_id )

	-- handicap
	GUI.handicap_id = CustomRace_SetupSpinner( GUI.table_id, GUI.options.handicap, UIText.MP_CUSTOM_RACE_HANDICAP, "handicap", "common_icons" )
	UIButtons.AddItem( GUI.handicap_id, UIEnums.MpHandicap.Off,		UIText.MP_CUSTOM_RACE_OFF, false )
	UIButtons.AddItem( GUI.handicap_id, UIEnums.MpHandicap.Low,		UIText.MP_CUSTOM_RACE_LOW, false )
	UIButtons.AddItem( GUI.handicap_id, UIEnums.MpHandicap.Medium,	UIText.MP_CUSTOM_RACE_MEDIUM, false )
	UIButtons.AddItem( GUI.handicap_id, UIEnums.MpHandicap.High,	UIText.MP_CUSTOM_RACE_HIGH, false )
	UIButtons.SetSelection( GUI.handicap_id, UIGlobals.CustomRaceSettings.handicap )

	if IsSplitScreen() == false then
		-- upgrades
		GUI.toggles[GUI.options.upgrades] = CustomRace_SetupToggle( GUI.table_id, GUI.options.upgrades, UIText.MP_CUSTOM_RACE_UPGRADES, "horsepower", "fe_icons" )
		UIButtons.SetSelection( GUI.toggles[GUI.options.upgrades], UIGlobals.CustomRaceSettings.upgrades )
	end
end

function CustomRace_AdvancedTable()
	UIGlobals.CustomRaceSettings.damage		= UIButtons.GetSelection( GUI.damage_id )
	UIGlobals.CustomRaceSettings.respawn	= UIButtons.GetSelection( GUI.toggles[GUI.options.respawn] )
	UIGlobals.CustomRaceSettings.ai			= UIButtons.GetSelection( GUI.ai_id )
	UIGlobals.CustomRaceSettings.handicap	= UIButtons.GetSelection( GUI.handicap_id )

	if IsSplitScreen() == false then
		UIGlobals.CustomRaceSettings.timeout	= UIButtons.GetSelection( GUI.toggles[GUI.options.timeout] )
		UIGlobals.CustomRaceSettings.mods		= UIButtons.GetSelection( GUI.toggles[GUI.options.mods] )
		UIGlobals.CustomRaceSettings.upgrades	= UIButtons.GetSelection( GUI.toggles[GUI.options.upgrades] )
		UIGlobals.CustomRaceSettings.ai_padding	= UIButtons.GetSelection( GUI.ai_padding_id )
	end
	
	Amax.SetupRace( UIGlobals.CustomRaceSettings )
end