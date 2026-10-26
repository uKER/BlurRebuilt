PopupGUI = 
{
	finished	= false,
	error		= false,
	state		= nil,
	task		= nil,
	
	cancel		= false,	-- waiting for bar to slide out
	next_screen	= nil,		-- where to go
	go_subscreen = nil,		-- Subscreen?
	go_launch_screen = nil,	-- LaunchScreen?
	
	next_button_id		= -1, 
	back_button_id		= -1, 
	title_button_id		= -1, 
	message_button_id	= -1, 
	
	timer				= 0,
	join_started		= false,
	spawn_next			= nil,		-- Sequential call to another custom popup
	
	timeout				= -1,
	timeout_timer		= 0,
	
	profile_download_failed = false,
}

function PopupInit()
	if UIGlobals.PopupSpawned == false then
		InfoLineLock()
		StoreInfoLine_F()
		SetupInfoLine_F()
	end
	
	-- reset
	UIGlobals.PopupSpawned = false
	
	PopupGUI.send_next = false
	PopupGUI.send_back = false
	PopupGUI.send_arg1 = nil
	PopupGUI.send_arg2 = CustomPopup.CurrentType
	PopupGUI.send_arg3 = nil
	
	if CustomPopup.CurrentData == nil then
		PopupGUI.error = true
		print("Current data == nil : That's not right at all")
		return
	end

	UIScreen.SetPopupTimers(UIGlobals.screen_time.popup_start, UIGlobals.screen_time.popup_end)
	
	local popupLayer = UIEnums.Layer.LAYER_4
	
	-- Build name_to_index
	SCUI_Popup.name_to_index = { }
	for i,v in pairs( SCUI_Popup.elements ) do
		if IsString( v.name ) then
			SCUI_Popup.name_to_index[ v.name ] = i
		end
	end
		
	-- Run the init callback
	if CustomPopup.CurrentData.init_CB ~= nil then
		CustomPopup.CurrentData.init_CB()
	end

	-- Modify title
	local title = SCUI_Popup.elements[ SCUI_Popup.name_to_index[ "title" ] ]
	local ttl = CustomPopup.CurrentData.title_text_ID
	if IsNumber( ttl ) == true then
		title.text_id = ttl
		title.text_string = nil
	elseif IsString( ttl ) == true then
		title.text_id = nil
		title.text_string = ttl
	end

	-- Modify body
	local message = SCUI_Popup.elements[ SCUI_Popup.name_to_index[ "message" ] ]
	local msg = CustomPopup.CurrentData.message_text_ID
	if IsNumber( msg ) == true then
		message.text_id = msg
		message.text_string = nil
	elseif IsString( msg ) == true then
		message.text_id = nil
		message.text_string = msg
	end
	
	-- modify frame
	local frame_s = SCUI_Popup.elements[ SCUI_Popup.name_to_index[ "box_frame" ] ]

	-- add the menu options
	local menu_size_y = 0
	
	if CustomPopup.CurrentData.options ~= nil then
		local menu_id = SCUI_Popup.name_to_id[ "options" ]
		local menu_padding_y = UIButtons.GetXtVar( menu_id, "padding.top" )

		for i, v in ipairs( CustomPopup.CurrentData.options ) do
			local node_id = UIButtons.CloneXtGadgetByName( "TheCustomPopup.lua", "menu_node" )
			local option_id = UIButtons.FindChildByName( node_id, "option_name" )

			UIButtons.ChangeText( option_id, v.name )
			if v.locked ~= nil then
				UIButtons.AddListItem( menu_id, node_id, v.value, v.locked )

				if v.locked == true then
					UIButtons.ChangeColour(option_id, "Dark_White")
				end
			else
				UIButtons.AddListItem( menu_id, node_id, v.value )
			end

			local size_x, size_y, size_z = UIButtons.GetSize( node_id )
			menu_size_y = menu_size_y + size_y + menu_padding_y
		end
	end

	-- modify icon
	local icon = SCUI_Popup.elements[ SCUI_Popup.name_to_index[ "icon" ] ]
	if IsMultiplayerMode() == true then
		icon.object_name = "groups"
	else
		icon.object_name = "bio"
	end

	-- Position
	local pos = CustomPopup.CurrentData.pos
	if IsTable( pos ) == true then
		frame_s.pos = pos
	end
	
	local help_text =
	{
		SCUI_Popup.elements[ SCUI_Popup.name_to_index[ "next" ] ],
		SCUI_Popup.elements[ SCUI_Popup.name_to_index[ "back" ] ],
		SCUI_Popup.elements[ SCUI_Popup.name_to_index[ "info_1" ] ],
		SCUI_Popup.elements[ SCUI_Popup.name_to_index[ "info_2" ] ],
	}
	
	local info_1_o = SCUI_Popup.elements[ SCUI_Popup.name_to_index[ "info_1_o" ] ]
	local info_2_o = SCUI_Popup.elements[ SCUI_Popup.name_to_index[ "info_2_o" ] ]
	
	local is_english = ( UIGlobals.CurrentLanguage == UIEnums.Language.English )
	
	if is_english == false then
		help_text[ 3 ] = info_1_o
		help_text[ 4 ] = info_2_o
	end
	
	local count = 1
	local posX = -12
	
	-- Next
	local nxt = CustomPopup.CurrentData.next_text_ID
	if IsNumber( nxt ) then
		help_text[ count ].text_id = nxt
		count = count + 1
	end
	
	-- Back
	local bck = CustomPopup.CurrentData.back_text_ID
	if IsNumber( bck ) then
		help_text[ count ].text_id = bck
		count = count + 1
	end
	
	-- Info 1
	local inf = CustomPopup.CurrentData.x_text_ID
	if IsNumber( inf ) then
		help_text[ count ].text_id = inf
		count = count + 1
	end

	-- Info 2
	local help = CustomPopup.CurrentData.y_text_ID
	if IsNumber( help ) then
		help_text[ count ].text_id = help
		count = count + 1
	end
	
	-- Clear the rest of the info text
	for i = count, #help_text, 1 do
		help_text[ i ].pos.x = 0
	end

	AddSCUI_Elements( SCUI_Popup )

	-- turn off clone
	UIButtons.SetActive( SCUI_Popup.name_to_id[ "menu_node" ], false )

	-- store the ids
	PopupGUI.next_button_id	= SCUI_Popup.name_to_id[ "next" ]
	PopupGUI.back_button_id	= SCUI_Popup.name_to_id[ "back" ]
	PopupGUI.title_button_id = SCUI_Popup.name_to_id[ "title" ]
	PopupGUI.message_button_id = SCUI_Popup.name_to_id[ "message" ] 
	
	-- timeout
	PopupGUI.timeout = -1
	PopupGUI.timeout_timer = 0
	
	if CustomPopup.CurrentData.timeout ~= nil then
		PopupGUI.timeout = CustomPopup.CurrentData.timeout
	end
	
	-- box size
	local box_frame_id = SCUI_Popup.name_to_id[ "box_frame" ]
	local box_inner_id = SCUI_Popup.name_to_id[ "box_inner" ]
	local size_table = CustomPopup.CurrentData.size
	
	if IsTable( size_table ) == true then
		-- override
		UIButtons.ChangeSize( box_frame_id, size_table.x, size_table.y )
		UIButtons.ChangeSize( box_inner_id, size_table.x, size_table.y )
	else
		-- auto-size the box
		local frame_sy = 0
		local padding_y = title.pos.y
		
		frame_sy = frame_sy + padding_y
		
		local title_height = UIButtons.GetStaticTextHeight( SCUI_Popup.name_to_id[ "title" ] )
		frame_sy = frame_sy + title_height
		
		local msg_height = UIButtons.GetStaticTextHeight( SCUI_Popup.name_to_id[ "message" ] )
		frame_sy = frame_sy + msg_height
		
		if msg_height > 0 then
			local options_id = SCUI_Popup.name_to_id[ "options" ]
			local px, py, pz = UIButtons.GetPosition( options_id )
			UIButtons.ChangePosition( options_id, px, py + msg_height, pz )
			
			local diff_y = py - message.pos.y
			frame_sy = frame_sy + diff_y
		else
			frame_sy = frame_sy + padding_y
		end		
		
		frame_sy = frame_sy + menu_size_y
		
		frame_sy = frame_sy + padding_y
		
		local sx, sy, sz = UIButtons.GetSize( box_frame_id )
		UIButtons.ChangeSize( box_frame_id, sx, frame_sy, sz )
		UIButtons.ChangeSize( box_inner_id, sx, frame_sy, sz )
	end
	
	-- Add this last so it can be attached
	if CustomPopup.CurrentData.show_progress ~= nil then
		UIButtons.SetParent( AddLoadingSegs( -20, -20, 20, 20, UIEnums.Panel._2DAA ), SCUI_Popup.name_to_id[ "box_frame" ], UIEnums.Justify.BottomRight )
		local x, y, z = UIButtons.GetSize( SCUI_Popup.name_to_id[ "box_frame" ] )
		UIButtons.ChangeSize( SCUI_Popup.name_to_id[ "box_frame" ],x + 20, y, z )
		UIButtons.ChangeSize( SCUI_Popup.name_to_id[ "box_inner" ],x + 20, y, z )
	end
	
	-- save icon
	if CustomPopup.CurrentData.save_icon == true then
		UIButtons.SetActive( SCUI_Popup.name_to_id[ "saving_icon" ], true )
	end
