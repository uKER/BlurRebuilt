GUI =
{
	finished = false,

	options =
	{
		powerups = 1,
		respawn	 = 2,
		random	 = 3,
		bolt	 = 4,
		shock	 = 5,
		mine	 = 6,
		nitro	 = 7,
		barge	 = 8,
		shield	 = 9,
		shunt	 = 10,
		repair	 = 11,
	},

	powerups		= {},
	powerup_toggle	= nil,

	CanExit = function( _GUI )
		return false
	end,
}

function Init()
	AddSCUI_Elements()
	
	Amax.ChangeUiCamera( "Sp_3", UIGlobals.CameraLerpTime, 0 )
	
	StoreInfoLine()
	SetupInfoLine( UIText.INFO_A_SELECT, UIText.INFO_B_BACK_CONFIRM, UIText.INFO_LT_ADVANCED )

	GUI.helptext =
	{
		[GUI.options.powerups]	= UIText.MP_CRHELP_POWERUPS,
		[GUI.options.respawn]	= UIText.MP_CRHELP_POWERUPRESPAWN,
		[GUI.options.random]	= UIText.MP_CRHELP_RANDOMPOWERUPS,
		[GUI.options.bolt]		= UIText.MP_CRHELP_BOLT,
		[GUI.options.shock]		= UIText.MP_CRHELP_SHOCK,
		[GUI.options.mine]		= UIText.MP_CRHELP_MINE,
		[GUI.options.nitro]		= UIText.MP_CRHELP_NITRO,
		[GUI.options.barge]		= UIText.MP_CRHELP_BARGE,
		[GUI.options.shield]	= UIText.MP_CRHELP_SHIELD,
		[GUI.options.shunt]		= UIText.MP_CRHELP_SHUNT,
		[GUI.options.repair]	= UIText.MP_CRHELP_REPAIR,
	}

	GUI.table_id = SCUI.name_to_id["table"]

	Mp_ReadCustomRaceSettings() -- get latest settings

	CustomRace_CreateLeftColumn()
	CustomRace_CreateCentreColumn()
	CustomRace_CreateRightColumn()
end

function PostInit()
	UIButtons.ChangePanel( SetupScreenTitle( UIText.MP_CUSTOM_RACE_POWERUPS, SCUI.name_to_id["centre"], "race_settings" ), UIEnums.Panel._3DAA_WORLD, true )
	GUI.helpline_id = SetupBottomHelpBar( UIText.MP_CRHELP_GAMEMODE )

	CustomRace_UpdateHelptext()
	CustomRace_SetPowerupDefault()
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )	
	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end

	if message == UIEnums.Message.MenuNext and arg2 == true then

		-- if confirm is hit then we want to start save the settings
		local selection = UIButtons.GetTableSelection( GUI.table_id )
		if selection == GUI.options.powerups or selection == GUI.options.respawn then
			GUI.table_toggle = not GUI.table_toggle
			UIButtons.ActivateTableElement( GUI.table_id, GUI.table_toggle )
			PlaySfxNext()

		elseif selection == GUI.options.random then
			PlaySfxToggle()
			Amax.Toggle( GUI.random_id )
		else
			PlaySfxToggle()
			Amax.Toggle( GUI.powerups[selection] )
			
			CustomRace_SetPowerupDefault()
		end

	elseif message == UIEnums.Message.ButtonLeftTrigger and arg2 == true then
		if UIButtons.TableElementActivated( GUI.table_id ) == false then
			CustomRace_PowerupsTable()
	
			UISystem.PlaySound( UIEnums.SoundEffect.StickersPage )
			GoScreen( "Multiplayer\\Shared\\MpCustomRaceAdvanced.lua" )
		end

	elseif message == UIEnums.Message.MenuBack and arg2 == true then
		PlaySfxBack()
		
		if UIButtons.TableElementActivated( GUI.table_id ) == true then
			GUI.table_toggle = false
			UIButtons.ActivateTableElement( GUI.table_id, false )
		else
			CustomRace_PowerupsTable()
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

	if UIButtons.TableElementActivated( GUI.table_id ) == true then

		-- has the toggle override been changed
		local powerup_toggle = UIButtons.GetSelection( GUI.powerups_id )
		if powerup_toggle ~= GUI.powerup_toggle then
			GUI.powerup_toggle = powerup_toggle

			if powerup_toggle ~= 2 then
				CustomRace_TogglePowerups( powerup_toggle )
			end
		end
	end

	CustomRace_UpdateHelptext()
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

function CustomRace_TogglePowerups( toggle )
	for i, v in pairs( GUI.powerups ) do
		UIButtons.SetSelection( v, toggle )
	end
end

