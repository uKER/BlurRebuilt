GUI =
{
	finished = false,

	state =
	{
		summary		= 1,
		summary_end = 2,
		progression = 3,
		rank_up		= 4,
		end_wait	= 6,
	},

	current_state = 1,

	timelines = 
	{
		"show_ontrack",
		"show_challenge_bonus",
		"show_finish",
		"show_total",
		"summary_end",
		"show_progression",
	},

	times				= {},

	start_time			= 0.25,
	end_time			= 1.0,
	base_time			= 0.5,

	current_timeline	= 1,
	current_time		= 0,

	LevelUp			= 0,
	NumLevelUps		= 0,
	TotalFans		= 0,

	MaxTime			= 1.5,
	Timer			= 0,
	SummaryEndTime	= 1,
	RankUpTime		= 1,
	EndTime			= 0.5,

	-- oh god i did this a none clever way
	RankupPlayTime = 0.0,
	RankupPlayStep = 0.09,
}

function Init()
	AddSCUI_Elements( )
	UIButtons.CloneXtGadgetByName( "Multiplayer\\Ingame\\MpRaceResults.lua", "MpBgStuff" )
	
	local race_summary = Multiplayer.GetRaceSummary()
	if race_summary.showy_flourish_fans > 0 then
		UIButtons.SetActive( SCUI.name_to_id[ "showy_flourish_rb" ], true )
	end
	if race_summary.fan_favourite_fans > 0 then
		UIButtons.SetActive( SCUI.name_to_id[ "fan_favourite_rb" ], true )
	end

	-- need a vairable time based on how many challenge have been completed
	local challenge_time = 0
	local last_parent = SCUI.name_to_id["challenge_bonus_title"]

	-- only if we compelted some challenges, otherwise move the other two items closer
	if Multiplayer.GetRaceChallengesComplete() > 0 then
		for i = 1, Multiplayer.GetRaceChallengesComplete() do
			-- create the challenge pieces
			local id = UIButtons.CloneXtGadgetByName( "Multiplayer\\Ingame\\MpProgression.lua", "challenge_bonus" )

			UIButtons.SetParent( id, last_parent, UIEnums.Justify.BottomCentre )
			UIButtons.ChangeText( id, "MPL_RACE_SUMMARY_CHALLENGE" .. i )

			local end_time = UIButtons.GetXtVar( id, "time_lines.0.time.end" )
			local start_time = UIButtons.GetXtVar( id, "time_lines.0.time.start" )
			local time = end_time - start_time

			UIButtons.SetXtVar( id, "time_lines.0.time.end", end_time + ( time * (i-1) ) )
			UIButtons.SetXtVar( id, "time_lines.0.time.start", start_time + ( time * (i-1) ) )

			last_parent = id
			challenge_time = challenge_time + ( time - 0.1 )
		end
		challenge_time = challenge_time + 0.5
	else
		UIButtons.ChangePosition( SCUI.name_to_id["on_track_fans_title"], -25, 0, -100 )
		UIButtons.ChangePosition( SCUI.name_to_id["finish_bonus_title"], 25, 0, -100 )
	end	

	GUI.times[1] = GUI.start_time					-- on track
	GUI.times[2] = GUI.times[1] + GUI.base_time		-- challenge
	GUI.times[3] = GUI.times[2] + challenge_time	-- finish
	GUI.times[4] = GUI.times[3] + 1					-- total
	GUI.times[5] = GUI.times[4]	+ 0.25				-- summary end
	GUI.times[6] = GUI.times[5]						-- show progression


	-- create progression stuff
	GUI.FansTargetID	= SCUI.name_to_id[ "required_fans" ]
	GUI.FansID			= SCUI.name_to_id[ "current_fans" ]
	GUI.RankID			= SCUI.name_to_id[ "progression_rank" ]
	GUI.RankIcon		= SCUI.name_to_id[ "progression_rank_icon" ]

	-- store max size
	GUI.ProgressID = SCUI.name_to_id[ "progress" ]
	GUI.MaxSizeX, GUI.MaxSizeY, GUI.MaxSizeZ = UIButtons.GetSize( SCUI.name_to_id[ "backing" ] )
 
	Progression_Init()
end

