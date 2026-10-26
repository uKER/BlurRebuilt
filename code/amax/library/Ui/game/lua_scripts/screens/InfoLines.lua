--
function InfoLineSwitchContext( context )
	if context == nil then
		context = UIEnums.Context.Main
	end
	
	if InfoLineContext_Original == nil then
		InfoLineContext_Original = context
	end
	
	if InfoLineContext == InfoLineContext_Original and InfoLines ~= nil then
		InfoLines_Store_Original = InfoLines	-- store
	end
	
	if context == InfoLineContext_Original and InfoLines_Store_Original ~= nil then
		InfoLines = InfoLines_Store_Original	-- restore
	else
		-- default
		InfoLines = 
		{
			locked = false,
			force = false,
			commands = "",
			do_timelines = {  },
			spacing = 10,
		}
	end
	
	InfoLineContext = context
end

function InfoLineFlush()
	InfoLineContext = nil
	InfoLineContext_Original = nil
	InfoLines_Store_Original = nil
	InfoLines = nil
	
	InfoLineSwitchContext()	-- default
end

-- 
InfoLineFlush()	-- default on boot
--

function InfoLineClearCommands()
	InfoLines.commands = ""
end

function InfoLineFrameUpdate()
	if #InfoLines.do_timelines > 0 then
		for i, v in ipairs( InfoLines.do_timelines ) do
			UIButtons.TimeLineActive( v, true, 0 )
		end
		
		InfoLines.do_timelines = {  }	-- clear
	end
end

function InfoLineLock()
	InfoLines.locked = true
end

function InfoLineUnlock( flush )
	InfoLines.locked = false
	
	if flush == true then
		ProtectedLoadRunString( InfoLines.commands )
	end
	
	InfoLineClearCommands()
end

function InfoLineAllow()
	return ( InfoLines.locked ~= true or InfoLines.force == true )
end

function InfoLineCommand( command )
	local len = string.len( command )
	
	-- if we've already got this command on the end, then bail
	if string.find( InfoLines.commands, command, -( len + 1 ), true ) ~= nil then	-- +1 for the semi-colon
		return
	end
	
	InfoLines.commands = InfoLines.commands .. " " .. command .. ";"
end

function InfoLineSafeParam( param )
	if param == nil then
		return "nil"
	end
	
	if IsBoolean( param ) == true then
		return Select( param, "true", "false" )
	end
	
	if IsString( param ) == true then
		return "\"" .. param .. "\""
	end
	
	return param
end

-------------------
-- InfoLineContains
-------------------
function InfoLineContains_F( text )
	InfoLines.force = true
	InfoLineContains( text )
	InfoLines.force = false
end

function InfoLineContains( text )
	InfoLineContainsL( text )
end

function InfoLineContainsL_F( text )
	InfoLines.force = true
	InfoLineContainsL( text )
	InfoLines.force = false
end

function InfoLineContainsL( text )
	return __InfoLineContains( text, true )
end

function InfoLineContainsR_F( text )
	InfoLines.force = true
	InfoLineContainsR( text )
	InfoLines.force = false
end

function InfoLineContainsR( text )
	return __InfoLineContains( text, false )
end

----------------
-- StoreInfoLine
----------------
function StoreInfoLine_F()
	InfoLines.force = true
	StoreInfoLine()
	InfoLines.force = false
end

