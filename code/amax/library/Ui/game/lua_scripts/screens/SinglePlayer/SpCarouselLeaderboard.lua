GUI =
{
	finished = false, 
	carousel_branch = "Leaderboards",

	tier_info = { },
	
	tier_select = { },
	event_select = { },
	
	event_nodes = { },
	
	selection = { },
	
	
	current_selection = 0,
		
	current_tier_select_index = 1,
	current_event_select_index = 1,	
	
	change_leaderboard = false,
	cool_off_timer = 0.0,	
	cool_down_time = 1.5,
	
	slider = {},
	
	tier_selected = true,
	
	Displaying_Lost_Connection = false,

	CanExit = function( _GUI )
		PlaySfxGraphicBack()
		return true
	end,
}

function Init()

	AddSCUI_Elements()
	CarouselApp_SetScreenTimers()	
	StoreInfoLine()
	
	GUI.tier_info = SinglePlayer.TierInfo()	
		
	GUI.silder =  SCUI.name_to_id[ "Tiers" ] 
	
	GUI.tier_select = SCUI.name_to_id[ "Options" ]
		
	GUI.selection = SCUI.name_to_id[ "selected" ]
	
	SpCarouselLeaderboard_SetupTierSelectSlider(GUI.silder)
	
	
end

function PostInit()

	UIButtons.TimeLineActive( "Hide_Carousel", true )
	UIButtons.SetActive( SCUI.name_to_id["_EventSelect"], false, true )
	
	SetUpLeaderboardScreen()
	
	UIGlobals.SpLeaderboardCarouselFocusOnMenu = true
	
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end
	
	if IsControllerLocked() == true then
		return
	end
	
	if message == UIEnums.Message.MenuNext and arg2 == true then

	
	elseif message == UIEnums.Message.MenuBack and arg2 == true then
	
		PopScreen()
		
		CloseApp( UIEnums.RwBranch.Leaderboards)
		
		Amax.ChangeUiCamera( UIGlobals.CameraNames.SpCarousel, UIGlobals.CameraLerpTime, 0 )
	
	elseif message == UIEnums.Message.ButtonUp then
		
		if UIGlobals.SpLeaderboardCarouselFocusOnMenu == true then
			MoveSelectionUp()
			
			for i = 1, #GUI.event_nodes, 1 do
				UIButtons.SetSelected( GUI.event_nodes[ i ].event_slider, false )
			end
			
			GUI.tier_selected = true	
		end	
	
	elseif message == UIEnums.Message.ButtonDown then
	
		if UIGlobals.SpLeaderboardCarouselFocusOnMenu == true then
			MoveSelectionDown()
			
			--make sure we only have the right stuff selected
			for i = 1, #GUI.event_nodes, 1 do
				UIButtons.SetSelected( GUI.event_nodes[ i ].event_slider, false )
			end
							
			UIButtons.SetSelected( GUI.event_nodes[ GUI.current_tier_select_index ].event_slider, true )
			
			GUI.tier_selected = false			
		end
	
	elseif message == UIEnums.Message.ButtonY then
	
		--we want to change our focus
		if UIGlobals.SpLeaderboardCarouselFocusOnMenu == true then
			UIGlobals.SpLeaderboardCarouselFocusOnMenu = false
		else
			UIGlobals.SpLeaderboardCarouselFocusOnMenu = true
		end
		
		SetNewFocus()
		
	elseif message == UIEnums.Message.PopupNext and arg2 == UIEnums.CustomPopups.MultiplayerOnlineConnectionLost then
		PopScreen()
		
		CloseApp( UIEnums.RwBranch.Leaderboards)
		
		Amax.ChangeUiCamera( UIGlobals.CameraNames.SpCarousel, UIGlobals.CameraLerpTime, 0 )
	end
end

function FrameUpdate( time )


	local sliderIndex = (UIButtons.GetSelectionIndex( GUI.silder ) + 1)
	
	if change_leaderboard == true then
		GUI.cool_off_timer = (GUI.cool_off_timer - time)
	end

	if GUI.current_tier_select_index ~= sliderIndex then
	
		GUI.current_tier_select_index = sliderIndex
		
		-- turn everything off
		for i = 1, #GUI.event_nodes, 1 do		
			UIButtons.SetActive( GUI.event_nodes[ i ].node , false, true)		
		end	
		
		--activate our new node
		UIButtons.SetActive( GUI.event_nodes[ GUI.current_tier_select_index ].node , true, true)
		
		--make sure we know that the event has changed
		GUI.current_event_select_index  = -1
		
	end
	
	local eventSliderIndex =  (UIButtons.GetSelectionIndex( GUI.event_nodes[ GUI.current_tier_select_index ].event_slider ) + 1)
	
	if GUI.current_event_select_index ~= eventSliderIndex then 
	
		GUI.current_event_select_index = eventSliderIndex
	
		GUI.cool_off_timer = GUI.cool_down_time
		change_leaderboard = true
	
	end
	
	if change_leaderboard == true and GUI.cool_off_timer <= 0.0 then 
		change_leaderboard = false
		
		PopScreen()
		ReLoadLeaderboardScreen()
		
	end
	
