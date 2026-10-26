GUI =
{
	finished = false,
	
	safety_timer = UIGlobals.ResetTimerAfterWrecked - 0.3 - 0.3,

	wreck_screen_enabled = false,
	has_jump_the_gun = false,
}

function Init()
	AddSCUI_Elements( )
	
	SetupCorrectWreckedSequence()
	
	UIScreen.SetScreenTimers( 0.3, 0.3 )
	UIButtons.TimeLineActive( "start", false, 0 )

	UIButtons.ChangeText( SCUI.name_to_id[ "Wrecked_By" ], "MPL_WRECKED_BY" .. 0 )
	UIButtons.ChangeText( SCUI.name_to_id[ "Wrecked_By_accell" ], "MPL_WRECKED_BY" .. 0 )

	SetupWreckedScreenInfoline()
	
	UISystem.PlaySound( UIEnums.SoundEffect.WreckedActionIn )
end

function PostInit()
	local modsExistToDisplay = false
	
	if Amax.IsNetworkRace() == true then
		-- display wreckers mods
		local wrecker_mods = Amax.GetWreckerModLoadout( 0 )
		
		if wrecker_mods ~= nil then
			for i, v in ipairs( wrecker_mods.mods ) do
				UIButtons.ChangeTextureUV( SCUI.name_to_id[ "Wrecker_LoadOut" .. i ], 0, 0.03125 * v.type, 1 )
				UIButtons.ChangeColour( SCUI.name_to_id[ "Wrecker_LoadOut" .. i ], UIGlobals.CategoryColour[ v.category ] )
				UIButtons.ChangeTextureUV( SCUI.name_to_id[ "Wrecker_LoadOut_accell" .. i ], 0, 0.03125 * v.type, 1 )
				UIButtons.ChangeColour( SCUI.name_to_id[ "Wrecker_LoadOut_accell" .. i ], UIGlobals.CategoryColour[ v.category ] )
				modsExistToDisplay = true
			end
		end
		
		if modsExistToDisplay == true then
			if Amax.IsRaceMpDestruction() == true and NetRace.IsTeamGame() == false then
				UIButtons.ChangeText( SCUI.name_to_id[ "WreckerMods" ], UIText.MP_RIVAL_MODS )
				UIButtons.ChangeText( SCUI.name_to_id[ "WreckerMods_accell" ], UIText.MP_RIVAL_MODS )
			end
		end
	end
	
	if modsExistToDisplay == false then
		UIButtons.SetActive( SCUI.name_to_id[ "Loadout" ], false )
		UIButtons.SetActive( SCUI.name_to_id[ "Loadout_accell" ], false )
	end
	
	UIButtons.TimeLineActive( "show_wrecked", true, 0 )

	UIScreen.AddMessageNow( UIEnums.GameMessage.SetLocalPlayerIndex, 0 )

	if Multiplayer.IsElimination() then
		print("Is Elimination")
		UIButtons.ChangeText( SCUI.name_to_id[ "Wrecked" ], UIText.MP_ELIMINATED )
		UIButtons.ChangeText( SCUI.name_to_id[ "Wrecked_accell" ], UIText.MP_ELIMINATED )
	end
end

function StartLoop( time )
	--
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end

	if GUI.finished == true then 
		return
	end
	
	if message == UIEnums.GameFlowMessage.Respawn and arg1 == 0 and IsSplitScreen() == false then
		GUI.finished = true
		EndScreen()
	end
end

function FrameUpdate( time )
	if GUI.finished == true then 
		return
	end
	
	GUI.safety_timer = GUI.safety_timer - time

	if GUI.safety_timer <= 0 then
		GUI.finished = true
		EndScreen()
	end	
	
end

function SetupWreckedScreenInfoline()
	if Amax.IsNetworkRace() == true then
		SetupInfoLine( UIText.INFO_BACK_SHOW_SCORES )
	else
		SetupInfoLine()
	end
end

function EndLoop( time )
	--
end

function End()
	if Amax.IsNetworkRace() == true then
		SetupInfoLine()
	end
	
	-- Re-enable the HUD
	UIButtons.TimeLineActive( "start", true )
end


function SetupCorrectWreckedSequence()

	-- we can now play with the wrecked sequence length
	safety_timer = Amax.GetWreckedSequenceDuration( 0 ) - 0.3 - 0.3
		
	GUI.has_jump_the_gun = Amax.PlayerHasJumpTheGun( 0 )
	if GUI.has_jump_the_gun == true then
		
		-- If just jump the gun then show accellerated
		if Amax.PlayerHasLastGasp( 0 ) == false then
		print("but player does NOT have Last gasp mod")
		
			-- Disable regular render branch.. 
			UIButtons.SetActive( SCUI.name_to_id[ "WreckedRenderBranch" ], false )
			
			-- Enable accelerated
			UIButtons.SetActive( SCUI.name_to_id[ "WreckedRenderBranch_Accellerated" ], true )
			
			if GUI.has_jump_the_gun == true then
				-- show the jump the gun icon
				UIButtons.SetActive( SCUI.name_to_id[ "jump_the_gun_icon_accell" ], true )
			end
		else
			-- if both mods equipped we show nothing!
			UIButtons.SetActive( SCUI.name_to_id[ "WreckedRenderBranch" ], false )
			UIButtons.SetActive( SCUI.name_to_id[ "WreckedRenderBranch_Accellerated" ], false )
		end
	end
end