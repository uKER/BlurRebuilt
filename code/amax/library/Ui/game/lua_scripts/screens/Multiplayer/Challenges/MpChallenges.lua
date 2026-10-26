GUI =
{
	finished = false,

	CurrentPack			= -1,
	CurrentChallenge	= -1,
	CurrentClass		= -1,
	CurrentCar			= -1,

	ProgressSize		= 0,

	Challenges			= {},
	Cars				= {},

	PackDummys			= {},
	ChallengeNodes		= {},
	CarClassNodes		= {},
	CarChallengeNodes	= {},
	RewardNodes			= {},
	
	WinnerRewardNodes	= {},
	RepeatRewardNodes	= {},

	State =
	{
		Packs			= 1,
		Challenges		= 2,
		CarClass		= 3,
		CarChallenges	= 4,
	},
	CurrentState	= 1,

	Classes = 
	{
		[1] = UIEnums.MpVehicleClass.ClassA,
		[2] = UIEnums.MpVehicleClass.ClassB,
		[3] = UIEnums.MpVehicleClass.ClassC,
		[4] = UIEnums.MpVehicleClass.ClassD,
	},

	CanExit = function( _GUI )
		return false
	end,
}

function Init()
	if UIGlobals.Multiplayer.InLobby == true then
		PlaySfxGraphicBack()
	else
		PlaySfxGraphicNext()
	end
	
	AddSCUI_Elements()
	Amax.ChangeUiCamera( "Sp_3", UIGlobals.CameraLerpTime, 0 )
	
	StoreInfoLine()
	SetupInfoLine()
	
	if UIGlobals.Multiplayer.InLobby == false then
		net_SetRichPresence( UIEnums.RichPresence.Challenges )
	end
	
	GUI.ChallengeMenuID			= SCUI.name_to_id[ "challenges" ]
	GUI.CarClassMenuID			= SCUI.name_to_id[ "car_classes" ]
	GUI.CarChallengesMenuID		= SCUI.name_to_id[ "car_challenges" ]
	GUI.ChallengeNameID			= SCUI.name_to_id[ "challenge_name" ]
	GUI.ChallengeDescID			= SCUI.name_to_id[ "challenge_desc" ]
	GUI.ChallengeProgressID		= SCUI.name_to_id[ "challenge_amount" ]
	
	GUI.ProgressSize = UIButtons.GetSize( SCUI.name_to_id[ "backing" ] )
	GUI.CarRewards = Multiplayer.GetCarChallengeRewards()

	-- add challenge menu
	GUI.PacksMenuID = SCUI.name_to_id[ "packs" ]
	GUI.Packs = Multiplayer.GetChallengePacks()

	local new_map = Multiplayer.GetNewChallengePacksMap()

	for i, v in ipairs( GUI.Packs ) do

		local node_id = UIButtons.CloneXtGadgetByName( "Multiplayer\\Challenges\\MpChallenges.lua", "node_item" )
		UIButtons.AddListItem( GUI.PacksMenuID, node_id, v.category, v.locked )

		local name_id		= UIButtons.FindChildByName( node_id, "text" )
		local help_id		= UIButtons.FindChildByName( node_id, "helptext" )
		local icon_id		= UIButtons.FindChildByName( node_id, "icon" )
		local pack_frame_id = UIButtons.FindChildByName( node_id, "pack_frame" )
		local backing_id	= UIButtons.FindChildByName( node_id, "backing" )
		local progress_id	= UIButtons.FindChildByName( node_id, "progress" )
		local counter_id	= UIButtons.FindChildByName( node_id, "pack_counter" )
		
		if v.locked then
			UIButtons.LockNode( node_id )
			
			UIButtons.ChangeColour( name_id, "locked" )
			UIButtons.ChangeColour( icon_id, "locked" )
			UIButtons.ChangeColour( pack_frame_id, "locked" )
			UIButtons.ChangeColour( backing_id, "locked" )
			UIButtons.ChangeColour( progress_id, "locked" )
			
			UIButtons.ChangeText( name_id, "MPL_ABILITY_LOCKED" .. v.level+1 )
			UIShape.ChangeObjectName( icon_id, "style_mixed" )
		else
			UIButtons.ChangeText( name_id, v.name )
			UIButtons.ChangeText( help_id, v.desc )
			UIButtons.ChangeText( counter_id, "MPL_CHALLENGE_PACK" .. i .. "_PROGRESS" )
			
			UIShape.ChangeObjectName( icon_id, v.icon_name )
			SetupProgressBar( node_id, v.progress )

			-- show new text
			if new_map[i] == true then
				Mp_ShowItemNew( GUI.PacksMenuID, i - 1 )
			end
		end

		GUI.PackDummys[i] = node_id
	end

	-- create challenges
	for i = 1, 16 do
		GUI.ChallengeNodes[i] = UIButtons.CloneXtGadgetByName( "Multiplayer\\Challenges\\MpChallenges.lua", "node_challenge" )
		UIButtons.AddListItem( GUI.ChallengeMenuID, GUI.ChallengeNodes[i], i )
	end

	-- create car classes
	local class_data = Multiplayer.GetCarChallengeClasses()

	for i, v in ipairs( GUI.Classes ) do
		GUI.CarClassNodes[i] = UIButtons.CloneXtGadgetByName( "Multiplayer\\Challenges\\MpChallenges.lua", "node_car_class" )

		UIShape.ChangeObjectName( UIButtons.FindChildByName( GUI.CarClassNodes[i], "class_icon" ), UIGlobals.ClassIcons[v] )
		UIButtons.ChangeText( UIButtons.FindChildByName( GUI.CarClassNodes[i], "name" ), "MPL_CHALLENGE_CLASS_PROGRESS" .. v )

		UIButtons.AddListItem( GUI.CarClassMenuID, GUI.CarClassNodes[i], i )

		SetupProgressBar( GUI.CarClassNodes[i], class_data[i] )
	end

	-- create car challenges
	for i = 1, 15 do
		GUI.CarChallengeNodes[i] = UIButtons.CloneXtGadgetByName( "Multiplayer\\Challenges\\MpChallenges.lua", "node_car_challenge" )
		UIButtons.AddListItem( GUI.CarChallengesMenuID, GUI.CarChallengeNodes[i], i )
	end
	
	-- create rewards
	local pos_x = 0.5
	local pos_y = -7
	for i = 1, 3 do
		GUI.RewardNodes[i] = UIButtons.CloneXtGadgetByName( "Multiplayer\\Challenges\\MpChallenges.lua", "fan_node" )

		UIShape.ChangeSceneName( UIButtons.FindChildByName( GUI.RewardNodes[i], "icon" ), "common_icons" )
		UIShape.ChangeObjectName( UIButtons.FindChildByName( GUI.RewardNodes[i], "icon" ), "fan" )

		UIButtons.ChangePosition( GUI.RewardNodes[i], pos_x, pos_y, 0 )
		UIButtons.SetParent( GUI.RewardNodes[i], SCUI.name_to_id["challenge_frame"], UIEnums.Justify.BottomLeft )
		pos_y = pos_y - 4.25
	end

	-- create car winner rewards
	local pos_y = -4.75
	for i = 1, 3 do
		local index = i-1
		
		-- races
		GUI.RepeatRewardNodes[i] = UIButtons.CloneXtGadgetByName( "Multiplayer\\Challenges\\MpChallenges.lua", "reward_node" )
		UIButtons.SetParent( GUI.RepeatRewardNodes[i], SCUI.name_to_id["repeat_offender_frame"], UIEnums.Justify.BottomLeft )

		UIButtons.ChangeText( UIButtons.FindChildByName( GUI.RepeatRewardNodes[i], "name" ), "MPL_CAR_UPGRADE_TYPE" .. UIEnums.MpUpgradeChallenges.Races .. "_" .. index )		
		UIButtons.ChangePosition( GUI.RepeatRewardNodes[i], 0.5, pos_y, 0 )
		UIShape.ChangeObjectName( UIButtons.FindChildByName( GUI.RepeatRewardNodes[i], "icon" ), UIGlobals.CarUpgradeIcons[GUI.CarRewards["repeat_offender"][i]] )
		
		-- winner
		GUI.WinnerRewardNodes[i] = UIButtons.CloneXtGadgetByName( "Multiplayer\\Challenges\\MpChallenges.lua", "reward_node" )
		UIButtons.SetParent( GUI.WinnerRewardNodes[i], SCUI.name_to_id["winner_frame"], UIEnums.Justify.BottomLeft )

		UIButtons.ChangeText( UIButtons.FindChildByName( GUI.WinnerRewardNodes[i], "name" ), "MPL_CAR_UPGRADE_TYPE" .. UIEnums.MpUpgradeChallenges.Winner .. "_" .. index )		
		UIButtons.ChangePosition( GUI.WinnerRewardNodes[i], 0.5, pos_y, 0 )
		UIShape.ChangeObjectName( UIButtons.FindChildByName( GUI.WinnerRewardNodes[i], "icon" ), UIGlobals.CarUpgradeIcons[GUI.CarRewards["winner"][i]] )

		pos_y = pos_y - 3
	end

	if UIGlobals.Multiplayer.InLobby == true or Amax.CanUseShare() == false then
		SetupInfoLine(UIText.INFO_A_SELECT, UIText.INFO_B_BACK)
	else
		SetupInfoLine(UIText.INFO_A_SELECT, UIText.INFO_B_BACK, "GAME_SHARE_BUTTON")
	end