end

function PopupPostInit()
	UIButtons.TimeLineActive("Popup_Active",true,0,true)

	if UIEnums.CurrentPlatform == UIEnums.Platform.PC then
		-- [Blur2PC] The filter is centred on the screen; match the viewport's
		-- virtual dimensions exactly, without the authored widescreen overscan.
		UIButtons.ChangeSize( SCUI_Popup.name_to_id[ "filter" ], SCREEN_WIDTH, 480 )
	end

	local menu_id = SCUI_Popup.name_to_id[ "options" ]
	
	-- this will change the current selection on the lobby menu only
	if CustomPopup.CurrentType == UIEnums.CustomPopups.LobbyOptions and UIGlobals.MpLobbyMenuSelection ~= nil then
		UIButtons.SetSelection( menu_id, UIGlobals.MpLobbyMenuSelection )
	end
	-- Initial value
	if IsNumber( CustomPopup.CurrentData.default_option ) == true then
		UIButtons.SetSelection( menu_id, CustomPopup.CurrentData.default_option )
	end
	
	if CustomPopup.CurrentData.silent == true then
		UIButtons.SetActive( SCUI_Popup.name_to_id[ "popup_rb" ], false )
	else
		UISystem.PlaySound( UIEnums.SoundEffect.GraphicTextBox )
	end
	
	if CustomPopup.CurrentData.darken == false then
		UIButtons.SetActive( SCUI_Popup.name_to_id[ "filter" ], false )
	end
