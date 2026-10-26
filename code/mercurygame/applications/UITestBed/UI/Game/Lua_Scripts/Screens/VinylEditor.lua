GUI =
{
	MODE_INVALID			=-1,
	MODE_CHOOSE_STRIP_SLOT	= 0,
	MODE_PICK_SHAPE			= 1,
	MODE_PICK_COLOUR		= 2,
	MODE_SHAPE_SRT			= 3,
	MODE_GROUP_SRT			= 4,
	MODE_EDIT_SHAPE			= 5,
	MODE_EDIT_GROUP			= 6,
	
	-- Mode helper texts	
	edit_mode_title_text = 
	{
		[ UIEnums.VinylEditMode.Translate ]		= UIText.VNL_CHANGE_POSITION,
		[ UIEnums.VinylEditMode.Rotate ]		= UIText.VNL_CHANGE_ROTATION,
		[ UIEnums.VinylEditMode.Scale ]			= UIText.VNL_CHANGE_SCALE,
		[ UIEnums.VinylEditMode.Shear ]			= UIText.VNL_CHANGE_SHEAR,
		[ UIEnums.VinylEditMode.GradientRot ]	= UIText.VNL_CHANGE_GRADIENT_ROTATION,
		[ UIEnums.VinylEditMode.GradientTrans ]	= UIText.VNL_CHANGE_GRADIENT_POSITION,
		[ UIEnums.VinylEditMode.GradientScale ]	= UIText.VNL_CHANGE_GRADIENT_SCALE,
	},
	edit_mode_help_text = 
	{	
		[ UIEnums.VinylEditMode.Translate ]		= { "TRANS_X",		"TRANS_Y"	},
		[ UIEnums.VinylEditMode.Scale ]			= { "SCALE_Y",		"SCALE_X"	},
		[ UIEnums.VinylEditMode.Rotate ]		= { "ROTATION",		nil			},
		[ UIEnums.VinylEditMode.Shear ]			= { "SKEW_X",		"SKEW_Y"	},
		[ UIEnums.VinylEditMode.GradientTrans ]	= { "GRD_TRANS",	nil	},
		[ UIEnums.VinylEditMode.GradientScale ]	= { "GRD_SCALE",	nil	},
		[ UIEnums.VinylEditMode.GradientRot ]	= { "GRD_ROT",		nil	},
	},
	
	close_timer = 0,
	
	useGradient = false,
	colIndex = 0,
}