end

function PostInit()
	UIButtons.SetActive( SCUI.name_to_id["pack_frame"],	false, true )
	UIButtons.SetActive( SCUI.name_to_id["fan_reward_node"], false )
	UIButtons.SetActive( SCUI.name_to_id["reward_node"], false )
	UIButtons.SetActive( SCUI.name_to_id["node_challenge"], false )
	UIButtons.SetActive( SCUI.name_to_id["node_car_challenge"], false )
	UIButtons.SetActive( SCUI.name_to_id["node_car_class"], false )

	UIButtons.ChangePanel( SetupScreenTitle( UIText.MP_CHALLENGES, SCUI.name_to_id["centre"], "challenge", "fe_icons" ), UIEnums.Panel._3DAA_WORLD, true )

	-- start darkening
	UIButtons.SetActive( SCUI.name_to_id["darken"], UIGlobals.Multiplayer.InLobby )
	UIButtons.SetActive( SCUI.name_to_id["countdown"], UIGlobals.Multiplayer.InLobby )
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )	

	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end

	if message == UIEnums.Message.MenuNext and arg2 == true then

		if GUI.CurrentState == GUI.State.Packs then
			PlaySfxNext()
			SelectPack( true )

			-- special case for car pack
			if GUI.CurrentPack == UIEnums.MpChallengePack.Car then
				GUI.CurrentState = GUI.State.CarClass
				UIButtons.TimeLineActive( "change_class", true, 0.0 )
				
				SetupInfoLine(UIText.INFO_A_SELECT, UIText.INFO_B_BACK)
			else
				GUI.CurrentState = GUI.State.Challenges

				SetupInfoLine( UIText.INFO_B_BACK )
			end

		elseif GUI.CurrentState == GUI.State.CarClass then		
			GUI.CurrentState = GUI.State.CarChallenges
			PlaySfxNext()

			SelectClass( true )
			SetupInfoLine( UIText.INFO_B_BACK )
		end
	end
	
	if message == UIEnums.Message.MenuBack and arg2 == true then
		PlaySfxBack()

		if GUI.CurrentState == GUI.State.Packs then
			if UIGlobals.Multiplayer.InLobby == true then
				PopScreen()
			else
				GoScreen( "Multiplayer\\MpOnline.lua" )
			end

		elseif GUI.CurrentState == GUI.State.Challenges then
			GUI.CurrentState = GUI.State.Packs

			SelectPack( false )
			if UIGlobals.Multiplayer.InLobby == true then
				SetupInfoLine(UIText.INFO_A_SELECT, UIText.INFO_B_BACK)
			else
				SetupInfoLine(UIText.INFO_A_SELECT, UIText.INFO_B_BACK, "GAME_SHARE_BUTTON")
			end

		elseif GUI.CurrentState == GUI.State.CarClass then
			GUI.CurrentState = GUI.State.Packs

			SelectPack( false )
			if UIGlobals.Multiplayer.InLobby == true then
				SetupInfoLine(UIText.INFO_A_SELECT, UIText.INFO_B_BACK)
			else
				SetupInfoLine(UIText.INFO_A_SELECT, UIText.INFO_B_BACK, "GAME_SHARE_BUTTON")
			end
			
		elseif GUI.CurrentState == GUI.State.CarChallenges then
			GUI.CurrentState = GUI.State.CarClass

			SelectClass( false )
			SetupInfoLine(UIText.INFO_A_SELECT, UIText.INFO_B_BACK)
		end
	end

	if message == UIEnums.Message.ButtonLeftShoulder and arg2 == true then

		if Amax.CanUseShare() == true then
			if UIGlobals.Multiplayer.InLobby == false then
				if GUI.CurrentState == GUI.State.Packs then
					UIGlobals.ShareFromWhatPopup=-1
					SetupCustomPopup( UIEnums.CustomPopups.SharingOptions )
				end
			end
		end

	elseif message == UIEnums.Message.PopupNext and arg2 == UIEnums.CustomPopups.SharingOptions then

		local index = UIButtons.GetSelectionIndex( GUI.PacksMenuID )

		StoreScreen( UIEnums.ScreenStorage.FE_SOCIAL_NETWORK, "Multiplayer\\Challenges\\MpChallenges.lua" )

		if arg3 == UIEnums.ShareOptions.Facebook then

			Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.MPChallengePackComplete, 1, index)
			GoScreen( "Shared\\Facebook.lua", UIEnums.Context.Blurb )

		elseif arg3 == UIEnums.ShareOptions.Twitter then

			Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.MPChallengePackComplete, 0, index)
			GoScreen( "Shared\\Twitter.lua", UIEnums.Context.Blurb )

		elseif arg3 == UIEnums.ShareOptions.Blurb then

			Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.MPChallengePackComplete, 2, index)

		end
	end

