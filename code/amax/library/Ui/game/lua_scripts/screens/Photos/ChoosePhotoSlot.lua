GUI =
{
	finished = false,	

	PhotoTaskSearch = 0,
	PhotoTaskUpload = 1,
	PhotoTaskFinished = 2,

	PhotoTask = 0,

	doing_photo_task = false,
	CanExit = function( _GUI )
		return false
	end,

	slot_empty = {},

	overwriting=false,
	
	ids = {  },
	node_list_id = -1,
	
	num_items = 10,

}

function Init()
	AddSCUI_Elements()
	
	StoreInfoLine()
	SetupMenuOptions()
	

	GUI.node_list_id = SCUI.name_to_id[ "node_list" ]
	
	UIGlobals.PhotoSlotContext=UIScreen.Context()
		
end

function PostInit()

	GUI.PhotoTask = GUI.PhotoTaskSearch

	LSP.Enable(true)
	LSP.SetUserIndex(Profile.GetPrimaryPad())

	LSP.ListFilesByOwner()

	GUI.doing_photo_task=true
	
	for i = 1, GUI.num_items, 1 do
		-- clone new node
		local node_id = UIButtons.CloneXtGadgetByName( "SpSCUI", "_slot_selection_node" )
		UIButtons.AddListItem( GUI.node_list_id, node_id, i )
		
		GUI.ids[ #GUI.ids + 1 ] = 
		{
			node = node_id,
			slot = UIButtons.FindChildByName( node_id, "_text_slot" ),
			text = UIButtons.FindChildByName( node_id, "_text" ),
		}
	end
	
	UIScreen.BlockInputToContext( true )
end

function StartLoop( time )
	--
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if GUI.doing_photo_task == true then
		return
	end

	if GUI.finished == true then
		return
	end

	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end
	
	if message == UIEnums.Message.MenuBack and arg2 == true then 
		PlaySfxBack()
		PopScreen()
	elseif message == UIEnums.Message.MenuNext and arg2 == true then 

		local allowOnline = true
		if Profile.PadProfileOnline( Profile.GetPrimaryPad() ) == true then
			allowOnline = not NetServices.UserHasAgeRestriction()
		end

		if allowOnline == true then
			local slot = UIButtons.GetSelection( GUI.node_list_id )	

			if GUI.slot_empty[slot] == true then
				GUI.overwriting=false
				DoUpload(slot, GUI)
			else
				GUI.overwriting=true
				SetupCustomPopup( UIEnums.CustomPopups.OverwritePhotoServerSlot )
			end
		else
			SetupCustomPopup( UIEnums.CustomPopups.FailedAgeCheck )
		end

	end
end

function FrameUpdate( time )
	if GUI.doing_photo_task == true then

		still_pumping,error = LSP.PumpCurrentTask()

		if still_pumping == false then

			GUI.PhotoTask = GUI.PhotoTask + 1

			if GUI.PhotoTask == GUI.PhotoTaskUpload then
				
				GUI.doing_photo_task = false

				local searches_table = LSP.GetReturnedSlotData()
				
				for i, v in ipairs( searches_table ) do
					UIButtons.SetNodeItemLocked( GUI.node_list_id, i - 1, false )
					UIButtons.ChangeText( GUI.ids[ i ].slot, "GAME_PHOTO_SLOT_NAME_" .. v.index-1 )					
					UIButtons.ChangeText( GUI.ids[ i ].text, "GAME_PHOTO_SLOT_INFO_" .. v.index )
					GUI.slot_empty[i] = v.empty
				end
				
				for i = #searches_table + 1, #GUI.ids, 1 do
					UIButtons.SetNodeItemLocked( GUI.node_list_id, i - 1, true )
				end

			elseif GUI.PhotoTask == GUI.PhotoTaskFinished then

				GUI.doing_photo_task = false

				if error ~= 0 then
					if error == 2003 then
						PopupSpawn( UIEnums.CustomPopups.ContentServerUploadBandwidthError )
					else
						if Profile.PadProfileOnline( Profile.GetPrimaryPad() ) == false then
							if net_CanReconnectToDemonware() == false then
								SetupCustomPopup( UIEnums.CustomPopups.MultiplayerOnlineConnectionLost )
							else
								net_StartServiceConnection(true, nil, false)
							end
						else
							PopupSpawn( UIEnums.CustomPopups.ContentServerGeneralError )
						end
					end
				else
					UIScreen.CancelPopup()
					PopScreen()
				end
			end
		end
	end
end

function Render()
	--
end

function EnterEnd()
	RestoreInfoLine()
end

function EndLoop( time )
	--
end

function End()
	--
end

function DoUpload(slot,localGUI)

	localGUI.doing_photo_task=true

	-- upload photo
	LSP.Enable( true )
	LSP.SetFileName( "Photo.jpg" )
	LSP.SetFileSlot( slot )
	LSP.SetUserIndex( Profile.GetPrimaryPad() )
	
	local ok=LSP.UploadCurrentPhoto()

	if ok == true then
		if localGUI.overwriting == true then
			PopupSpawn( UIEnums.CustomPopups.ContentServerData )
		else
			SetupCustomPopup( UIEnums.CustomPopups.ContentServerData )
		end
		
		localGUI.PhotoTask = localGUI.PhotoTaskUpload
	else
		if localGUI.overwriting == true then
			PopupSpawn( UIEnums.CustomPopups.ContentServerCantUpload )
		else
			SetupCustomPopup( UIEnums.CustomPopups.ContentServerCantUpload )
		end
		
		localGUI.doing_photo_task = false
		localGUI.PhotoTask = localGUI.PhotoTaskFinished
	end

end