function Init()
	AddSCUI_Elements( )

	-- Grab shortcuts to gadgets
	GUI.scene_id			= SCUI.name_to_id[ "VinylScene" ]
	GUI.shape_picker_id		= SCUI.name_to_id[ "Shapes" ]
	GUI.col_picker_id		= SCUI.name_to_id[ "VinylColourPicker" ]
	GUI.grd_picker_id		= SCUI.name_to_id[ "GradientColour" ]
	GUI.strip_id			= SCUI.name_to_id[ "VinylStripBar" ]
	GUI.layer_menu_id		= SCUI.name_to_id[ "MenuEditLayer" ]
	GUI.group_menu_id		= SCUI.name_to_id[ "MenuEditGroup" ]
	GUI.shape_edit_help_id	= SCUI.name_to_id[ "TextEditModeHelp" ]
	GUI.help_title_id		= SCUI.name_to_id[ "HelpTitle" ]
	
	-- Store object to be activated / selected when mode changes
	GUI.active_list = 
	{
		[GUI.MODE_CHOOSE_STRIP_SLOT] = 
		{
--			GUI.strip_id,
		},
		[GUI.MODE_PICK_SHAPE] = 
		{
			GUI.shape_picker_id,
		},
		[GUI.MODE_PICK_COLOUR] = 
		{
			GUI.col_picker_id,
		},
		[GUI.MODE_SHAPE_SRT] = 
		{
			GUI.shape_edit_help_id,
			SCUI.name_to_id[ "TextHelpLine1" ],
			SCUI.name_to_id[ "TextHelpLine2" ],
			SCUI.name_to_id[ "LeftTrigger" ],
			SCUI.name_to_id[ "RightTrigger" ],
		},
		[GUI.MODE_GROUP_SRT] = 
		{
			GUI.shape_edit_help_id,
			SCUI.name_to_id[ "TextHelpLine1" ],
			SCUI.name_to_id[ "TextHelpLine2" ],
			SCUI.name_to_id[ "LeftTrigger" ],
			SCUI.name_to_id[ "RightTrigger" ],
		},
		[GUI.MODE_EDIT_SHAPE] = 
		{
			GUI.layer_menu_id,
		},
		[GUI.MODE_EDIT_GROUP] = 
		{
			GUI.group_menu_id,
		}
	}
	GUI.selected_list = 
	{
		[GUI.MODE_CHOOSE_STRIP_SLOT] = 
		{
			GUI.strip_id,
		},
		[GUI.MODE_PICK_SHAPE] = 
		{
			GUI.shape_picker_id,
		},
		[GUI.MODE_PICK_COLOUR] = 
		{
			GUI.shape_picker_id,
		},
		[GUI.MODE_SHAPE_SRT] = 
		{
			GUI.scene_id,
		},
		[GUI.MODE_GROUP_SRT] = 
		{
			GUI.scene_id,
		},
		[GUI.MODE_EDIT_SHAPE] = 
		{
		},
		[GUI.MODE_EDIT_GROUP] = 
		{
		},
	}
	
	-- Info line for each mode
	GUI.info_line_text =
	{
		[GUI.MODE_CHOOSE_STRIP_SLOT] = 
		{
			UIText.VNL_IL_A_CHOOSE_SHAPE,
			UIText.VNL_IL_B_BACK,
			UIText.VNL_IL_X_CUT,
			UIText.VNL_IL_Y_INSERT,
			UIText.VNL_IL_LB_HIGHLIGHT,
		},
		[GUI.MODE_PICK_SHAPE] = 
		{
			UIText.VNL_IL_A_ACCEPT,
			UIText.VNL_IL_B_BACK,
		},
		[GUI.MODE_PICK_COLOUR] = 
		{
			UIText.VNL_IL_A_ACCEPT,
			UIText.VNL_IL_B_BACK,
			UIText.VNL_IL_Y_GRADIENT_COLOUR,
		},
		[GUI.MODE_SHAPE_SRT] = 
		{
			UIText.VNL_IL_A_ACCEPT,
			UIText.VNL_IL_B_BACK,
			UIText.VNL_IL_X_CHANGE_COLOUR,
			UIText.VNL_IL_Y_STAMP,
		},
		[GUI.MODE_GROUP_SRT] = 
		{
			UIText.VNL_IL_A_ACCEPT,
			UIText.VNL_IL_B_BACK,
		},
		[GUI.MODE_EDIT_SHAPE] = 
		{
			UIText.VNL_IL_A_ACCEPT,
			UIText.VNL_IL_B_BACK,
		},
	}
	-- Info line for each mode
	GUI.timeline_name =
	{
--		[GUI.MODE_CHOOSE_STRIP_SLOT]	= "strip_bar",
		[GUI.MODE_PICK_SHAPE]			= "pick_shape",
		[GUI.MODE_PICK_COLOUR]			= "pick_colour",
		[GUI.MODE_SHAPE_SRT]			= "change_shape_app",
		[GUI.MODE_GROUP_SRT]			= "change_group_app",
		[GUI.MODE_EDIT_SHAPE]			= "edit_shape",
		[GUI.MODE_EDIT_GROUP]			= "edit_shape",
	}
	
	GUI.help_title_text =
	{
		[GUI.MODE_CHOOSE_STRIP_SLOT]	= UIText.VNL_TITLE_CHOOSE_SLOT,
		[GUI.MODE_PICK_SHAPE]			= UIText.VNL_CHANGE_VINYL_SHAPE,
		[GUI.MODE_PICK_COLOUR]			= UIText.VNL_TITLE_CHOOSE_COLOUR,
		[GUI.MODE_SHAPE_SRT]			= UIText.VNL_CHANGE_APPEARANCE,
		[GUI.MODE_GROUP_SRT]			= UIText.VNL_CHANGE_GROUP_APPEARANCE,
		[GUI.MODE_EDIT_SHAPE]			= UIText.VNL_EDIT_SHAPE,
		[GUI.MODE_EDIT_GROUP]			= UIText.VNL_EDIT_GROUP,
	}
	
		
	-- Setup mode
	GUI.mode = GUI.MODE_INVALID
	ChangeMode( GUI.MODE_CHOOSE_STRIP_SLOT )
	
	-- Setup Will's shape picker
	local count = UIShape.NumberOfSceneShapes( "Shapes" )
	for i = 1, count do
		AddShapeItem( GUI.shape_picker_id, "Shapes", i - 1 ) 
	end
	
	VinylEditor.SceneType( UIEnums.VinylSceneType.Group )
	VinylEditor.ClearScene()	