end

function FrameUpdate( time )

	if GUI.CurrentState == GUI.State.Packs then
		local selection = UIButtons.GetSelection( GUI.PacksMenuID )
		if GUI.CurrentPack ~= selection then
			GUI.CurrentPack = selection

			ChangePack( selection )
		end

	elseif GUI.CurrentState == GUI.State.Challenges then
		local selection = UIButtons.GetSelection( GUI.ChallengeMenuID )
		if GUI.CurrentChallenge ~= selection then
			GUI.CurrentChallenge = selection

			if GUI.CurrentPack == UIEnums.MpChallengePack.Car then

			else
				RefreshChallenge( selection )
			end
		end
	
	elseif GUI.CurrentState == GUI.State.CarClass then
		local selection = UIButtons.GetSelection( GUI.CarClassMenuID )
		if GUI.CurrentClass ~= selection then
			GUI.CurrentClass = selection

			RefreshCarChallengeList( selection )
		end
	
	elseif GUI.CurrentState == GUI.State.CarChallenges then
		local selection = UIButtons.GetSelection( GUI.CarChallengesMenuID )
		if GUI.CurrentCar ~= selection then
			GUI.CurrentCar = selection

			RefreshCarChallenge( selection )
		end
	end
end

function EnterEnd()
	RestoreInfoLine()
	
	if UIGlobals.Multiplayer.InLobby == true then
		PlaySfxGraphicNext()
	else
		PlaySfxGraphicBack()
	end
