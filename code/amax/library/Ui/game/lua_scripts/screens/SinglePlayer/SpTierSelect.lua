GUI =
{
	finished = false,
	carousel_branch = "Races",
	
	CanExit = function( _GUI )
		if UIGlobals.SpSelectMode == UIGlobals.SpSelectMode_Tier then
			UISystem.PlaySound( UIEnums.SoundEffect.GraphicBackward )
			return true
		end
		
		return false
	end,
}

function Init()
	AddSCUI_Elements( )
	DeferCam_Init( "Sp_3" )
	
	CarouselApp_SetScreenTimers()
	net_SetRichPresence( UIEnums.RichPresence.Career )

	StoreInfoLine()
	SetupInfoLine( UIText.INFO_A_SELECT, UIText.INFO_B_BACK, UIText.INFO_Y_DIFFICULTY, "GAME_SHARE_BUTTON" )
	
	-- Make sure everything is actually unlocked.
	local unlocks = SinglePlayer.ProcessUnlocks()

	-- Reset selection
	if Sp_ReturnFromGame() == false then
		UIGlobals.Sp = { }
		UIGlobals.SpSelectMode = UIGlobals.SpSelectMode_Tier
		UIGlobals.Sp.CurrentEvent = nil
	end
			
	-- Refresh SinglePlayer tables
	UIGlobals.Sp.GameInfo = SinglePlayer.GameInfo()
	UIGlobals.Sp.TierInfo = SinglePlayer.TierInfo()
	UIGlobals.Sp.BossInfo = SinglePlayer.BossInfo()
	
	GUI.MenuId = SCUI.name_to_id[ "menu" ]
	
	if UIGlobals.FriendDemandAttemptFromMessage == false then
		PlaySPMovieFullScreen( UIEnums.SPVideoConfig.VIDEO_TUTORIAL_1 )
	end
end

function PostInit()
	UIButtons.TimeLineActive( "Hide_Carousel", true )
		
	GUI.node_id = { }
	for i, v in ipairs( UIGlobals.Sp.TierInfo ) do
		local node_id = UIButtons.CloneXtGadgetByName( "SpSCUI", "Tier_Node" )
		
		if v.unlocked then
			local items_id = UIButtons.CloneXtGadgetByName( "SpSCUI", "tier_OnBoss" .. i )
			
			UIButtons.SetParent( items_id,  UIButtons.FindChildByName( node_id, "TS_GreyCity" ), UIEnums.Justify.MiddleCentre )
			UIButtons.ChangeText( UIButtons.FindChildByName( node_id, "TS_Stars" ), "SPL_TIER_" .. (v.tier-1) .. "STARSANDMAX" )
			
			if  UIGlobals.CurrentLanguage ~= UIEnums.Language.English then
				UIButtons.SetActive( UIButtons.FindChildByName( node_id, "TS_Subtitle" ), true )
				UIButtons.ChangeText( UIButtons.FindChildByName( node_id, "TS_Subtitle" ), v.name )
			end
			
			-- [Feature creep] #36025 : Evil Shannon doesn't 'reveal herself' until her boss demands are completed
			if i == #UIGlobals.Sp.TierInfo then
				GUI.b9_c_id = items_id
				if SpTierSelect_RevealEvilShannon() == false then
					UIButtons.SetAlphaIntensity( UIButtons.FindChildByName( GUI.b9_c_id, "b9_c" ), nil, 0.0 )
				end
			end
		else
			UIButtons.ChangeTexture( { filename = "boss" .. v.tier .. "_bg" }, 0, UIButtons.FindChildByName( node_id, "TS_GreyCity" ) )
			UIButtons.SetActive( UIButtons.FindChildByName( node_id, "TS_GreyCity" ), true )
			
			UIButtons.SetActive( UIButtons.FindChildByName( node_id, "TS_LockedIcon" ), true )
			UIButtons.SetActive( UIButtons.FindChildByName( node_id, "TS_Stars" ), false, true )
			
			local locked_id = UIButtons.CloneXtGadgetByName( "SpSCUI", "TS_LockedTitle" )
			UIButtons.SetParent( locked_id, UIButtons.FindChildByName( node_id, "TS_LockedRoot" ), UIEnums.Justify.BottomCentre )
			UIButtons.ChangeText( UIButtons.FindChildByName( locked_id, "TS_LockedNumber" ), "GAME_NUM_" .. v.starRequirement )
			
			local sillowet = UIButtons.FindChildByName( node_id, "Silhouette" )
			UIButtons.SetActive( sillowet, true )
			
			local boss_index = v.tier
			if i == #UIGlobals.Sp.TierInfo then -- [Bone] #36025
				boss_index = 1
			end
			UIButtons.ChangeTexture( { filename = "boss" .. boss_index .. "_cutout" }, 0, sillowet )
		end
		
		if v.new == true then
			UIButtons.SetActive( UIButtons.FindChildByName( node_id, "TS_NewIcon" ), true )
		end
			
		-- UIButtons.ChangeText( UIButtons.FindChildByName( node_id, "Tier_Title" ), v.name )
		
		UIButtons.AddListItem( GUI.MenuId, node_id, i )
		
		GUI.node_id[ i ] = node_id
	end
	
	-- Jump straight into the event select	
	if Sp_ReturnFromGame() == true then
		UIButtons.SetSelection( GUI.MenuId, UIGlobals.Sp.CurrentTier )
		
		if UIGlobals.SpReturnMode == UIGlobals.SpSelectMode_Tier then
			UIGlobals.SpSelectMode = UIGlobals.SpReturnMode
			UIGlobals.SpReturnMode = nil
		else
			SpTierSelect_GoEventSelect()
		end
	end
	
	GUI.selection = UIButtons.GetSelection( GUI.MenuId )
	
	-- Start the movie
	UISystem.PauseMovie( GUI.selection, false )