end

function PostInit()
--	VinylEditor.SetColourPicker( GUI.col_picker_id )
	
	if VinylEditor.IsRoot() == true then
		UIButtons.TimeLineActive( "fold_layer", false, 0, true )
	else
		UIButtons.TimeLineActive( "fold_layer", true, 0.5, true )
	end
end

function StartLoop( time )
	return true
end

function ChangeMode( new_mode )
	if new_mode == -1 then
		return
	end
	
	-- Turn off old mode
	if GUI.mode ~= GUI.MODE_INVALID then
		for i,v in ipairs( GUI.active_list[ GUI.mode ] ) do
			UIButtons.SetActive( v, false )
		end
		
		for i,v in ipairs( GUI.selected_list[ GUI.mode ] ) do
			UIButtons.SetSelected( v, false )
		end
	end
	
	UIButtons.SetActive( GUI.grd_picker_id, false )
	
	if IsString( GUI.timeline_name[ GUI.mode ] ) == true then
		UIButtons.TimeLineActive( GUI.timeline_name[ GUI.mode ], false )
	end

	GUI.prev_mode = GUI.mode
	GUI.mode = new_mode
	 
	if IsNumber( GUI.help_title_text[ GUI.mode ] ) == true then
		UIButtons.SetActive( GUI.help_title_id, true )
		UIButtons.ChangeText( GUI.help_title_id, GUI.help_title_text[ GUI.mode ] )
		UIButtons.ResetTextPrint( GUI.help_title_id )
	else
		UIButtons.SetActive( GUI.help_title_id, true )
	end

	if IsString( GUI.timeline_name[ GUI.mode ] ) == true then
		UIButtons.TimeLineActive( GUI.timeline_name[ GUI.mode ], true )
	end
	
	-- Turn on new mode
	if GUI.mode ~= GUI.MODE_INVALID then
		for i,v in ipairs( GUI.active_list[ GUI.mode ] ) do
			UIButtons.SetActive( v, true )
		end
		
		for i,v in ipairs( GUI.selected_list[ GUI.mode ] ) do
			UIButtons.SetSelected( v, true )
		end
		
		-- Change the help text
		if IsTable( GUI.info_line_text[ GUI.mode ] ) == true then
			-- Turn all off
			for i = 1, 5, 1 do
				local id = SCUI.name_to_id[ "InfoLine" .. i ]
				UIButtons.SetActive( id, false )
			end
			-- Turn needed on
			for i,v in pairs( GUI.info_line_text[ GUI.mode ] ) do
				local id = SCUI.name_to_id[ "InfoLine" .. i ]
				UIButtons.SetActive( id, true )
				UIButtons.ChangeText( id, v )
			end
			
			-- Check gradient mode mode
			if GUI.mode == GUI.MODE_PICK_COLOUR then
				ChangeColourInfoLine()
			end
		end		
	end	
end

function ChangeColourInfoLine()
	if GUI.create_item == true then
		UIButtons.SetActive( SCUI.name_to_id[ "InfoLine" .. 3 ], false )
		UIButtons.SetActive( SCUI.name_to_id[ "InfoLine" .. 4 ], false )
	else
		if GUI.useGradient == true then
			UIButtons.SetActive( SCUI.name_to_id[ "InfoLine" .. 4 ], true )
			UIButtons.ChangeText( SCUI.name_to_id[ "InfoLine" .. 3 ], UIText.VNL_IL_Y_FLAT_COLOUR )
			UIButtons.ChangeText( SCUI.name_to_id[ "InfoLine" .. 4 ], UIText.VNL_IL_X_COLOUR_EDGE )
		else
			UIButtons.SetActive( SCUI.name_to_id[ "InfoLine" .. 4 ], false )
			UIButtons.ChangeText( SCUI.name_to_id[ "InfoLine" .. 3 ], UIText.VNL_IL_Y_GRADIENT_COLOUR )
		end
		
		UIButtons.SetActive( GUI.grd_picker_id, GUI.useGradient )
		UpdateGradientAlternateColour()
	end
end

