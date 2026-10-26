
function CustomRace_SetupSpinner( table_id, value, name, icon_name, sheet_name )
	local node_id = UIButtons.CloneXtGadgetByName( "Multiplayer\\Shared\\MpCustomRace.lua", "spinner_node" )
	local spinner_id = UIButtons.FindChildByName( node_id, "spinner" )
	local icon_id = UIButtons.FindChildByName( node_id, "icon" )

	Amax.ChangeText( spinner_id, name )

	if icon_name ~= nil and sheet_name ~= nil then
		UIShape.ChangeSceneName( icon_id, sheet_name )
		UIShape.ChangeObjectName( icon_id, icon_name )
	end
	UIButtons.AddTableRow( table_id, node_id, value )

	return spinner_id
end

function CustomRace_SetupItem( table_id, value, name, icon_name, sheet_name )
	local node_id = UIButtons.CloneXtGadgetByName( "Multiplayer\\Shared\\MpCustomRace.lua", "item_node" )
	local name_id = UIButtons.FindChildByName( node_id, "name" )
	local icon_id = UIButtons.FindChildByName( node_id, "icon" )

	Amax.ChangeText( name_id, name )

	if icon_name ~= nil and sheet_name ~= nil then
		UIShape.ChangeSceneName( icon_id, sheet_name )
		UIShape.ChangeObjectName( icon_id, icon_name )
	end
	UIButtons.AddTableRow( table_id, node_id, value )
end

function CustomRace_SetupToggle( table_id, value, name, icon_name, sheet_name )
	local node_id = UIButtons.CloneXtGadgetByName( "Multiplayer\\Shared\\MpCustomRace.lua", "toggle_node" )
	local toggle_id = UIButtons.FindChildByName( node_id, "toggle" )
	local icon_id = UIButtons.FindChildByName( node_id, "icon" )

	Amax.ChangeText( toggle_id, name )

	if icon_name ~= nil and sheet_name ~= nil then
		UIShape.ChangeSceneName( icon_id, sheet_name )
		UIShape.ChangeObjectName( icon_id, icon_name )
	end
	UIButtons.AddTableRow( table_id, node_id, value )

	return toggle_id
end

function CustomRace_SetupPowerupIcon( spinner_id, colour )
	local icon_id = UIButtons.FindChildByName( spinner_id, "icon" )

	UIButtons.ChangeColour( icon_id, colour )
	UIButtons.ChangeSize( icon_id, 4.5, 4.5, 0 )
end