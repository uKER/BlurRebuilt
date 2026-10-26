GUI =
{
	finished = false,
	carousel_branch = "Stickers",
	
	category_colours = 
	{
		"Support_4", -- Normal stickers
		"Main_1", -- Multiplayer stickers
		"Support_0", -- Powerup stickers
		"Support_2", -- Boss stickers
	},
	
	boss_stickers = 
	{
		7, 10, 12, 16, 19, 21, 27, 30, 35,
	},
	
	current_page = 1,
	max_pages = 3,
	max_achievements = 0,
	
	sticker_node_list_ids = { },
	sticker_node_ids = { },
	
	current_mode = 0,
	
	ps3_trophy_values = 
	{
		"gfx_trophy_platinum", -- just in case
		"gfx_trophy_gold",
		"gfx_trophy_silver",
		"gfx_trophy_bronze",
	},
	
	do_refresh = false,
	do_page = nil,

	CanExit = function( _GUI )
		return true
	end,
}

function Init()
	AddSCUI_Elements()
	Amax.ChangeUiCamera( "Op_1", 0.6, 0 )
	
	CarouselApp_SetScreenTimers()
	
	UISystem.PlaySound( UIEnums.SoundEffect.GraphicForward )
	net_SetRichPresence( UIEnums.RichPresence.Stickerbook )
	
	StoreInfoLine()
end

function PostInit()
	UIButtons.SetActive( SCUI.name_to_id["stickerNode"], false )
	
	GUI.pager_dummy = SCUI.name_to_id[ "pager" ]
	GUI.max_achievements = Amax.GetNumberOfAchievements(0)
	local page = 0
	
	for i=1, GUI.max_achievements do
		if i % 20 == 1 then
			page = page + 1
			GUI.sticker_node_list_ids[page] = UIButtons.CloneXtGadgetByName( "Shared\\Stickers.lua", "stickerTable" )
			UIButtons.SetParent( GUI.sticker_node_list_ids[page], GUI.pager_dummy, UIEnums.Justify.MiddleCentre )
			UIButtons.SetSelected( GUI.sticker_node_list_ids[page], page == 1 )
			local x,y,z = UIButtons.GetPosition( GUI.sticker_node_list_ids[page] )
			UIButtons.ChangePosition( GUI.sticker_node_list_ids[page], x + (page+1)*50,y,z )
		end
		
		GUI.sticker_node_ids[i] = UIButtons.CloneXtGadgetByName( "Shared\\Stickers.lua", "stickerNode" )
		UIButtons.AddListItem( GUI.sticker_node_list_ids[page], GUI.sticker_node_ids[i], i )
	end
	
	UIButtons.SetActive( GUI.sticker_node_list_ids[GUI.current_page], true )
	
	GUI.max_pages = page
	UIButtons.ChangeText( SCUI.name_to_id["txt_page"], "GAME_PROGRESS_" .. GUI.current_page .. "_" .. GUI.max_pages )
	
	SetupBossNodes()
	
	RefreshProgressDisplay()
	RefreshStickerTable()
	
	if UIEnums.CurrentPlatform == UIEnums.Platform.PS3 then
		UIButtons.SetActive( SCUI.name_to_id["txt_score"], false, true )
	end
	
	UIButtons.ChangeText( SCUI.name_to_id["txt_achievement_num"], "GAME_PROGRESS_" .. 1 .. "_" .. GUI.max_achievements )
	UIButtons.TimeLineActive( "right_fade", true, 0 )
	UIButtons.TimeLineActive( "Hide_Carousel", true )
	UIButtons.PrivateTimeLineActive( GUI.sticker_node_ids[1], "flash", true, 0 ) -- we come in to the screen on location 1
	
	-- plonk this here for now, we can put some more funky logic for what counts as "viewing" un-viewed stickers later if we wish
	Amax.HasNewStickers( false )
	
	Stickers_SetupInfoLines()

