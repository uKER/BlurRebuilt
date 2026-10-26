UISystem.LoadLuaScript( "Screens\\Debug\\DebugItemBase.lua" )

GUI =
{
	stringNo = 0,
	finished = false,
	leaving_debug_menu = false,
	
	city_table = GameData.GetCities(true),
}

--------------
-- Debug Items
--------------
CitySelectGadget = DebugItem:New()
function CitySelectGadget:AddData()

	UIButtons.ClearItems(self.ID)
	for i,v in ipairs(GUI.city_table) do
		UIButtons.AddItem(self.ID, v.id, v.name, false)
	end
end



RouteSelectGadget = DebugItem:New()
function RouteSelectGadget:AddData()

	CurrentCity = UIButtons.GetSelection(DebugMenu[1][1]:GetID())
	local route_table = GameData.GetRoutes(CurrentCity)

	GUI.stringNo = 0
	UIButtons.ClearItems(self.ID)
	for i,v in ipairs(route_table) do
		UIButtons.StoreString(GUI.stringNo, v.debug_tag)
		UIButtons.AddItem(self.ID, v.id, GUI.stringNo, true)
		GUI.stringNo = GUI.stringNo + 1
	end
	
	CurrentRoute = UIButtons.GetSelection(DebugMenu[2][1]:GetID())
end

function RouteSelectGadget:Update()

	if CurrentCity ~= UIButtons.GetSelection(DebugMenu[1][1]:GetID()) then
		self:AddData()
		UpdateRouteShapes( "Shp_Route", 4, CurrentCity, CurrentRoute, "Shp_StartLine" )
	end
	
	if CurrentRoute ~= UIButtons.GetSelection(DebugMenu[2][1]:GetID()) then
		
		CurrentRoute = UIButtons.GetSelection(DebugMenu[2][1]:GetID())
		UpdateRouteShapes( "Shp_Route", 4, CurrentCity, CurrentRoute, "Shp_StartLine" )
	end
end


LapSelectGadget = DebugItem:New()
function LapSelectGadget:AddData()

	UIButtons.ClearItems(self.ID)

	--UIButtons.AddItem(self.ID,99,UIText.CMN_INFINITE,false)
	UIButtons.AddItem(self.ID,0,UIText.CMN_NONE,false)
	UIButtons.AddItem(self.ID,1,UIText.CMN_1,false)
	UIButtons.AddItem(self.ID,2,UIText.CMN_2,false)
	UIButtons.AddItem(self.ID,3,UIText.CMN_3,false)
	UIButtons.AddItem(self.ID,4,UIText.CMN_4,false)
	UIButtons.AddItem(self.ID,5,UIText.CMN_5,false)
	UIButtons.AddItem(self.ID,6,UIText.CMN_6,false)
	UIButtons.AddItem(self.ID,7,UIText.CMN_7,false)
	UIButtons.AddItem(self.ID,8,UIText.CMN_8,false)
	UIButtons.AddItem(self.ID,9,UIText.CMN_9,false)
	UIButtons.AddItem(self.ID,10,UIText.CMN_10,false)
	UIButtons.AddItem(self.ID,11,UIText.CMN_11,false)
	UIButtons.AddItem(self.ID,12,UIText.CMN_12,false)
	UIButtons.AddItem(self.ID,13,UIText.CMN_13,false)
	UIButtons.AddItem(self.ID,14,UIText.CMN_14,false)
	UIButtons.AddItem(self.ID,15,UIText.CMN_15,false)
	UIButtons.AddItem(self.ID,16,UIText.CMN_16,false)
	UIButtons.AddItem(self.ID,17,UIText.CMN_17,false)
	UIButtons.AddItem(self.ID,18,UIText.CMN_18,false)
	UIButtons.AddItem(self.ID,19,UIText.CMN_19,false)
	UIButtons.AddItem(self.ID,20,UIText.CMN_20,false)

end

EnvironmentSelectGadget = DebugItem:New()
function EnvironmentSelectGadget:AddData()

	UIButtons.ClearItems(self.ID)
	UIButtons.AddItem(self.ID,0,UIText.CMN_NO,false)

	for i,v in ipairs(GUI.city_table) do
		UIButtons.AddItem(self.ID, v.id, v.name, false)
	end
end

