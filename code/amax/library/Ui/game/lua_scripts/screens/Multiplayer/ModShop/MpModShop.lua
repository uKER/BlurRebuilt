GUI =
{
	finished = false,

	CanExit = function( _GUI )
		return false
	end,

	Stage =
	{
		Loadout		= 1,
		Slot		= 2,
		Ability		= 3,
	},

	CurrentAbility = -1,


	CurrentStage = 1,
	Countdown = 0,

	LoadoutGadgets = {},
	PreviewGadgets = {},
	AbilityGadgets = {},
	ChoosenAbilities = {},
	BackupAbilities = {},

	SlotGadgets = {},
	CurrentSlot = 1,
	CurrentSelection = -1,
	NewLoadout = false,

	ShouldSave = false,

	LookingAtEmptyLoadout = false,

	Previews = {},
}

function Init()
	if UIGlobals.Multiplayer.InLobby == true then
		PlaySfxGraphicBack()
	else
		PlaySfxGraphicNext()
	end
	
	AddSCUI_Elements()
	Amax.ChangeUiCamera( "Sp_3", UIGlobals.CameraLerpTime, 0 )
		
	StoreInfoLine()
	SetupInfoLine()
	
	if UIGlobals.Multiplayer.InLobby == false then
		net_SetRichPresence( UIEnums.RichPresence.ModShop )
	end

	GUI.LoadoutsID				= SCUI.name_to_id["loadouts"]
	GUI.LoadoutPreviewID		= SCUI.name_to_id["loadout_preview"]
	GUI.CategorysID				= SCUI.name_to_id["categorys"]
	GUI.AbilitiesID				= SCUI.name_to_id["abilities"]

	GUI.AbilityNameID			= SCUI.name_to_id["ability_name"]
	GUI.AbilityDescID			= SCUI.name_to_id["ability_desc"]

	GUI.LoadoutPreviewTextID	= SCUI.name_to_id["loadout_preview_text"]

	GUI.Loadouts = Multiplayer.GetAbilityLoadouts(UIGlobals.Multiplayer.InLobby)
	GUI.UnlockMap = Multiplayer.GetAbilityUnlockMap()

	UIGlobals.Multiplayer.ModShopActive = true
end