end

function End()
	if UIGlobals.Multiplayer.InLobby == true then
		Amax.ChangeUiCamera( UIGlobals.CameraNames.MpLobby, UIGlobals.CameraLerpTime, 0 )
	end
end

function ChangePack( selection )

	-- special case for 
	if GUI.CurrentPack == UIEnums.MpChallengePack.Car then
		UIButtons.SetActive( GUI.CarClassMenuID, true )
		UIButtons.SetActive( GUI.ChallengeMenuID, false )
		
		RefreshCarChallengeList( selection )
	else
		UIButtons.SetActive( GUI.CarClassMenuID, false )
		UIButtons.SetActive( GUI.ChallengeMenuID, true )

		RefreshChallengeList( selection )
	end
end

function SelectPack( select )
	GUI.CurrentChallenge = -1 -- force challenges to refresh

	UIButtons.TimeLineActive( "select_pack", select )
	UIButtons.SetSelected( GUI.PacksMenuID, not select )

	-- move selected up and others off screen. Yeh, do one
	local index = UIButtons.GetSelectionIndex( GUI.PacksMenuID )+1
	for i, v in ipairs( GUI.Packs ) do
		if i == index then
			UIButtons.PrivateTimeLineActive( GUI.PackDummys[i], "move_up_" .. i, select )
		else
			UIButtons.PrivateTimeLineActive( GUI.PackDummys[i], "slide_out", select )
		end
	end

	-- special case for car pack
	if GUI.CurrentPack == UIEnums.MpChallengePack.Car then
		UIButtons.SetSelected( GUI.CarClassMenuID, select )
		UIButtons.TimeLineActive( "show_classes", select )

		if select == true then
			UIButtons.SetSelection( GUI.ChallengeMenuID, 1 ) -- default to top
		end
	else
		UIButtons.SetSelected( GUI.ChallengeMenuID, select )
		UIButtons.TimeLineActive( "show_challenges", select )

		if select == true then
			UIButtons.SetSelection( GUI.ChallengeMenuID, 1 ) -- default to top
		end
	end

	-- hide new
	Multiplayer.ChallengePackViewed( index - 1 )
	Mp_HideItemNew( GUI.PacksMenuID, index - 1 )
