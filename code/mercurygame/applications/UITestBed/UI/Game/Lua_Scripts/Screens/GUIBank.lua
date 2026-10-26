-- screen sizes
if UIHardware.WideScreen() then
	SCREEN_WIDTH = (640 * (4 / 3)) + 0.5 -- 853.333
	SCREEN_LEFT = ((640 - SCREEN_WIDTH) / 2) - 0.25
	SCREEN_RIGHT = SCREEN_LEFT + SCREEN_WIDTH
else
	SCREEN_LEFT = 0
	SCREEN_WIDTH = 640
	SCREEN_RIGHT = SCREEN_LEFT + SCREEN_WIDTH
end

-- screen sizes
Screen =
{
	-- normal edges
	top		= 0,
	bottom	= 480,
	left	= SCREEN_LEFT,
	right	= SCREEN_RIGHT,
	width	= SCREEN_WIDTH,
	height	= 480,
	-- safe areas
	safe =
	{
		top		= 480 * 0.075,
		bottom	= 480 * 0.925,
		left	= SCREEN_LEFT + SCREEN_WIDTH * 0.075,
		right	= SCREEN_RIGHT - SCREEN_WIDTH * 0.075,
		width	= SCREEN_WIDTH * 0.85,
		height	= 480 * 0.85,
	},
}

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

	-- set the profile index
	UIGlobals.ProfileSaveIndex = profileIndex
	
	return true
end

-- can we enumerate with the given profile
function CanEnumerate( profileIndex )
	-- only save out "proper" profiles
	if UIGlobals.ProfileTable[ profileIndex ] ~= UIEnums.Profile.GamerProfile then
		return false
	end

	-- set the profile index
	UIGlobals.ProfileEnumerateIndex = profileIndex
	
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
	
	-- Overide table?
	local table = SCUI
	
	if IsTable( overide ) then 
		table = overide
	end
	
	-- Safety
	if IsTable( table ) == false or IsTable( table.elements ) == false then
		return
	end
	
	-- Create name_to_table if it doesn't exist
	if IsTable( table.name_to_id ) == false then
		table.name_to_id = { }
	end
	
	-- Create name_to_table if it doesn't exist
	if IsTable( table.name_to_index ) == false then
		table.name_to_index = { }
	end
	
	-- Add elephants		
	for i,v in pairs( table.elements ) do
		local addItem = true
		
		-- resolve font heights
--		ResolveFontHeight( v )
		
		if IsString( v.name ) then
			-- Dont add anything with the specified prefix
			if string.sub( v.name, 1, 1 ) == "_" then
				addItem = false
			end
		end
	
		-- Do we add it?
		if addItem == true then
			v.ID = UIButtons.AddButton( v )
			table.name_to_id[ v.name ] = v.ID
		end
		
		if IsString( v.name ) then
			table.name_to_index[ v.name ] = i
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

function SetupInfoLine( ... )
	SetupInfoLine2( unpack( arg ) )
	
	-- make sure we're active
	for i = 1, arg.n, 1 do
		InfoLineActive( i, true, false )
	end
end

function SetupInfoLine2( ... )		
	-- number of info texts
	local num = 8
	
	local font_height = 20 --UIGlobals.FontSize.VerySmall
	local font_style = "Primary"
	
	if GUI.info_line == nil then
		-- setup default objects
		local info_text =
		{
			layer = UIEnums.Layer.LAYER_3,
			type = UIEnums.ButtonTypes.TEXT,
			pos = { x = Screen.safe.right, y = Screen.safe.bottom },
			size = { x = 0, y = font_height },
			justify = UIEnums.Justify.BottomRight,
			text = UIText.CMN_NOWT,
			styles = { name = font_style, },
			colour = "255 128 128 128",
		}
		
		-- black bar
		local bar =
		{
			layer = UIEnums.Layer.LAYER_3,
			type = UIEnums.ButtonTypes.GRAPHIC,
			pos = { x = 320, y = info_text.pos.y - info_text.size.y - 2, },
			size = { x = Screen.width + 2, y = 100, },
			justify = UIEnums.Justify.TopCentre,
			colour = "192 0 0 0",
			textures =
			{
				{
					name = UITexture.Textures.null,
					pos = { u = 0, v = 0, },
					size = { u = 1, v = 1, },
				},
			},
		}
		
		-- add them
		GUI.info_line = {  }
		
		GUI.info_line.bar_id = UIButtons.AddButton( bar )		
		
		for i = 1, num, 1 do
			GUI.info_line[ i ] = {  }
			GUI.info_line[ i ].id = UIButtons.AddButton( info_text )
			GUI.info_line[ i ].width = 0	-- default
		end
	end
	
	-- all inactive to start
	for i = 1, num, 1 do
		UIButtons.SetActive( GUI.info_line[ i ].id, false )
		GUI.info_line[ i ].active = false
	end
	
	if arg.n > num then
		arg.n = num
	end
	
	-- change text ...
	for i = 1, arg.n, 1 do
		UIButtons.SetActive( GUI.info_line[ i ].id, true )
		
		if arg[ i ] == nil then
			arg[ i ] = UIText.CMN_NOWT
		end
		
		GUI.info_line[ i ].text = arg[ i ]
		UIButtons.ChangeText( GUI.info_line[ i ].id, arg[ i ] )
		-- calculate string length
		GUI.info_line[ i ].width = UISystem.GetStringLength( arg[ i ], font_height, font_style )
		
		-- set active
		GUI.info_line[ i ].active = true
	end
	
	-- ... and update positions
	local accum = 0
	local spacing = font_height / 2 + 2
	
	for i = 1, arg.n, 1 do
		UIButtons.ChangePosition( GUI.info_line[ i ].id, Screen.safe.right - accum, Screen.safe.bottom )
		accum = accum + GUI.info_line[ i ].width + spacing
	end
	
	if arg.n == 0 then --and UIGlobals.IsIngame == true then
		UIButtons.SetActive( GUI.info_line.bar_id, false )
	else
		UIButtons.SetActive( GUI.info_line.bar_id, true )
	end