function PostInit()
	UIButtons.SetActive( SCUI.name_to_id["node_loadout"], false )
	UIButtons.SetActive( SCUI.name_to_id["node_ability"], false )
	UIButtons.SetActive( SCUI.name_to_id["node_preview_ability"], false )

	Multiplayer.FeatureViewed(UIEnums.MPFeatures.ModShop)

	UIButtons.ChangePanel( SetupScreenTitle( UIText.MP_MOD_SHOP, SCUI.name_to_id["centre"], "custom_shop" ), UIEnums.Panel._3DAA_WORLD, true )

	-- create loadout menu
	local current = Multiplayer.GetCurrentLoadOutType(UIGlobals.Multiplayer.InLobby)
	for i, v in ipairs( GUI.Loadouts ) do
		GUI.LoadoutGadgets[i] = SetupLoadout( i - 1, v.slots )

		UIButtons.AddListItem( GUI.LoadoutsID, GUI.LoadoutGadgets[i], i )

		-- turn current tick on
		if i == current + 1 then
			UIButtons.PrivateTimeLineActive( GUI.LoadoutGadgets[i], "loadout_selected", true, 1.0 )
			UIButtons.PrivateTimeLineActive( UIButtons.FindChildByName( GUI.LoadoutGadgets[i], "background_box" ), "loadout_selected", true, 0.0 )
		end
	end

	UIButtons.SetSelectionByIndex( GUI.LoadoutsID, current )

	-- create category menu
	GUI.AbilityPacks = Multiplayer.GetAbilityPacks()

	-- create preview menu
	for i = 1, 3 do
		GUI.PreviewGadgets[i] = SetupPreviewItem( "GAME_STR_Ability" .. i, "GAME_STR_Deal more Power-up damage", "deep_impact" )
		UIButtons.AddListItem( GUI.LoadoutPreviewID, GUI.PreviewGadgets[i], i )
	end

	-- create abilities menu
	GUI.Abilities = Multiplayer.GetAbilities( GUI.AbilityPacks[1].type )

	for i, v in ipairs( GUI.Abilities ) do
		GUI.AbilityGadgets[i] = SetupAbility( v.icon_name )
		UIButtons.AddListItem( GUI.AbilitiesID, GUI.AbilityGadgets[i], i )
	end

	-- start darkening
	UIButtons.SetActive( SCUI.name_to_id["darken"], UIGlobals.Multiplayer.InLobby )
	UIButtons.SetActive( SCUI.name_to_id["countdown"], UIGlobals.Multiplayer.InLobby )

	RefreshPreview( UIButtons.GetSelection( GUI.LoadoutsID ) )
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )	

	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end

	if GUI.CurrentStage == GUI.Stage.Loadout and message == UIEnums.Message.PopupNext and arg2 == UIEnums.CustomPopups.ModShopOptions then

		if arg3 == UIEnums.ModShopOptions.Use then
			if GUI.Loadouts[UIButtons.GetSelection( GUI.LoadoutsID )].can_use == true then
				-- turn all others off
				for i, v in ipairs( GUI.LoadoutGadgets ) do
					UIButtons.PrivateTimeLineActive( UIButtons.FindChildByName( v, "background_box" ), "loadout_selected", false )
				end

				GUI.ShouldSave = true

				local node_id = GUI.LoadoutGadgets[UIButtons.GetSelection( GUI.LoadoutsID )]

				-- turn selection animation on
				UIButtons.PrivateTimeLineActive( UIButtons.FindChildByName( node_id, "background_box" ), "loadout_selected", true, 0.0 )

				Multiplayer.SetCurrentLoadOutType( UIButtons.GetSelection( GUI.LoadoutsID ) - 1, UIGlobals.Multiplayer.InLobby )

				if UIGlobals.Multiplayer.InLobby == true then
					NetRace.ChangeLocalPlayerLoadout( Profile.GetPrimaryPad() )
					UIGlobals.Multiplayer.LobbyUpdateLoadout = true
				end
			end

		
		elseif arg3 == UIEnums.ModShopOptions.Edit then

			GUI.CurrentStage = GUI.CurrentStage + 1
			SelectLoadout( UIButtons.GetSelection( GUI.LoadoutsID ), true )

			GUI.ShouldSave = true

			-- auto move on when making new
			GUI.NewLoadout = not GUI.Loadouts[UIButtons.GetSelection( GUI.LoadoutsID )].can_use
			
		elseif arg3 == UIEnums.ModShopOptions.Rename then
			RenameLoadout( UIButtons.GetSelection( GUI.LoadoutsID ), arg1 )
		end
	end
	
	if message == UIEnums.Message.MenuNext and arg2 == true then
		if GUI.CurrentStage == GUI.Stage.Loadout then

			UIGlobals.Multiplayer.CanEquipMods = GUI.Loadouts[UIButtons.GetSelection( GUI.LoadoutsID )].can_use
			SetupCustomPopup( UIEnums.CustomPopups.ModShopOptions )

		elseif GUI.CurrentStage == GUI.Stage.Slot then
			if GUI.CurrentSlot <= #GUI.SlotGadgets then
				GUI.CurrentStage = GUI.CurrentStage + 1
				SelectSlot( GUI.CurrentSlot, true )
				PlaySfxNext()
			end

		elseif GUI.CurrentStage == GUI.Stage.Ability then
			GUI.CurrentStage = GUI.CurrentStage - 1

			SelectAbility( UIButtons.GetSelection( GUI.AbilitiesID ) )
			SelectSlot( GUI.CurrentSlot, false )
			
			UISystem.PlaySound( UIEnums.SoundEffect.ModShopAttach )
		end

	elseif message == UIEnums.Message.ButtonLeft and arg2 == true then
		MoveLeft()

	elseif message == UIEnums.Message.ButtonRight and arg2 == true then
		MoveRight()

	elseif message == UIEnums.Message.MenuBack and arg2 == true then
		PlaySfxBack()
		
		GUI.CurrentSelection = -1

		if GUI.CurrentStage == GUI.Stage.Loadout then
			if GUI.ShouldSave == true then
				--Check sticker, don't actually save as we don't save when we're in the lobby
				Amax.CheckStickerProgress(UIEnums.StickerType.CustomiseALoadout, 1)
			end
			
			if UIGlobals.Multiplayer.InLobby == true then
				PopScreen()
			else
				if GUI.ShouldSave == true then
					StartAsyncSave()
				end
				
				GoScreen( "Multiplayer\\MpOnline.lua" )
			end
			
			if UIGlobals.Multiplayer.InLobby == true then
				PlaySfxGraphicNext()
			else
				PlaySfxGraphicBack()
			end

		elseif GUI.CurrentStage == GUI.Stage.Slot then
		
			Multiplayer.SetAbilityLoadout( UIButtons.GetSelection( GUI.LoadoutsID ) - 1, GUI.ChoosenAbilities, UIGlobals.Multiplayer.InLobby )
			GUI.Loadouts = Multiplayer.GetAbilityLoadouts(UIGlobals.Multiplayer.InLobby)

			SelectLoadout( UIButtons.GetSelection( GUI.LoadoutsID ), false )
			GUI.CurrentStage = GUI.CurrentStage - 1

			-- need to update the changes
			if UIGlobals.Multiplayer.InLobby == true then
				NetRace.ChangeLocalPlayerLoadout( Profile.GetPrimaryPad() )
				UIGlobals.Multiplayer.LobbyUpdateLoadout = true
			end

			if GUI.LookingAtEmptyLoadout == true or UIGlobals.Multiplayer.InLobby == true then
				SetupInfoLine(UIText.INFO_A_SELECT, UIText.INFO_B_BACK)
			else
				SetupInfoLine(UIText.INFO_A_SELECT, UIText.INFO_B_BACK, "GAME_SHARE_BUTTON")
			end
		
		elseif GUI.CurrentStage == GUI.Stage.Ability then
			SelectSlot( GUI.CurrentSlot, false )
			GUI.CurrentStage = GUI.CurrentStage - 1
		end

	elseif GUI.CurrentStage == GUI.Stage.Loadout and message == UIEnums.Message.KeyboardFinished then
		GUI.ShouldSave = true

	elseif message == UIEnums.Message.ButtonLeftShoulder and arg2 == true then

		if Amax.CanUseShare() == true then
			if GUI.LookingAtEmptyLoadout == false and UIGlobals.Multiplayer.InLobby == false then
				if GUI.CurrentStage == GUI.Stage.Loadout then
					UIGlobals.ShareFromWhatPopup=-1
					SetupCustomPopup( UIEnums.CustomPopups.SharingOptions )
				end
			end
		end

	elseif message == UIEnums.Message.PopupNext and arg2 == UIEnums.CustomPopups.SharingOptions then

		local index = UIButtons.GetSelection( GUI.LoadoutsID )

		StoreScreen( UIEnums.ScreenStorage.FE_SOCIAL_NETWORK, "Multiplayer\\ModShop\\MpModShop.lua" )

		if arg3 == UIEnums.ShareOptions.Facebook then

			Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.OwnMod, 1, index-1)
			GoScreen( "Shared\\Facebook.lua", UIEnums.Context.Blurb )

		elseif arg3 == UIEnums.ShareOptions.Twitter then

			Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.OwnMod, 0, index-1)
			GoScreen( "Shared\\Twitter.lua", UIEnums.Context.Blurb )

		elseif arg3 == UIEnums.ShareOptions.Blurb then

			Amax.CreateBlurb(UIEnums.SocialNetworkingItemType.OwnMod, 2, index-1)

		end
	
	end