function PostInit()
	UIButtons.SetActive( SCUI.name_to_id["challenge_bonus"], false )
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

	GUI.current_time = GUI.current_time + time

	if GUI.current_state == GUI.state.summary then
		if GUI.current_time > GUI.times[GUI.current_timeline] then

			UIButtons.TimeLineActive( GUI.timelines[GUI.current_timeline], true )
			GUI.current_timeline = GUI.current_timeline + 1

			-- catch no challenge bonus
			if Multiplayer.GetRaceChallengesComplete() == 0 and GUI.timelines[GUI.current_timeline] == "show_challenge_bonus" then
				GUI.current_timeline = GUI.current_timeline + 1
			end

			if GUI.current_timeline > #GUI.timelines then
				GUI.current_state = GUI.state.summary_end
				GUI.current_time = 0
			end
		end

	elseif GUI.current_state == GUI.state.summary_end then
		if GUI.current_time >= GUI.SummaryEndTime then
			GUI.current_state = GUI.state.progression
			GUI.current_time = 0

			-- This sets GUI.current_time to the correct offset based on how close we are to leveling up
			Progression_SetupEffect( GUI.CurrentFans - GUI.PreviousFans, GUI.TargetFans - GUI.PreviousFans )
		end

	elseif GUI.current_state == GUI.state.progression then
		if GUI.current_time >= GUI.MaxTime then
			GUI.current_time = GUI.MaxTime

			GUI.current_state = GUI.state.rank_up
		end

		-- check we arnt maxed out otherwise we will get divides by zero
		if GUI.Progression.maxed == false then
			-- calculate progress
			local target = GUI.TargetFans - GUI.PreviousFans
			local fans = target * ( GUI.current_time / GUI.MaxTime )

			GUI.RankupPlayTime = GUI.RankupPlayTime + time

			-- cap at total fans
			if fans >= ( GUI.TotalFans - GUI.PreviousFans ) then
				fans = GUI.TotalFans - GUI.PreviousFans
				GUI.RankupPlayTime = 0 -- force not to play
			end

			-- play the tick sound
			if GUI.RankupPlayTime > GUI.RankupPlayStep then
				GUI.RankupPlayTime = ( GUI.RankupPlayTime - GUI.RankupPlayStep )

				UISystem.PlaySound( UIEnums.SoundEffect.FansCountUp )
			end

			Progression_UpdateEffect( fans, target, GUI.PreviousFans, GUI.Progression.previous_level + GUI.LevelUp )
		end

		-- go and show the unlocks
		if GUI.current_state == GUI.state.rank_up then

			GUI.LevelUp = GUI.LevelUp + 1
			if GUI.LevelUp > GUI.NumLevelUps then
				GUI.LevelUp	= GUI.NumLevelUps
				GUI.current_state = GUI.state.unlocks
				GUI.current_time = 0
			else
				Progression_RankUp( GUI.Progression.previous_level + GUI.LevelUp )
			end
			
			GUI.current_time = 0
		end

	elseif GUI.current_state == GUI.state.rank_up then

		if GUI.current_time > GUI.RankUpTime then
			-- we cant level up anymore
			if GUI.TotalFans == Multiplayer.LevelProgressionData( GUI.Progression.previous_level + GUI.LevelUp ).fans_previous then
				GUI.current_state = GUI.state.unlocks
				GUI.current_time = 0
			else
				-- finished showing challenges so go back to leveling up
				GUI.current_state = GUI.state.progression
				Progression_RefreshLevel()
			end
		end

	elseif GUI.current_state == GUI.state.unlocks then

		GUI.current_state = GUI.state.end_wait
		GUI.current_time = 0

		UIButtons.TimeLineActive( "show_progression", false )
		UIButtons.TimeLineActive( "show_unlocks", false )

		UISystem.PlaySound( UIEnums.SoundEffect.MPRankAway )

	elseif GUI.current_state == GUI.state.end_wait then
		if GUI.current_time >= GUI.EndTime then
			-- if we have some unlocks to show, then go and show them
			if Progression_ShowUnlocks( GUI.Progression.unlocks ) == true then
				GoScreen( "Multiplayer\\Ingame\\MpUnlocks.lua" )
			else
				GoScreen( "Multiplayer\\Ingame\\MpRaceAwards.lua" )
			end
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