function MessageUpdate( message, arg1, arg2 )
	if SCUI_MessageUpdate( message, arg1, arg2 ) == true then
		return
	end
	
	if message == UIEnums.Message.MenuNext and arg2 == true then
		if GUI.mode == GUI.MODE_CHOOSE_STRIP_SLOT then
			-- Layer selected from strip bar
			if VinylEditor.IsGroupSlot() == true then
				GUI.change_mode = GUI.MODE_EDIT_GROUP
			elseif VinylEditor.IsEmptySlot() == true then
				GUI.create_item = true
				GUI.change_mode = GUI.MODE_PICK_SHAPE
				GUI.useGradient = false
				GUI.colIndex = 0
				UIButtons.TimeLineActive( "strip_bar", false )
			else
				GUI.change_mode = GUI.MODE_EDIT_SHAPE
			end
			UIButtons.SetActive( GUI.grd_picker_id, GUI.useGradient )			
		elseif GUI.mode == GUI.MODE_PICK_SHAPE then
			-- Shape selected from picker
			local selection = UIButtons.GetSelectionIndex( GUI.shape_picker_id )
						
			if GUI.create_item == true then
				GUI.change_mode = GUI.MODE_PICK_COLOUR
				GUI.create_shape = selection
			else
				GUI.change_mode = GUI.MODE_CHOOSE_STRIP_SLOT
				VinylEditor.SelectedSetShape( selection )
				UIButtons.TimeLineActive( "strip_bar", true )
			end
			
		elseif GUI.mode == GUI.MODE_PICK_COLOUR then
			if GUI.create_item == true then
				VinylEditor.InsertNewItem()
				VinylEditor.SelectedSetShape( GUI.create_shape )
				
				VinylEditor.SetEditMode( UIEnums.VinylEditMode.Translate )
				UpdateEditMode( UIEnums.VinylEditMode.Translate )
				GUI.change_mode = GUI.MODE_SHAPE_SRT
				GUI.create_item = false
				
				-- Colour picked
				local c = VinylEditor.PickerGetColour( GUI.col_picker_id )
				local a = VinylEditor.PickerGetAlpha( GUI.col_picker_id )
				VinylEditor.SelectedSetColour( 0, c )
				VinylEditor.SelectedSetColour( 1, c )
				VinylEditor.SelectedSetAlpha( 0, a )
				VinylEditor.SelectedSetAlpha( 1, a )
			else
				GUI.change_mode = GUI.MODE_CHOOSE_STRIP_SLOT
				UIButtons.TimeLineActive( "strip_bar", true )
			end
		elseif GUI.mode == GUI.MODE_SHAPE_SRT then
			-- Finished editing selection
			GUI.change_mode = GUI.MODE_CHOOSE_STRIP_SLOT
			UpdateEditMode( UIEnums.VinylEditMode.None )
			UIButtons.TimeLineActive( "strip_bar", true )
		elseif GUI.mode == GUI.MODE_EDIT_GROUP then
			-- Picked from 'edit group' menu
			local selection = UIButtons.GetSelection( GUI.group_menu_id )
			if selection == 0 then
				VinylEditor.SelectedEnterGroup()
				UIButtons.TimeLineActive( "fold_layer", true, 0, true )
				GUI.change_mode = GUI.MODE_CHOOSE_STRIP_SLOT
			elseif selection == 1 then
				GUI.change_mode = GUI.MODE_GROUP_SRT		
				UIButtons.TimeLineActive( "strip_bar", false )
				
				-- Reset this mode
				VinylEditor.SetEditMode( UIEnums.VinylEditMode.Translate )
				UpdateEditMode( UIEnums.VinylEditMode.Translate )
			elseif selection == 2 then
				VinylEditor.SelectedExplodeGroup()
				GUI.change_mode = GUI.MODE_CHOOSE_STRIP_SLOT
			elseif selection == 2 then
				-- Save group				
			end				
		elseif GUI.mode == GUI.MODE_EDIT_SHAPE then
			-- Picked from 'edit shape' menu
			local selection = UIButtons.GetSelection( GUI.layer_menu_id )
			if selection == 0 then -- Colour
				VinylEditor.PickerSetColour( GUI.col_picker_id, VinylEditor.SelectedGetColour( 0 ) )
				VinylEditor.PickerSetAlpha(  GUI.col_picker_id, VinylEditor.SelectedGetAlpha( 0 ) )
				
				GUI.useGradient = VinylEditor.SelectedGetUseGradient()
				UIButtons.SetActive( GUI.grd_picker_id, GUI.useGradient )
				if GUI.useGradient == false then
					GUI.colIndex = 0
				end
				
				GUI.change_mode = GUI.MODE_PICK_COLOUR
			elseif selection == 1 then -- Pos
				GUI.change_mode = GUI.MODE_SHAPE_SRT
				
				-- Reset this mode
				VinylEditor.SetEditMode( UIEnums.VinylEditMode.Translate )
				UpdateEditMode( UIEnums.VinylEditMode.Translate )
			elseif selection == 2 then -- Shape
				GUI.change_mode = GUI.MODE_PICK_SHAPE
			end	

			UIButtons.TimeLineActive( "strip_bar", false )
		end
		
	elseif message == UIEnums.Message.ButtonX and arg2 == true then
		if GUI.mode == GUI.MODE_CHOOSE_STRIP_SLOT and VinylEditor.IsEmptySlot() == false then
			VinylEditor.Cut()
		elseif GUI.mode == GUI.MODE_PICK_COLOUR and GUI.useGradient == true then
			if GUI.colIndex == 0 then
				GUI.colIndex = 1
			else
				GUI.colIndex = 0
			end

			-- Set picker to new selection
			VinylEditor.PickerSetColour( GUI.col_picker_id, VinylEditor.SelectedGetColour( GUI.colIndex ) )
			VinylEditor.PickerSetAlpha(  GUI.col_picker_id, VinylEditor.SelectedGetAlpha( GUI.colIndex ) )
			
			UpdateGradientAlternateColour()
			
		end
		
	elseif message == UIEnums.Message.ButtonY and arg2 == true then
		if GUI.mode == GUI.MODE_CHOOSE_STRIP_SLOT then
			VinylEditor.Paste()
		elseif GUI.mode == GUI.MODE_SHAPE_SRT then
			VinylEditor.Stamp()
		elseif GUI.mode == GUI.MODE_PICK_COLOUR then
			if GUI.create_item == false then
				if GUI.useGradient == true then
					GUI.useGradient = false
					GUI.colIndex = 0
					
					-- Reset picker
					VinylEditor.PickerSetColour( GUI.col_picker_id, VinylEditor.SelectedGetColour( 0 ) )
					VinylEditor.PickerSetAlpha(  GUI.col_picker_id, VinylEditor.SelectedGetAlpha( 0 ) )
				else
					GUI.useGradient = true
			
					local i = VinylEditor.PickerGetColour( GUI.col_picker_id )
					-- Unify 
					VinylEditor.SelectedSetColour( 0, i )
					VinylEditor.SelectedSetColour( 1, i )
					VinylEditor.SelectedSetAlpha( 1, VinylEditor.SelectedGetAlpha( 0 ) )
				end

				VinylEditor.SelectedSetUseGradient( GUI.useGradient )
				ChangeColourInfoLine()
			end
		end	
	elseif (message == UIEnums.Message.ButtonLeftShoulder or message == UIEnums.Message.ButtonRightShoulder) and arg2 == true then
		if GUI.mode == GUI.MODE_PICK_COLOUR then
			--[[
			-- Get
			local dir = -1
			if message == UIEnums.Message.ButtonRightShoulder then
				dir = 1
			end
			local alpha = VinylEditor.SelectedGetAlpha( GUI.colIndex )
			
			-- Move
			local step = 0.1
			alpha = alpha + (dir * step)
			
			-- Clamp
			if alpha < 0 then
				alpha = 0
			elseif alpha > 1 then
				alpha = 1
			end
			]]--
			
			-- Set
			
		end 		
	elseif message == UIEnums.Message.MenuBack and arg2 == true then
		if GUI.create_item == true then
			if GUI.mode == GUI.MODE_CHOOSE_STRIP_SLOT then
				if VinylEditor.IsRoot() == false then
					UIButtons.TimeLineActive( "fold_layer", false )
					GUI.close_timer = 0.6
				end
			elseif GUI.mode == GUI.MODE_PICK_SHAPE then
				ChangeMode( GUI.MODE_CHOOSE_STRIP_SLOT )
				GUI.create_item = false
				UIButtons.TimeLineActive( "strip_bar", true )
			elseif GUI.mode == GUI.MODE_PICK_SHAPE then
				ChangeMode( GUI.MODE_PICK_COLOUR )
			elseif GUI.mode == GUI.MODE_PICK_COLOUR then
				ChangeMode( GUI.MODE_PICK_SHAPE )
			end
		else
			if GUI.mode == GUI.MODE_CHOOSE_STRIP_SLOT then
				if VinylEditor.IsRoot() == false then
					UIButtons.TimeLineActive( "fold_layer", false )
					GUI.close_timer = 0.6
				end
			else
				ChangeMode( GUI.MODE_CHOOSE_STRIP_SLOT )
				UIButtons.TimeLineActive( "strip_bar", true )
			end
		end
	elseif message == UIEnums.Message.ButtonRightShoulder and GUI.mode == GUI.MODE_CHOOSE_STRIP_SLOT then
	
		if VinylEditor.SceneType() == UIEnums.VinylSceneType.Livery then
			local list = VinylEditor.CurrentList()
				
			-- Change render list to edit
			if list == UIEnums.VinylList.LiveryTop then
				list = UIEnums.VinylList.LiveryLeft
			elseif list == UIEnums.VinylList.LiveryLeft then
				list = UIEnums.VinylList.LiveryRight
			elseif list == UIEnums.VinylList.LiveryRight then
				list = UIEnums.VinylList.LiveryFront
			elseif list == UIEnums.VinylList.LiveryFront then
				list = UIEnums.VinylList.LiveryBack
			elseif list == UIEnums.VinylList.LiveryBack then
				list = UIEnums.VinylList.LiveryTop
			else
				print( "Invalid list", list )
				return
			end
			-- Set
			print( "Setting list mode", list )
			VinylEditor.CurrentList( list )
		end

	elseif message == UIEnums.Message.ButtonLeftTrigger and arg2 == true then
		-- Switch edit mode
		local editMode = VinylEditor.GetEditMode()
		local newMode = editMode
		
		if GUI.mode == GUI.MODE_SHAPE_SRT then
			if editMode == UIEnums.VinylEditMode.Scale then
				newMode = UIEnums.VinylEditMode.Translate
			elseif editMode == UIEnums.VinylEditMode.Rotate then
				newMode = UIEnums.VinylEditMode.Scale
			elseif editMode == UIEnums.VinylEditMode.Shear then
				newMode = UIEnums.VinylEditMode.Rotate
			elseif editMode == UIEnums.VinylEditMode.GradientRot then
				newMode = UIEnums.VinylEditMode.Shear
			elseif editMode == UIEnums.VinylEditMode.GradientTrans then
				newMode = UIEnums.VinylEditMode.GradientRot
			elseif editMode == UIEnums.VinylEditMode.GradientScale then
				newMode = UIEnums.VinylEditMode.GradientTrans
			end
		elseif GUI.mode == GUI.MODE_GROUP_SRT then
			if editMode == UIEnums.VinylEditMode.Scale then
				newMode = UIEnums.VinylEditMode.Translate
			elseif editMode == UIEnums.VinylEditMode.Rotate then
				newMode = UIEnums.VinylEditMode.Scale
			end
		end
				
		UpdateEditMode( newMode )
	elseif message == UIEnums.Message.ButtonRightTrigger and arg2 == true then
		-- Switch edit mode
		local editMode = VinylEditor.GetEditMode()
		local newMode = editMode
		
		if GUI.mode == GUI.MODE_SHAPE_SRT then
			if editMode == UIEnums.VinylEditMode.Translate then
				newMode = UIEnums.VinylEditMode.Scale
			elseif editMode == UIEnums.VinylEditMode.Scale then
				newMode = UIEnums.VinylEditMode.Rotate
			elseif editMode == UIEnums.VinylEditMode.Rotate then
				newMode = UIEnums.VinylEditMode.Shear
			elseif editMode == UIEnums.VinylEditMode.Shear then
				newMode = UIEnums.VinylEditMode.GradientRot
			elseif editMode == UIEnums.VinylEditMode.GradientRot then
				newMode = UIEnums.VinylEditMode.GradientTrans
			elseif editMode == UIEnums.VinylEditMode.GradientTrans then
				newMode = UIEnums.VinylEditMode.GradientScale
			end
		elseif GUI.mode == GUI.MODE_GROUP_SRT then
			if editMode == UIEnums.VinylEditMode.Translate then
				newMode = UIEnums.VinylEditMode.Scale
			elseif editMode == UIEnums.VinylEditMode.Scale then
				newMode = UIEnums.VinylEditMode.Rotate
			end
		end
		
		UpdateEditMode( newMode )
	end	