end

function PopupStartLoop(time)
	return true
end


function PopupMessageUpdate(message,arg1,arg2)
	if PopupGUI.error == true or PopupGUI.finished == true then
		return
	end
	
	-- run the message call back
	if CustomPopup.CurrentData.message_CB ~= nil then
		if CustomPopup.CurrentData.message_CB( message, arg1, arg2, arg3, arg4 ) == true then
			return
		end
	end
	
	-- process my messages
	if message == UIEnums.Message.MenuNext and CustomPopup.CurrentData.no_next ~= true then
		PlaySfxNext()
		
		PopupGUI.finished = true
		PopupGUI.cancel = true
		PopupGUI.next_screen = CustomPopup.CurrentData.next_screen
		PopupGUI.go_subscreen = CustomPopup.CurrentData.go_subscreen
		PopupGUI.go_launch_screen = CustomPopup.CurrentData.go_launch_screen
	
		-- run the next callback
		if CustomPopup.CurrentData.next_CB ~= nil then
			CustomPopup.CurrentData.next_CB()
		end
		
		PopupGUI.send_next = true
		PopupGUI.send_back = false
		PopupGUI.send_arg1 = arg1
		PopupGUI.send_arg2 = CustomPopup.CurrentType
		PopupGUI.send_arg3 = nil

		if CustomPopup.CurrentData.options ~= nil then
			PopupGUI.send_arg3 = UIButtons.GetSelection( SCUI_Popup.name_to_id[ "options" ] )
		end
		
	elseif message == UIEnums.Message.MenuBack and CustomPopup.CurrentData.no_back ~= true then
		PlaySfxBack()
		
		PopupGUI.finished = true
		PopupGUI.cancel = true
		PopupGUI.next_screen = CustomPopup.CurrentData.prev_screen
		PopupGUI.go_subscreen = CustomPopup.CurrentData.go_subscreen
		PopupGUI.go_launch_screen = CustomPopup.CurrentData.go_launch_screen
		
		-- run the back callback
		if CustomPopup.CurrentData.back_CB ~= nil then
			CustomPopup.CurrentData.back_CB()
		end
		
		PopupGUI.send_next = false
		PopupGUI.send_back = true
		PopupGUI.send_arg1 = arg1
		PopupGUI.send_arg2 = CustomPopup.CurrentType
		PopupGUI.send_arg3 = nil
	end