end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end
	
	if message == UIEnums.Message.ButtonLeft or message == UIEnums.Message.ButtonRight then
	end

	if message == UIEnums.Message.ButtonLeftShoulder and arg2 == true then

		local index= UIButtons.GetSelection( GUI.sticker_node_list_ids[GUI.current_page] ) - 1

		if Amax.CanUseShare() == true and Amax.CanShareSticker(index) == true then
			UIGlobals.ShareFromWhatPopup=-1
			SetupCustomPopup( UIEnums.CustomPopups.SharingOptions )
		end

	elseif message == UIEnums.Message.PopupNext and arg2 == UIEnums.CustomPopups.SharingOptions then

		local index= UIButtons.GetSelection( GUI.sticker_node_list_ids[GUI.current_page] ) - 1
		
		if arg3 == UIEnums.ShareOptions.Facebook then
			
			Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.Achievement, 1, index)
			
			StoreScreen( UIEnums.ScreenStorage.FE_SOCIAL_NETWORK, "Shared\\Stickers.lua" )
			GoScreen( "Shared\\Facebook.lua", UIEnums.Context.Blurb )
		
		elseif arg3 == UIEnums.ShareOptions.Twitter then

			Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.Achievement, 0, index)

			StoreScreen( UIEnums.ScreenStorage.FE_SOCIAL_NETWORK, "Shared\\Stickers.lua" )
			GoScreen( "Shared\\Twitter.lua", UIEnums.Context.Blurb )

		elseif arg3 == UIEnums.ShareOptions.Blurb then

			Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.Achievement, 2, index)
			
		end		
	end
	
	if message == UIEnums.Message.ButtonRightTrigger then
		if GUI.max_pages > GUI.current_page then
			Page(true, true)
			GUI.do_refresh = true
		end
	end
	
	-- handles paging, but since it is deferred to the next frame, we must only react to ONE paging
	-- request (since there are multiple ways to page, e.g. left trigger and left on the dpad, if these
	-- are pressed at the same time then it blows up)
	if GUI.do_refresh ~= true then
	
		if message == UIEnums.Message.ButtonLeftTrigger then
			if 1 < GUI.current_page then
				Page(false, true)
				GUI.do_refresh = true
			end
		end
		
		if message == UIEnums.Message.ButtonRight then
			if GUI.max_pages > GUI.current_page then
				if UIButtons.GetSelectionIndex( GUI.sticker_node_list_ids[GUI.current_page] ) % 4 == 3 then
					GUI.do_page = true
				end
			end
			GUI.do_refresh = true
		end
		
		if message == UIEnums.Message.ButtonLeft then
			if 1 < GUI.current_page then
				if UIButtons.GetSelectionIndex( GUI.sticker_node_list_ids[GUI.current_page] ) % 4 == 0 then
					GUI.do_page = false
				end
			end
			GUI.do_refresh = true
		end
		
		if message == UIEnums.Message.ButtonDown then
			GUI.do_refresh = true
		end
		
		if message == UIEnums.Message.ButtonUp then
			GUI.do_refresh = true
		end
	end

	if message == UIEnums.Message.MenuBack and arg2 == true then
		PlaySfxBack()
	end
	
end

function StartLoop( time )
	DeferCam_Update( time )
end

function FrameUpdate( time )
	DeferCam_Update( time )
	
	if GUI.do_page ~= nil then
		Page( GUI.do_page, false )
		GUI.do_page = nil
	end

	if GUI.do_refresh == true then
		GUI.do_refresh = false
		RefreshProgressDisplay()
		RefreshStickerTable()
		
		local location = UIButtons.GetSelection( GUI.sticker_node_list_ids[GUI.current_page] )
		UIButtons.TimeLineActive( "flash", false )
		UIButtons.PrivateTimeLineActive( GUI.sticker_node_ids[location], "flash", true, 0 )
		UIButtons.ChangeText( SCUI.name_to_id["txt_achievement_num"], "GAME_PROGRESS_" .. location .. "_" .. GUI.max_achievements )
		
		Stickers_SetupInfoLines()
	end