function Progression_Init()
	-- progress bar data
	GUI.Progression		= Multiplayer.GetProgressionForRace()
	GUI.LevelData		= Multiplayer.LevelProgressionData( GUI.Progression.previous_level )
	GUI.NumLevelUps		= GUI.Progression.level_ups
	GUI.LevelUp			= 0

	GUI.TotalFans		= GUI.Progression.fans
	GUI.CurrentFans		= GUI.Progression.previous_fans
	GUI.PreviousFans	= GUI.LevelData.fans_previous
	GUI.TargetFans		= GUI.LevelData.fans_needed

	-- setup level up data
	if GUI.Progression.maxed == false then
		Progression_UpdateEffect( GUI.CurrentFans - GUI.PreviousFans, GUI.TargetFans - GUI.PreviousFans, GUI.PreviousFans, GUI.Progression.previous_level )
	else
		Progression_UpdateEffect( GUI.CurrentFans, GUI.TargetFans, 0, GUI.Progression.level )
	end

	Mp_RankIcon( GUI.RankIcon, GUI.Progression.previous_level - 1, GUI.Progression.legend )
	Mp_RankIcon( GUI.PlayerRankIconID, GUI.Progression.previous_level - 1, GUI.Progression.legend )
end


function Progression_RankUp( rank )
	-- change rank and icon
	Amax.ChangeText( GUI.RankID,			"MPL_LEVEL_RANK" .. rank )
	Mp_RankIcon( GUI.RankIcon, rank - 1, GUI.Progression.legend )
	
	Multiplayer.PostRankedUp()

	UIButtons.TimeLineActive( "ranked_up", true, 0 )
	UIButtons.TimeLineActive( "show_unlocks", true )
	UISystem.PlaySound( UIEnums.SoundEffect.RankUp )
end


function Progression_RefreshLevel()
	GUI.LevelData		= Multiplayer.LevelProgressionData( GUI.Progression.previous_level + GUI.LevelUp )

	GUI.PreviousFans	= GUI.LevelData.fans_previous
	GUI.TargetFans		= GUI.LevelData.fans_needed

	Progression_SetupEffect( GUI.PreviousFans - GUI.PreviousFans, GUI.TargetFans - GUI.PreviousFans )
	Progression_UpdateEffect( 0, ( GUI.TargetFans - GUI.PreviousFans ), GUI.PreviousFans, GUI.Progression.previous_level + GUI.LevelUp )
end


function Progression_SetupEffect( current_fans, target_fans )
	if target_fans == 0 then
		GUI.current_time = GUI.MaxTime
	else
		GUI.current_time = GUI.MaxTime * ( current_fans / target_fans )
	end
end

function Progression_UpdateEffect( current_fans, target_fans, previous_fans, rank )

	-- ceil this
	current_fans = math.ceil( current_fans )
	target_fans = math.ceil( target_fans )
	previous_fans = math.ceil( previous_fans )

	-- calculate size
	local size_x = GUI.MaxSizeX * ( current_fans / target_fans )
	UIButtons.ChangeSize( GUI.ProgressID, size_x, GUI.MaxSizeY, GUI.MaxSizeZ )

	-- change text
	UIButtons.ChangeText( GUI.FansID,		"MPL_LEVEL_NUM_FANS" .. ( previous_fans + current_fans ) )
	UIButtons.ChangeText( GUI.FansTargetID, "MPL_LEVEL_FANS_NEEDED" .. ( ( previous_fans + target_fans ) - ( previous_fans + current_fans ) ) .. "_" .. rank + 1)
	
	-- change rank
	Amax.ChangeText( GUI.RankID,			"MPL_LEVEL_RANK" .. rank )
end


function Progression_ShowUnlocks( unlocks )
	if #unlocks.features > 0 then
		return true
	elseif #unlocks.playlists > 0 then
		return true
	elseif #unlocks.features > 0 then
		return true
	elseif #unlocks.packs > 0 then
		return true
	elseif #unlocks.upgrades > 0 then
		return true
	elseif #unlocks.abilities > 0 then
		return true
	elseif #unlocks.cars > 0 then
		return true
	end
	
	return false
end