end

function FrameUpdate( time )
	if GUI.finished == true then
		return
	end
	
	if GUI.CurrentStage == GUI.Stage.Loadout then
		local selection = UIButtons.GetSelection( GUI.LoadoutsID )
		if selection ~= GUI.CurrentSelection then
			GUI.CurrentSelection = selection

			RefreshPreview( selection )
		end

	elseif GUI.CurrentStage == GUI.Stage.Slot then
		if GUI.CurrentSlot ~= GUI.CurrentSelection then
			GUI.CurrentSelection = GUI.CurrentSlot

			if GUI.CurrentSlot <= #GUI.SlotGadgets then
				RefreshAbilities( GUI.CurrentSlot )
			end
		end
	
	elseif GUI.CurrentStage == GUI.Stage.Ability then
		local selection = UIButtons.GetSelection( GUI.AbilitiesID )
		if selection ~= GUI.CurrentSelection then
			GUI.CurrentSelection = selection
			
			RefreshAbility( selection )
		end
	end
end

function EnterEnd()
	if UIGlobals.Multiplayer.InLobby == true then
		Amax.ChangeUiCamera( UIGlobals.CameraNames.MpLobby, UIGlobals.CameraLerpTime, 0 )
	end

	RestoreInfoLine()
end

function End()
	UIGlobals.Multiplayer.ModShopActive = false
