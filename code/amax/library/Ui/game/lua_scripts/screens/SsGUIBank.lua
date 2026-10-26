function IsSplitScreen()
	return ( ( Amax.GetGameMode() == UIEnums.GameMode.SplitScreen ) and ( Amax.IsGameModeSplitScreen() == true ) )
end

function AreAllSplitScreenHumanPlayersWrecked()
	return ( Amax.AreAllSplitScreenHumanPlayersWrecked() )
end

function AreAllSplitScreenHumanPlayersEliminated()
	return ( Amax.AreAllSplitScreenHumanPlayersEliminated() )
end

function Splitscreen_GetSortedScores( scores )
	local player_to_score = {  }
	
	if #scores < #UIGlobals.Splitscreen.players then
		print( "we don't have enough scores for the players - wtf - should never happen!" )
		return player_to_score
	end
	
	for p, p_table in ipairs( UIGlobals.Splitscreen.players ) do
		player_to_score[ p ] = { player_index = p, score = scores[ p ].score, pos = scores[ p ].pos }
	end
	
	local sort_function = function( a, b )
			if a.score == b.score then
				return a.player_index < b.player_index
			else
				return a.score > b.score
			end
		end
	
	table.sort( player_to_score, sort_function )
	
	return player_to_score
end

function Splitscreen_Clear()
	UIGlobals.Splitscreen.primary_pad = -1
	UIGlobals.Splitscreen.players = {  }
	UIGlobals.Splitscreen.pad_to_player = {  }
	
	-- flag full profiles that are _not_ the original primary pad back to pre-load
	for i = 0, 3, 1 do
		if UIGlobals.ProfileState[ i ] == UIEnums.Profile.GamerProfile and i ~= UIGlobals.splitscreen_primary_pad_original then
			UIGlobals.ProfileState[ i ] = UIEnums.Profile.PreLoad
		end
	end
end