end

function InfoLineContains( text )
	if GUI.info_line == nil then
		return
	end
	
	for i = 1, #GUI.info_line, 1 do
		if GUI.info_line[ i ].active == true and GUI.info_line[ i ].text == text then
			return true
		end
	end
	
	return false
end

function InfoLineActive( num, active, by_text_id )
	if GUI.info_line == nil then
		return
	end
	
	if by_text_id == false and num > #GUI.info_line then
		return
	end
	
	if by_text_id == nil then
		by_text_id = true
	end
	
	local alpha = 1
	
	if active == false then
		alpha = 0.5
	end
	
	local id = nil
	
	if by_text_id == true then
		-- find in the list
		for i = 1, #GUI.info_line, 1 do
			if GUI.info_line[ i ].text == num then
				id = GUI.info_line[ i ].id
				break
			end
			
			if i == #GUI.info_line then
				--print( "not found", num, #GUI.info_line )
			end
		end
	else
		id = GUI.info_line[ num ].id
	end
	
	-- update
	if id ~= nil then
		UIButtons.SetAlphaIntensity( id, alpha )
	end
end

function StoreInfoLine()
	if GUI.info_line ~= nil then
		-- create the initial state if it doesn't exist
		if GUI.info_line.state == nil then
			GUI.info_line.state = {  }
		end
		-- and a new state table
		GUI.info_line.state[ #GUI.info_line.state + 1 ] = {  }
		local state = GUI.info_line.state[ #GUI.info_line.state ]
		
		for i = 1, #GUI.info_line, 1 do
			if GUI.info_line[ i ].active == true then
				state[ #state + 1 ] = GUI.info_line[ i ].text
			end
		end
		
		return state
	end
end

function RestoreInfoLine()
	if GUI.info_line ~= nil and IsTable( GUI.info_line.state ) and #GUI.info_line.state > 0 then
		SetupInfoLine2( unpack( GUI.info_line.state[ #GUI.info_line.state ] ) )
		-- free up the last state table
		SetNil( GUI.info_line.state[ #GUI.info_line.state ] )
		GUI.info_line.state[ #GUI.info_line.state ] = nil
	end
end

function PushInfoLine( text )
	if GUI.info_line == nil then
		SetupInfoLine( UIText.CMN_NOWT )
	end
	
	local state = StoreInfoLine()
	state[ #state + 1 ] = text
	RestoreInfoLine()
end

function PopInfoLine()
	if GUI.info_line == nil then
		return
	end
	
	local state = StoreInfoLine()
	state[ #state ] = nil
	RestoreInfoLine()
end

function SetupNextAndBack()
	SetupInfoLine( UIText.INF_NEXT_A, UIText.INF_BACK_B )
end

function SetupMenuOptions()
	SetupInfoLine( UIText.INF_NEXT_A, UIText.INF_BACK_B, UIText.INF_SELECT_DP )
end

function SetupNext()
	SetupInfoLine( UIText.INF_NEXT_A )
end

function SetupBack()
	SetupInfoLine( UIText.INF_BACK_B )
end

-- can we save with the given profile?
function CanSave( profileIndex )
	-- if people have no device/corrupt profiles
	if UIGlobals.DoNotSave[ profileIndex ] == true then
		return false
	end
	
	-- only save out "proper" profiles
	if UIGlobals.ProfileState[ profileIndex ] ~= UIEnums.Profile.GamerProfile then
		return false
	end

	-- set the save index
	UIGlobals.ProfileSaveIndex = profileIndex
	
	return true
end