end


function PopupFrameUpdate(time)
	if PopupGUI.error == true then
		return
	end
	
	if CustomPopup.CurrentData.update_CB ~= nil then
		CustomPopup.CurrentData.update_CB( time )
	end
	
	-- check for timeout
	if PopupGUI.timeout ~= -1 then
		PopupGUI.timeout_timer = PopupGUI.timeout_timer + time
		
		if PopupGUI.timeout_timer >= PopupGUI.timeout then
			if CustomPopup.CurrentData.timeout_CB ~= nil then
				CustomPopup.CurrentData.timeout_CB()
				PopupGUI.timeout = -1 -- reset
			end
		end
	end
		
	-- check for cancelling	
	if PopupGUI.cancel == true then
		-- just cancel - any screen changes are done in PopupEnd()
		UIScreen.CancelPopup()
		-- reset
		PopupGUI.cancel = false
	end	
end


function PopupRender()
end

function PopupEnterEnd()
	if PopupGUI.spawn_next == nil then
		RestoreInfoLine_F()
		InfoLineUnlock( true )
		
		-- [Bone] #37144 - Bloom after changing difficulty
		if UIGlobals.AsyncSave_FailMessageActive ~= true then
			UIButtons.TimeLineActive("Popup_Active",false)
		end
	else
		UIGlobals.PopupSpawned = true
	end
end

function PopupEndLoop(time)
	return true
end

function Popup_BoolToInt( param )
	return Select( ( param == true ), 1, 0 )
end

function PopupEnd()
	if CustomPopup.CurrentData.end_CB ~= nil then
		CustomPopup.CurrentData.end_CB()
	end	

	if PopupGUI.send_next == true then
		UIScreen.AddMessage( UIEnums.Message.PopupNext, PopupGUI.send_arg1, PopupGUI.send_arg2, PopupGUI.send_arg3 )
	end
	
	if PopupGUI.send_back == true then
		UIScreen.AddMessage( UIEnums.Message.PopupBack, PopupGUI.send_arg1, PopupGUI.send_arg2, PopupGUI.send_arg3 )
	end
	
	UIScreen.AddMessage( UIEnums.Message.PopupEnd, nil, PopupGUI.send_arg2, Popup_BoolToInt( PopupGUI.send_next ), Popup_BoolToInt( PopupGUI.send_back ) )
		
	-- clear my data pointer
	CustomPopup.CurrentData = nil
	
	if PopupGUI.go_launch_screen == true and IsNumber( PopupGUI.next_screen ) == true then
		LaunchScreen( PopupGUI.next_screen )
		
		PopupGUI.next_screen = nil
		PopupGUI.go_subscreen = nil
		PopupGUI.go_launch_screen = nil
		
	elseif IsString( PopupGUI.next_screen ) == true then
		-- screen change?
		print( "Custom Popup Going to ", PopupGUI.next_screen )
		
		if PopupGUI.go_subscreen == true then
			GoSubScreen( PopupGUI.next_screen )
		else
			GoScreen( PopupGUI.next_screen, PopupGUI.context )
		end
		
		PopupGUI.next_screen = nil
		PopupGUI.go_subscreen = nil
		PopupGUI.go_launch_screen = nil

	elseif IsNumber( PopupGUI.spawn_next ) == true then
		print("Spawn",PopupGUI.spawn_next)

		CustomPopup.CurrentType = PopupGUI.spawn_next
	
		-- Spawn another custom popup?
		CustomPopup.CurrentData = CustomPopup.Data[ PopupGUI.spawn_next ]
		
		-- Sanity
		if CustomPopup.CurrentData == nil then
			print("Error Spawned next custom popup : ", PopupGUI.spawn_next, "not found" )
			CustomPopup.CurrentData = CustomPopup.Data[ UIEnums.CustomPopups.Default ]
		else
			UIScreen.AddPopup("TheCustomPopup.lua", true)
		end
	end
	
	SCUI_Popup			= nil

	PopupInit			= nil
	PopupPostInit		= nil
	PopupStartLoop		= nil
	PopupMessageUpdate	= nil
	PopupFrameUpdate	= nil
	PopupRender			= nil
	PopupEnterEnd		= nil
	PopupEndLoop		= nil
	PopupEnd			= nil
end
