-- [Blur2PC] Retail Blur PC graphics options screen, ported from the bytecode embedded in Blur.exe 1.2
-- (Screens\Shared\Options_Graphics.lua, decompiled with unluac). The screen layout (OptionsList and the
-- SDR_* sliders) comes from the retail UI data. Differences from retail:
--   * no mouse handling: retail also selects rows with the mouse (getMouseButton, UIButtons.GetParent,
--     UIButtons.SetCurrentItemByID), which this source's UI does not have;
--   * Rebuilt adds a saved render FPS limit and explicit display-mode labels (T274).
-- See docs/analysis.md "Graphics options (PC)".

GUI =
{
	finished = false,
	carousel_branch = "Options",
	OutputSelection = -1,

	GetSelections = function()
		UIGlobals.OptionsTable.resolution		= UIButtons.GetSelection( GUI.ResolutionId )
		UIGlobals.OptionsTable.fullscreen		= UIButtons.GetSelection( GUI.FullScreenId )
		UIGlobals.OptionsTable.aa				= UIButtons.GetSelection( GUI.AAId )
		UIGlobals.OptionsTable.vsync			= UIButtons.GetSelection( GUI.VSyncId )
		UIGlobals.OptionsTable.graphics_quality	= UIButtons.GetSelection( GUI.GraphicsQualityId )
		UIGlobals.OptionsTable.max_fps = UIButtons.GetSelection( GUI.FrameLimitId )
	end,

	CanExit = function( _GUI )
		return false
	end,

	OptionPresets = {},
	PrevPreset = 0,
}

function Init()
	AddSCUI_Elements()

	GUI.MessageListId		= SCUI.name_to_id.OptionsList
	GUI.GraphicsQualityId	= SCUI.name_to_id.SDR_GraphicsQuality
	GUI.ResolutionId		= SCUI.name_to_id.SDR_Resolution
	GUI.FullScreenId		= SCUI.name_to_id.SDR_FullScreen
	GUI.AAId				= SCUI.name_to_id.SDR_AA
	GUI.VSyncId				= SCUI.name_to_id.SDR_VSync
	-- [Blur2PC] T274: retain the owning screen for popup callbacks in either UI context.
	UIGlobals.GraphicsOptionsGUI = GUI

	-- graphical detail 1-4
	for i = 1, 4 do
		UIButtons.AddItem( GUI.GraphicsQualityId, i, i, false )
	end

	UIButtons.AddItem( GUI.FullScreenId, 0, "GAME_OPTION_WINDOWED", true )
	UIButtons.AddItem( GUI.FullScreenId, 1, "GAME_OPTION_FULLSCREEN", true )

	UIButtons.AddItem( GUI.VSyncId, 0, UIText.CMN_OFF, false )
	UIButtons.AddItem( GUI.VSyncId, 1, UIText.CMN_ON, false )

	-- resolution 0 is the "fit the window" entry and is not offered
	local resolution_count = Amax.GraphicResolutionCount()
	for i = 2, resolution_count do
		UIButtons.AddItem( GUI.ResolutionId, i - 1, "GAME_OPTION_RESOLUTION" .. ( i - 1 ), true )
	end

	local aa_count = Amax.GraphicAACount()
	for i = 1, aa_count do
		UIButtons.AddItem( GUI.AAId, i - 1, "GAME_OPTION_AATYPE" .. ( i - 1 ), true )
	end

	UIButtons.SetSelection( GUI.GraphicsQualityId,	UIGlobals.OptionsTable.graphics_quality )
	UIButtons.SetSelection( GUI.ResolutionId,		UIGlobals.OptionsTable.resolution )
	UIButtons.SetSelection( GUI.FullScreenId,		UIGlobals.OptionsTable.fullscreen )
	UIButtons.SetSelection( GUI.VSyncId,			UIGlobals.OptionsTable.vsync )
	UIButtons.SetSelection( GUI.AAId,				UIGlobals.OptionsTable.aa )

	GUI.HelpTextId, GUI.HelpNodeId = SetupBottomHelpBar( nil )
	UIButtons.SetActive( GUI.HelpNodeId, false )

	GUI.PrevPreset = UIButtons.GetSelectionIndex( GUI.GraphicsQualityId )

	if UIGlobals.SaveGraphicOptions == true then
		OptionsGraphics_BackupSettings()
	end
	UIGlobals.SaveGraphicOptions = nil

	StoreInfoLine()
	UpdateInfoLine()