end


function Modified()
	local result = false

	for i, v in pairs( GUI.BackupAbilities ) do
		if v.type ~= GUI.ChoosenAbilities[i].type then
			result = true
			break
		end
	end

	return result
end

function MoveLeft()
	if GUI.CurrentStage == GUI.Stage.Slot then
		if GUI.CurrentSlot > 1 then
			PlaySfxLeft()

			UIButtons.PrivateTimeLineActive( GUI.SlotGadgets[GUI.CurrentSlot], "selected", false )
			GUI.CurrentSlot = GUI.CurrentSlot - 1
			UIButtons.PrivateTimeLineActive( GUI.SlotGadgets[GUI.CurrentSlot], "selected", true )
		end
	end
end

function MoveRight()
	if GUI.CurrentStage == GUI.Stage.Slot then
		if GUI.CurrentSlot < #GUI.SlotGadgets then
			PlaySfxRight()
			
			UIButtons.PrivateTimeLineActive( GUI.SlotGadgets[GUI.CurrentSlot], "selected", false )
			GUI.CurrentSlot = GUI.CurrentSlot + 1
			UIButtons.PrivateTimeLineActive( GUI.SlotGadgets[GUI.CurrentSlot], "selected", true )
		end
	end
end

function RestoreLoadout()
	GUI.ChoosenAbilities = CloneTable( GUI.BackupAbilities )

	for i = 1, 3 do
		if GUI.ChoosenAbilities[i].empty == true then
			UIShape.ChangeSceneName( GUI.SlotGadgets[i], "common_icons" )
			UIShape.ChangeObjectName( GUI.SlotGadgets[i], "style_mixed" )
			UIButtons.ChangeColour( GUI.SlotGadgets[i], "main_black" )
		else
			UIShape.ChangeSceneName( GUI.SlotGadgets[i], "ability_icons" )
			UIShape.ChangeObjectName( GUI.SlotGadgets[i], GUI.ChoosenAbilities[i].icon_name )
			UIButtons.ChangeColour( GUI.SlotGadgets[i], UIGlobals.CategoryColour[GUI.ChoosenAbilities[i].pack] )
		end
	end
end


