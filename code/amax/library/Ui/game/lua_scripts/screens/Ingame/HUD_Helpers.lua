-- hud helper functions
HUD =
{
	ViewportId = 
	{
		[ 1 ] = -1,
		[ 2 ] = -1,
		[ 3 ] = -1,
		[ 4 ] = -1,
	},
	
	ViewportId3d = 
	{
		[ 1 ] = -1,
		[ 2 ] = -1,
		[ 3 ] = -1,
		[ 4 ] = -1,
	},
	
	--[[
	ConvertXtTable = function( name )
		local table = SCUI.elements[ SCUI.name_to_index[ name ] ]
		
		if table == nil then
			print( "ERROR: ConvertXtTable() - can't find the Xt table! - if it crashes please try updating your Sorcery" )
			table = {  }
		else
			-- resolve font heights
			ResolveFontHeight( table )
			
			-- auto-attach to viewport if we don't have a parent
			if table.parent == nil then
				--print( "auto_attach to viewport", table.name )
				table._parent_attach = table.parent_attach
				table.parent_attach = nil
			end
		end
		
		return table
	end,
	]]--
	
	--[[
	ManualCentreToViewport = function( i, v )
		v.no_parent = true
				
		local sl, sr, st, sb = GetScreenBounds( GUI.num_players, i, false )
		
		v.pos.x = sl + ( sr - sl ) / 2
		v.pos.y = st + ( sb - st ) / 2
	end,
	]]--
	
	--[[	
	ManualViewportScale = function( v )
		local screen_scale = HUD.GetScreenScale()
		
		v.size.x = v.size.x * screen_scale
		v.size.y = v.size.y * screen_scale
	end,
	]]--
	
	GetScreenScale = function()
		local viewport_scale = 1
		
		if GUI.num_players == 2 then --and GUI.vertical_split == true then
			viewport_scale = 0.8 --0.75
		elseif GUI.num_players > 1 then
			viewport_scale = 0.7
		end
		
		return viewport_scale
	end,
	
	-- build parents for the player's viewports
	SetupPlayerViewports = function()
		--
		local viewport =
		{
			type = UIEnums.ButtonTypes.DUMMY,
			pos = { x = -1, y = -1 },
			size = { x = -1, y = -1, },
			scale = { x = 1, y = 1, },
			justify = UIEnums.Justify.TopLeft,
			colour_style = "!32 128 128 128",
			textures = 
			{
				{
					name = UITexture.Textures.null,
					pos = { u = 0, v = 0, },
					size = { u = 0, v = 0, },
				},
			},
		}
		
		local viewport_3d =
		{
			type = UIEnums.ButtonTypes.DUMMY,
			pos = { x = -1, y = -1 },
			size = { x = -1, y = -1, },
			scale = { x = 1, y = 1, },
			justify = UIEnums.Justify.TopLeft,
			colour_style = "!64 255 128 128",
			textures = 
			{
				{
					name = UITexture.Textures.null,
					pos = { u = 0, v = 0, },
					size = { u = 0, v = 0, },
				},
			},
			panel = 7,
		}
		
		-- viewport scale
		local viewport_scale = HUD.GetScreenScale()
		
		-- function to scale viewports
		local ScaleViewport = function( table, viewport_scale )
			local inverse_scale = 1 / viewport_scale
			table.size.x = table.size.x * inverse_scale
			table.size.y = table.size.y * inverse_scale
			table.scale.x = viewport_scale
			table.scale.y = viewport_scale
		end
		
		-- add them
		for i = 1, GUI.num_players, 1 do
			local sl, sr, st, sb = GetScreenBounds( GUI.num_players, i )
			viewport.pos.x = sl + ( sr - sl ) / 2
			viewport.pos.y = st + ( sb - st ) / 2
			viewport.size.x = sr - sl
			viewport.size.y = sb - st
			viewport_3d.pos.x =  ( sl + ( sr - sl ) / 2 - 320 ) / 10
			viewport_3d.pos.y = -( st + ( sb - st ) / 2 - 240 ) / 10
			viewport_3d.size.x = ( sr - sl ) / 10
			viewport_3d.size.y = ( sb - st ) / 10
			
			local scale_x = Select( IsSplitScreen(), 1, 1 )
			local scale_y = Select( IsSplitScreen(), 0.975, 1 )
			
			-- spectate scale in y
			if UIScreen.GetCurrentScreen() == "multiplayer\\ingame\\mpspectate.lua" then
				scale_y = 0.875
			end
			
			viewport.size.x = viewport.size.x * scale_x
			viewport.size.y = viewport.size.y * scale_y
			viewport_3d.size.x = viewport_3d.size.x * scale_x
			viewport_3d.size.y = viewport_3d.size.y * scale_y
			
			ScaleViewport( viewport, viewport_scale )
			ScaleViewport( viewport_3d, viewport_scale )
			viewport.justify = UIEnums.Justify.MiddleCentre
			viewport_3d.justify = UIEnums.Justify.MiddleCentre
			HUD.ViewportId[ i ] = UIButtons.AddButton( viewport )
			HUD.ViewportId3d[ i ] = UIButtons.AddButton( viewport_3d )
		end
	end,
	
	--[[
	-- add panel
	AddPanel = function( panel )
		-- add for each player
		for i = 1, GUI.num_players, 1 do
			-- the player's item table
			if GUI.item_table[ i ] == nil then
				GUI.item_table[ i ] = {  }
			end
		
			-- add each item
			for j = 1, #panel, 1 do
				local item = panel[ j ]
				
				if item ~= nil then
					local v = CloneTable( item.table )
					
					if v ~= nil then						
						-- callback function
						if item.build_func ~= nil then
							item.build_func( i, v )
						end
						
						-- set player index
						v.player = i
						
						--show_table( v, 1 )
						
						-- add the item
						GUI.item_table[ i ][ item.hud_id ] = {  }
						local player_item = GUI.item_table[ i ][ item.hud_id ]
						
						player_item.id = UIButtons.AddButton( v )
						
						-- attach to viewport?
						if v.no_parent ~= true then
							if v._parent_attach == nil then
								v._parent_attach = UIEnums.Justify.TopLeft
							end
							-- attach
							UIButtons.SetParent( player_item.id, HUD.ViewportId[ i ], v._parent_attach )
						end
					else
						print( "item.table is nil - skipping" )
					end
				else
					print( "item is nil - skipping" )
				end
			end	
		end
	end,
	]]--
}