end

function PostInit()
	-- Reuse the retail slider/node template, with a distinct list value.
	UIButtons.NodeListScan( GUI.MessageListId )
	for i = 0, 6 do
		UIButtons.SetNodeSortKey( GUI.MessageListId, i, i * 2 )
	end
	GUI.FrameLimitNode = UIButtons.CloneXtGadgetByName( "Shared\\Options_Graphics.lua", "Nde_VSync" )
	GUI.FrameLimitId = UIButtons.FindChildByName( GUI.FrameLimitNode, "SDR_VSync" )
	UIButtons.SetActive( GUI.FrameLimitNode, true, true )
	UIButtons.AddListItem( GUI.MessageListId, GUI.FrameLimitNode, 7 )
	UIButtons.SetNodeSortKey( GUI.MessageListId, 7, 9, true )
	UIButtons.NodeListSort( GUI.MessageListId )
	Amax.ChangeText( GUI.FrameLimitId, "GAME_OPTION_MAX_FPS" )
	Amax.ChangeText( GUI.FullScreenId, "GAME_OPTION_DISPLAY_MODE" )
	-- Fit the extra row above the existing bottom help bar at every UI resolution.
	local sx, sy, sz = UIButtons.GetScale( GUI.MessageListId )
	local x, y, z = UIButtons.GetPosition( GUI.MessageListId )
	UIButtons.ChangeScale( GUI.MessageListId, sx * 0.8, sy * 0.8, sz )
	UIButtons.ChangePosition( GUI.MessageListId, x, y + 2, z )

	local frame_limits = { 0, 30, 60, 90, 120, 144 }
	local current_limit = UIGlobals.OptionsTable.max_fps or 0
	local found = false
	for _, limit in ipairs( frame_limits ) do
		if limit == 0 then
			UIButtons.AddItem( GUI.FrameLimitId, limit, "GAME_OPTION_FPS_UNLIMITED", true )
		else
			-- AddItem copies the stored text into the slider's own list.
			UIButtons.StoreString( 0, tostring( limit ) )
			UIButtons.AddItem( GUI.FrameLimitId, limit, 0, true )
		end
		found = found or current_limit == limit
	end
	if not found then
		UIButtons.StoreString( 0, tostring( current_limit ) )
		UIButtons.AddItem( GUI.FrameLimitId, current_limit, 0, true )
	end

	UIButtons.SetSelection( GUI.FrameLimitId, current_limit )
	local s = GetScreenOtions().selection
	if IsNumber( s ) == true then
		UIButtons.SetSelectionByIndex( GUI.MessageListId, s )
	else
		UIButtons.SetSelectionByIndex( GUI.MessageListId, 0 )
	end
end

function StartLoop( time )
	--
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end

	if message == UIEnums.Message.MenuBack and arg2 == true then
		PlaySfxBack()
		OptionsGraphics_RevertSettings()
		GoScreen( "Shared\\Options.lua" )
	elseif message == UIEnums.Message.MenuNext and arg2 == true then
		PlaySfxNext()
		local selected = UIButtons.GetSelection( GUI.MessageListId )
		print( "selected " .. selected )
		if selected == 4 then
			GoScreen( "Shared\\Options_ImageAdjust.lua" )
		elseif selected == 5 then
			GUI.do_update_options = true
		end
	end
end