function CustomRace_SetPowerupDefault()
	local on_count = 0
	local off_count = 0
	
	-- check the current powerup toggles
	for i, v in pairs( GUI.powerups ) do
		local value = UIButtons.GetSelection( v )
		if value == 1 then
			on_count = on_count + 1
		else
			off_count = off_count + 1
		end
	end
	
	-- default to off
	if on_count == 0 then
		UIButtons.SetSelection( GUI.powerups_id, 0 )

	elseif off_count == 0 then -- default to on
		UIButtons.SetSelection( GUI.powerups_id, 1 )
		
	else -- custom
		UIButtons.SetSelection( GUI.powerups_id, 2 )
	end
end

function CustomRace_CreateLeftColumn( )
	UIButtons.AddTableCol( GUI.table_id )

	-- on/off toggle
	GUI.powerups_id = CustomRace_SetupSpinner( GUI.table_id, GUI.options.powerups, UIText.MP_CUSTOM_RACE_POWERUPS, "perk_option", "perk_icons" )
	UIButtons.AddItem( GUI.powerups_id, 0,		UIText.MP_CUSTOM_RACE_OFF, false )
	UIButtons.AddItem( GUI.powerups_id, 1,		UIText.MP_CUSTOM_RACE_ON, false )
	UIButtons.AddItem( GUI.powerups_id, 2,		UIText.MP_CUSTOM_RACE_CUSTOM, false )
	UIButtons.SetSelection( GUI.powerups_id, 1 )

	-- nitro
	GUI.powerups[GUI.options.nitro] = CustomRace_SetupToggle( GUI.table_id, GUI.options.nitro, UIText.MP_POWERUP_NITRO, "perk_nitro", "perk_icons" )
	UIButtons.SetSelection( GUI.powerups[GUI.options.nitro], BoolToNumber( UIGlobals.CustomRaceSettings.powerups.nitro ) )
	CustomRace_SetupPowerupIcon( GUI.powerups[GUI.options.nitro], "Nitro_Glow" )
	
	-- bolt
	GUI.powerups[GUI.options.bolt] = CustomRace_SetupToggle( GUI.table_id, GUI.options.bolt, UIText.MP_POWERUP_BOLT, "perk_bolt", "perk_icons" )
	UIButtons.SetSelection( GUI.powerups[GUI.options.bolt], BoolToNumber( UIGlobals.CustomRaceSettings.powerups.bolt ) )
	CustomRace_SetupPowerupIcon( GUI.powerups[GUI.options.bolt], "Blindfire_Glow" )

	-- mine
	GUI.powerups[GUI.options.mine] = CustomRace_SetupToggle( GUI.table_id, GUI.options.mine, UIText.MP_POWERUP_MINE, "perk_mine", "perk_icons" )
	UIButtons.SetSelection( GUI.powerups[GUI.options.mine], BoolToNumber( UIGlobals.CustomRaceSettings.powerups.mine ) )
	CustomRace_SetupPowerupIcon( GUI.powerups[GUI.options.mine], "Flash_Glow" )
end

function CustomRace_CreateCentreColumn( )
	UIButtons.AddTableCol( GUI.table_id )

	-- respawn
	GUI.respawn_id = CustomRace_SetupSpinner( GUI.table_id, GUI.options.respawn, UIText.MP_CUSTOM_RACE_POWERUP_RESPAWN, "drop_mode", "common_icons" )
	UIButtons.AddItem( GUI.respawn_id, UIEnums.MpPowerupRespawnMode.None,		UIText.MP_POWERUP_RESPAWN_MODE_NEVER, false )
	UIButtons.AddItem( GUI.respawn_id, UIEnums.MpPowerupRespawnMode.Normal,		UIText.MP_POWERUP_RESPAWN_MODE_NORMAL, false )
	UIButtons.AddItem( GUI.respawn_id, UIEnums.MpPowerupRespawnMode.Long,		UIText.MP_POWERUP_RESPAWN_MODE_LONG, false )
	UIButtons.AddItem( GUI.respawn_id, UIEnums.MpPowerupRespawnMode.VeryLong,	UIText.MP_POWERUP_RESPAWN_MODE_VERY_LONG, false )
	UIButtons.AddItem( GUI.respawn_id, UIEnums.MpPowerupRespawnMode.AlwaysOn,	UIText.MP_POWERUP_RESPAWN_MODE_ALWAYS_ON, false )
	UIButtons.SetSelection( GUI.respawn_id, UIGlobals.CustomRaceSettings.powerup_respawn )

	-- barge
	GUI.powerups[GUI.options.barge] = CustomRace_SetupToggle( GUI.table_id, GUI.options.barge, UIText.MP_POWERUP_BARGE, "perk_barge", "perk_icons" )
	UIButtons.SetSelection( GUI.powerups[GUI.options.barge], BoolToNumber( UIGlobals.CustomRaceSettings.powerups.barge ) )
	CustomRace_SetupPowerupIcon( GUI.powerups[GUI.options.barge], "Barge_Glow" )

	-- repair
	GUI.powerups[GUI.options.repair] = CustomRace_SetupToggle( GUI.table_id, GUI.options.repair, UIText.MP_POWERUP_REPAIR, "perk_repair", "perk_icons" )
	UIButtons.SetSelection( GUI.powerups[GUI.options.repair], BoolToNumber( UIGlobals.CustomRaceSettings.powerups.repair ) )
	CustomRace_SetupPowerupIcon( GUI.powerups[GUI.options.repair], "Repair_Glow" )
	
	-- shock
	GUI.powerups[GUI.options.shock] = CustomRace_SetupToggle( GUI.table_id, GUI.options.shock, UIText.MP_POWERUP_SHOCK, "perk_shock", "perk_icons" )
	UIButtons.SetSelection( GUI.powerups[GUI.options.shock], BoolToNumber( UIGlobals.CustomRaceSettings.powerups.shock ) )
	CustomRace_SetupPowerupIcon( GUI.powerups[GUI.options.shock], "Shock_Glow" )
		
	if UIGlobals.CustomRaceSettings.game_mode == UIEnums.MpRaceType.Destruction then
		UIButtons.SetActive( UIButtons.GetNodeID( GUI.table_id, GUI.options.shock ), false )
	end