function Splitscreen_ClearMessages()
	UIGlobals.SplitscreenMessages = {  }
	
	for i, v in ipairs( UIGlobals.Splitscreen.players ) do
		UIGlobals.SplitscreenMessages[ #UIGlobals.SplitscreenMessages + 1 ] = 
		{
			finished = false,
			finished_position = -1,
			finished_timer = 0,
			wrecked = false,
			wrecked_timer = 0,
		}
	end
end

function Splitscreen_GetDummyId( player_index )
	local num_players = #UIGlobals.Splitscreen.players
	local dummy_id = nil
	
	if num_players == 2 then
		if player_index == 1 then
			dummy_id = SCUI.name_to_id[ "dummy_top" ]
		else
			dummy_id = SCUI.name_to_id[ "dummy_bottom" ]
		end
		
	elseif num_players == 3 then
		if player_index == 1 then
			dummy_id = SCUI.name_to_id[ "dummy_top" ]
		elseif player_index == 2 then
			dummy_id = SCUI.name_to_id[ "dummy_3" ]
		else
			dummy_id = SCUI.name_to_id[ "dummy_4" ]
		end
	end
	
	if dummy_id == nil then
		dummy_id = SCUI.name_to_id[ "dummy_" .. player_index ]
	end
	
	return dummy_id
end

function Splitscreen_GetDummyName( player_index )
	local num_players = #UIGlobals.Splitscreen.players
	
	if num_players == 2 then
		if player_index == 1 then
			return "top"
		else
			return "bottom"
		end
		
	elseif num_players == 3 then
		if player_index == 1 then
			return "top"
		elseif player_index == 2 then
			return "3"
		else
			return "4"
		end
	end
	
	return "" .. player_index
end

function Splitscreen_SetFrontendSplits()
	local num_players = #UIGlobals.Splitscreen.players
	
	if num_players == 2 then
		UIButtons.SetActive( SCUI.name_to_id[ "split_x_top" ], false )
		UIButtons.SetActive( SCUI.name_to_id[ "split_x_bottom" ], false )
		
	elseif num_players == 3 then
		UIButtons.SetActive( SCUI.name_to_id[ "split_x_top" ], false )
	end
end

-- aligned to safe area
-- do we need to support 2-player vertical split?
function GetScreenBounds( num_players, i, safe_area )
	if safe_area == nil then
		safe_area = true
	end
			
	local sl = Screen.safe.left
	local sr = Screen.safe.right
	local st = Screen.safe.top
	local sb = Screen.safe.bottom
	
	if safe_area == false then
		sl = Screen.left
		sr = Screen.right
		st = Screen.top
		sb = Screen.bottom
	end
	
	local offset_x = 4
	local offset_y = 4
	
	if safe_area == false then
		offset_x = 0
		offset_y = 0
	end
	
	local cx = 320
	local cy = 240

	if num_players == 2 then
		if i == 1 then
			sb = cy - offset_y
		elseif i == 2 then
			st = cy + offset_y
		end
	elseif num_players == 3 then
		if i == 1 then
			local hx = ( sr - sl ) * 0.25
			sl = cx - hx - offset_x
			sr = cx + hx + offset_x
			sb = cy - offset_y
		elseif i == 2 then
			sr = cx - offset_x
			st = cy + offset_y
		elseif i == 3 then
			sl = cx + offset_x
			st = cy + offset_y
		end		
	elseif num_players == 4 then
		if i == 1 then
			sr = cx - offset_x
			sb = cy - offset_y
		elseif i == 2 then
			sl = cx + offset_x
			sb = cy - offset_y
		elseif i == 3 then
			sr = cx - offset_x
			st = cy + offset_y
		elseif i == 4 then
			sl = cx + offset_x
			st = cy + offset_y
		end
	end
	
	return sl, sr, st, sb
end

function Splitscreen_AddSplits()
	local num_players = Amax.GetNumViewports()
	
	if num_players == 1 then
		return
	end
	
	-- split lines
	local black_line =
	{
		panel = 4,
		type = UIEnums.ButtonTypes.GRAPHIC,
		pos = { x = -1, y = -1 },
		size = { x = -1, y = -1, },
		scale = { x = 1, y = 1, },
		justify = UIEnums.Justify.TopLeft,
		colour_style = "!255 0 0 0",
		textures = 
		{
			{
				name = UITexture.Textures.null,
				pos = { u = 0, v = 0, },
				size = { u = 0, v = 0, },
			},
		},
	}

	local line_size_v = 4
	local line_size_h = 8

	-- do we have a vertical split option?
	
	if num_players == 3 or num_players == 4 or num_players == 2 then
		-- middle split
		black_line.pos.x = 320
		black_line.pos.y = 240
		black_line.size.x = Screen.width + 2
		black_line.size.y = line_size_h
		black_line.justify = UIEnums.Justify.MiddleCentre
		UIButtons.AddButton( black_line )
	end

	if num_players == 4 then
		-- middle split
		black_line.pos.x = 320
		black_line.pos.y = 240
		black_line.size.x = line_size_v
		black_line.size.y = Screen.height + 2
		black_line.justify = UIEnums.Justify.MiddleCentre
		UIButtons.AddButton( black_line )
	end
		
	if num_players == 3 then
		-- middle-bottom split
		black_line.size.x = line_size_v
		black_line.size.y = Screen.height / 2 + 2
		black_line.justify = UIEnums.Justify.TopCentre
		UIButtons.AddButton( black_line )
		
		-- black squares either side of player 1
		local black_square =
		{
			name = "black_square",
			panel = 4,
			type = UIEnums.ButtonTypes.GRAPHIC,
			pos = { x = -1, y = -1 },
			size = { x = -1, y = -1, },
			scale = { x = 1, y = 1, },
			justify = UIEnums.Justify.TopLeft,
			colour_style = "!255 0 0 0",
			textures = 
			{
				{
					name = UITexture.Textures.null,
					pos = { u = 0, v = 0, },
					size = { u = 0, v = 0, },
				},
			},
		}
		
		local sl, sr, st, sb = GetScreenBounds( num_players, 1, false )
		
		-- left
		black_square.pos.x = sl
		black_square.pos.y = 240
		black_square.size.x = sl - Screen.left + 2
		black_square.size.y = Screen.height / 2 + 2
		black_square.justify = UIEnums.Justify.BottomRight
		UIButtons.AddButton( black_square )
		
		-- right
		black_square.pos.x = sr
		black_square.justify = UIEnums.Justify.BottomLeft
		UIButtons.AddButton( black_square )
	end
end