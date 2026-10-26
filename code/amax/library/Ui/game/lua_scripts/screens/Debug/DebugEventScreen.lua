UISystem.LoadLuaScript( "Screens\\Debug\\DebugItemBase.lua" )

YPos = 0
YNextPos = 0

NumberOfItems=0
CurrentItemSelected=1

GUI =
{
	stringNo = 0,
	finished = false,
	leaving_debug_menu = false,
}

--------------
-- Debug Items
--------------
EventsGadget = DebugItem:New()
function EventsGadget:AddData()

	local event_table = GameData.GetEvents()
	UIButtons.ClearItems(self.ID)
	for i,v in ipairs(event_table) do
		UIButtons.StoreString(GUI.stringNo, v.event_tag)
		UIButtons.AddItem(self.ID, v.event_id, GUI.stringNo, true)
		GUI.stringNo = GUI.stringNo + 1
	end

end

VehicleSelectGadget = DebugItem:New()
function VehicleSelectGadget:AddData()

	local vehicle_table = GameData.GetVehicles()
	for i,v in ipairs(vehicle_table) do
		UIButtons.StoreString(GUI.stringNo, v.name)
		UIButtons.AddItem(self.ID, v.id, v.name, false)
		GUI.stringNo = GUI.stringNo + 1
	end
end

VehicleSelectGadget1 = VehicleSelectGadget:New()

DifficultySelectGadget = DebugItem:New()
function DifficultySelectGadget:AddData()

	UIButtons.ClearItems(self.ID)
	UIButtons.AddItem(self.ID,0,UIText.RBC_DIFFICULTY_VERY_EASY,false)
	UIButtons.AddItem(self.ID,1,UIText.RBC_DIFFICULTY_EASY,false)
	UIButtons.AddItem(self.ID,2,UIText.RBC_DIFFICULTY_MEDIUM,false)
	UIButtons.AddItem(self.ID,3,UIText.RBC_DIFFICULTY_HARD,false)
	UIButtons.AddItem(self.ID,4,UIText.RBC_DIFFICULTY_EXPERT,false)
end

PickupSlotSelectGadget = DebugItem:New()
function PickupSlotSelectGadget:AddData()

	UIButtons.ClearItems(self.ID)

	UIButtons.AddItem(self.ID,1,UIText.CMN_1,false)
	UIButtons.AddItem(self.ID,2,UIText.CMN_2,false)
	UIButtons.AddItem(self.ID,3,UIText.CMN_3,false)
	UIButtons.AddItem(self.ID,4,UIText.CMN_4,false)
	UIButtons.AddItem(self.ID,5,UIText.CMN_5,false)
	UIButtons.AddItem(self.ID,6,UIText.CMN_6,false)

end

ShieldSlotSelectGadget = DebugItem:New()
function ShieldSlotSelectGadget:AddData()

	UIButtons.ClearItems(self.ID)

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

end

HealthSlotSelectGadget = DebugItem:New()
function HealthSlotSelectGadget:AddData()

	UIButtons.ClearItems(self.ID)

	UIButtons.AddItem(self.ID,0,UIText.CMN_NONE,false)
	UIButtons.AddItem(self.ID,1,UIText.CMN_1,false)
	UIButtons.AddItem(self.ID,2,UIText.CMN_2,false)
	UIButtons.AddItem(self.ID,3,UIText.CMN_3,false)
	UIButtons.AddItem(self.ID,4,UIText.CMN_4,false)
	UIButtons.AddItem(self.ID,5,UIText.CMN_5,false)

end

-------------
-- Debug Menu
-------------
DebugMenu = 
{
	{EventsGadget,UIText.DBG_Event,"EventID",},

	{VehicleSelectGadget1,UIText.CMN_VEHICLE,"Vehicle",},

	{DifficultySelectGadget,UIText.CMN_DIFFICULTY,"Difficulty",},

	{PickupSlotSelectGadget,UIText.CMN_PICKUP_SLOTS,"PickupSlots",},
	{ShieldSlotSelectGadget,UIText.CMN_SHIELD_SLOTS,"ShieldSlots",},
	{HealthSlotSelectGadget,UIText.CMN_HEALTH_SLOTS,"HealthSlots",},
}