function StoreInfoLine()
	if InfoLineAllow() == false then
		InfoLineCommand( "StoreInfoLine()" )
		return
	end
		
	local _GUI = ContextTable[ InfoLineContext ].GUI

	if _GUI.info_line == nil then
		return
	end
	
	-- create the initial state if it doesn't exist
	if _GUI.info_line.state == nil then
		_GUI.info_line.state = {  }
	end
	-- and a new state table
	_GUI.info_line.state[ #_GUI.info_line.state + 1 ] = { L = {  }, R = {  }, }
	
	local state  = _GUI.info_line.state[ #_GUI.info_line.state ]
	local stateL = _GUI.info_line.state[ #_GUI.info_line.state ].L
	local stateR = _GUI.info_line.state[ #_GUI.info_line.state ].R
	
	-- left
	if _GUI.info_line.L ~= nil then
		for i = 1, #_GUI.info_line.L, 1 do
			if _GUI.info_line.L[ i ].active == true then
				stateL[ #stateL + 1 ] = _GUI.info_line.L[ i ].text
			end
		end
	end
	-- right
	if _GUI.info_line.R ~= nil then
		for i = 1, #_GUI.info_line.R, 1 do
			if _GUI.info_line.R[ i ].active == true then
				stateR[ #stateR + 1 ] = _GUI.info_line.R[ i ].text
			end
		end
	end
	
	return state
end

------------------
-- RestoreInfoLine
------------------
function RestoreInfoLine_F()
	InfoLines.force = true
	RestoreInfoLine()
	InfoLines.force = false
end

function RestoreInfoLine()
	if InfoLineAllow() == false then
		InfoLineCommand( "RestoreInfoLine()" )
		return
	end
	
	local _GUI = ContextTable[ InfoLineContext ].GUI
	
	if _GUI.info_line ~= nil and IsTable( _GUI.info_line.state ) and #_GUI.info_line.state > 0 then
		__SetupInfoLineL2( unpack( _GUI.info_line.state[ #_GUI.info_line.state ].L ) )
		__SetupInfoLineR2( unpack( _GUI.info_line.state[ #_GUI.info_line.state ].R ) )
		-- free up the last state table
		SetNil( _GUI.info_line.state[ #_GUI.info_line.state ] )
		_GUI.info_line.state[ #_GUI.info_line.state ] = nil
	end
end

---------------
-- PushInfoLine
---------------
function PushInfoLine_F( text )
	InfoLines.force = true
	PushInfoLine( text )
	InfoLines.force = false
end

function PushInfoLine( text )
	PushInfoLineL( text )
end

function PushInfoLineL_F( text )
	InfoLines.force = true
	PushInfoLineL( text )
	InfoLines.force = false
end

function PushInfoLineL( text )
	__PushInfoLine( text, true )
end

function PushInfoLineR_F( text )
	InfoLines.force = true
	PushInfoLineR( text )
	InfoLines.force = false
end

function PushInfoLineR( text )
	__PushInfoLine( text, false )
end

----------------
-- SetupInfoLine
----------------
function SetupInfoLine_F( ... )
	InfoLines.force = true
	SetupInfoLine( unpack( arg ) )
	InfoLines.force = false
end

function SetupInfoLine( ... )
	SetupInfoLineL( unpack( arg ) )
	--SetupInfoLineR()
end

function SetupInfoLineL_F( ... )
	InfoLines.force = true
	SetupInfoLineL( unpack( arg ) )
	InfoLines.force = false
end

function SetupInfoLineL( ... )
	__SetupInfoLine( arg, true )
end

function SetupInfoLineR_F( ... )
	InfoLines.force = true
	SetupInfoLineR( unpack( arg ) )
	InfoLines.force = false
end

function SetupInfoLineR( ... )
	__SetupInfoLine( arg, false )
end

-------------------
-- internal helpers - these should _NOT_ be called directly!
-------------------
function __SetupInfoLine( arg, left )	
	if InfoLineAllow() == false then
		local c = Select( left, "SetupInfoLineL", "SetupInfoLineR" ) .. "( "
		for i = 1, arg.n, 1 do
			c = c .. InfoLineSafeParam( arg[ i ] )
			if i ~= arg.n then
				c = c .. ", "
			end
		end
		c = c .. " )"
		InfoLineCommand( c )
		return
	end
	
	__SetupInfoLine2( arg, left )
end

function __DoSetupInfoLine2( arg, left )
	local _GUI = ContextTable[ InfoLineContext ].GUI
	
	if _GUI.info_line == nil then
		return true
	end
	
	local t = Select( left, _GUI.info_line.L, _GUI.info_line.R )
	
	if t == nil then
		return true
	end
	
	-- check numbers
	local active_count = 0
	
	for i, v in ipairs( t ) do
		if t[ i ].active == true then
			active_count = active_count + 1
		end
	end
	
	if arg.n == 0 or arg.n ~= active_count then
		return true
	end
	
	-- check current state
	for i = 1, arg.n, 1 do
		if t[ i ].active == false or t[ i ].text ~= arg[ i ] then
			return true
		end
	end
	
	return false
end

function __SetupInfoLineL2( ... )
	__SetupInfoLine2( arg, true )
end

function __SetupInfoLineR2( ... )
	__SetupInfoLine2( arg, false )
end

function __SetupInfoLine2( arg, left )
	local _GUI = ContextTable[ InfoLineContext ].GUI
	
	if __DoSetupInfoLine2( arg, left ) == false then
		return
	end
	
	local num = 8	-- TODO - less?
	local spacing = InfoLines.spacing
	
	if _GUI.info_line == nil then
		_GUI.info_line = {  }
		_GUI.info_line.L = {  }
		_GUI.info_line.R = {  }
	end
	
	local t = Select( left, _GUI.info_line.L, _GUI.info_line.R )
	local gadget_name = Select( left, "_info_lineL", "_info_lineR" )
	local timeline_name = Select( left, "SetupInfoLineL", "SetupInfoLineR" )
	local start_x = Select( left, Screen.safe.left, Screen.safe.right )
	local mult = Select( left, 1, -1 )
	
	if #t == 0 then
		-- add them
		for i = 1, num, 1 do
			t[ i ] = {  }
			t[ i ].id = UIButtons.CloneXtGadgetByName( "SCUIBank", gadget_name, InfoLineContext )
			t[ i ].width = 0
		end
	end
	
	InfoLines.do_timelines[ #InfoLines.do_timelines + 1 ] = timeline_name
	
	-- all inactive to start
	for i = 1, num, 1 do
		UIButtons.SetActive( t[ i ].id, false )
		t[ i ].active = false
	end
	
	if arg.n > num then
		arg.n = num
	end
	
	-- change text
	for i = 1, arg.n, 1 do
		UIButtons.SetActive( t[ i ].id, true )
		
		if arg[ i ] == nil then
			arg[ i ] = UIText.CMN_NOWT
		end
		
		t[ i ].text = arg[ i ]
		UIButtons.ChangeText( t[ i ].id, arg[ i ] )
		
		t[ i ].width = UIButtons.GetStaticTextLength( t[ i ].id )
		t[ i ].active = true
	end
	
	-- update positions
	local accum = 0
	
	for i = 1, arg.n, 1 do
		UIButtons.ChangePosition( t[ i ].id, start_x + ( accum * mult ), Screen.safe.bottom )
		accum = accum + t[ i ].width + spacing
	end
end

function __InfoLineContains( text, left )
	if InfoLineAllow() == false then
		local c = Select( left, "InfoLineContainsL", "InfoLineContainsR" ) .. "( "
		c = c .. InfoLineSafeParam( text )
		c = c .. " )"
		InfoLineCommand( c )
		return
	end
	
	local _GUI = ContextTable[ InfoLineContext ].GUI

	if _GUI.info_line == nil then
		return
	end
	
	local t = Select( left, _GUI.info_line.L, _GUI.info_line.R )
	
	if t == nil then
		return
	end
	
	for i = 1, #t, 1 do
		if t[ i ].active == true and t[ i ].text == text then
			return true
		end
	end
	
	return false
end

function __PushInfoLine( text, left )
	if InfoLineAllow() == false then
		local c = Select( left, "PushInfoLineL", "PushInfoLineR" ) .. "( "
		c = c .. InfoLineSafeParam( text )
		c = c .. " )"
		InfoLineCommand( c )
		return
	end
	
	local _GUI = ContextTable[ InfoLineContext ].GUI
	
	local SetupFunc = Select( left, SetupInfoLineL, SetupInfoLineR )

	if _GUI.info_line == nil then
		SetupFunc()
	end
	
	local t = Select( left, _GUI.info_line.L, _GUI.info_line.R )
	
	if t == nil then
		SetupFunc()
	end
	
	local ContainsFunc = Select( left, InfoLineContainsL, InfoLineContainsR )
	
	if ContainsFunc( text ) == false then
		local s = StoreInfoLine()
		local state = Select( left, s.L, s.R )
		state[ #state + 1 ] = text
		RestoreInfoLine()
	end
end

----------------------
-- standard info lines
----------------------
function SetupNextAndBack_F()
	InfoLines.force = true
	SetupNextAndBack()
	InfoLines.force = false
end

function SetupNextAndBack()
	SetupInfoLineL( UIText.INFO_A_NEXT, UIText.INFO_B_BACK )
end

function SetupMenuOptions_F()
	InfoLines.force = true
	SetupMenuOptions()
	InfoLines.force = false
end

function SetupMenuOptions()
	SetupInfoLineL( UIText.INFO_A_SELECT, UIText.INFO_B_BACK )
end

function SetupNext_F()
	InfoLines.force = true
	SetupNext()
	InfoLines.force = false
end

function SetupNext()
	SetupInfoLineL( UIText.INFO_A_NEXT )
end

function SetupBack_F()
	InfoLines.force = true
	SetupBack()
	InfoLines.force = false
end

function SetupBack()
	SetupInfoLineL( UIText.INFO_B_BACK )
end