function RefreshPreview( index )

	GUI.LookingAtEmptyLoadout = false

	for i, v in ipairs( GUI.Loadouts[index].slots ) do

		local name_id = UIButtons.FindChildByName( GUI.PreviewGadgets[i], "name" )
		local desc_id = UIButtons.FindChildByName( GUI.PreviewGadgets[i], "desc" )
		local icon_id = UIButtons.FindChildByName( GUI.PreviewGadgets[i], "icon" )

		if v.empty == false then

			UIButtons.ChangeText( name_id, v.name )
			UIButtons.ChangeText( desc_id, v.desc )
			UIButtons.ChangeColour( name_id, "Main_White" )

			UIShape.ChangeSceneName( icon_id, "ability_icons" )
			UIShape.ChangeObjectName( icon_id, v.icon_name )
			UIButtons.ChangeColour( icon_id, UIGlobals.CategoryColour[v.pack] )
		else

			GUI.LookingAtEmptyLoadout = true
	
			UIButtons.ChangeText( name_id, UIText.MP_EMPTY_SLOT )
			UIButtons.ChangeText( desc_id, UIText.CMN_NOWT )
			UIButtons.ChangeColour( name_id, "Main_Black" )

			UIShape.ChangeSceneName( icon_id, "common_icons" )
			UIShape.ChangeObjectName( icon_id, "style_mixed" )
			UIButtons.ChangeColour( icon_id, "Main_Black" )
		end
	end

	if GUI.LookingAtEmptyLoadout == true or UIGlobals.Multiplayer.InLobby == true or Amax.CanUseShare() == false then
		SetupInfoLine(UIText.INFO_A_SELECT, UIText.INFO_B_BACK)
	else
		SetupInfoLine(UIText.INFO_A_SELECT, UIText.INFO_B_BACK, "GAME_SHARE_BUTTON")
	end

end

function RefreshAbilities( index )
	-- create abilities menu
	GUI.Abilities = Multiplayer.GetAbilities( GUI.AbilityPacks[index].type )
	local new_map = Multiplayer.GetNewAbilitiesMap()

	for i, v in ipairs( GUI.Abilities ) do

		local icon_id = UIButtons.FindChildByName( GUI.AbilityGadgets[i], "ability_icon" )
		local rank_id = UIButtons.FindChildByName( GUI.AbilityGadgets[i], "unlock_rank" )
		local new_id  = UIButtons.FindChildByName( GUI.AbilityGadgets[i], "new" )

		if (Amax.IsGameModeRanked() == false and UIGlobals.Multiplayer.InLobby == true) or GUI.UnlockMap[v.type].unlocked == true then	
			UIButtons.ChangeText( rank_id, UIText.CMN_NOWT )
			--UIButtons.ChangeText( rank_id, "MPL_ABILITY_LOCKED" .. GUI.UnlockMap[v.type].level+1 ) -- +1 since level in range [0,49] but displayed [1,50]

			UIShape.ChangeSceneName( icon_id, "ability_icons" )
			UIShape.ChangeObjectName( icon_id, v.icon_name )
			UIButtons.ChangeColour( icon_id, UIGlobals.CategoryColour[GUI.AbilityPacks[index].type] )
			UIButtons.SetNodeItemLocked( GUI.AbilitiesID, i-1, false )

			-- show new
			UIButtons.SetActive( new_id, new_map[v.type] )
		else
			UIButtons.ChangeText( rank_id, "MPL_ABILITY_LOCKED" .. GUI.UnlockMap[v.type].level+1 ) -- +1 since level in range [0,49] but displayed [1,50]

			UIShape.ChangeSceneName( icon_id, "common_icons" )
			UIShape.ChangeObjectName( icon_id, "style_mixed" )
			UIButtons.ChangeColour( icon_id, "locked" )
			UIButtons.SetNodeItemLocked( GUI.AbilitiesID, i-1, true )

			-- hide new
			UIButtons.SetActive( new_id, false )
		end

		-- highlight current
		if GUI.ChoosenAbilities[GUI.CurrentSlot].type == v.type then
			RefreshAbility( i )
		end
	end
end


function RefreshAbility( index )
	local ability = GUI.Abilities[index]

	UIButtons.ChangeText( GUI.AbilityNameID, ability.name )
	UIButtons.ChangeText( GUI.AbilityDescID, ability.desc )

	-- remove the new
	UIButtons.SetActive( UIButtons.FindChildByName( GUI.AbilityGadgets[index], "new" ), false )
	Multiplayer.AbilityViewed( ability.type )
	
	--UIShape.ChangeSceneName( GUI.SlotGadgets[GUI.CurrentSlot], "ability_icons" )
	--UIShape.ChangeObjectName( GUI.SlotGadgets[GUI.CurrentSlot], GUI.Abilities[index].icon_name )
end


