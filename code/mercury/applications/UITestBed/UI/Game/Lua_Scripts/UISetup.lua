----------------------------------------
--initial entry point for the whole UI--
----------------------------------------

show_table_spacing = nil

function show_table(tbl,recurse)
	show_table_spacing = ""
	my_show_table(tbl,recurse)
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
	UIScreen.SetInitFunction( "ScreenInit" )
	UIScreen.SetEndFunction( "End" )
	--
	UISystem.LoadLuaScript( "UIGlobals.lua" )
	UISystem.LoadLuaScript("UIFontSequences.lua")
	
	--UISystem.LoadLuaScript("UIDebug.lua")	
		
	UISystem.LoadLuaScript( "Screens\\GUIBank.lua" )

	UISystem.LoadLuaScript( "Popups\\GUIBank.lua" )
	--UISystem.LoadLuaScript("Popups\\CustomPopupData.lua")
	--UISystem.LoadLuaScript("Popups\\TheCustomPopup.lua")
	
	UIScreen.SetInitialScreen( "Main.lua" )
	--UIScreen.SetInitialScreen( "Intro\\Welcome.lua" )
end
