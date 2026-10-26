GUI =
{
	finished = false,
	
	screen_timer = 0,
	lerp_timer = 0,
	car_timer = 0,
	rank_timer = 0,
	
	lerp_fan_pool = 0,
	
	current_mode = 0, -- 0: rest_state, 1: lerping_fans, 2: rank_up, 3:new_cars, 4:all_done
	
	loading_done = false,
	
	unlocks = {  },
	vehicles_unlocked = { }, -- all vehicles unlocked by this rank up
	
	-- store class icon names
	class_icon_names = 
	{
		[ UIEnums.VehicleClass.A ] = "class_a",
		[ UIEnums.VehicleClass.B ] = "class_b",
		[ UIEnums.VehicleClass.C ] = "class_c",
		[ UIEnums.VehicleClass.D ] = "class_d",
	},
	
	-- store style names
	style_names =
	{
		[ UIEnums.VehicleUiStyle.Balanced ]		= UIText.RBC_STYLE_BALANCED,
		[ UIEnums.VehicleUiStyle.Drifty ]		= UIText.RBC_STYLE_DRIFTY,
		[ UIEnums.VehicleUiStyle.VeryDrifty ]	= UIText.RBC_STYLE_VERY_DRIFTY,
		[ UIEnums.VehicleUiStyle.Grippy ]		= UIText.RBC_STYLE_GRIPPY,
		[ UIEnums.VehicleUiStyle.VeryGrippy ]	= UIText.RBC_STYLE_VERY_GRIPPY,
		[ UIEnums.VehicleUiStyle.OffRoad ]		= UIText.RBC_STYLE_OFF_ROAD,
	},
	
	CanExit = function( _GUI )
		return false
	end,
	
	-- sound effect for rank up bar
	RankupPlayTime = 0.0,
	RankupPlayStep = 0.09,
}

function Init()
	AddSCUI_Elements()
	UIScreen.SetScreenTimers( 0, 0.3 )
	
	StoreInfoLine()
	UIGlobals.ShowingUnlocks = true
end