end

function SelectClass( select, class )
	UIButtons.TimeLineActive( "select_class", select )
	UIButtons.TimeLineActive( "show_car_challenge", select )

	-- move selected up and others off screen. Yeh, do one
	local index = UIButtons.GetSelectionIndex( GUI.CarClassMenuID )+1
	for i, v in ipairs( GUI.Classes ) do
		if i == index then
			UIButtons.PrivateTimeLineActive( UIButtons.FindChildByName( GUI.CarClassNodes[i], "frame" ), "move_up_" .. i, select )
		else
			UIButtons.PrivateTimeLineActive( UIButtons.FindChildByName( GUI.CarClassNodes[i], "frame" ), "slide_out", select )
		end
	end

	-- move car head on off
	UIButtons.PrivateTimeLineActive( GUI.PackDummys[UIButtons.GetSelectionIndex( GUI.PacksMenuID )+1], "slide_out", select )

	UIButtons.SetSelected( GUI.CarClassMenuID, not select )
	UIButtons.SetSelected( GUI.CarChallengesMenuID, select )

	if select == true then
		UIButtons.SetSelection( GUI.CarChallengesMenuID, 1 ) -- default to top
		RefreshCarChallenge( 1 )
	end
end

function RefreshChallenge( selection )

	local challenge = GUI.Challenges[selection]

	UIButtons.ChangeText( GUI.ChallengeNameID, "MPL_CHALLENGE" .. challenge.type .. "_NAME" )
	UIButtons.ChangeText( GUI.ChallengeDescID, "MPL_CHALLENGE" .. challenge.type .. "_DESC")
	UIButtons.ChangeText( GUI.ChallengeProgressID, "MPL_CHALLENGE" .. challenge.type .. "_PROGRESS")

	SetupProgressBar( SCUI.name_to_id["challenge_frame"], challenge.progress )
	RefreshChallengeRewards( GUI.Challenges[selection], selection )