end

function EnterEnd()
	UIButtons.TimeLineActive( "Hide_Carousel", false )
	
	UISystem.PlaySound( UIEnums.SoundEffect.GraphicBackward )
	RestoreInfoLine()
end

function EndLoop( time )
	--
end

function End()
end

function Page(left, byTrigger)
	UISystem.PlaySound( UIEnums.SoundEffect.StickersPage )
	
	local table = GUI.sticker_node_list_ids[GUI.current_page]
	local current_index = UIButtons.GetSelectionIndex( table )
	
	if left == false then
		UIButtons.TimeLineActive( "page_" .. GUI.current_page, false )
		GUI.current_page = GUI.current_page - 1
		UIButtons.SetSelected( table, false )
	elseif left == true then
		GUI.current_page = GUI.current_page + 1
		UIButtons.SetSelected( table, false )
		UIButtons.TimeLineActive( "page_" .. GUI.current_page, true )
	end
	
	local new_table = GUI.sticker_node_list_ids[GUI.current_page]
	UIButtons.SetSelected( new_table, true )
	
	if ( GUI.current_page ~= GUI.max_pages or current_index < GUI.max_achievements % 20 ) and byTrigger == true then
		UIButtons.SetSelectionByIndex( new_table, current_index )
	elseif ( GUI.current_page ~= GUI.max_pages or current_index - (current_index % 4) < GUI.max_achievements % 20 ) and byTrigger == false then
		if left == true then
			UIButtons.SetSelectionByIndex( new_table, current_index - (current_index % 4) )
		elseif left == false then
			UIButtons.SetSelectionByIndex( new_table, current_index + 3 - (current_index % 4) )
		end
	end
	
	UIButtons.ChangeText( SCUI.name_to_id["txt_page"], "GAME_PROGRESS_" .. GUI.current_page .. "_" .. GUI.max_pages )
	
	if left == true then
		UIButtons.TimeLineActive( "left_fade", true )
		UIButtons.TimeLineActive( "move_right", true, 0 )
		if GUI.max_pages == GUI.current_page then
			UIButtons.TimeLineActive( "right_fade", false )
		end
	elseif left == false then
		UIButtons.TimeLineActive( "right_fade", true )
		UIButtons.TimeLineActive( "move_left", true, 0 )
		if GUI.current_page == 1 then
			UIButtons.TimeLineActive( "left_fade", false )
		end
	end
end

function RefreshStickerTable()
	for i=1,20 do
		local location = i + (GUI.current_page - 1) * 20
		if location <= GUI.max_achievements then
			for j=1,4 do
				SetupDividingLines( location, j )
				local iconIndex, iconCategory = Amax.GetAchievementTextureInfo( location - 1 )
				local iconQuadId = UIButtons.FindChildByName( GUI.sticker_node_ids[location], "hide_node_" .. j)
				local u,v = GetQuadUV( j )
				
				-- display a quadrant of either the sticker, or the "sticker hidden" icon
				if Amax.IsStickerInQuadrantComplete( location - 1, j ) == true then
					UIButtons.ChangeTextureUV( iconQuadId, 0, (iconIndex * (1/64)) + u, v )
					UIButtons.ChangeColour( iconQuadId, GUI.category_colours[iconCategory + 1] )
				else
					UIButtons.ChangeTextureUV( iconQuadId, 0, (61/64) + u, v )
					UIButtons.ChangeColour( iconQuadId, "Dark_White" )
				end
			end
		end
	end
end