function PostInit()

	GUI.unlocks = Sp_UnlocksFilter( SinglePlayer.ProcessUnlocks() )
	GUI.rank_info = SinglePlayer.RankInfo()
	local sp_game_info = SinglePlayer.GameInfo()
	
	GUI.dummy_id = SCUI.name_to_id[ "Root" ]
	GUI.progress_bar_id = UIButtons.FindChildByName( GUI.dummy_id, "progress" )
	GUI.bar_size_x = UIButtons.GetSize( GUI.progress_bar_id )
	
	GUI.total_fans = sp_game_info.fans
	GUI.previous_total_fans = UIGlobals.Sp_PreviousFans
	GUI.next_rank_fan_bound = GUI.rank_info[FindNextRankFromFans( GUI.previous_total_fans )].fans
	GUI.current_rank_fan_bound = GUI.rank_info[FindRankFromFans( GUI.previous_total_fans )].fans
	
	-- did we achieve more fans than the next rank bound requires?
	GUI.ranked_up = ( GUI.total_fans >= GUI.next_rank_fan_bound )
	GUI.max_rank = GUI.previous_total_fans >= GUI.rank_info[#GUI.rank_info].fans
	
	if GUI.max_rank ~= true and UIGlobals.Sp_PreviousFans ~= nil then
		StartProcessFanGain( GUI.previous_total_fans )
	elseif UIGlobals.Sp_PreviousFans ~= nil then
		SetupMaxRank( GUI.previous_total_fans )
	end
	
	-- set this up here so we guarantee it's only done once
	GUI.txt_fans_last_race_id = UIButtons.FindChildByName( GUI.dummy_id, "fan_total" )
	UIButtons.ChangeText( GUI.txt_fans_last_race_id, "GAME_NUM_" .. (GUI.total_fans - GUI.previous_total_fans) )
	
	-- it's possible we may unlock a vehicle even without ranking up (via boss demands)
	GUI.hasUnlockedVehicles = #(SinglePlayer.VehicleUnlocks().cars) ~= 0

end

function StartLoop( time )
	--
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )

	-- the user is impatient and wishes to skip forwards (we only allow this if we're done loading in the background)
	if message == UIEnums.Message.MenuNext then
		if GUI.loading_done == true then
			if GUI.ranked_up == true then
				GUI.current_mode = 3
			else
				GUI.current_mode = 4
			end
		end
	end
	
	-- catches the loading message, allowing the player to press A to continue
	if message == UIEnums.GameMessage.UISceneReady then
		GUI.loading_done = true		
	end
end

function FrameUpdate( time )
	GUI.screen_timer = GUI.screen_timer + time
	
	-- if we're still on the animation that begins the screen, do nothing but send off timeline messages that control this flow
	if GUI.current_mode == 0 then
		if GUI.screen_timer > 2 then	
			GUI.current_mode = 1 -- begin lerping up
		elseif GUI.screen_timer > 1.25 then
			UIButtons.TimeLineActive( "show_progression", true )
		end
		return
	end
	
	-- if we are lerping up
	if GUI.current_mode == 1 then
		UIButtons.TimeLineActive( "lerping_up", true )
		
		--play sound
		GUI.RankupPlayTime = GUI.RankupPlayTime + time
		if GUI.RankupPlayTime > GUI.RankupPlayStep then
			GUI.RankupPlayTime = ( GUI.RankupPlayTime - GUI.RankupPlayStep )

			UISystem.PlaySound( UIEnums.SoundEffect.FansCountUp )
		end
		
		if GUI.max_rank ~= true then
			UpdateProcessFanGain( time )
			
			-- are we done lerping up?
			if math.floor(GUI.next_rank_fan_bound - GUI.lerped_fans - GUI.previous_total_fans) == 0 then
				
				-- we've just ranked up!
				GUI.ranked_up = true
				UIButtons.TimeLineActive( "finished_lerp", true, 0 )
				UIButtons.TimeLineActive( "lerping_up", false )
				UIButtons.TimeLineActive( "rank_up", true, 0 )
				UISystem.PlaySound( UIEnums.SoundEffect.RankUp )
				GUI.lerp_fan_pool = GUI.next_rank_fan_bound - GUI.previous_total_fans
				GUI.rank_timer = 0
				GUI.current_mode = 2
				
				-- was it the max rank?
				if GUI.next_rank_fan_bound == GUI.rank_info[#GUI.rank_info].fans then
					GUI.max_rank = true
				end
				
			elseif GUI.total_fans == GUI.previous_total_fans + GUI.lerped_fans then
				UIButtons.TimeLineActive( "finished_lerp", true, 0 )
				
				--did we unlock anything?
				if GUI.ranked_up == true or GUI.hasUnlockedVehicles == true then
					GUI.current_mode = 3 -- onwards to car unlocks!
				else
					UIButtons.TimeLineActive( "finished_lerp_all", true, 0 )
					GUI.current_mode = 4 -- boooooring, all done
				end
			end
		else
			MaxRankLerp( time )
			-- are we done lerping up?
			if GUI.total_fans == GUI.previous_total_fans + GUI.lerped_fans then
				UIButtons.TimeLineActive( "finished_lerp", true, 0 )
				GUI.current_mode = 4
			end
		end
		
	-- if we are ranking up
	elseif GUI.current_mode == 2 then
	
		-- new rank number
		if GUI.rank_timer > 0.185 then
			if GUI.max_rank == true then
				Amax.ChangeText( GUI.txt_fan_status_id, UIText["SP_RANK_" .. #GUI.rank_info] )
			else
				Amax.ChangeText( GUI.txt_fan_status_id, UIText["SP_RANK_" .. FindRankFromFans( GUI.previous_total_fans + GUI.lerp_fan_pool ) ] )
			end
		end
		
		GUI.rank_timer = GUI.rank_timer + time
		
		if GUI.rank_timer > 2.3 then
			GUI.current_mode = 1
			UIButtons.TimeLineActive( "lerping_up", true, 0 )
			if GUI.max_rank ~= true then
				NewRankUpdate()
			else
				SetupMaxRank( GUI.previous_total_fans )
			end
		end
		
	-- if we are displaying cars
	elseif GUI.current_mode == 3 then
		GUI.car_timer = GUI.car_timer + time
		if GUI.car_timer > 1.5 then
			GoScreen("Multiplayer\\Ingame\\MpUnlocks.lua")
		end
	
	-- all done
	elseif GUI.current_mode == 4 and GUI.loading_done == true then
		PopScreen()
	end
end

function EnterEnd()
	--RestoreInfoLine()
	--SinglePlayer.ClearUnlocks()
	UIButtons.TimeLineActive( "hide_unlocks", true )
	UIButtons.TimeLineActive( "show_progression", false )
	UISystem.PlaySound( UIEnums.SoundEffect.MPRankAway )
end

function EndLoop( time )
	--	
end

function End()
	UIGlobals.ShowingUnlocks = false
	-- safety first
	--UIGlobals.Sp_PreviousFans = nil
end

function SetupMaxRank( previousFans )
	GUI.lerped_fans = 0

	GUI.txt_total_fans_id = UIButtons.FindChildByName( GUI.dummy_id, "current_fans" )
	GUI.txt_target_fans_id = UIButtons.FindChildByName( GUI.dummy_id, "required_fans" )
	GUI.txt_fan_status_id = UIButtons.FindChildByName( GUI.dummy_id, "progression_rank" )
	
	UIButtons.ChangeText( GUI.txt_total_fans_id, "GAME_UNLOCKS_FANS_" .. previousFans + GUI.lerped_fans )
	UIButtons.ChangeText( GUI.txt_target_fans_id, UIText.SP_MAX_RANK )
	Amax.ChangeText( GUI.txt_fan_status_id, UIText.SP_RANK_25 )
	
	UpdateProgressBar( 1, 1 )
	
	GUI.lerp_target_fans = GUI.total_fans
	GUI.lerp_timer = 0
end

function MaxRankLerp( time )
	-- lerp to our target over some lovely hard coded number of seconds
	local lerp_time = 1.5
	
	GUI.lerp_timer = GUI.lerp_timer + time
	
	if GUI.lerp_timer > lerp_time then
		GUI.lerp_timer = lerp_time
	end
	
	GUI.lerped_fans = ( GUI.lerp_target_fans - GUI.previous_total_fans ) * ( GUI.lerp_timer / lerp_time )
	
	UIButtons.ChangeText( GUI.txt_total_fans_id, "GAME_UNLOCKS_FANS_" .. GUI.previous_total_fans + GUI.lerped_fans )
end

function StartProcessFanGain( previousFans )
	-- starting out?
	if GUI.lerped_fans == nil then
		GUI.lerped_fans = 0
	else
		UIButtons.TimeLineActive( "rank_reset", true, 0 )
	end

	
	GUI.txt_total_fans_id = UIButtons.FindChildByName( GUI.dummy_id, "current_fans" )
	GUI.txt_target_fans_id = UIButtons.FindChildByName( GUI.dummy_id, "required_fans" )
	GUI.txt_fan_status_id = UIButtons.FindChildByName( GUI.dummy_id, "progression_rank" )
	
	UIButtons.ChangeText( GUI.txt_total_fans_id, "GAME_UNLOCKS_FANS_" .. previousFans + GUI.lerped_fans )
	UIButtons.ChangeText( GUI.txt_target_fans_id, "GAME_UNLOCKS_NEXT_RANK_" .. GUI.next_rank_fan_bound - previousFans .. "_TO_" .. GUI.rank_info[FindNextRankFromFans( previousFans )].name )
	Amax.ChangeText( GUI.txt_fan_status_id, UIText["SP_RANK_" .. FindRankFromFans( previousFans + GUI.lerp_fan_pool ) ] )
	
	UpdateProgressBar( previousFans - GUI.current_rank_fan_bound, GUI.next_rank_fan_bound - GUI.current_rank_fan_bound )
	
	-- figure out our target to lerp the progress bar to
	if GUI.total_fans >= GUI.next_rank_fan_bound then
		GUI.lerp_target_fans = GUI.next_rank_fan_bound
	else
		GUI.lerp_target_fans = GUI.total_fans
	end
	
	GUI.lerp_timer = 0
	
end

-- given a frame time, wil lerp between the current GUI.lerped_fans and a target GUI.lerp_target_fans
function UpdateProcessFanGain( time )
	-- lerp to our target over some lovely hard coded number of seconds
	local lerp_time = 1.5
	
	GUI.lerp_timer = GUI.lerp_timer + time
	
	if GUI.lerp_timer > lerp_time then
		GUI.lerp_timer = lerp_time
	end
	
	GUI.lerped_fans = (( GUI.lerp_target_fans - GUI.lerp_fan_pool - GUI.previous_total_fans ) * ( GUI.lerp_timer / lerp_time )) + GUI.lerp_fan_pool
	
	UIButtons.ChangeText( GUI.txt_total_fans_id, "GAME_UNLOCKS_FANS_" .. GUI.previous_total_fans + GUI.lerped_fans )
	UIButtons.ChangeText( GUI.txt_target_fans_id, "GAME_UNLOCKS_NEXT_RANK_" .. math.floor(GUI.next_rank_fan_bound - GUI.lerped_fans - GUI.previous_total_fans) .. "_TO_" .. GUI.rank_info[FindNextRankFromFans( GUI.previous_total_fans + GUI.lerped_fans )].name )
	UpdateProgressBar( ( GUI.previous_total_fans - GUI.current_rank_fan_bound ) + GUI.lerped_fans, GUI.next_rank_fan_bound - GUI.current_rank_fan_bound )
end

function NewRankUpdate()
	-- recalculate all our bounds and values for the new rank
	local next_rank = FindNextRankFromFans( GUI.next_rank_fan_bound + 1 )
	GUI.current_rank_fan_bound = GUI.next_rank_fan_bound
	GUI.next_rank_fan_bound = GUI.rank_info[next_rank].fans
	
	-- add all unlocked vehicles to our queue
	for i,v in pairs( GUI.rank_info[next_rank].vehicles ) do
		VehicleUnlocked( i )
	end
	
	-- begin the lerp process again
	StartProcessFanGain( GUI.current_rank_fan_bound + 1 )
end

-- given a number of fans, finds the current rank
function FindRankFromFans( num_fans )
	return ( FindNextRankFromFans( num_fans ) - 1 )
end

-- given a number of fans, finds the rank that will next be unlocked
function FindNextRankFromFans( num_fans )
	
	-- logic below breaks otherwise
	if num_fans == 0 then
		return 2
	end
	
	local rankInfo = GUI.rank_info
	
	for i,v in ipairs(rankInfo) do
		if v.fans > num_fans then
			return ( v.index )
		end
	end
	
	-- max rank
	return #rankInfo
end

function UpdateProgressBar( current, max )
	UIButtons.ChangeSizeX( GUI.progress_bar_id, ( GUI.bar_size_x / max ) * current )
end

-- adds to a queue of unlocked vehicles to deal with
function VehicleUnlocked( id )
	GUI.vehicles_unlocked[ #GUI.vehicles_unlocked + 1 ] = id
end
