GUI =
{
	finished	= false,
	countdown	= 5,
}

function Init()
	AddSCUI_Elements()
	
	-- setup player gadgets
	local gadgets = {}
	local pos_x = 0
	local pos_y = 0
	local pos_z = 0

	for i = 1, #UIGlobals.Splitscreen.players do
		gadgets[i] = UIButtons.CloneXtGadgetByName( "Multiplayer\\Ingame\\MpSplitscreenResults.lua", "gamer_picture" )

		UIButtons.ChangePosition( gadgets[i], pos_x, pos_y, pos_z )
		UIButtons.SetParent( gadgets[i], SCUI.name_to_id["centre"], UIEnums.Justify.TopLeft )

		UIButtons.ChangeColour( UIButtons.FindChildByName( gadgets[i], "frame" ), "locked" )
		UIButtons.ChangeColour( UIButtons.FindChildByName( gadgets[i], "name" ), "locked" )
		UIButtons.ChangeColour( gadgets[i], "locked" )

		pos_x = pos_x + 10
		pos_y = pos_y - 11

		-- cast a mad wizard spell on the effects to offset it
		local end_time = UIButtons.GetXtVar( gadgets[i], "time_lines.0.time.end" )
		local start_time = UIButtons.GetXtVar( gadgets[i], "time_lines.0.time.start" )
		local time = end_time - start_time - 0.1

		UIButtons.SetXtVar( gadgets[i], "time_lines.0.time.end", end_time + ( time * (i-1) ) )
		UIButtons.SetXtVar( gadgets[i], "time_lines.0.time.start", start_time + ( time * (i-1) ) )
	end

	-- display the players in order
	local scores = Multiplayer.GetSplitscreenScores( true )
	local sorted_scores = Splitscreen_GetSortedScores( scores )
	
	for i = 1, #sorted_scores, 1 do
		local player_index = sorted_scores[ i ].player_index
		local p_table = UIGlobals.Splitscreen.players[ player_index ]

		local position_id = UIButtons.FindChildByName( gadgets[i], "position" )
		local name_id = UIButtons.FindChildByName( gadgets[i], "name" )
		local points_id = UIButtons.FindChildByName( gadgets[i], "points" )
		local frame_id = UIButtons.FindChildByName( gadgets[i], "frame" )

		local player_position = sorted_scores[ i ].pos
		UIButtons.ChangeText( position_id, "GAME_POS_" .. player_position )
		UIButtons.ChangeText( name_id, "PROFILE_PAD" .. p_table.pad .. "_NAME" )
		UIButtons.ChangeText( points_id, "MPL_SPLITSCREEN" .. ( player_index - 1 ) .. "_POINTS_RACE" )

		UIButtons.ChangeColour( frame_id, UIGlobals.Splitscreen.colours[ p_table.pad ] )
		UIButtons.ChangeColour( name_id, UIGlobals.Splitscreen.colours[ p_table.pad ] )
		UIButtons.ChangeColour( gadgets[i], "!255 255 255 255" )

		UIButtons.SetAlphaIntensity( gadgets[i], nil, 1.0 )
		UIButtons.SetAlphaIntensity( name_id, nil, 1.0 )
		UIButtons.SetAlphaIntensity( frame_id, nil, 1.0 )
		LocalGamerPicture( gadgets[i], p_table.pad )

		UIButtons.SetActive( position_id, true )
	end
end

function PostInit()
	UIButtons.SetActive( SCUI.name_to_id["gamer_picture"], false, true )
end

function FrameUpdate( time )
	GUI.countdown = GUI.countdown - time

	if GUI.countdown <= 0 then
		GUI.countdown = 0
		GoScreen( "Multiplayer\\Ingame\\MpRaceAwards.lua" )
	end
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end
	
	if GUI.finished == true then
		return
	end
end

function EnterEnd()
	--
end

function End()
	--
end