function SelectLoadout( index, active )
	GUI.CurrentSelection = -1

	UIButtons.SetSelected( GUI.LoadoutsID, not active )
	UIButtons.TimeLineActive( "show_abilities", active )

	for i, v in pairs( GUI.LoadoutGadgets ) do
		if GUI.LoadoutGadgets[index] == v then
			UIButtons.PrivateTimeLineActive( UIButtons.FindChildByName( v, "dummy_loadout" ), "move_up" .. index, active )
			UIButtons.PrivateTimeLineActive( v, "edit_loadout", active )

			UIButtons.SetActive( UIButtons.FindChildByName( v, "left_arrow" ), active )
			UIButtons.SetActive( UIButtons.FindChildByName( v, "right_arrow" ), active )

			-- force on when showing
			UIButtons.PrivateTimeLineActive( UIButtons.FindChildByName( v, "left_arrow" ), "gadget_activated", active, 1.0 )
			UIButtons.PrivateTimeLineActive( UIButtons.FindChildByName( v, "right_arrow" ), "gadget_activated", active, 1.0 )

			if active == true then
				GUI.SlotGadgets = {}
				for i = 1, 3 do
					GUI.SlotGadgets[i] = UIButtons.FindChildByName( v, "slot_" .. i )
					
					-- fill out loadout
					GUI.ChoosenAbilities[i] = GUI.Loadouts[index].slots[i]
				end
				-- activate current one
				GUI.CurrentSlot = 1

				-- make backup and add confirm button
				GUI.SlotGadgets[4] = UIButtons.FindChildByName( v, "confirm" )

				GUI.BackupAbilities = CloneTable( GUI.ChoosenAbilities )
			end
			
			UIButtons.PrivateTimeLineActive( GUI.SlotGadgets[GUI.CurrentSlot], "selected", active )
		else
			UIButtons.PrivateTimeLineActive( v, "move_off", active )
			UIButtons.PrivateTimeLineActive( UIButtons.FindChildByName( v, "dummy_loadout" ), "move_off", active )
		end
	end

	SetupInfoLine( UIText.INFO_A_SELECT, UIText.INFO_B_BACK )
end


function SelectSlot( index, active )

	for i, v in ipairs( GUI.Abilities ) do
		-- highlight current
		if GUI.ChoosenAbilities[GUI.CurrentSlot].type == v.type then
			UIButtons.SetSelection( GUI.AbilitiesID, i )
		end
	end

	local node_id = GUI.LoadoutGadgets[UIButtons.GetSelection( GUI.LoadoutsID )]

	GUI.CurrentSelection = -1
	UIButtons.SetSelected( GUI.AbilitiesID, active )

	UIButtons.PrivateTimeLineActive( UIButtons.FindChildByName( node_id, "slot_1" ), "select_slot", active )
	UIButtons.PrivateTimeLineActive( UIButtons.FindChildByName( node_id, "slot_2" ), "select_slot", active )
	UIButtons.PrivateTimeLineActive( UIButtons.FindChildByName( node_id, "slot_3" ), "select_slot", active )

	UIButtons.PrivateTimeLineActive( GUI.SlotGadgets[GUI.CurrentSlot], "selected", not active )

	-- show arrows
	UIButtons.SetActive( UIButtons.FindChildByName( node_id, "left_arrow" ), not active )
	UIButtons.SetActive( UIButtons.FindChildByName( node_id, "right_arrow" ), not active )
end