end


function RefreshChallengeList( selection )

	GUI.Challenges = Multiplayer.GetChallenges( selection )

	-- turn all off
	for i, v in ipairs( GUI.ChallengeNodes ) do
		UIButtons.SetActive( v, false )
	end

	-- turn on active challenges
	for i, v in ipairs( GUI.Challenges ) do
		UIButtons.SetActive( GUI.ChallengeNodes[i], true )
		UIButtons.ChangeText( UIButtons.FindChildByName( GUI.ChallengeNodes[i], "challenge_name" ), "MPL_CHALLENGE" .. v.type .. "_NAME" )

		SetupProgressBar( GUI.ChallengeNodes[i], v.progress )

		RefreshChallengeTiers( GUI.ChallengeNodes[i], v )
	end
end


function RefreshCarChallengeList( selection )

	GUI.Cars = Multiplayer.GetCarChallenges( selection, false )

	local challenges = 
	{
		[1] = "winner",
		[2] = "repeat_offender",
	}

	-- turn all off
	for i, v in ipairs( GUI.CarChallengeNodes ) do
		UIButtons.SetActive( v, false )
	end

	-- turn on active challenges
	for i, car in ipairs( GUI.Cars ) do
		UIButtons.SetActive( GUI.CarChallengeNodes[i], true )

		local car_icon = UIButtons.FindChildByName( GUI.CarChallengeNodes[i], "car_icon" )
		local lock_icon = UIButtons.FindChildByName( GUI.CarChallengeNodes[i], "locked_icon" )
		local rank_text = UIButtons.FindChildByName( GUI.CarChallengeNodes[i], "rank_text" )

		if car.unlocked == true then
			UIButtons.SetActive( car_icon, true, true )
			UIButtons.SetActive( lock_icon, false )
			UIButtons.SetActive( rank_text, false )

			UIShape.ChangeSceneName( car_icon, car.sheet )
			UIShape.ChangeObjectName( car_icon, car.icon )
			UIButtons.SetNodeItemLocked( GUI.CarChallengesMenuID, i-1, false )
			
			-- setup tier data
			for j, challenge in ipairs( challenges ) do
				for k = 1, car[challenge].tiers do
					-- turn on each pip
					if k < car[challenge].tier or car[challenge].complete == true then -- completed
						UIButtons.ChangeColour( UIButtons.FindChildByName( GUI.CarChallengeNodes[i], challenge .. k ), "Support_0" )
					
					elseif k == car[challenge].tier then
						UIButtons.ChangeColour( UIButtons.FindChildByName( GUI.CarChallengeNodes[i], challenge .. k ), "Support_4" )
					else
						UIButtons.ChangeColour( UIButtons.FindChildByName( GUI.CarChallengeNodes[i], challenge .. k ), "Locked" )
					end
				end
			end
		else
			-- locked
			UIButtons.SetActive( car_icon, false, true )
			UIButtons.SetActive( lock_icon, true )
			UIButtons.SetActive( rank_text, true )

			UIButtons.ChangeText( rank_text, "MPL_UNLOCK_RANK" .. car.rank_needed )
			UIButtons.SetNodeItemLocked( GUI.CarChallengesMenuID, i-1, true )
		end
	end
end


