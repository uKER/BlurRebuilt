GUI =
{
	finished = false,
	
	current_mode = UIEnums.PhotoMode.Camera,
	
	do_camera = false,
	do_effects = false,
	do_controls = false,
	do_photo = false,
	do_colour = false,
	
	can_save = false,
	
	inactive_slider_ids = {  },
	colour_option_selected = false,
	
	values = 
	{
		brightness = 0,
		contrast = 0,
		saturation = 0,
		hint_amount = 0,
		shutter_angle = 0,
		idepth = 0,
		aperture = 0,
	},
	
	enable_render_in = -1,	-- frame to enable UI again in
}

function Init()
	AddSCUI_Elements()

	Amax.SendMessage( UIEnums.GameFlowMessage.Pause )
	Amax.StartPhotoCam()

	LSP.SetInPhotoUI(false);
	
	GUI.current_mode = UIEnums.PhotoMode.Camera

	GUI.options = 
	{
		{	-- brightness
			icon = "Brightness",
			text = UIText.HUD_PHOTO_EFFECT_BRIGHTNESS,
			num = 11, min = -0.25, max = 0.25, default = 5,
		},
		{	-- contrast
			icon = "Contrast",
			text = UIText.HUD_PHOTO_EFFECT_CONTRAST,
			num = 11, min = -0.5, max = 0.5, default = 5,
		},
		{	-- saturation
			icon = "Saturation",
			text = UIText.HUD_PHOTO_EFFECT_SATURATION,
			num = 11, min = -1, max = 1, default = 5,
		},		
		{	-- shutter angle
			icon = "Shutter Angle",
			text = UIText.HUD_PHOTO_EFFECT_SHUTTER_ANGLE,
			num = 11, min = 0, max = 10, default = 2,
		},
		{	-- aperture
			icon = "Aperture",
			text = UIText.HUD_PHOTO_EFFECT_APERTURE,
			num = 11, min = 0, max = 1,
		},
		{	-- hint amount
			icon = "Hint_Amount",
			text = UIText.HUD_PHOTO_EFFECT_HINT_AMOUNT,
			num = 11, min = 0, max = 1,
		},
		{	-- hint colour
			icon = "Hint_Colour",
			text = UIText.HUD_PHOTO_EFFECT_HINT_COLOUR,
		},
	}
	
	if UIEnums.CurrentPlatform == UIEnums.Platform.PS3 then
		UIShape.ChangeObjectName( SCUI.name_to_id[ "controls_icon" ], "ps3_controller_options" )
	end
	
	SetupScreenTitle( UIText.HUD_PHOTO_TITLE, SCUI.name_to_id["screen_tile"], "photo", nil, nil, nil, nil, nil, UIEnums.Panel._2DAA, true )
end

