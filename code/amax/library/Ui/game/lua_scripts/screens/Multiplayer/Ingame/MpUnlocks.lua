GUI =
{
	finished = false,
	
	total_unlocks = {},
	 
	unlock_order =
	{
		"playlists",
		"features",
		"packs",
		"upgrades",
		"abilities",
		"cars",
	},
	
	unlock_times =
	{
		2.2, -- playlist
		2.0, -- features
		2.2, -- challenge pack
		2.2, -- upgrades
		2.7, -- abilities
		2.2, -- cars
	},
	
	unlock_titles =
	{
		UIText.MP_UNLOCK_TYPE_PLAYLIST,
		UIText.MP_UNLOCK_TYPE_FEATURE,
		UIText.MP_UNLOCK_TYPE_PACK,
		UIText.MP_UNLOCK_TYPE_CAR_UPGRADE,
		UIText.MP_UNLOCK_TYPE_MOD,
		UIText.MP_UNLOCK_TYPE_CAR,
	},
	
	unlock_templates =
	{
		"playlist_template",
		"feature_template",
		"feature_template",
		"upgrade_template",
		"mod_template",
		"car_template",
	},
		
	current_type		= 1,	-- current type index
	current_unlock		= 1,	-- current unlock of the current type
	
	max					= 0,	-- amount of unlocks in the current type
	timer				= 0,
	end_time			= 0.75,
	
	finished_unlocks	= false,
}

function Init()
	AddSCUI_Elements( )
	UIButtons.CloneXtGadgetByName( "Multiplayer\\Ingame\\MpRaceResults.lua", "MpBgStuff" )
	
	-- cant add this in GUI because they are out of scope
	GUI.unlock_functions =
	{
		MpUnlocks_SetupPlaylist,
		MpUnlocks_SetupFeature,
		MpUnlocks_SetupFeature,
		MpUnlocks_SetupUpgrade,
		MpUnlocks_SetupAbility,
		MpUnlocks_SetupCar,
	}
	
	if Amax.GetGameMode() == UIEnums.GameMode.SinglePlayer then
		GUI.total_unlocks = SinglePlayer.VehicleUnlocks()
	else
		local progression = Multiplayer.GetProgressionForRace()
		if IsTable( progression ) == true then
			GUI.total_unlocks = progression.unlocks
		end
	end
end

function PostInit()
	if IsTable( GUI.total_unlocks ) == true then
		-- spin until we find a unlock table with items to show
		for i = 1, #GUI.unlock_order do
			if MpUnlocks_SetupType( GUI.current_type ) == true then
				break
			end
			GUI.current_type = GUI.current_type + 1
		end
		
		MpUnlocks_SetupUnlock( GUI.current_type, GUI.current_unlock )
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
end

function FrameUpdate( time )
	if GUI.finished == true then
		return
	end
	
	GUI.timer = GUI.timer - time
	
	if GUI.finished_unlocks == true then
		if GUI.timer <= 0 then
			if Amax.GetGameMode() == UIEnums.GameMode.SinglePlayer then
				PopScreen()
			else
				GoScreen( "Multiplayer\\Ingame\\MpRaceAwards.lua" )
			end
		end
		return -- no further logic
	end
	
	if GUI.timer <= 0 then
		-- move on to next index
		GUI.current_unlock = GUI.current_unlock + 1
		
		if GUI.current_unlock > GUI.max then
			-- move on to next type
			GUI.current_type = GUI.current_type + 1
			
			if GUI.current_type > #GUI.unlock_order then
				MpUnlocks_Finish()
			else
				-- spin until we find a unlock table with items to show
				for i = 1, #GUI.unlock_order do
					if MpUnlocks_SetupType( GUI.current_type ) == true then
						break
					end
					GUI.current_type = GUI.current_type + 1
				end
			
				-- just incase we run over
				if GUI.current_type > #GUI.unlock_order then
					MpUnlocks_Finish()
				end
			end
		end
		
		-- do we have a valid unlock to show
		if GUI.finished_unlocks == false and #GUI.unlocks > 0 then
			MpUnlocks_SetupUnlock( GUI.current_type, GUI.current_unlock )
		end
	end
end

function Render()
	--
end

function EndLoop( time )
	--
end

function End()
end

function MpUnlocks_Finish()
	UIButtons.TimeLineActive( "hide", true )

	GUI.finished_unlocks = true
	GUI.timer = GUI.end_time
end

function MpUnlocks_SetupType( type )
	-- how many unlocks do we have
	GUI.unlocks = GUI.total_unlocks[GUI.unlock_order[type]]
	GUI.current_unlock = 1	
	
	if IsTable( GUI.unlocks ) == false then
		return false
	end
	
	UIButtons.TimeLineActive( "show_type", true, 0 )
	
	GUI.max		= #GUI.unlocks
	GUI.timer	= GUI.unlock_times[type]
	
	-- turn all off
	for i, v in ipairs( GUI.unlock_templates ) do
		UIButtons.SetActive( SCUI.name_to_id[v], false )
	end
	
	if GUI.max > 0 then
		-- turn current on
		UIButtons.SetActive( SCUI.name_to_id[GUI.unlock_templates[type]], true )
		UIButtons.ChangeText( SCUI.name_to_id["unlocked_text"], GUI.unlock_titles[type] ) 
		return true
	end
	
	return false
end

