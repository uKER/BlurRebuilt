GUI =
{
	finished = false,
	show_rival = false,		-- TODO
	countdown_time = 3,
	countdown_clamp = 3,
	
	anims = 
	{	
		"scale_x",
		"scale_y",
	},

	packshot_stages = 
	{
		{	cam = UIEnums.AnimatedCameraSequenceType.RoutePack, time = 3, px = 10, py = 0, sx = 140, sy = 18, anim = "scale_y" },
	},

	rival_stages = 
	{
		{	cam = UIEnums.AnimatedCameraSequenceType.RivalSeq01, time = 1.5, player = 1,	px = 20, py = 24,	sx = 18, sy = 96, anim = "scale_y"	},
		{	cam = UIEnums.AnimatedCameraSequenceType.RivalSeq02, time = 1.5, player = 1,	px =-20, py = 24,										},
	},
	
	countdown_stages = 
	{
		{	cam = UIEnums.AnimatedCameraSequenceType.RaceStart4,	time = 1, player = true,	px = 0, py = 0, sx = 18, sy = 48,anim = "scale_y"	},
		{	cam = UIEnums.AnimatedCameraSequenceType.RaceStart3,	time = 1, player = true,	px = 40, py = 0, sx = 160, sy = 18, anim = "scale_y" },
--		{	cam = UIEnums.AnimatedCameraSequenceType.RaceStart2,	time = 1, player = true,	px =-40, py = 0, sx = 160, sy = 18, anim = "scale_y", forwards = false },
--		{	cam = UIEnums.AnimatedCameraSequenceType.RaceStart1,	time = 1, player = true },
		{	cam = UIEnums.AnimatedCameraSequenceType.RaceStartGO,	time = 1, player = true,	px = 0, py = 0, anim = "scale_y" },
	},

	
	SEQ_PACKSHOT	= 0,
	SEQ_RIVAL		= 1,
	SEQ_COUNTDOWN	= 2,
	SEQ_HUD			= 3,
}

