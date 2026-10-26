GUI =
{
	finished = false,
	
	unlocks = {  },
	
	screen_id = -1,
	render_branch_ids = {  },
	current_branch = 0,
	
	carousel_branch = "Races",
	
	CanExit = function( _GUI )
		return false
	end,

	playing_video = false,
	unlock_kind = {  },
	unlock_type = { },
}

function Init()
	AddSCUI_Elements()
	CarouselApp_SetScreenTimers()
	
	StoreInfoLine()
	SetupInfoLine(UIText.INFO_A_NEXT, "GAME_SHARE_BUTTON")
	
	GUI.screen_id = SCUI.name_to_id[ "screen" ]
end

function PostInit()
	GUI.unlocks = Sp_UnlocksFilter( SinglePlayer.ProcessUnlocks() )
	
	for i, v in pairs( GUI.unlocks ) do
		if v.kind == "tier" then
			SpUnlocks_ProcessTier( v ) -- unlocked a new bad guy grr
		elseif v.kind == "event" then
			SpUnlocks_ProcessEvent( v )  -- bad guy come out to play grr * 2
		elseif v.kind == "rank" then
			SpUnlocks_ProcessRank( v )
		elseif v.kind == "vehicle" then
			SpUnlocks_ProcessVehicle( v ) -- beaten bad guy boo hoo
		elseif v.kind == "mod" then
			SpUnlocks_ProcessMod( v )
		end
	end
	
	SpUnlocks_Next()
end

function StartLoop( time )
	--
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if message == UIEnums.Message.ButtonLeftShoulder and arg2 == true then

		if Amax.CanUseShare() == true then
			UIGlobals.ShareFromWhatPopup=-1
			SetupCustomPopup( UIEnums.CustomPopups.SharingOptions )
		end

	elseif message == UIEnums.Message.PopupNext and arg2 == UIEnums.CustomPopups.SharingOptions then

		local type = 0
		if arg3 == UIEnums.ShareOptions.Facebook then
			type = 1
		end
		
		if GUI.unlock_type[ GUI.current_branch ][1] == "Tier" then 

			Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.UnlockedTier, type, GUI.unlocks[ GUI.current_branch ].tierId)
		
		elseif GUI.unlock_type[ GUI.current_branch ][1] == "Event" then 
			
			Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.UnlockedEvent, type, GUI.unlocks[ GUI.current_branch ].eventId)
		
		elseif GUI.unlock_type[ GUI.current_branch ][1] == "Rank" then 
		
			Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.UnlockedRank, type, GUI.unlocks[ GUI.current_branch ].rank)
		
		elseif GUI.unlock_type[ GUI.current_branch ][1] == "Vehicle" then 
		
			Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.UnlockedVehicle, type, GUI.unlocks[ GUI.current_branch ].vehicleId)
		
		elseif GUI.unlock_type[ GUI.current_branch ][1] == "BossVehicle" then 
		
			Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.UnlockedBossVehicle, type, GUI.unlocks[ GUI.current_branch ].vehicleId)
		
		elseif GUI.unlock_type[ GUI.current_branch ][1] == "Mod" then 
		
			Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.UnlockedMod, type, GUI.unlocks[ GUI.current_branch ].modId)
			
		end				
		
		StoreScreen( UIEnums.ScreenStorage.FE_SOCIAL_NETWORK, "SinglePlayer\\SpUnlocks.lua" )

		if arg3 == UIEnums.ShareOptions.Facebook then
			GoScreen( "Shared\\Facebook.lua", UIEnums.Context.Blurb )
		else
			GoScreen( "Shared\\Twitter.lua", UIEnums.Context.Blurb )
		end

	end	
	
	if message == UIEnums.Message.MenuNext then
		SpUnlocks_Next()
	end

	if message == UIEnums.Message.MovieFinished and arg1 == 0 then
		UIButtons.TimeLineActive( "movie_start", false )
		GUI.playing_video = false
	end

end

function FrameUpdate( time )
	--
end

function EnterEnd()
	RestoreInfoLine()
	SinglePlayer.ClearUnlocks()
end

function EndLoop( time )
	--
end

function End()	
	UISystem.DestroyMovie( 0 ) 
end

function SpUnlocks_Next()
	-- if movie playing
	
	if GUI.playing_video == true then
		UIButtons.TimeLineActive( "movie_start", false )
		GUI.playing_video = false		
		return
	else
		--this goes here to stop some corruption on the movie as it is fading out
		UISystem.DestroyMovie( 0 ) 
	end

	if GUI.current_branch == #GUI.render_branch_ids then
		CloseCurrentApp()
	else
		GUI.current_branch = GUI.current_branch + 1
		
		local id = GUI.render_branch_ids[ GUI.current_branch ]
		local rz_min = -7
		local rz_max =  7
		
		UIButtons.SetActive( id, true )
		UIButtons.PrivateTimeLineActive( id, "unlocked", true, 0 )
		
		local rx, ry, rz = UIButtons.GetOrientation( id )
		UIButtons.ChangeOrientation( id, rx, ry, rz + math.random( rz_min, rz_max ) )
			
			if GUI.unlock_kind[GUI.current_branch][1] ~= nil then
				print("playing Movie", GUI.unlock_kind[GUI.current_branch][1])
				UISystem.InitMovie( 0, GUI.unlock_kind[GUI.current_branch][1], false, nil, false, true )
				UISystem.EnableMovieLooping( 0, false )

				GUI.playing_video = true
				UIButtons.TimeLineActive( "movie_start", true, 0 )
			end

	end