PartyModeGadget = DebugItem:New()
function PartyModeGadget:AddData()

	UIButtons.ClearItems(self.ID)

	UIButtons.AddItem(self.ID,0,UIText.CMN_NO,false)
	UIButtons.AddItem(self.ID,1,UIText.CMN_YES,false)
end

ViewPortSelectGadget = DebugItem:New()
function ViewPortSelectGadget:AddData()

	UIButtons.ClearItems(self.ID)

	UIButtons.AddItem(self.ID,1,UIText.CMN_1,false)
	UIButtons.AddItem(self.ID,2,UIText.CMN_2,false)
	UIButtons.AddItem(self.ID,3,UIText.CMN_3,false)
	UIButtons.AddItem(self.ID,4,UIText.CMN_4,false)

end

SplitscreenRaceModeSelectGadget = DebugItem:New()
function SplitscreenRaceModeSelectGadget:AddData()

	UIButtons.ClearItems(self.ID)

	local splitScreen_table = GameData.GetSplitScreenEvents()
	for i,v in ipairs(splitScreen_table) do
		UIButtons.AddItem(self.ID, v.id, v.name, false)
	end
	
end

-------------
-- Debug Menu
-------------
DebugMenu = 
{
	{
		CitySelectGadget,
		UIText.CMN_CITY,
		"City",
	},
	{
		RouteSelectGadget,
		UIText.CMN_ROUTE,
		"Route",
	},
	{
		LapSelectGadget,
		UIText.CMN_LAPS,
		"Laps",
	},
	{
		EnvironmentSelectGadget,
		UIText.CMN_ENVIRONMENT_OVERIDE,
		"Environment",
	},
	{
		PartyModeGadget,
		UIText.CMN_PARTY_MODE,
		"PartyMode",
	},
	{
		ViewPortSelectGadget,
		UIText.CMN_VIEWPORTS,
		"Viewports",
	},
	{
		SplitscreenRaceModeSelectGadget,
		UIText.CMN_SPLIT_SCREEN_RACEMODE,
		"SplitScreenRaceMode",
	},
}

NumberOfItems=0
CurrentItemSelected=1

function Init()
	AddSCUI_Elements( )
	
	UIGlobals.LoadFromDebug = true
	UIGlobals.Sp = nil	-- reset

	GUI.stringNo = 0

	local y=48
	for i,v in ipairs(DebugMenu) do
		y=v[1]:AddButton(v[2],48,y,v[3])
		v[1]:AddData()
		NumberOfItems=NumberOfItems+1
	end


	local leveldata=Amax.GetLevelData()
	
	-- print(leveldata)
	-- show_table(leveldata,true)
	
	for i,v in ipairs(DebugMenu) do
		v[1]:AddData()
		v[1]:SetSelection(leveldata)
	end

	MoveMenu(0)
end

function PostInit()
	UpdateRouteShapes( "Shp_Route", 4, CurrentCity, CurrentRoute, "Shp_StartLine" )
end

function StartLoop( time )
	--
end

function MessageUpdate( message, arg1, arg2 )
	if SCUI_MessageUpdate( message, arg1, arg2 ) == true then
		return
	end

	if GUI.finished == true then
		return
	end
	
	if GUI.leaving_debug_menu == true then
		if message == UIEnums.GameFlowMessage.StartGameRendering then
			GUI.leaving_debug_menu = false
			GoScreen("Intro\\StartScreen.lua")
		else
			return
		end
	end		
	
	if message == UIEnums.Message.MenuBack and arg2 == true then
		GUI.leaving_debug_menu = true		
		Amax.SetGameMode(UIEnums.GameMode.Nothing)
		Amax.SetDebugEvent(false)
		Amax.SetUICarToMultiplayer(false)		
		Amax.SendMessage(UIEnums.GameFlowMessage.LoadUIScene)
	elseif message == UIEnums.Message.MenuNext and arg2 == true then
		LaunchGame()
	elseif message == UIEnums.Message.ButtonLeftShoulder and arg2 == true then
		GoScreen("Debug\\DebugUI.lua")
	elseif message == UIEnums.Message.AutomationLoadGame then
		UIGlobals.Splitscreen.players = 
		{
			{ pad = 0, },
			{ pad = 1, },
			{ pad = 2, },
			{ pad = 3, },
	}
		Splitscreen_ClearMessages()
		
		StoreScreen( UIEnums.ScreenStorage.FE_RETURN ) -- Store current
		GoLoadingScreen("Loading\\LoadingGame.lua")
	elseif message == UIEnums.Message.ButtonLeftTrigger and arg2 == true then
		GoScreen("Debug\\DebugEventScreen.lua")
	elseif message == UIEnums.Message.ButtonRightTrigger and arg2 == true then
		SetupRaceTable()
		GoScreen("Debug\\DebugCarScreen.lua")
	elseif message == UIEnums.Message.ButtonUp and arg2 == true then
		MoveMenu(-1)
	elseif message == UIEnums.Message.ButtonDown and arg2 == true then
		MoveMenu(1)
	elseif (message == UIEnums.Message.ButtonLeft or message == UIEnums.Message.ButtonRight) and arg2 == true then
		if CurrentItemSelected == 1 then
			UIButtons.SetSelectionByIndex(DebugMenu[CurrentItemSelected+1][1]:GetID(),0)
		end
	end