function FrameUpdate( time )
	local preset = UIButtons.GetSelectionIndex( GUI.GraphicsQualityId )
	if GUI.PrevPreset ~= preset then
		GUI.PrevPreset = preset
	end

	if GUI.do_update_options == true then
		GUI.GetSelections()
		Amax.UpdateGraphicsOptions( UIGlobals.OptionsTable )
		ChangeResolution( Amax.GetScreenResolution( UIGlobals.OptionsTable.resolution ) )
		GUI.do_update_options = false
		SetupCustomPopup( UIEnums.CustomPopups.ConfirmVideoSettings )
	end

	UpdateInfoLine()
end

function Render()
	--
end

function EndLoop( time )
	--
end

function End()
	GetScreenOtions().selection = UIButtons.GetSelectionIndex( GUI.MessageListId )
	GUI.GetSelections()
	RestoreInfoLine()
end

-- Image adjust and apply take A; the sliders only B
function UpdateInfoLine()
	local selected = UIButtons.GetSelection( GUI.MessageListId )
	if selected ~= GUI.selected_option then
		GUI.selected_option = selected
		if selected == 4 or selected == 5 then
			SetupInfoLine( UIText.INFO_B_BACK, UIText.INFO_A_SELECT )
		else
			SetupInfoLine( UIText.INFO_B_BACK )
		end
		local help = nil
		if selected == 7 then
			help = "GAME_OPTION_FPS_HELP"
		elseif UIButtons.GetSelectionIndex( GUI.MessageListId ) == 4 then
			help = "GAME_OPTION_VSYNC_HELP"
		end
		UIButtons.SetActive( GUI.HelpNodeId, help ~= nil )
		if help then UIButtons.ChangeText( GUI.HelpTextId, help ) end
	end
end

-- The settings in force when the screen opened (or last confirmed)
function OptionsGraphics_BackupSettings()
	UIGlobals.RevertOptionsTable = {}
	UIGlobals.RevertOptionsTable.max_fps = UIGlobals.OptionsTable.max_fps
	UIGlobals.RevertOptionsTable.graphics_quality	= UIGlobals.OptionsTable.graphics_quality
	UIGlobals.RevertOptionsTable.resolution			= UIGlobals.OptionsTable.resolution
	UIGlobals.RevertOptionsTable.fullscreen			= UIGlobals.OptionsTable.fullscreen
	UIGlobals.RevertOptionsTable.vsync				= UIGlobals.OptionsTable.vsync
	UIGlobals.RevertOptionsTable.aa					= UIGlobals.OptionsTable.aa
end

-- Puts the backed-up settings back on the sliders, in the options table and on the display
function OptionsGraphics_RevertSettings()
	local _GUI = UIGlobals.GraphicsOptionsGUI
	UIButtons.SetSelection( _GUI.FrameLimitId, UIGlobals.RevertOptionsTable.max_fps )
	UIGlobals.OptionsTable.max_fps = UIGlobals.RevertOptionsTable.max_fps

	UIButtons.SetSelection( _GUI.GraphicsQualityId,	UIGlobals.RevertOptionsTable.graphics_quality )
	UIButtons.SetSelection( _GUI.ResolutionId,		UIGlobals.RevertOptionsTable.resolution )
	UIButtons.SetSelection( _GUI.FullScreenId,		UIGlobals.RevertOptionsTable.fullscreen )
	UIButtons.SetSelection( _GUI.VSyncId,			UIGlobals.RevertOptionsTable.vsync )
	UIButtons.SetSelection( _GUI.AAId,				UIGlobals.RevertOptionsTable.aa )

	UIGlobals.OptionsTable.graphics_quality	= UIGlobals.RevertOptionsTable.graphics_quality
	UIGlobals.OptionsTable.resolution		= UIGlobals.RevertOptionsTable.resolution
	UIGlobals.OptionsTable.fullscreen		= UIGlobals.RevertOptionsTable.fullscreen
	UIGlobals.OptionsTable.vsync			= UIGlobals.RevertOptionsTable.vsync
	UIGlobals.OptionsTable.aa				= UIGlobals.RevertOptionsTable.aa

	Amax.UpdateGraphicsOptions( UIGlobals.RevertOptionsTable )
	ChangeResolution( Amax.GetScreenResolution( UIGlobals.RevertOptionsTable.resolution ) )
end