function Init()
	if IsSplitScreen() == true then
		Amax.SetNumViewports( #UIGlobals.Splitscreen.players )
	end

	Splitscreen_AddSplits()
	
	GUI.num_players = Amax.GetNumViewports()

	-- [Bone] #34942 Set future camera	
	for i = 1, GUI.num_players, 1 do
		Amax.StartAnimatedCamera( UIEnums.AnimatedCameraSequenceType.RaceStartGO, true, i-1, i-1, true )
	end
	
	AddSCUI_Elements( )
	Amax.SetGlobalFade(1.0)
	UIScreen.SetScreenTimers(0,0)

	GUI.wipe_id = SCUI.name_to_id[ "Wipe" ]
	
	UIScreen.AddMessage( UIEnums.UIGameMessage.RaceStartsIn, GUI.countdown_clamp )
	
	UIScreen.AddMessageNow( UIEnums.GameMessage.SetLocalPlayerIndex, 0 )
	
	GUI.countdown_time = 3
	UISystem.PlaySound( UIEnums.SoundEffect.RaceStartCountdown )	-- misses the first one otherwise
	GUI.timer = 0
	GUI.stage_index = 1
	GUI.seq_index = GUI.SEQ_COUNTDOWN
end

function IntroRace_ResetAnims()
	for i,v in ipairs( GUI.anims ) do
		UIButtons.TimeLineActive( v, true, 10, true )
	end
end

function PostInit()
	if UIEnums.CurrentPlatform == UIEnums.Platform.PC then
		-- [Blur2PC] The stencil opening keeps its camera-authored size and animation;
		-- only the purple fill behind it must span the current viewport.
		UIButtons.ChangeSizeX( SCUI.name_to_id[ "BlackBG" ], SCREEN_WIDTH )
	end

	GUI.countdown_id = { }
	for i = 1, GUI.num_players, 1 do
		local player_index = i - 1
		local t =
		{
			UIButtons.CloneXtGadgetByName( "hud_objects", "CountdownNode1" ),
			UIButtons.CloneXtGadgetByName( "hud_objects", "CountdownNode2" ),
			UIButtons.CloneXtGadgetByName( "hud_objects", "CountdownNode3" ),
		}
		
		GUI.countdown_id[ i ] = { }
		for j,v in ipairs( t ) do
			local id = UIButtons.FindChildByName( v, "CDText" )
			UIButtons.ChangePanel( id, UIEnums.Panel._VP0 + player_index )
			
			GUI.countdown_id[ i ][ j ] = id
		end
		
		UIScreen.AddMessageNow( UIEnums.GameMessage.SetLocalPlayerIndex, player_index )
	end
	
	if Amax.IsRaceMpDestruction() == true then
		GUI.seq_index = GUI.SEQ_COUNTDOWN
		GUI.countdown_time = 3
		UISystem.PlaySound( UIEnums.SoundEffect.RaceStartCountdown )	-- misses the first one otherwise
		
		UIButtons.TimeLineActive( "countdown_intro", true, 3.0, true )
	end

	IntroRace_InitSequence()
end

function StartLoop( time )
	--
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end
end

function FrameUpdate( time )
	if GUI.finished == true then
		return
	end
	
	if GUI.countdown_time > 0 then
		local old_time = GUI.countdown_time
		GUI.countdown_time = GUI.countdown_time - time
		if GUI.countdown_time < 0 then
			GUI.countdown_time = -1
		end
		
		local tmp = GUI.countdown_time
		if GUI.countdown_time > GUI.countdown_clamp then
			tmp = GUI.countdown_clamp
		else
			-- Do sound effect
			if math.ceil( old_time ) ~= math.ceil( tmp ) and tmp >= 0 then
				if math.ceil( tmp ) ~= 0 then
					UISystem.PlaySound( UIEnums.SoundEffect.RaceStartCountdown )
				end
			end
		end

		UIScreen.AddMessage( UIEnums.UIGameMessage.RaceStartsIn, GUI.countdown_time )
	end

	-- Advance timer
	GUI.timer = GUI.timer - time
	if GUI.timer <= 0 then
		GUI.timer = 0
--		print(GUI.stage_index,#GUI.stages)
		-- Advance stage
		GUI.stage_index = GUI.stage_index + 1
		if GUI.stage_index > #GUI.stages then
			GUI.stage_index = 1

			-- Advance sequence
			GUI.seq_index = GUI.seq_index + 1
			
			if GUI.seq_index == GUI.SEQ_RIVAL and GUI.show_rival ~= true then
				GUI.seq_index = GUI.seq_index + 1
			end
			
			if GUI.seq_index == GUI.SEQ_HUD then
--				GUI.seq_index = GUI.SEQ_PACKSHOT
--				IntroRace_InitSequence()
				UIGlobals.Ingame.IntroHUD = true
				
				GoScreen("Ingame\\Hud.lua")
			else
				IntroRace_InitSequence()
			end
		else
			IntroRace_InitStage()
			Amax.ResetViewpointVisibility()

			if GUI.seq_index == GUI.SEQ_COUNTDOWN then
				for i = 1, GUI.num_players, 1 do
					for j,v in ipairs( GUI.countdown_id[ i ] ) do
						UIButtons.SetActive( v, GUI.stage_index == j )
					end
				end
			end
			
			if GUI.seq_index == GUI.SEQ_COUNTDOWN and GUI.stage_index == #GUI.stages then
				for i = 1, GUI.num_players, 1 do
					-- Duration, vehicle, viewport
					Amax.BlendAnimatedCamera( 1, i - 1, i - 1 )
				end
			end
		end
	end
end
	
function IntroRace_InitStage()
	if IsTable( GUI.stages ) == false then
		print("Invalid stage table", GUI.stage_index, GUI.seq_index )
		return
	end
	
	local shot = GUI.stages[ GUI.stage_index ]
	if IsTable( shot ) == false then
		print("Invalid cam table", GUI.stage_index, GUI.seq_index )
		return
	end
	
	-- Check for sorcery data
	IntroRace_ReadAnimTable( shot )
	
	-- Set new time
	GUI.timer = shot.time
	
	-- Change camera?
	if IsNumber( shot.cam ) == true then
		if shot.player == true then
			for i = 1, GUI.num_players, 1 do
				Amax.StartAnimatedCamera( shot.cam, false, i-1, i-1 )
			end
		else
			Amax.StartAnimatedCamera( shot.cam, false, shot.player, shot.player )
		end
	end
	
	-- Swipe position
	if IsNumber( shot.px ) == true and IsNumber( shot.py ) == true then 
--		print("Change position",shot.px, shot.py)
		UIButtons.ChangePosition( GUI.wipe_id, shot.px, shot.py )
	end
	
	-- Swipe size
	if IsNumber( shot.sx ) == true and IsNumber( shot.sy ) == true then
		UIButtons.ChangeSize( GUI.wipe_id, shot.sx, shot.sy )
	end
	
	-- animation
	IntroRace_ResetAnims()
	
	local forwards = shot.forwards
	local time = 1
	if forwards ~= false then
		forwards = true
		time = 0
	end
	
	if IsString( shot.anim ) == true then
		UIButtons.TimeLineActive( shot.anim, forwards, time, forwards )
	end
end

function IntroRace_ReadAnimTable( table )
	if IsTable( table ) == false then
		return
	end
	
	local t = Amax.GetAnimatedCameraParams( table.cam )
	if IsTable( t ) == false then
		return
	end
	
	-- Invalid data
	if t.sx <= 0 and t.sy <= 0 then
		return
	end
	
	-- Screen position
	table.px = (t.px / 1.5) + 320
	table.py = (t.py / 1.5) + 240
	
	-- Size
	table.sx = (t.sx / 1.5)
	table.sy = (t.sy / 1.5)

	-- [Blur2PC] Screen-spanning horizontal apertures follow the viewport width.
	-- Keep narrower apertures (the vertical countdown slit) at their authored
	-- size and position. Camera parameters use the original 1280x720 canvas.
	if UIEnums.CurrentPlatform == UIEnums.Platform.PC and t.sx >= 1280 then
		local stretch = SCREEN_WIDTH / (1280 / 1.5)
		table.px = 320 + (table.px - 320) * stretch
		table.sx = table.sx * stretch
	end
	
	-- Animation
	table.anim = t.anim
	
	table.forwards = t.forwards
end	

function IntroRace_InitSequence()

	if GUI.seq_index == GUI.SEQ_PACKSHOT then
		GUI.stages = GUI.packshot_stages
--		print("Pack shots", #GUI.stages)
	end
	
	if GUI.seq_index == GUI.SEQ_RIVAL then
		GUI.stages = GUI.rival_stages
--		print("Rival shots", #GUI.stages)
	end
	
	if GUI.seq_index == GUI.SEQ_COUNTDOWN then
		GUI.stages = GUI.countdown_stages
--		print("Countdown", #GUI.stages)
	end
	
	IntroRace_InitStage()
end

function Render()
	--
end

function EndLoop( time )
	--
end

function End()
	--
	UISystem.PlaySound( UIEnums.SoundEffect.RaceStart )
end