function SelectAbility( index )
	GUI.CurrentSelection = -1
	
	UIShape.ChangeSceneName( GUI.SlotGadgets[GUI.CurrentSlot], "ability_icons" )
	UIShape.ChangeObjectName( GUI.SlotGadgets[GUI.CurrentSlot], GUI.Abilities[index].icon_name )
	UIButtons.ChangeColour( GUI.SlotGadgets[GUI.CurrentSlot], UIGlobals.CategoryColour[GUI.Abilities[index].pack] )

	-- store ability
	GUI.ChoosenAbilities[GUI.CurrentSlot] = GUI.Abilities[index]

	UIButtons.PrivateTimeLineActive( GUI.SlotGadgets[GUI.CurrentSlot], "select_ability", true, 0 )

	-- auto move on if its new
	if GUI.NewLoadout then
		MoveRight()
	end

	-- update preview
	local name_id = UIButtons.FindChildByName( GUI.PreviewGadgets[GUI.CurrentSlot], "name" )
	local desc_id = UIButtons.FindChildByName( GUI.PreviewGadgets[GUI.CurrentSlot], "desc" )
	local icon_id = UIButtons.FindChildByName( GUI.PreviewGadgets[GUI.CurrentSlot], "icon" )

	UIButtons.ChangeText( name_id, GUI.Abilities[index].name )
	UIButtons.ChangeText( desc_id, GUI.Abilities[index].desc )
	UIButtons.ChangeColour( name_id, "Support_1" )

	UIShape.ChangeSceneName( icon_id, "ability_icons" )
	UIShape.ChangeObjectName( icon_id, GUI.Abilities[index].icon_name )
	UIButtons.ChangeColour( icon_id, UIGlobals.CategoryColour[GUI.Abilities[index].pack] )
end

function SetupLoadout( index, slots )
	local root_id = UIButtons.CloneXtGadgetByName( "Multiplayer\\ModShop\\MpModShop.lua", "node_loadout" )
	local name_id = UIButtons.FindChildByName( root_id, "loadout_name" )
	local index_id = UIButtons.FindChildByName( root_id, "loadout_index" )

	UIButtons.ChangeText( index_id, "MPL_LOADOUT_INDEX" .. index )
	if UIGlobals.Multiplayer.InLobby == true then
		UIButtons.ChangeText( name_id, "MPL_LOADOUT_NAME_LOBBY" .. index )
	else
		UIButtons.ChangeText( name_id, "MPL_LOADOUT_NAME" .. index )
	end

	-- setup slots
	for i, v in ipairs( slots ) do
		if v.empty == false then
			local icon_id = UIButtons.FindChildByName( root_id, "slot_" .. i )
	
			UIShape.ChangeSceneName( icon_id, "ability_icons" )
			UIShape.ChangeObjectName( icon_id, v.icon_name )
			UIButtons.ChangeColour( icon_id, UIGlobals.CategoryColour[v.pack] )
		end
	end

	return root_id
end


function SetupCategory( name, icon, colour )
	local root_id = UIButtons.CloneXtGadgetByName( "Multiplayer\\ModShop\\MpModShop.lua", "node_category" )

	local name_id = UIButtons.FindChildByName( root_id, "category_name" )
	local icon_id = UIButtons.FindChildByName( root_id, "category_icon" )

	UIButtons.ChangeText( name_id, name )
	UIButtons.ChangeColour( icon_id, colour )
	UIShape.ChangeObjectName( icon_id, icon )

	return root_id
end


function SetupPreviewItem( name, desc, icon )
	local root_id = UIButtons.CloneXtGadgetByName( "Multiplayer\\ModShop\\MpModShop.lua", "node_preview_ability" )

	local name_id = UIButtons.FindChildByName( root_id, "name" )
	local desc_id = UIButtons.FindChildByName( root_id, "desc" )
	local icon_id = UIButtons.FindChildByName( root_id, "icon" )

	UIButtons.ChangeText( name_id, name )
	UIButtons.ChangeText( desc_id, desc )
	UIShape.ChangeObjectName( icon_id, icon )

	return root_id
end


function SetupAbility( icon )
	local root_id = UIButtons.CloneXtGadgetByName( "Multiplayer\\ModShop\\MpModShop.lua", "node_ability" )
	UIShape.ChangeObjectName( UIButtons.FindChildByName( root_id, "ability_icon" ), icon )

	return root_id
end


function RenameLoadout( index, arg )
	UIHardware.StartKeyboard( arg,  "MPL_LOADOUT_NAME" .. index - 1, UIText.CMN_NOWT, UIText.MP_LOADOUT_NAME_HELP, 16, UIEnums.XboxKeyboardType.GamertagHighlight )
	UIGlobals.MpModShopKeyboardCheck = UIButtons.GetSelection( GUI.LoadoutsID ) - 1
end