end

function StartLoop( time )
	DeferCam_Update( time )
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end
	
	if IsControllerLocked() == true then
		return
	end

	-- Change difficulty	
	if UIGlobals.SpSelectMode == UIGlobals.SpSelectMode_Tier then
		if message == UIEnums.Message.ButtonY and arg2 == true then
			UIGlobals.OptionsTable = Amax.Options()
			SetupCustomPopup( UIEnums.CustomPopups.SpChangeDifficulty )	
		end
	end
	
	if UIGlobals.SpSelectMode == UIGlobals.SpSelectMode_Tier then
		if message == UIEnums.Message.ButtonLeftShoulder and arg2 == true then

			if Amax.CanUseShare() == true then
				local selection = UIButtons.GetSelection( GUI.MenuId )
				UIGlobals.ShareShowTier=UIGlobals.Sp.TierInfo[ selection ].unlocked
				SetupCustomPopup( UIEnums.CustomPopups.SpTierSelectSharingOptions )
			end

		elseif message == UIEnums.Message.PopupNext and arg2 == UIEnums.CustomPopups.SharingOptions then

			local selection = UIButtons.GetSelection( GUI.MenuId )
			
			if arg3 == UIEnums.ShareOptions.Facebook then
				
				Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.FanStatus + UIGlobals.SharingOptionsChosen, 1, GUI.selection )
				
				StoreScreen( UIEnums.ScreenStorage.FE_SOCIAL_NETWORK, "SinglePlayer\\SpTierSelect.lua" )
				GoScreen( "Shared\\Facebook.lua", UIEnums.Context.Blurb )
			
			elseif arg3 == UIEnums.ShareOptions.Twitter then

				Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.FanStatus + UIGlobals.SharingOptionsChosen, 0, GUI.selection )

				StoreScreen( UIEnums.ScreenStorage.FE_SOCIAL_NETWORK, "SinglePlayer\\SpTierSelect.lua" )
				GoScreen( "Shared\\Twitter.lua", UIEnums.Context.Blurb )

			elseif arg3 == UIEnums.ShareOptions.Blurb then

				Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.FanStatus + UIGlobals.SharingOptionsChosen, 2, GUI.selection )

			end		

		end
	end
	
	if message == UIEnums.Message.MenuNext and arg2 == true then
		if UIGlobals.SpSelectMode == UIGlobals.SpSelectMode_Tier then
		
			SinglePlayer.MarkNotNew( UIGlobals.Sp.TierInfo[ GUI.selection ].tierId )
			UIButtons.SetActive( UIButtons.FindChildByName( GUI.node_id[ GUI.selection ], "TS_NewIcon"), false )
			
			if UIGlobals.Sp.TierInfo[ GUI.selection ].unlocked == true then
				UIGlobals.Sp.CurrentTier = GUI.selection
				UIGlobals.Sp.CurrentEvent = nil
				UIGlobals.SpSelectMode = UIGlobals.SpSelectMode_Event
				
				SpTierSelect_GoEventSelect()
				
				PlaySfxNext()
				UISystem.PlaySound( UIEnums.SoundEffect.GraphicForward )
			else
				PlaySfxError()
			end
		end
	end
end

function SpTierSelect_GoEventSelect()
	UIButtons.SetSelected( GUI.MenuId, false )	
	UISystem.PauseMovie( GUI.selection, true )
	PushScreen( "SinglePlayer\\SpEventSelect.lua" )	
end

function FrameUpdate( time )
	DeferCam_Update( time )

	-- This was set after boss demand's complete 'reveal' sequence	
	if UIGlobals.Sp.RevealEvilShannon == true then
		print("Revealing evil shannon")
		UIButtons.SetAlphaIntensity( UIButtons.FindChildByName( GUI.b9_c_id, "b9_c" ), nil, 1.0 )
		UIGlobals.Sp.RevealEvilShannon = nil
	end
	
	-- [Bone] - 37714
	if UIGlobals.Sp.EnableTierSelect == true then
		UIButtons.SetSelected( GUI.MenuId, true )
		UIGlobals.Sp.EnableTierSelect = nil
	end
	
	--
	local selection = UIButtons.GetSelection( GUI.MenuId )
	if GUI.selection ~= selection then
		UIButtons.PrivateTimeLineActive( GUI.node_id[ selection ], "flash", true, 0 )

		UISystem.PauseMovie( GUI.selection, true )
		GUI.selection = selection
		UIButtons.TimeLineActive( "TierSelected", true, 0.0 )
		UISystem.PauseMovie( GUI.selection, false )
		
		if UIGlobals.Sp.TierInfo[ selection ].unlocked == true then
			SetupInfoLine( UIText.INFO_A_SELECT, UIText.INFO_B_BACK, UIText.INFO_Y_DIFFICULTY, "GAME_SHARE_BUTTON" )
		else
			SetupInfoLine( UIText.INFO_B_BACK, UIText.INFO_Y_DIFFICULTY, "GAME_SHARE_BUTTON" )
		end
	end
end

function Render()
	--
end

function EnterEnd()
	-- Clear out temps
	if UIGlobals.SpReturnMode == nil then
		UIGlobals.SpSelectMode = nil
		UIGlobals.Sp = nil
	end
	
	RestoreInfoLine()
	
	UIButtons.TimeLineActive( "Hide_Carousel", false )
end


function EndLoop( time )
	--
end

function End()
end

function SpTierSelect_RevealEvilShannon()
	for j, k in ipairs( UIGlobals.Sp.BossInfo[ #UIGlobals.Sp.TierInfo ].demands ) do
		if k.complete == false then
			return false
		end
	end
	return true
end
