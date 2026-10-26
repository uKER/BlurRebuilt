----------------------------------------
--initial entry point for the whole UI--
----------------------------------------

function show_table(tbl,recurse)
	show_table_spacing = ""
	my_show_table(tbl,recurse)
	show_table_spacing = nil
end

function my_show_table(tbl,recurse)
	if tbl == nil then
		print("show_table : ERROR - Table == nil")
		return
	end

	print(show_table_spacing .. "{")
	show_table_spacing = show_table_spacing .. "   "
	
	for k,v in pairs(tbl) do
		if type(v) == "table" and recurse~=nil then
			print(show_table_spacing .. "TABLE: " .. k);
			my_show_table(v,recurse);
		else
			print(show_table_spacing .. k .. " = ",v)
		end
   	end

	show_table_spacing = string.sub(show_table_spacing,0,-4)
	print( show_table_spacing .. "}")
end

function show_globals(show_functions)
	for k,v in pairs(_G) do
		if type(v) ~= "function" or show_functions==true then
			print(k .. " = " , v)
		end
	end	
end

function UISetup( initParam )
--	UIScreen.SetInitFunction( "ScreenInit" )
--	UIScreen.SetEndFunction( "End" )
	--
	UISystem.LoadLuaScript( "UIGlobals.lua" )
	UISystem.LoadLuaScript("UIFontSequences.lua")
	--UISystem.LoadLuaScript("UIDebug.lua")	
	UISystem.LoadLuaScript( "Screens\\GUIBank.lua" )
	
	-- Timeline event script.
	UISystem.LoadLuaScript( "Screens\\Commands.lua" )
	
	-- Load popup scripts.
	UISystem.LoadLuaScript("Popups\\CustomPopupData.lua")
	
	-- Set the first script.
	UIGlobals.LaunchMode = initParam
	UIGlobals.DevMode = (UIGlobals.LaunchMode ~= UIEnums.LaunchMode.Master and UIGlobals.LaunchMode ~= UIEnums.LaunchMode.Demo)
	
	if UIGlobals.LaunchMode == UIEnums.LaunchMode.Final then
		print("LaunchMode == Final")
	elseif UIGlobals.LaunchMode == UIEnums.LaunchMode.Debug then
		print("LaunchMode == Debug")
	elseif UIGlobals.LaunchMode == UIEnums.LaunchMode.DebugRb then
		print("LaunchMode == Debug : RaceBook")
	elseif UIGlobals.LaunchMode == UIEnums.LaunchMode.Demo then
		print("LaunchMode == Public Demo")
	elseif UIGlobals.LaunchMode == UIEnums.LaunchMode.Master then
		print("LaunchMode == MASTER")
	elseif UIGlobals.LaunchMode == UIEnums.LaunchMode.Automation then
		print("LaunchMode == Automation")
	else
		print("## WARNING : LaunchMode == Unknown ##", initParam)
	end
	
	if UIGlobals.DevMode == true then
		print("- Developer mode")
	end
	
	if UIGlobals.LaunchMode == UIEnums.LaunchMode.Debug or UIGlobals.LaunchMode == UIEnums.LaunchMode.Automation then
		-- Amax stuff (assume pad0)
		UIGlobals.ProfilePressedStart = 0
		
		Profile.Setup( UIGlobals.ProfilePressedStart, true )	-- just blagged them a profile
		UIGlobals.ProfileState[ UIGlobals.ProfilePressedStart ] = UIEnums.Profile.Blagged
		Profile.LockToPad( UIGlobals.ProfilePressedStart )
		Profile.SetPrimaryPad( UIGlobals.ProfilePressedStart )
		
		--GameProfile.SetPrimary( UIGlobals.ProfilePressedStart )
		GameProfile.InitPrimary()
		
		Profile.AllowProfileChanges( false )
		Profile.ActOnProfileChanges( false )
		Profile.AllowAllPadInput( false )
		GameProfile.ClearGameProfile( UIGlobals.ProfilePressedStart, true )	-- [Blur2PC] T230: apply the profile's controls
		
		Amax.SetGameMode(UIEnums.GameMode.SinglePlayer)
		
		Amax.SendMessage(UIEnums.GameFlowMessage.EnteredDebugMenu)
		Amax.SetGameMode(UIEnums.GameMode.Debug)
		Amax.CacheGlobalAnimationResources()

		UIScreen.SetInitialScreen( "Debug\\DebugScreen.lua" )
	else
		UIScreen.SetInitialScreen( "Intro\\Legal.lua" )
	end
end