function RefreshCarChallenge( selection )

	local car = GUI.Cars[selection]

	local challenges = 
	{
		[1] = "repeat_offender",
		[2] = "winner",
	}

	local nodes =
	{
		[1] = GUI.RepeatRewardNodes,
		[2] = GUI.WinnerRewardNodes,
	}

	-- car data
	UIButtons.ChangeText( SCUI.name_to_id["car_name"], car.name )

	local man_square_id = SCUI.name_to_id["manufacturer_logo_square"]
	local man_rect_id	= SCUI.name_to_id["manufacturer_logo_rect"]

	if car.texture_square == true then
		UIButtons.SetActive( man_square_id, true )
		UIButtons.SetActive( man_rect_id, false )
		UIButtons.ChangeTexture( { filename = car.texture }, 0, man_square_id )
	else
		UIButtons.SetActive( man_square_id, false )
		UIButtons.SetActive( man_rect_id, true )
		UIButtons.ChangeTexture( { filename = car.texture }, 0, man_rect_id )
	end

	-- set up both challenges
	for i, v in ipairs( challenges ) do
		
		UIButtons.ChangeText( SCUI.name_to_id[v .. "_name"], "MPL_CAR_CHALLENGE" .. car.id .. "_" .. car[v].type .. "_NAME" )
		UIButtons.ChangeText( SCUI.name_to_id[v .. "_desc"], "MPL_CAR_CHALLENGE" .. car.id .. "_" .. car[v].type .. "_DESC")
		UIButtons.ChangeText( SCUI.name_to_id[v .. "_amount"], "MPL_CAR_CHALLENGE" .. car.id .. "_" .. car[v].type .. "_PROGRESS")

		SetupProgressBar( SCUI.name_to_id[v .. "_frame"], car[v].progress )

		-- set up reward
		for j = 1, 3 do
			local node = nodes[i][j]
			local name_id = UIButtons.FindChildByName( node, "name" )
			local icon_id = UIButtons.FindChildByName( node, "icon" )
		
			-- [WillB] #36774 - some cars cant be resprayed so swap the car upgrades out for the fan rewards
			if v == "winner" then
				if car.can_respray == true then
					-- change car upgrades
					UIButtons.ChangeText( name_id, "MPL_CAR_UPGRADE_TYPE" .. UIEnums.MpUpgradeChallenges.Winner .. "_" .. j-1 )		
					UIShape.ChangeSceneName( icon_id, "fe_icons" )
					UIShape.ChangeObjectName( icon_id, UIGlobals.CarUpgradeIcons[GUI.CarRewards["winner"][j]] )
				else
					-- change fan reward text
					UIButtons.ChangeText( name_id, "MPL_CHALLENGE" .. UIEnums.MpUpgradeChallenges.Winner .. "_" .. j-1 .. Select( j == car[v].tier, "_FAN_REWARD", "_FANS" ) )
					UIShape.ChangeSceneName( icon_id, "common_icons" )
					UIShape.ChangeObjectName( icon_id, "fan" )
				end
			end
			
			RefreshCarChallengeRewards( node, car[v], j )
		end
	end

	-- update new
	Multiplayer.CarChallengeViewed( car.id )
end


