-- HUD panels the attach to each viewport
HUD_Panels =
{
	-- standard items for each player for every game mode
	AddStandardPanel = function()
		--[[
		local item_main_parent =
		{
			hud_id = GUI.HUD.MAIN_PARENT,
			table = HUD.ConvertXtTable( "__main_parent" ),
			build_func = function( i, v )
				--
			end,
		}
		--
		local item_mirror_parent =
		{
			hud_id = GUI.HUD.MIRROR_PARENT,
			table = HUD.ConvertXtTable( "__mirror_parent" ),
			build_func = function( i, v )
				--
			end,
		}
		--
		local item_messages =
		{
			hud_id = GUI.HUD.MESSAGES,
			table = HUD.ConvertXtTable( "__hud_messages" ),
			build_func = function( i, v )
				--
			end,
		}
		--
		local item_rule_info =
		{
			hud_id = GUI.HUD.RULE_INFO,
			table = HUD.ConvertXtTable( "__rule_info" ),
			build_func = function( i, v )
				--
			end,
		}
		]]--
		
		-- the items in the panel
		local items =
		{
			--item_main_parent,
		}
		
		-- add it
		--HUD.AddPanel( items )
	end,
}

-- extra HUD items that are _not_ attached to viewports
HUD_Items =
{
	--
}