end

function EnterEnd()

	UIButtons.TimeLineActive( "Hide_Carousel", false )
	
	--dont want the leaderboards thinking we are still in the carousel
	UIGlobals.SpLeaderboardInCarousel = false
	
	RestoreInfoLine()
end

function End()
	
end

function SpCarouselLeaderboard_SetupTierSelectSlider( slider )

	if slider == nil then
		print("ERROR - SpCarouselLeaderboard_SetupTierSelectSlider( slider ) : slider is not valid!!")
		return
	elseif GUI.tier_info == nil then
		print("ERROR - SpCarouselLeaderboard_SetupTierSelectSlider( slider ) : GUI.tier_info is not valid!!")
		return
	end
	
	for i, v in ipairs( GUI.tier_info ) do		
		UIButtons.AddItem( slider, v.tierId , v.name, false )
		
		local node_id = UIButtons.CloneXtGadgetByName( "SinglePlayer\\SpCarouselLeaderboard.lua", "_EventSelect" )
		local event_slider_id = UIButtons.FindChildByName( node_id, "_Event" )
		
		GUI.event_nodes[ i ] = { node = node_id, event_slider = event_slider_id, }
		
		UIButtons.SetActive( node_id, false, true)
				
		for j, w in ipairs( v.events ) do		
			
			UIButtons.AddItem( event_slider_id, j, "SPL_LEADERBOARD_EVENT_SELECT_NAME" .. SinglePlayer.EventInfo(w).kind .. j , false )
			
		end
		
		UIButtons.SetParent( node_id, SCUI.name_to_id[ "Options" ], UIEnums.Justify.MiddleCentre )
						
	end	
	
	UIButtons.SetActive( GUI.event_nodes[ (UIButtons.GetSelectionIndex( GUI.silder ) + 1) ].node , true, true)
		
	GUI.current_tier_select_index = (UIButtons.GetSelectionIndex( GUI.silder ) + 1)
	
	for i = 1, #GUI.event_nodes, 1 do
		UIButtons.SetSelected( GUI.event_nodes[ i ].event_slider, false )
	end
		
	UIButtons.SetSelected( GUI.silder, true)
	
end

function SetUpLeaderboardScreen()

	--let the leaderboard screen know that we are coming from the carousel screen
	UIGlobals.SpLeaderboardInCarousel = true
	
	--Set the default leaderboard
	Amax.SetCurrentEvent(  SinglePlayer.EventInfo((GUI.tier_info[GUI.current_tier_select_index].events[UIButtons.GetSelection( GUI.event_nodes[ 1 ].event_slider  )])).eventId )
	
	PushScreen( "SinglePlayer\\SpShowLeaderboard.lua" )

end

function ReLoadLeaderboardScreen()

	--Move to the currently selected leaderboard
	
	Amax.SetCurrentEvent(  SinglePlayer.EventInfo((GUI.tier_info[GUI.current_tier_select_index].events[UIButtons.GetSelection( GUI.event_nodes[ GUI.current_tier_select_index ].event_slider  )])).eventId  )
	
	PushScreen( "SinglePlayer\\SpShowLeaderboard.lua" )
	
end

function MoveSelectionUp()

	GUI.current_selection = GUI.current_selection -1
	
	if GUI.current_selection < 0 then
		GUI.current_selection = 0
	end
	
	SetSelectedItem()
	
end

function MoveSelectionDown()

	GUI.current_selection = GUI.current_selection +1
	
	if GUI.current_selection > 1 then
		GUI.current_selection = 1
	end
	
	SetSelectedItem()
end

function SetSelectedItem()
	local distance = (-5 * GUI.current_selection)
	local x, y, z = UIButtons.GetPosition( GUI.tier_select )
	UIButtons.ChangePosition(GUI.selection,x,distance,z)
end

function SetNewFocus()

	if UIGlobals.SpLeaderboardCarouselFocusOnMenu == true then
		UIButtons.SetActive(GUI.selection, true, true)
		UIButtons.SetSelected(GUI.tier_select, true)
	else
		UIButtons.SetActive(GUI.selection, false, true)
		UIButtons.SetSelected(GUI.tier_select, false)
	end
	
end