end

function LaunchGame()
	StoreScreen( UIEnums.ScreenStorage.FE_RETURN ) -- Store current

	SetupRaceTable()
	Amax.LoadTextureClone( GUI.route_texture_name )
	GoLoadingScreen("Loading\\LoadingGame.lua")
end

function FrameUpdate( time )
	for i,v in ipairs(DebugMenu) do
		v[1]:Update()
	end
	
	if GUI.leaving_debug_menu == true then
		return
	end
	
	if true and GUI.finished == false and UIGlobals.LaunchMode == UIEnums.LaunchMode.Automation then
		print("GOGO Automation")
		
		-- Force debug profile
		Profile.Setup( arg1, true )	-- just blagged them a profile
--		UIGlobals.ProfileState[ arg1 ] = UIEnums.Profile.Blagged
		Profile.LockToPad( arg1 )
		Profile.SetPrimaryPad( arg1 )
		
		GameProfile.InitPrimary()
		
		Profile.AllowProfileChanges( false )
		Profile.ActOnProfileChanges( false )
		Profile.AllowAllPadInput( false )
		
		Amax.SendMessage(UIEnums.GameFlowMessage.EnteredDebugMenu)
		Amax.SetGameMode(UIEnums.GameMode.SinglePlayer)
		
		-- Quit start screen
		UIButtons.TimeLineActive( "exit", true )
		UIButtons.TimeLineActive( "start_fade", false )
		UIScreen.SetScreenTimers(0,0.7)

		StoreScreen( UIEnums.ScreenStorage.FE_RETURN ) -- If you quit return to start screen
		
		UIGlobals.AlreadySetupRace = true
		UIGlobals.LoadFromDebug = true
		local DebugEvent = 
		{
		   Difficulty =                         1,
		   PickupSlots =                        3,
		   HealthSlots =                        5,
		   Vehicle =                            5392512,
		   ShieldSlots =                        6,
		   EventID =                            4597671,
		}
		if UIGlobals.automation_event_id == 0 then
			DebugEvent.EventID = -319285474
		elseif UIGlobals.automation_event_id == 1 then
			DebugEvent.EventID = -1433716086
		elseif UIGlobals.automation_event_id == 2 then
			DebugEvent.EventID = 4597671
		elseif UIGlobals.automation_event_id == 3 then
		end
		
		UIGlobals.automation_event_id = UIGlobals.automation_event_id + 1
		
		if UIGlobals.automation_event_id > 3 then
			UIGlobals.automation_event_id = 0
		end
		
		Amax.LoadTextureClone( "usa_amboy_3" )
				
		show_table( DebugEvent )
		UIGlobals.DebugEvent = DebugEvent
		Amax.SetupDebugRace(DebugEvent)
				
		GoLoadingScreen("Loading\\LoadingGame.lua")
	end
end

function Render()
	--
end

function EndLoop( time )
	--
end

function End()
	--
end

function MoveMenu(move)

	CurrentItemSelected=CurrentItemSelected+move

	if CurrentItemSelected < 1 then
		CurrentItemSelected=NumberOfItems
	end

	if CurrentItemSelected > NumberOfItems then
		CurrentItemSelected=1
	end

	for i,v in ipairs(DebugMenu) do
		v[1]:Activate(false)
	end

	DebugMenu[CurrentItemSelected][1]:Activate(true)
end

function SetupRaceTable()
	DebugRace = {}
	for i,v in ipairs(DebugMenu) do
		v[1]:WriteToStructure(DebugRace)
	end

	-- show_table(DebugRace,true)

	Amax.SetupRace(DebugRace)
end