end

function UpdateEditMode( newMode )
	if newMode == GUI.editMode then
		return
	end

	
	GUI.editMode = newMode

	if GUI.edit_mode_title_text[ newMode ] == nil then
		UIButtons.SetActive( GUI.shape_edit_help_id, false )
	else
		-- Update the helper text
		UIButtons.SetActive( GUI.shape_edit_help_id, true )
		UIButtons.ChangeText( GUI.shape_edit_help_id, GUI.edit_mode_title_text[ newMode ] )
	
		if IsTable( GUI.edit_mode_help_text[ newMode ] ) == true then
			local active = false
			-- Helper lines
			for i = 1, 2, 1 do
				local gadgetNane = "TextHelpLine" .. i
				local string = GUI.edit_mode_help_text[ newMode ][ i ]
				local active = IsString( string )
				if active == true then
					UIButtons.ChangeText( SCUI.name_to_id[ gadgetNane ], "VNL_SELECTED_" .. string )
				end
				UIButtons.SetActive( SCUI.name_to_id[ gadgetNane ], active )
			end
		end
	end
	VinylEditor.SetEditMode( newMode )
end

function UpdateGradientAlternateColour()
	if GUI.useGradient == false then
		return
	end	
	
	local index = 0
	if GUI.colIndex == 0 then
		index = 1
	end
	
	local c = VinylEditor.SelectedGetColour( index )
	local a, r, g, b = VinylEditor.ResolveColourARGB( c, 1 )
	local my_col = "!" .. a .. " " .. r .. " " .. g .. " " .. b
	UIButtons.ChangeColour( GUI.grd_picker_id, my_col )