function MpUnlocks_RandomTrail()
	
	local index = math.random(4)
	UIButtons.TimeLineActive( "trail_" .. index, true, 0, true )
	
	-- turn on
	UIButtons.SetActive( SCUI.name_to_id["trail_" .. index .. "_a"], true )
	UIButtons.SetActive( SCUI.name_to_id["trail_" .. index .. "_b"], true )
end

function MpUnlocks_SetupUnlock( type, index )
	UIButtons.TimeLineActive( "show_unlock", true, 0 )

	local next_unlock = GUI.unlocks[index]
	if IsTable( next_unlock ) == true then
		GUI.unlock_functions[type]( next_unlock )
		GUI.timer = GUI.unlock_times[type]
		
		MpUnlocks_RandomTrail()
	end
end

function MpUnlocks_SetupPlaylist( unlock )
	local root = SCUI.name_to_id["playlist_template"]
	local thumbnail_id	= UIButtons.FindChildByName( root, "thumbnail" )

	UIButtons.ChangeText( UIButtons.FindChildByName( root, "name" ), unlock.name )
	UIButtons.ChangeText( UIButtons.FindChildByName( root, "desc" ), unlock.desc )
	UIShape.ChangeObjectName( UIButtons.FindChildByName( root, "icon" ), unlock.icon )
	UIButtons.ChangeTexture( { filename = unlock.texture }, 0, thumbnail_id )
end

function MpUnlocks_SetupFeature( unlock )
	local root = SCUI.name_to_id["feature_template"]
	
	UIButtons.ChangeText( UIButtons.FindChildByName( root, "name" ), unlock.name )
	UIButtons.ChangeText( UIButtons.FindChildByName( root, "desc" ), unlock.desc )
	UIShape.ChangeSceneName( UIButtons.FindChildByName( root, "icon" ), unlock.sheet )
	UIShape.ChangeObjectName( UIButtons.FindChildByName( root, "icon" ), unlock.icon )
end

function MpUnlocks_SetupUpgrade( unlock )
	local root = SCUI.name_to_id["upgrade_template"]
	local car_icon_id = UIButtons.FindChildByName( root, "car_icon" )
	
	local prefix = unlock.challenge .. "_" .. unlock.tier
	local car_info = GameData.GetVehicle( Amax.GetCurrentCarID() )

	-- upgrade info
	UIButtons.ChangeText( UIButtons.FindChildByName( root, "title" ), "MPL_CAR_UPGRADE_NAME" .. prefix )
	UIButtons.ChangeText( UIButtons.FindChildByName( root, "desc" ), "MPL_CAR_UPGRADE_DESC" .. prefix )
	UIButtons.ChangeText( UIButtons.FindChildByName( root, "name" ), "MPL_CAR_UPGRADE_TYPE" .. prefix )
	UIShape.ChangeObjectName( UIButtons.FindChildByName( root, "icon" ), UIGlobals.CarUpgradeIcons[unlock.type] )
	
	-- car info
	UIButtons.ChangeText( UIButtons.FindChildByName( root, "car_name" ), car_info.name )
	UIShape.ChangeSceneName( car_icon_id, car_info.sheet )
	UIShape.ChangeObjectName( car_icon_id, car_info.icon )
	
	-- swap unlock text
	UIButtons.ChangeText( SCUI.name_to_id["unlocked_text"], Select( unlock.challenge == UIEnums.MpUpgradeChallenges.Races, UIText.MP_UNLOCK_TYPE_CAR_UPGRADE, UIText.MP_UNLOCK_TYPE_CAR_PAINT ) )
	UIButtons.TimeLineActive( "show_type", true, 0 )
end

function MpUnlocks_SetupAbility( unlock )
	local root = SCUI.name_to_id["mod_template"]
	
	UIButtons.ChangeText( UIButtons.FindChildByName( root, "name" ), unlock.name )
	UIButtons.ChangeText( UIButtons.FindChildByName( root, "desc" ), unlock.desc )
	UIShape.ChangeObjectName( UIButtons.FindChildByName( root, "icon" ), unlock.icon )
end

function MpUnlocks_SetupCar( unlock )
	local root			= SCUI.name_to_id["car_template"]
	local name_id		= UIButtons.FindChildByName( root, "name" )
	local class_id		= UIButtons.FindChildByName( root, "class" )
	local style_id		= UIButtons.FindChildByName( root, "style" )
	local thumbnail_id	= UIButtons.FindChildByName( root, "thumbnail" )
	local man_rect_id	= UIButtons.FindChildByName( root, "manufacturer_rect" )
	local man_square_id = UIButtons.FindChildByName( root, "manufacturer_square" )
	
	local car_info = GameData.GetVehicle( unlock.id )
	
	if car_info.man_texture_square == true then
		UIButtons.SetActive( man_square_id, true )
		UIButtons.SetActive( man_rect_id, false )
		UIButtons.ChangeTexture( { filename = car_info.man_texture }, 0, man_square_id )
	else
		UIButtons.SetActive( man_square_id, false )
		UIButtons.SetActive( man_rect_id, true )
		UIButtons.ChangeTexture( { filename = car_info.man_texture }, 0, man_rect_id )
	end
	
	-- change name, texture and class icon/style name
	UIButtons.ChangeText( name_id, car_info.name )
	UIButtons.ChangeTexture( { filename = car_info.tag }, 0, thumbnail_id )
	UIShape.ChangeObjectName( class_id, UIGlobals.ClassIcons[ car_info.class ] )
	UIButtons.ChangeText( style_id, UIGlobals.CarStyles[ car_info.style ] )
end