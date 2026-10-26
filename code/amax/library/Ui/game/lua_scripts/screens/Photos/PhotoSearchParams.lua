GUI =
{
	finished = false,	

	stringNo = 0,

	menu_active = 0,

	CanExit = function( _GUI )
		return false
	end,
	
	node_list_id_location = -1,
	node_list_id_vehicles = -1,
}

function Init()
	AddSCUI_Elements()
	
	StoreInfoLine()
	SetupInfoLine( UIText.INFO_PHOTO_SEARCH_A, UIText.INFO_B_BACK )
	
	GUI.node_list_id_location = SCUI.name_to_id[ "node_list_location" ]
	GUI.node_list_id_vehicles = SCUI.name_to_id[ "node_list_vehicles" ]
	
	ActivateMenus()
end

function PostInit()

	LSP.Enable(true)
	LSP.SetUserIndex(Profile.GetPrimaryPad())

	LSP.ClearTags()
	LSP.AddTag(0,1)	--the photo tag
	
	-- locations
	local city_table = GameData.GetCities()
	
	PhotoSearchAddItem_Location( -1, UIText.CMN_ANY, "style_mixed", "common_icons" )
	
	for i, v in ipairs( city_table ) do
		PhotoSearchAddItem_Location( v.id, v.name, v.icon_name, "location_icons" )
	end
	
	-- vehicles
	local vehicle_table = GameData.GetVehicles()
	
	PhotoSearchAddItem_Vehicle( -1, UIText.CMN_ANY, "style_mixed", "common_icons" )
	
	for i, v in ipairs( vehicle_table ) do
		PhotoSearchAddItem_Vehicle( v.id, v.name, v.icon_name, "car_icons" )
	end
	
	SetupScreenTitle( UIText.RBU_PHOTO_SEARCH_PAGE_TITLE, SCUI.name_to_id["screen"], "search_online", "fe_icons"  )
	
end

function StartLoop( time )
	--
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if GUI.finished == true then
		return
	end

	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end
	
	if message == UIEnums.Message.MenuBack and arg2 == true then 
		PlaySfxBack()
		GoScreen("Photos\\Photos.lua" )

	elseif message == UIEnums.Message.MenuNext and arg2 == true then 
		
		local location = UIButtons.GetSelection( GUI.node_list_id_location )
		local vehicle = UIButtons.GetSelection( GUI.node_list_id_vehicles )

		if location ~= -1 then
			LSP.AddTag(1, location)
		end

		if vehicle ~= -1 then
			LSP.AddTag(2, vehicle)
		end

		GoScreen("Photos\\PhotoSearch.lua" )

	elseif message == UIEnums.Message.ButtonDown and arg2 == true then

		GUI.menu_active = GUI.menu_active + 1

		if GUI.menu_active == 2 then
			GUI.menu_active = 0
		end

		ActivateMenus()

	elseif message == UIEnums.Message.ButtonUp and arg2 == true then

		GUI.menu_active = GUI.menu_active - 1

		if GUI.menu_active == -1 then
			GUI.menu_active = 1
		end

		ActivateMenus()

	end
end

function FrameUpdate( time )
	--
end

function Render()
	--
end

function EnterEnd()
	RestoreInfoLine()
end

function EndLoop( time )
	--
end

function End()
	--
end

function ActivateMenus()
	UIButtons.SetSelected( GUI.node_list_id_location, false )
	UIButtons.SetSelected( GUI.node_list_id_vehicles, false )

	if GUI.menu_active == 0 then
		UIButtons.SetSelected( GUI.node_list_id_location, true )
	elseif GUI.menu_active == 1 then
		UIButtons.SetSelected( GUI.node_list_id_vehicles, true )
	end
end

function PhotoSearchAddItem_Location( value, text, icon_name, icon_sheet )
	local node_id = UIButtons.CloneXtGadgetByName( "Photos\\PhotoSearchParams.lua", "_node_location" )
	UIButtons.AddListItem( GUI.node_list_id_location, node_id, value )
	
	local text_id = UIButtons.FindChildByName( node_id, "_text_location" )
	UIButtons.ChangeText( text_id, text )
	
	local icon_id = UIButtons.FindChildByName( node_id, "_icon_location" )
	UIShape.ChangeSceneName( icon_id, icon_sheet )
	UIShape.ChangeObjectName( icon_id, icon_name )
end

function PhotoSearchAddItem_Vehicle( value, text, icon_name, icon_sheet )
	local node_id = UIButtons.CloneXtGadgetByName( "Photos\\PhotoSearchParams.lua", "_node_vehicle" )
	UIButtons.AddListItem( GUI.node_list_id_vehicles, node_id, value )
	
	local text_id = UIButtons.FindChildByName( node_id, "_text_vehicle" )
	UIButtons.ChangeText( text_id, text )
	
	local icon_id = UIButtons.FindChildByName( node_id, "_icon_vehicle" )
	UIShape.ChangeSceneName( icon_id, icon_sheet )
	UIShape.ChangeObjectName( icon_id, icon_name )
end