function RefreshProgressDisplay()
	local location = UIButtons.GetSelection( GUI.sticker_node_list_ids[GUI.current_page] )
	local numStickers = Amax.GetNumStickersInAchievement(location - 1)
	
	Amax.PopulateAchievementName( location - 1, SCUI.name_to_id["txt_AchievementName"] )
	local iconIndex, iconCategory = Amax.GetAchievementTextureInfo( location - 1 )
	local score = Amax.GetAchievementScore(location - 1)
	local signedIn = UIGlobals.ProfileState[ Profile.GetPrimaryPad() ] == UIEnums.Profile.GamerProfile
	if UIEnums.CurrentPlatform == UIEnums.Platform.Xenon then
		if signedIn == true then
			UIButtons.ChangeText( SCUI.name_to_id["txt_score"], "GAME_NUM_" .. score )
		else
			UIButtons.SetActive( SCUI.name_to_id["txt_score"], false, true )
		end
	elseif UIEnums.CurrentPlatform == UIEnums.Platform.PS3 then
		for i=2,4 do
			UIButtons.SetActive( SCUI.name_to_id[GUI.ps3_trophy_values[i]], (i==score and signedIn == true) )
		end
	end
	
	local complete = true
	
	for i=1, 4 do
		if i <= numStickers then
			local progress, goal, oneShot = Amax.GetStickerInformation( location - 1, i - 1, SCUI.name_to_id["Desc_" .. i] )
			SetupProgressBar( progress, goal, oneShot, i )
		end
		
		local iconQuadId = SCUI.name_to_id["hide_" .. i]
		local u,v = GetQuadUV( i )
		if Amax.IsStickerInQuadrantComplete( location - 1, i ) == true then
			UIButtons.ChangeTextureUV( iconQuadId, 0, (iconIndex * (1/64)) + u, v )
			UIButtons.ChangeColour( iconQuadId, GUI.category_colours[iconCategory + 1] )
		else
			complete = false
			UIButtons.ChangeTextureUV( iconQuadId, 0, (61/64) + u, v )
			UIButtons.ChangeColour( iconQuadId, "Dark_White" )
		end
		
	end
	
	local boss_sticker_offset=1
	if UIEnums.CurrentPlatform == UIEnums.Platform.PS3 then
		boss_sticker_offset=0
	end
	
	-- check if we want to display the boss picture in this one
	for i=1,#GUI.boss_stickers do
		local cutout_id = UIButtons.FindChildByName( GUI.boss_nodes[i], "b" .. i .. "_c" )
		local backing_id = UIButtons.FindChildByName( GUI.boss_nodes[i], "b" .. i .. "_l2" )
		UIButtons.SetActive( cutout_id, (GUI.boss_stickers[i]+boss_sticker_offset) == location )
		if complete == true then
			UIButtons.SetActive( backing_id, true )
			UIButtons.PrivateTimeLineActive( backing_id, "gadget_selected", (GUI.boss_stickers[i]+boss_sticker_offset) == location, 0 )
			UIButtons.SetAlphaIntensity( cutout_id, nil, 1 )
		else
			UIButtons.SetActive( backing_id, false )
			UIButtons.PrivateTimeLineActive( UIButtons.FindChildByName( GUI.boss_nodes[i], "b" .. i .. "_l2" ), "gadget_selected", false, 0 )
			UIButtons.SetAlphaIntensity( cutout_id, nil, 0 )
		end
	end
	
	DeactivateUnusedProgressBars( numStickers )
	DeactivateUnusedTables()
end

function SetupProgressBar( progress, goal, oneShot, sticker )
	local progress_id = SCUI.name_to_id["Fill_Progress_" .. sticker]

	UIButtons.ChangeScale( progress_id, (progress/goal), 1 )
	UIButtons.SetActive( SCUI.name_to_id["Fill_" .. sticker], true, true )
	
	if goal == progress then
		UIButtons.SetActive( SCUI.name_to_id["Tick_" .. sticker], true, true )
	else
		UIButtons.SetActive( SCUI.name_to_id["Tick_" .. sticker], false, true )	
	end
	
	if oneShot == false then
		UIButtons.ChangeText( SCUI.name_to_id["Amount_" .. sticker], "GAME_PROGRESS_" .. math.floor( progress ) .. "_" .. math.floor( goal ) )
	elseif oneShot ~= nil then
		if goal == progress then
			UIButtons.ChangeText( SCUI.name_to_id["Amount_" .. sticker], UIText.ACH_UI_COMPLETE )
		else
			UIButtons.ChangeText( SCUI.name_to_id["Amount_" .. sticker], UIText.ACH_UI_INCOMPLETE )
		end
	end