function Init()
	AddSCUI_Elements( )

	GUI.stringNo = 0

	local x=48
	local y=48
	for i,v in ipairs(DebugMenu) do
		y=v[1]:AddButton(v[2],x,y,v[3])
		v[1]:AddData()
		NumberOfItems=NumberOfItems+1
	end

	eventID,vehicleID,diffID,pickup_slots,shield_slots,health_slots=Amax.GetCurrentDebugEventID()

	print(eventID,vehicleID,diffID,pickup_slots,shield_slots,health_slots)

	UIButtons.SetSelection(DebugMenu[1][1]:GetID(), eventID)
	UIButtons.SetSelection(DebugMenu[2][1]:GetID(), vehicleID)
	UIButtons.SetSelection(DebugMenu[3][1]:GetID(), diffID)
	UIButtons.SetSelection(DebugMenu[4][1]:GetID(), pickup_slots)
	UIButtons.SetSelection(DebugMenu[5][1]:GetID(), shield_slots)
	UIButtons.SetSelection(DebugMenu[6][1]:GetID(), health_slots)

	MoveMenu(0)
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
	
	if message == UIEnums.Message.MenuNext and arg2 == true then
		Amax.SetGameMode(UIEnums.GameMode.SinglePlayer)

		--Amax.SendMessage(UIEnums.GameFlowMessage.SaveGameLoaded)

		StoreScreen( UIEnums.ScreenStorage.FE_RETURN ) -- Store current

		SetupDebugEventTable()

		print(UIButtons.GetSelection(DebugMenu[1][1]:GetID()), UIButtons.GetSelection(DebugMenu[2][1]:GetID()), 
				UIButtons.GetSelection(DebugMenu[3][1]:GetID()), UIButtons.GetSelection(DebugMenu[4][1]:GetID()), 
					UIButtons.GetSelection(DebugMenu[5][1]:GetID()), UIButtons.GetSelection(DebugMenu[6][1]:GetID()))

		Amax.SetCurrentDebugEventID(UIButtons.GetSelection(DebugMenu[1][1]:GetID()), UIButtons.GetSelection(DebugMenu[2][1]:GetID()), 
										UIButtons.GetSelection(DebugMenu[3][1]:GetID()), UIButtons.GetSelection(DebugMenu[4][1]:GetID()), 
											UIButtons.GetSelection(DebugMenu[5][1]:GetID()), UIButtons.GetSelection(DebugMenu[6][1]:GetID()))
											
		-- PR - setup what event we're on
		Sp_NewGame()
		
		UIGlobals.Sp = { TierInfo = SinglePlayer.TierInfo(), CurrentTier = -1, CurrentEvent = -1, }
			
		for i, v in ipairs( UIGlobals.Sp.TierInfo ) do
			for j, w in ipairs( v.events ) do
				if SinglePlayer.EventInfo( w ).eventId == UIGlobals.DebugEvent.EventID then
					UIGlobals.Sp.CurrentTier = i
					UIGlobals.Sp.CurrentEvent = j
					break
				end
			end
		end
		
		if UIGlobals.Sp.CurrentTier == -1 or UIGlobals.Sp.CurrentEvent == -1 then
			print( "*** WARNING - couldn't find sp event - defaulting Lua to T0E0 ***" )
			UIGlobals.Sp.CurrentTier = 1
			UIGlobals.Sp.CurrentEvent = 1
		end
		
		Amax.LoadTextureClone( "usa_amboy_3" )
		GoLoadingScreen("Loading\\LoadingGame.lua")
		
	elseif message == UIEnums.Message.ButtonX and arg2 == true then
		print(UIButtons.GetSelection(DebugMenu[1][1]:GetID()), UIButtons.GetSelection(DebugMenu[2][1]:GetID()), 
				UIButtons.GetSelection(DebugMenu[3][1]:GetID()), UIButtons.GetSelection(DebugMenu[4][1]:GetID()), 
					UIButtons.GetSelection(DebugMenu[5][1]:GetID()), UIButtons.GetSelection(DebugMenu[6][1]:GetID()))		

	elseif message == UIEnums.Message.MenuBack and arg2 == true then
		GUI.leaving_debug_menu = true		
		Amax.SetGameMode(UIEnums.GameMode.Nothing)
		Amax.SetDebugEvent(false)
		Amax.SetUICarToMultiplayer(false)		
		Amax.SendMessage(UIEnums.GameFlowMessage.LoadUIScene)

	elseif message == UIEnums.Message.ButtonLeftTrigger and arg2 == true then
		GoScreen("Debug\\DebugLoadReplays.lua")

	elseif message == UIEnums.Message.ButtonRightTrigger and arg2 == true then
		Amax.SendMessage(UIEnums.GameFlowMessage.EnteredDebugMenu)
		Amax.SetGameMode(UIEnums.GameMode.Debug)
		GoScreen("Debug\\DebugScreen.lua")

	elseif message == UIEnums.Message.ButtonUp and arg2 == true then
		MoveMenu(-1)

	elseif message == UIEnums.Message.ButtonDown and arg2 == true then
		MoveMenu(1)

	end
end

function FrameUpdate( time )
	for i,v in ipairs(DebugMenu) do
		v[1]:Update()
	end

	if GUI.leaving_debug_menu == true then
		return
	end
	
	YNextPos=152-((CurrentItemSelected-1)*20)
	YPos=YPos+((YNextPos-YPos)*(time*4))
	MoveMenu(0)

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

	local x=48
	local y=YPos
	local tempNumberOfItems=0
	for i,v in ipairs(DebugMenu) do
		y=v[1]:MoveButton(x,y)
		tempNumberOfItems=tempNumberOfItems+1
	end	
	
end

function SetupDebugEventTable()
	
	UIGlobals.DebugEvent = {}
	for i,v in ipairs(DebugMenu) do
		v[1]:WriteToStructure(UIGlobals.DebugEvent)
	end

	show_table(UIGlobals.DebugEvent,true)

	Amax.SetupDebugRace(UIGlobals.DebugEvent)

	UIGlobals.AlreadySetupRace = true

end