end

function SpUnlocks_ProcessTier( t )
	--print( "SpUnlocks_ProcessTier" )
	--show_table( t )
	
	local tier = SinglePlayer.TierInfo()[ t.tier ]
	--show_table( tier )
	
	local branch = UIButtons.CloneXtGadgetByName( "SinglePlayer\\SpUnlocks.lua", "_tier_branch" )
	UIButtons.SetParent( branch, GUI.screen_id, UIEnums.Justify.MiddleCentre )
	
	UIButtons.ChangeText( UIButtons.FindChildByName( branch, "_tier_num" ), "GAME_NUM_" .. tier.starRequirement )
	UIButtons.ChangeText( UIButtons.FindChildByName( branch, "_tier_name" ), tier.name )
	
	local boss = SinglePlayer.BossInfo()[ t.tier ]
	--show_table( boss )
	
	-- TODO - tier boss pic --UIButtons.FindChildByName( branch, "_tier_boss_pic" )
	UIButtons.ChangeText( UIButtons.FindChildByName( branch, "_tier_boss_name" ), boss.name )
	UIButtons.ChangeText( UIButtons.FindChildByName( branch, "_tier_boss_bio" ), boss.bio )
	
	GUI.render_branch_ids[ #GUI.render_branch_ids + 1 ] = branch
	GUI.unlock_kind[ #GUI.unlock_kind + 1 ] = { "Ui\\Movies\\atvi.bik" }
	GUI.unlock_type[ #GUI.unlock_type + 1 ] = { "Tier" }
end

function SpUnlocks_ProcessEvent( e )
	-- these are only boss events
	--print( "SpUnlocks_ProcessBossEvent" )
	--show_table( e )
	
	local branch = UIButtons.CloneXtGadgetByName( "SinglePlayer\\SpUnlocks.lua", "_boss_branch" )
	UIButtons.SetParent( branch, GUI.screen_id, UIEnums.Justify.MiddleCentre )
	
	local boss = SinglePlayer.BossInfo()[ e.tier ]
	--show_table( boss )
	
	-- TODO - tier boss pic --UIButtons.FindChildByName( branch, "_boss_pic" )
	UIButtons.ChangeText( UIButtons.FindChildByName( branch, "_boss_name" ), boss.name )
	
	GUI.render_branch_ids[ #GUI.render_branch_ids + 1 ] = branch
	GUI.unlock_kind[ #GUI.unlock_kind + 1 ] = { "Ui\\Movies\\AmaxAttract.bik" }
	GUI.unlock_type[ #GUI.unlock_type + 1 ] = { "Event" }
end

function SpUnlocks_ProcessRank( r )
	--print( "SpUnlocks_ProcessRank" )
	--show_table( r )
	
	local branch = UIButtons.CloneXtGadgetByName( "SinglePlayer\\SpUnlocks.lua", "_rank_branch" )
	UIButtons.SetParent( branch, GUI.screen_id, UIEnums.Justify.MiddleCentre )
	
	UIButtons.ChangeText( UIButtons.FindChildByName( branch, "_rank_num" ), "GAME_NUM_" .. r.rank )
	UIButtons.ChangeText( UIButtons.FindChildByName( branch, "_rank_fan_amount" ), "GAME_NUM_" .. SinglePlayer.RankInfo()[ r.rank ].fans )
	
	GUI.render_branch_ids[ #GUI.render_branch_ids + 1 ] = branch
	GUI.unlock_kind[ #GUI.unlock_kind + 1 ] = {  }
	GUI.unlock_type[ #GUI.unlock_type + 1 ] = { "Rank" }
end

function SpUnlocks_ProcessVehicle( v )
	--print( "SpUnlocks_ProcessVehicle" )
	--show_table( v )
	
	local car = GameData.GetVehicle( v.vehicleId )
	--show_table( car )
	
	local branch = UIButtons.CloneXtGadgetByName( "SinglePlayer\\SpUnlocks.lua", "_car_branch" )
	UIButtons.SetParent( branch, GUI.screen_id, UIEnums.Justify.MiddleCentre )
	
	UIButtons.ChangeText( UIButtons.FindChildByName( branch, "_car_num" ), "GAME_NUM_" .. v.rank )
	-- TODO - car pic --UIButtons.FindChildByName( branch, "_car_pic" )
	UIShape.ChangeObjectName( UIButtons.FindChildByName( branch, "_car_icon" ), car.icon )
	UIButtons.ChangeText( UIButtons.FindChildByName( branch, "_stat_car_name" ), car.name )
	
	if car.man_texture_square == true then
		local man_square_id = UIButtons.FindChildByName( branch, "_stat_man_square" )
		UIButtons.SetActive( man_square_id, true )
		UIButtons.ChangeTexture( { filename = car.man_texture }, 0, man_square_id )
	else
		local man_rect_id = UIButtons.FindChildByName( branch, "_stat_man_rect" )
		UIButtons.SetActive( man_rect_id, true )
		UIButtons.ChangeTexture( { filename = car.man_texture }, 0, man_rect_id )
	end
	
	local class_icon_names = 
	{
		[ UIEnums.VehicleClass.A ] = "class_a",
		[ UIEnums.VehicleClass.B ] = "class_b",
		[ UIEnums.VehicleClass.C ] = "class_c",
		[ UIEnums.VehicleClass.D ] = "class_d",
	}
	
	UIShape.ChangeObjectName( UIButtons.FindChildByName( branch, "_stat_class_icon" ), class_icon_names[ car.class ] )
	
	local style_names =
	{
		[ UIEnums.VehicleUiStyle.Balanced ]		= UIText.RBC_STYLE_BALANCED,
		[ UIEnums.VehicleUiStyle.Drifty ]		= UIText.RBC_STYLE_DRIFTY,
		[ UIEnums.VehicleUiStyle.VeryDrifty ]	= UIText.RBC_STYLE_VERY_DRIFTY,
		[ UIEnums.VehicleUiStyle.Grippy ]		= UIText.RBC_STYLE_GRIPPY,
		[ UIEnums.VehicleUiStyle.VeryGrippy ]	= UIText.RBC_STYLE_VERY_GRIPPY,
		[ UIEnums.VehicleUiStyle.OffRoad ]		= UIText.RBC_STYLE_OFF_ROAD,
	}
	
	UIButtons.ChangeText( UIButtons.FindChildByName( branch, "_stat_style_text" ), style_names[ car.style ] )
	
	Amax.SetChickletsValue( UIButtons.FindChildByName( branch, "_chick_acceleration" ), car.acceleration )
	Amax.SetChickletsValue( UIButtons.FindChildByName( branch, "_chick_speed" ), car.speed )
	Amax.SetChickletsValue( UIButtons.FindChildByName( branch, "_chick_off_road" ), car.stability )
	Amax.SetChickletsValue( UIButtons.FindChildByName( branch, "_chick_health" ), car.strength )
	
	if v.boss == true then
		--print( "boss car" )
		
		local boss_pic_id = UIButtons.FindChildByName( branch, "_car_boss_pic" )
		UIButtons.SetActive( boss_pic_id, true )
		-- TODO - boss pic
		
		local boss_mod_id = UIButtons.FindChildByName( branch, "_car_mod_icon" )
		UIButtons.SetActive( boss_mod_id, true )
		-- TODO - boss mod

		GUI.render_branch_ids[ #GUI.render_branch_ids + 1 ] = branch
		GUI.unlock_kind[ #GUI.unlock_kind + 1 ] = { "Ui\\Movies\\BizarreSplash.bik" }
		GUI.unlock_type[ #GUI.unlock_type + 1 ] = { "BossVehicle" }
	else
	
		GUI.render_branch_ids[ #GUI.render_branch_ids + 1 ] = branch
		GUI.unlock_kind[ #GUI.unlock_kind + 1 ] = { }
		GUI.unlock_type[ #GUI.unlock_type + 1 ] = { "Vehicle" }
	end
end

function SpUnlocks_ProcessMod( m )
	--print( "SpUnlocks_ProcessMod" )
	--show_table( m )
	
	local mod = Multiplayer.GetAbility( m.modId )
	--show_table( mod )
	
	local branch = UIButtons.CloneXtGadgetByName( "SinglePlayer\\SpUnlocks.lua", "_mod_branch" )
	UIButtons.SetParent( branch, GUI.screen_id, UIEnums.Justify.MiddleCentre )
	
	local num_id = UIButtons.FindChildByName( branch, "_mod_num" )
	local pic_id = UIButtons.FindChildByName( branch, "_mod_pic" )
	local icon_id = UIButtons.FindChildByName( branch, "_mod_icon" )
	local name_id = UIButtons.FindChildByName( branch, "_mod_name" )
	local desc_id = UIButtons.FindChildByName( branch, "_mod_desc" )
	local boss_id = UIButtons.FindChildByName( branch, "_mod_boss_pic" )
	
	UIButtons.ChangeText( num_id, "GAME_NUM_" .. m.rank )
	-- TODO - mod pic
	UIShape.ChangeObjectName( icon_id, mod.icon_name )
	UIButtons.ChangeText( name_id, mod.name )
	UIButtons.ChangeText( desc_id, mod.desc )
	
	if m.boss == true then
		--print( "boss mod" )
		UIButtons.SetActive( boss_id, true )
		-- TODO - boss pic
	end
	
	GUI.render_branch_ids[ #GUI.render_branch_ids + 1 ] = branch
	GUI.unlock_kind[ #GUI.unlock_kind + 1 ] = {  }
	GUI.unlock_type[ #GUI.unlock_type + 1 ] = { "Mod" }
end