end

function DeactivateUnusedProgressBars( slotsTaken )
	for i=slotsTaken + 1, 4 do
		UIButtons.SetActive( SCUI.name_to_id["Fill_" .. i], false, true )
	end
end

function DeactivateUnusedTables()
	for i=1, GUI.max_pages do
		--UIButtons.SetActive( GUI.sticker_node_list_ids[i], true--[[( i == GUI.current_page )]] )
	end
end

function Stickers_SetupInfoLines()

	local index = UIButtons.GetSelection( GUI.sticker_node_list_ids[GUI.current_page] ) - 1
	local options = {  }

	options[ #options + 1 ] = UIText.INFO_B_BACK

	if Amax.CanUseShare() == true and Amax.CanShareSticker(index) == true then
		options[ #options + 1 ] = "GAME_SHARE_BUTTON"
	end
		
	SetupInfoLine( unpack( options ) )
end

function SetupDividingLines(location, quadrant)
	local nextQuadRound = quadrant + 1
	if nextQuadRound == 5 then
		nextQuadRound = 1
	end
	
	local dividingLine = Amax.DividingLineBetweenQuadrants( location - 1, quadrant, nextQuadRound )
	
	if quadrant == 1 then
		UIButtons.SetActive( UIButtons.FindChildByName( GUI.sticker_node_ids[location], "LineVerticalTop_Sticker" ), dividingLine )
	elseif quadrant == 2 then
		UIButtons.SetActive( UIButtons.FindChildByName( GUI.sticker_node_ids[location], "LineHorizontalRight_Sticker" ), dividingLine )
	elseif quadrant == 3 then
		UIButtons.SetActive( UIButtons.FindChildByName( GUI.sticker_node_ids[location], "LineVerticalBottom_Sticker" ), dividingLine )
	else
		UIButtons.SetActive( UIButtons.FindChildByName( GUI.sticker_node_ids[location], "LineHorizontalLeft_Sticker" ), dividingLine )
	end
end

function SetupBossNodes()
	GUI.boss_nodes = {}
	for i=1,#GUI.boss_stickers do
		local items_id = UIButtons.CloneXtGadgetByName( "SpSCUI", "tier_OnBoss" .. i )
		UIButtons.SetActive( UIButtons.FindChildByName( items_id, "b" .. i .. "_bg" ), false )
		GUI.boss_nodes[i] = items_id
		UIButtons.SetParent( items_id,  SCUI.name_to_id["BoxFrame_Progress"], UIEnums.Justify.MiddleCentre )
		UIButtons.ChangeScale( items_id, 1.6, 1.6 )
		if i == 1 then -- shannon v1 seems to need moving down more for some reason? others are all consistent
			UIButtons.ChangePosition( items_id, 0, -17 )
		else
			UIButtons.ChangePosition( items_id, 0, -5 )
		end
		local cutout_id = UIButtons.FindChildByName( GUI.boss_nodes[i], "b" .. i .. "_c" )
		UIButtons.SetActive( cutout_id, false )
	end
end

-- Gets the amount of local addition required to correctly u/v a quad of a sticker on the icon sheet
function GetQuadUV( quad )
	local u = 0
	local v = 0
	if quad == 3 or quad == 4 then
		v = 0.5
	end
	if quad == 2 or quad == 3 then
		u = 1/128
	end
	
	return u,v
end
