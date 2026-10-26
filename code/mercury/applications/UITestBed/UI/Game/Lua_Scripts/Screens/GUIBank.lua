-- screen storage
function GetCurrentScreen( subscreen_only )
	if subscreen_only == true then
		if GUI ~= nil then
			if GUI.current_subscreen ~= nil then
				return GUI.current_subscreen.xt_record
			else
				return ""
			end
		else
			return ""
		end
	end
	
	if GUI.current_subscreen ~= nil then
		return GUI.current_subscreen.xt_record
	end
	
	return UIScreen.GetCurrentScreen()
end

function StoreScreen( index, name )
	if name == nil then
		name = GetCurrentScreen()
	end

	if IsNumber( index ) == false or IsString( name ) == false then
		print( "Invalid params : StoreScreen( number, string ) : ", type( index ), type( name ) )
		return
	end

	UIGlobals.ScreenStore[ index ] = name
end

function GetStoredScreen( index )
	if IsNumber( index ) == false then
		print( "Invalid params : GetStoredScreen( number, string ) : ", type( index ) )
		return "INVALID_SCREEN"
	end
	
	local screen = UIGlobals.ScreenStore[ index ]
	
	if IsString( screen ) == false then
		print( "Invalid index", index )
		return "INVALID_SCREEN"
	end
	
	return screen
end

ScreenTabSize = SCREEN_WIDTH / 16
ScreenRowStart = 28
ScreenVTabSize = 48
ScreenEdgeSize = SCREEN_WIDTH / 13
ScreenVEdgeSize = 44

GUIBank =
{
	ScreenLeftEdge = SCREEN_LEFT + ScreenEdgeSize,
	ScreenRightEdge = SCREEN_RIGHT - ScreenEdgeSize,
	ScreenColumn1 = SCREEN_LEFT + ScreenEdgeSize + (ScreenTabSize*1),
	ScreenColumn2 = SCREEN_LEFT + ScreenEdgeSize + (ScreenTabSize*2),
	ScreenColumn3 = SCREEN_LEFT + ScreenEdgeSize + (ScreenTabSize*3),
	ScreenColumn4 = SCREEN_LEFT + ScreenEdgeSize + (ScreenTabSize*4),
	ScreenColumnR1 = SCREEN_RIGHT - ScreenEdgeSize - (ScreenTabSize*1),
	ScreenColumnR2 = SCREEN_RIGHT - ScreenEdgeSize - (ScreenTabSize*2),
	ScreenColumnR3 = SCREEN_RIGHT - ScreenEdgeSize - (ScreenTabSize*3),
	ScreenColumnR4 = SCREEN_RIGHT - ScreenEdgeSize - (ScreenTabSize*4),
	ScreenTopEdge = ScreenRowStart,
	ScreenBottomEdge = 480 - ScreenVEdgeSize,
	ScreenRow1 = ScreenRowStart+(ScreenVTabSize*0),
	ScreenRow2 = ScreenRowStart+(ScreenVTabSize*1),
	ScreenRow3 = ScreenRowStart+(ScreenVTabSize*2),
	ScreenRow4 = ScreenRowStart+(ScreenVTabSize*3),
	
	--
}

function ScreenInit()
	--print( "ScreenInit()" )
	
	-- display profile states (debug)
	local text =
	{
		layer = UIEnums.Layer.TOP,
		type = UIEnums.ButtonTypes.TEXT,
		pos = { x = GUIBank.ScreenLeftEdge, y = GUIBank.ScreenTopEdge },
		size = { x = 0, y = 20 },
		justify = UIEnums.Justify.TopLeft,
		text_string = "",
	}
	
	-- profile names
	for i = 0, 3, 1 do
		text.text_string = "PROFILE_PAD" .. i .. "_NAME"
		--UIButtons.AddButton( text )
		text.pos.y = text.pos.y + text.size.y
	end
	
	text.pos.y = GUIBank.ScreenTopEdge
	text.pos.x = GUIBank.ScreenLeftEdge + 150
	
	-- profile status
	for i = 0, 3, 1 do
		text.text_string = "PROFILE_PAD" .. i .. "_STATUS"
		--UIButtons.AddButton( text )
		text.pos.y = text.pos.y + text.size.y
	end	
	
	-- call the normal screen Init
	Init()
end

-- can we save with the given profile?
function CanSave( profileIndex )	
	-- if people have no device/corrupt profiles
	if UIGlobals.DoNotSave[ profileIndex ] == true then
		return false
	end
	
	-- only save out "proper" profiles
	if UIGlobals.ProfileTable[ profileIndex ] ~= UIEnums.Profile.GamerProfile then
		return false
	end

	-- set the save index
	UIGlobals.ProfileSaveIndex = profileIndex
	
	return true
end

-- Helpers
function IsString( v )
	return type( v ) == "string"
end

function IsFunction( v )
	return type( v ) == "function"
end

function IsTable( v )
	return type( v ) == "table"
end

function IsNumber( v )
	return type( v ) == "number"
end

-- Run a string as a function ... carefully
function ProtectedLoadRunString( string )
	if pcall( loadstring( string ) ) == false then
		-- Show error
		print( "ProtectedLoadRunString : error", pcall( loadstring( string ) ) )
	else
		local func = loadstring( string )
		if IsFunction( func ) then
			func()			
			return true
		end
	end
	
	return false
end

function AddSCUI_Elements( overide )
	--print( "AddSCUI_Elements()", table_name )
	
	if table_name == nil then
		table_name = "SCUI"
	end
	
	-- get the global table
	local table = _G[ table_name ]
	
	if table == nil then
		-- create it
		_G[ table_name ] = {  }
		table = _G[ table_name ]
		
		--print( "ERROR: AddSCUI_Elements() - _G[ table_name ] is nil", table_name )
		--return
	end
	
	discard_prefix = GUI.SCUI_discard_prefix
	
	if discard_prefix == nil then
		discard_prefix = "_"
	end
	
	table.name_to_id = {  }
	table.name_to_index = {  }
	
	if IsTable( table ) and IsTable( table.elements ) then
		for i, v in pairs( table.elements ) do
			local add = true
			local has_name = false
			
			if IsString( v.name ) and string.len( v.name ) ~= 0 then
				has_name = true
			end
			
			if has_name == true then
				-- check prefix and don't add if they match
				if discard_prefix ~= nil then
					if string.sub( v.name, 1, string.len( discard_prefix ) ) == discard_prefix then
						add = false
					end
				end
			end
			
			-- add button
			if add == true then
				v.ID = UIButtons.AddButton( v )
				
				if has_name == true then
					table.name_to_id[ v.name ] = v.ID
				end
			end
			
			-- name to index table
			if has_name == true then
				table.name_to_index[ v.name ] = i
			end
		end
	end
end

function GoScreen( next )
	if GUI == false or next == nil then
		return
	end
	
	if GUI.finished == true then
		return
	end
	
	UIScreen.SetNextScreen( next )
	
	GUI.finished = true
end

function TimeLineActive( name, active )
	UIButtons.TimeLineActive( name, active )
end

function SCUI_MessageUpdate( message )
	if IsTable( SCUI ) == false then
		return false
	end
	
	if GUI.finished == true then
		return false
	end

	if message == UIEnums.Message.MenuNext and IsString( SCUI.on_next ) then
		return ProtectedLoadRunString( SCUI.on_next )
	elseif message == UIEnums.Message.MenuBack and IsString( SCUI.on_back ) then
		return ProtectedLoadRunString( SCUI.on_back )
	end
	
	return false
end