function RefreshChallengeRewards( challenge, selection )

	for i = 1, 3 do
		local fans_id = UIButtons.FindChildByName( GUI.RewardNodes[i], "fans" )
		local icon_id = UIButtons.FindChildByName( GUI.RewardNodes[i], "icon" )
		local tick_id = UIButtons.FindChildByName( GUI.RewardNodes[i], "tick" )

		-- doesnt have this tier
		if i > challenge.tiers then
			UIButtons.ChangeText( fans_id, UIText.CMN_NOWT )
			UIButtons.SetActive( GUI.RewardNodes[i], false )
		else
			-- change fan reward text
			UIButtons.ChangeText( fans_id, "MPL_CHALLENGE" .. challenge.type .. "_" .. i - 1 .. "_FANS" )
		
			UIButtons.SetActive( GUI.RewardNodes[i], true )
			UIButtons.SetActive( tick_id, false )

			if i < challenge.tier or challenge.complete == true then -- completed
				UIButtons.SetActive( tick_id, true )
				UIButtons.ChangeColour( icon_id, "Support_0", true )
				UIButtons.ChangeColour( fans_id, "Support_0", true )

			elseif i == challenge.tier then -- active
				UIButtons.ChangeColour( icon_id, "Support_4", true )
				UIButtons.ChangeColour( fans_id, "Support_4", true )
				
				UIButtons.ChangeText( fans_id, "MPL_CHALLENGE" .. challenge.type .. "_" .. i - 1 .. "_FAN_REWARD" )

			else -- not active yet
				UIButtons.ChangeColour( icon_id, "Locked", true )
				UIButtons.ChangeColour( fans_id, "Locked", true )
			end
		end
	end
end


function RefreshCarChallengeRewards( reward_node, challenge, index )
	local tick_id = UIButtons.FindChildByName( reward_node, "tick" )
	local icon_id = UIButtons.FindChildByName( reward_node, "icon" )
	local name_id = UIButtons.FindChildByName( reward_node, "name" )
	
	-- doesnt have this tier
	if index > challenge.tiers then
		UIButtons.SetActive( reward_node, false )
	else
		UIButtons.SetActive( reward_node, true )
		UIButtons.SetActive( tick_id, false )

		if index < challenge.tier or challenge.complete == true then -- completed
			UIButtons.SetActive( tick_id, true )
			UIButtons.ChangeColour( icon_id, "Support_0", true )
			UIButtons.ChangeColour( name_id, "Support_0", true )

		elseif index == challenge.tier then -- active
			UIButtons.ChangeColour( icon_id, "Support_4", true )
			UIButtons.ChangeColour( name_id, "Support_4", true )

		else -- not active yet
			UIButtons.ChangeColour( icon_id, "Locked", true )
			UIButtons.ChangeColour( name_id, "Locked", true )
		end
	end
end


function RefreshChallengeTiers( node, challenge )
	-- show tier progress
	for i = 1, 3 do
		local tier_id = UIButtons.FindChildByName( node, "part_" .. i )

		-- doesnt have this tier
		if i > challenge.tiers then
			UIButtons.SetActive( tier_id, false )
		else
			-- its been completed
			if i < challenge.tier or challenge.complete == true then -- completed
				UIButtons.ChangeColour( tier_id, "Support_0" )
				UIButtons.SetActive( tier_id, true )
			
			elseif i == challenge.tier then
				UIButtons.ChangeColour( tier_id, "Support_4" )
				UIButtons.SetActive( tier_id, true )
			else
				UIButtons.ChangeColour( tier_id, "Locked" )
				UIButtons.SetActive( tier_id, true )
			end
		end
	end
end


function SetupChallenge( sValue, uTextID, bUnlocked )
	local root_id = UIButtons.CloneXtGadgetByName( "Multiplayer\\Challenges\\MpChallenges.lua", "node_challenge" )
	local text_id = UIButtons.FindChildByName( root_id, "challenge_name" )

	UIButtons.ChangeText( text_id, uTextID )
	UIButtons.AddListItem( GUI.ChallengeMenuID, root_id, sValue, not bUnlocked )

	return root_id
end


function SetupProgressBar( sRoot, sValue )
	local backing_id = UIButtons.FindChildByName( sRoot, "backing" )
	local progress_id = UIButtons.FindChildByName( sRoot, "progress" )

	local size_x = UIButtons.GetSize( backing_id )
	local x, size_y = UIButtons.GetSize( progress_id )

	UIButtons.ChangeSize( progress_id, ( size_x * sValue ), size_y, 0.0 )
end