end

function CustomRace_CreateRightColumn( )
	UIButtons.AddTableCol( GUI.table_id )

	-- random
	GUI.random_id = CustomRace_SetupToggle( GUI.table_id, GUI.options.random, UIText.MP_CUSTOM_RACE_RANDOM_POWERUPS, "perk_question", "perk_icons" )
	UIButtons.SetSelection( GUI.random_id, UIGlobals.CustomRaceSettings.random_powerups )
	
	print( "UIGlobals.CustomRaceSettings.random_powerups", UIGlobals.CustomRaceSettings.random_powerups )

	-- shunt
	GUI.powerups[GUI.options.shunt] = CustomRace_SetupToggle( GUI.table_id, GUI.options.shunt, UIText.MP_POWERUP_SHUNT, "perk_shunt", "perk_icons" )
	UIButtons.SetSelection( GUI.powerups[GUI.options.shunt], BoolToNumber( UIGlobals.CustomRaceSettings.powerups.shunt ) )
	CustomRace_SetupPowerupIcon( GUI.powerups[GUI.options.shunt], "Shunt_Glow" )
	
	-- shield
	GUI.powerups[GUI.options.shield] = CustomRace_SetupToggle( GUI.table_id, GUI.options.shield, UIText.MP_POWERUP_SHIELD, "perk_shield", "perk_icons" )
	UIButtons.SetSelection( GUI.powerups[GUI.options.shield], BoolToNumber( UIGlobals.CustomRaceSettings.powerups.shield ) )
	CustomRace_SetupPowerupIcon( GUI.powerups[GUI.options.shield], "Shield_Glow" )
end

function CustomRace_PowerupsTable()
	UIGlobals.CustomRaceSettings.random_powerups	= UIButtons.GetSelection( GUI.random_id )
	UIGlobals.CustomRaceSettings.powerup_respawn	= UIButtons.GetSelection( GUI.respawn_id )
	
	-- store powerups
	UIGlobals.CustomRaceSettings.powerups.bolt		= NumberToBool( UIButtons.GetSelection( GUI.powerups[GUI.options.bolt] ) )
	UIGlobals.CustomRaceSettings.powerups.barge		= NumberToBool( UIButtons.GetSelection( GUI.powerups[GUI.options.barge] ) )
	UIGlobals.CustomRaceSettings.powerups.nitro		= NumberToBool( UIButtons.GetSelection( GUI.powerups[GUI.options.nitro] ) )
	UIGlobals.CustomRaceSettings.powerups.shunt		= NumberToBool( UIButtons.GetSelection( GUI.powerups[GUI.options.shunt] ) )
	UIGlobals.CustomRaceSettings.powerups.shock		= NumberToBool( UIButtons.GetSelection( GUI.powerups[GUI.options.shock] ) )
	UIGlobals.CustomRaceSettings.powerups.repair	= NumberToBool( UIButtons.GetSelection( GUI.powerups[GUI.options.repair] ) )
	UIGlobals.CustomRaceSettings.powerups.mine		= NumberToBool( UIButtons.GetSelection( GUI.powerups[GUI.options.mine] ) )
	UIGlobals.CustomRaceSettings.powerups.shield	= NumberToBool( UIButtons.GetSelection( GUI.powerups[GUI.options.shield] ) )
	
	Amax.SetupRace( UIGlobals.CustomRaceSettings )
end