end


function FrameUpdate( time )
	if GUI.close_timer > 0 then
		GUI.close_timer = GUI.close_timer - time
		if GUI.close_timer <= 0 then
			VinylEditor.SelectedExitGroup()
			UIButtons.TimeLineActive( "fold_layer", true, 0.5, true )
		end
	end

	-- Deffered change mode
	if IsNumber( GUI.change_mode ) == true then
		ChangeMode( GUI.change_mode )
		GUI.change_mode = nil
	end
	
	if GUI.create_item == false then
		if GUI.mode == GUI.MODE_PICK_COLOUR then
			-- Colour picked
			VinylEditor.SelectedSetColour( GUI.colIndex, VinylEditor.PickerGetColour( GUI.col_picker_id ) )
			VinylEditor.SelectedSetAlpha( GUI.colIndex, VinylEditor.PickerGetAlpha( GUI.col_picker_id ) )

		elseif GUI.mode == GUI.MODE_PICK_SHAPE and VinylEditor.IsEmptySlot() == false then
			-- Shape selected from picker
			local selection = UIButtons.GetSelectionIndex( GUI.shape_picker_id )
			VinylEditor.SelectedSetShape( selection )
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
	--
end


function AddShapeItem( listId, filename, index )
	local node_index        = SCUI.name_to_index[ "_ShapeNode" ]
	local backing_index     = SCUI.name_to_index[ "_ShapeBacking" ]
	local backing_index2    = SCUI.name_to_index[ "_ShapeBacking2" ]
	local shape_index       = SCUI.name_to_index[ "_Shape" ]

	if node_index ~= nil and backing_index ~= nil and shape_index ~= nil and backing_index2 ~= nil then
		local node			= SCUI.elements[ node_index ]
		local backing		= SCUI.elements[ backing_index ]
		local backing2		= SCUI.elements[ backing_index2 ]
		local shape			= SCUI.elements[ shape_index ]
		-- modifiy the shape attributes

		shape.scene_name = filename
		shape.object_name = "shape_index_" .. index

		local nodeId		= UIButtons.AddButton( node )
		local backing2Id	= UIButtons.AddButton( backing2 )
		local backingId		= UIButtons.AddButton( backing )
		local shapeId		= UIButtons.AddButton( shape )
		-- sort out the parents.
		UIButtons.SetParent( nodeId, listId, UIEnums.Justify.MiddleCentre )
		UIButtons.SetParent( backingId, nodeId, UIEnums.Justify.MiddleCentre )
		UIButtons.SetParent( backing2Id, nodeId, UIEnums.Justify.MiddleCentre )
		UIButtons.SetParent( shapeId, backingId, UIEnums.Justify.MiddleCentre )
		UIButtons.RenderBranchScan( nodeId, UIEnums.ButtonTypes.NODE ) 
	else
		print( "Invalid sorcery data" )
	end
end