function PostInit()
	PhotoMode_UpdateMode()
	
	for i = 1, #GUI.options, 1 do
		local node_id = AddListNode( nil, i )
		
		local parent_id = SCUI.name_to_id[ "icon" ]
		
		if i ~= 1 then
			parent_id = UIButtons.CloneXtGadgetByName( "Ingame\\PhotoMode.lua", "icon" )
		end
		
		UIButtons.SetParent( parent_id, node_id, UIEnums.Justify.MiddleCentre )
		UIButtons.AddListItem( SCUI.name_to_id[ "effects_node_list" ], node_id )
		
		local icon_id = parent_id
		local text_id = UIButtons.FindChildByName( parent_id, "text" )
		local slider_id = UIButtons.FindChildByName( parent_id, "slider" )
		
		UIShape.ChangeObjectName( icon_id, GUI.options[ i ].icon )
		UIButtons.ChangeText( text_id, GUI.options[ i ].text )
		
		if GUI.options[ i ].num ~= nil and GUI.options[ i ].num > 2 then			
			for j = 1, GUI.options[ i ].num, 1 do
				UIButtons.AddItem( slider_id, ( j - 1 ), UIText.IDS_CMN_NOWT, false )
			end
		else
			GUI.inactive_slider_ids[ #GUI.inactive_slider_ids + 1 ] = slider_id
			local iconX, iconY, iconZ = UIButtons.GetPosition( icon_id )
			local x, y, z = UIButtons.GetPosition( text_id )
			UIButtons.ChangePosition( text_id, x, iconY , z )
			
		end
		
		if GUI.options[ i ].default ~= nil then
			UIButtons.SetSelection( slider_id, GUI.options[ i ].default )
		end
		
		GUI.options[ i ].slider_id = slider_id
	end
	
	PhotoMode_UpdateFilters()
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
	
	if message == UIEnums.Message.ButtonX then
		if GUI.current_mode == UIEnums.PhotoMode.Camera then
			GUI.current_mode = UIEnums.PhotoMode.CamControls
			PlaySfxNext()
			PhotoMode_UpdateMode()
		end
	end
	
	if message == UIEnums.Message.ButtonY then
		if GUI.current_mode == UIEnums.PhotoMode.Camera then
			PlaySfxNext()
			GUI.current_mode = UIEnums.PhotoMode.Effects
			PhotoMode_UpdateMode()
		
		elseif GUI.current_mode == UIEnums.PhotoMode.Viewing then

		end
	end
	
	if message == UIEnums.Message.MenuBack then
		if GUI.current_mode == UIEnums.PhotoMode.Camera then
			PlaySfxBack()
			
			GoScreen( "InGame\\Paused.lua" )
			
		elseif GUI.current_mode == UIEnums.PhotoMode.CamControls then
			PlaySfxBack()
			
			GUI.current_mode = UIEnums.PhotoMode.Camera
			PhotoMode_UpdateMode()
			
		elseif GUI.current_mode == UIEnums.PhotoMode.Effects then
			PlaySfxBack()
			
			GUI.current_mode = UIEnums.PhotoMode.Camera
			PhotoMode_UpdateMode()
			
		elseif GUI.current_mode == UIEnums.PhotoMode.EffectsColour then
			PlaySfxBack()
			
			GUI.current_mode = UIEnums.PhotoMode.Effects
			PhotoMode_UpdateMode()
		
		elseif GUI.current_mode == UIEnums.PhotoMode.Viewing then
			PlaySfxBack()
			
			GUI.current_mode = UIEnums.PhotoMode.Camera
			PhotoMode_UpdateMode()
		end
	end
	
	if message == UIEnums.Message.MenuNext then
		if GUI.current_mode == UIEnums.PhotoMode.Camera then
			UISystem.PlaySound( UIEnums.SoundEffect.PhotoTaken )
			
			UISystem.Render( false )
			Amax.TakePhoto()
			GUI.enable_render_in = 1
			GUI.current_mode = UIEnums.PhotoMode.Viewing
			PhotoMode_UpdateMode()
		
		elseif GUI.current_mode == UIEnums.PhotoMode.Viewing then
			if GUI.can_save == true then
				PlaySfxNext()
				StartAsyncSavePhotos()
			end
			
		elseif GUI.current_mode == UIEnums.PhotoMode.Effects and GUI.colour_option_selected == true then
			PlaySfxNext()
		
			GUI.current_mode = UIEnums.PhotoMode.EffectsColour
			PhotoMode_UpdateMode()
		end
	end
end

function FrameUpdate( time )
	
	PhotoMode_UpdateMode()	-- meh
	
	PhotoMode_UpdateFilters()
	
	-- show the UI again
	if GUI.enable_render_in >= 0 then
		if GUI.enable_render_in == 0 then
			UISystem.Render( true )
		end
		
		GUI.enable_render_in = GUI.enable_render_in - 1
	end
end

function Render()
	--
end

function EndLoop( time )
	--
end

function End()
	Amax.EndPhotoCam()
end

function PhotoMode_UpdateMode()
	GUI.do_camera = false
	GUI.do_effects = false
	GUI.do_photo = false
	GUI.do_colour = false
	GUI.do_controls = false
			
	if GUI.current_mode == UIEnums.PhotoMode.Camera then
		SetupInfoLine( UIText.INFO_PHOTO_TAKE_A, UIText.INFO_B_BACK, UIText.INFO_PHOTO_TOGGLE_CONTROLS_X, UIText.INFO_PHOTO_EFFECTS_Y )
		GUI.do_camera = true
		
	elseif GUI.current_mode == UIEnums.PhotoMode.CamControls then
		SetupBack()
		
		GUI.do_controls = true
		
	elseif GUI.current_mode == UIEnums.PhotoMode.Effects then
		local selection = UIButtons.GetSelection( SCUI.name_to_id[ "effects_node_list" ] )
		
		if selection == 7 then	-- meh
			GUI.colour_option_selected = true
			SetupMenuOptions()
		else
			GUI.colour_option_selected = false
			SetupBack()
		end
		
		GUI.do_effects = true
		
	elseif GUI.current_mode == UIEnums.PhotoMode.EffectsColour then
		SetupInfoLine( UIText.INFO_B_BACK )
		GUI.do_colour = true
		GUI.do_controls = false
		
	elseif GUI.current_mode == UIEnums.PhotoMode.Viewing then
		GUI.can_save  = ( CanSave( Profile.GetPrimaryPad(), true ) == true )
		
		GUI.do_controls = false
		
		if GUI.can_save == true then
			SetupInfoLine( UIText.INFO_PHOTO_SAVE_A, UIText.INFO_B_BACK )
		else
			SetupBack()
		end
		
		GUI.do_photo = true
	end
	
	UIButtons.TimeLineActive( "reticule_on", GUI.do_camera )
	UIButtons.TimeLineActive( "controls_on", GUI.do_controls )
	UIButtons.TimeLineActive( "effects_on", GUI.do_effects )
	UIButtons.TimeLineActive( "colour_on", GUI.do_colour )
	
	if GUI.do_photo == true then
		UIButtons.TimeLineActive( "reticule_on", false, 0.01 )
		UIButtons.TimeLineActive( "controls_on", false, 0.01 )
	end
	
	UIButtons.SetActive( SCUI.name_to_id[ "photo_dummy" ], GUI.do_photo, true )
	
	UIButtons.SetSelected( SCUI.name_to_id[ "colour_picker" ], GUI.do_colour )
	UIButtons.SetSelected( SCUI.name_to_id[ "effects_node_list" ], GUI.do_effects )
	
	for i = 1, #GUI.inactive_slider_ids, 1 do
		UIButtons.SetActive( GUI.inactive_slider_ids[ i ], false )
	end
	
	Amax.LockPhotoCamControls( GUI.do_effects or GUI.do_photo or GUI.do_colour )
end

function PhotoMode_GetValue( i )
	local selection = UIButtons.GetSelection( GUI.options[ i ].slider_id )
	return ( GUI.options[ i ].min + ( ( GUI.options[ i ].max - GUI.options[ i ].min ) / ( GUI.options[ i ].num - 1 ) ) * selection )
end

function PhotoMode_UpdateFilters()
	-- update values
	GUI.values.brightness			= PhotoMode_GetValue( 1 )
	GUI.values.contrast				= PhotoMode_GetValue( 2 )
	GUI.values.saturation			= PhotoMode_GetValue( 3 )	
	GUI.values.shutter_angle		= PhotoMode_GetValue( 4 )
	GUI.values.max_shutter_angle	= GUI.options[ 4 ].max		-- meh
	GUI.values.aperture				= PhotoMode_GetValue( 5 )	
	GUI.values.hint_amount			= PhotoMode_GetValue( 6 )
	
	Amax.SetPhotoFilters( GUI.values, SCUI.name_to_id[ "colour_picker" ] )
end

-- this is called when we click the autofocus button
function PhotoMode_OnAutoFocus()
	local GUI = ContextTable[ UIEnums.Context.Main ].GUI

	local slider_id = GUI.options[ 5 ].slider_id		-- aperture
	local current = UIButtons.GetSelection( slider_id )

	-- if it's zero, raise it so we can see what we've focused on
	if current == 0 then
		UIButtons.SetSelection( slider_id, 3 )
	end
end
