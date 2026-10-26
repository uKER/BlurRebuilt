GUI =
{
	finished = false,	

	PhotoTaskSearch = 0,
	PhotoTaskGetMetaData = 1,
	PhotoTaskDownload = 2,
	PhotoTaskFinished = 3,

	PhotoTask = 0,

	doing_photo_task = false,
	CanExit = function( _GUI )
		return false
	end,
	
	ids = {  },
	node_list_id = -1,
	
	num_items = 5,
	
	can_enter_fullscreen = false,
	
	preview = { },
	
	photo_select = 1,
	photo_preview = 2,	
	
	current_view = 1,
	
	timeout = 0,
}

function Init()
	AddSCUI_Elements()
	
	StoreInfoLine()
	SetupInfoLine( UIText.INFO_OPEN_A, UIText.INFO_B_BACK )
	
	GUI.node_list_id = SCUI.name_to_id[ "node_list" ]
end

function PostInit()
	for i = 1, GUI.num_items, 1 do
		-- clone new node
		local node_id = UIButtons.CloneXtGadgetByName( "SpSCUI", "_slot_selection_node" )
		UIButtons.AddListItem( GUI.node_list_id, node_id, i - 1 )
		
		GUI.ids[ #GUI.ids + 1 ] = 
		{
			node = node_id,
			slot = UIButtons.FindChildByName( node_id, "_text_slot" ),
			text = UIButtons.FindChildByName( node_id, "_text" ),
		}
	end
	
	if UIGlobals.PhotoDoMyLIst == true then
		GUI.PhotoTask = GUI.PhotoTaskSearch
		LSP.Enable(true)
		LSP.SetUserIndex(Profile.GetPrimaryPad())
		LSP.ListFilesByOwner()
		GUI.doing_photo_task=true
		
		GUI.timeout = 0
	else
		FillMenu()
	end
	
	SetupScreenTitle( UIText.RBU_PHOTO_MY_ONLINE_PHOTOS_PAGE_TITLE, SCUI.name_to_id["screen"], "online_photos", "fe_icons"  )
	
	SetupPreview()
	
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
	
		if GUI.current_view == GUI.photo_preview then
			HidePreview()
		elseif GUI.current_view == GUI.photo_select then
			PlaySfxBack()
			GoScreen("Photos\\Photos.lua" )
		end
		
	elseif message == UIEnums.Message.MenuNext and arg2 == true then 

		if GUI.current_view == GUI.photo_preview then
			PushScreen("Photos\\ViewPhoto.lua")
		elseif GUI.current_view == GUI.photo_select then
			local search_index = UIButtons.GetSelection( GUI.node_list_id )	

			GUI.doing_photo_task=true

			SetupCustomPopup( UIEnums.CustomPopups.ContentServerData )

			LSP.DownloadPhoto(search_index)

			GUI.PhotoTask = GUI.PhotoTaskDownload
			
			GUI.timeout = 0

		end

	end
end

function FrameUpdate( time )
	if GUI.doing_photo_task == true then

		still_pumping,error = LSP.PumpCurrentTask()

		if still_pumping == false then

			GUI.PhotoTask = GUI.PhotoTask + 1

			if GUI.PhotoTask == GUI.PhotoTaskGetMetaData then
				
				GUI.doing_photo_task = false

				if LSP.GetNumSearchResults() == 0 then
					
					if LSP.IsConnected() == true then
						SetupCustomPopup( UIEnums.CustomPopups.ContentServerNoUserPhotos )
					else
						SetupCustomPopup( UIEnums.CustomPopups.ContentServerGeneralError )
					end

				else
					FillMenu()
				end

			elseif GUI.PhotoTask == GUI.PhotoTaskFinished then

				GUI.doing_photo_task = false

				UIScreen.CancelPopup()

				if error ~= 0 then
					if error == 2006 then
						PopupSpawn( UIEnums.CustomPopups.ContentServerDownloadBandwidthError )
					else
						PopupSpawn( UIEnums.CustomPopups.ContentServerGeneralError )
					end
				else
	
					LSP.DecodeDownloadedPhoto()
					
					ShowPreview()

					GUIBank.photo_viewing_from = GUIBank.PhotoViewFrom.MyOnline

					GUIBank.photo_viewing_download=true
					
					GUI.can_enter_fullscreen = true
					
					UIButtons.TimeLineActive( "Photo_Loaded", true, 0 )
					--GoScreen( "Photos\\ViewPhoto.lua" )
				end
			end
		
		else
		
			GUI.timeout = GUI.timeout + time
			
			if GUI.timeout > 30 then
				
				GUI.PhotoTask = GUI.PhotoTaskFinished
				GUI.doing_photo_task = false
				UIScreen.CancelPopup()
				PopupSpawn( UIEnums.CustomPopups.ContentServerGeneralError )
				
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

function FillMenu()
	local searches_table = LSP.GetReturnedSearchData()
	
	for i, v in ipairs( searches_table ) do
		UIButtons.SetNodeItemLocked( GUI.node_list_id, i - 1, false )
		UIButtons.ChangeText( GUI.ids[ i ].slot, "GAME_PHOTO_SEARCH_NAME_" .. v.index )
		UIButtons.ChangeText( GUI.ids[ i ].text, "GAME_PHOTO_SEARCH_INFO_" .. v.index )
	end
	
	for i = #searches_table + 1, #GUI.ids, 1 do
		UIButtons.SetNodeItemLocked( GUI.node_list_id, i - 1, true )
	end
end

function SetupPreview()

	GUI.preview = CreatePhotoPreviewPage( SCUI.name_to_id["screen"] )
	UIButtons.SetActive( GUI.preview.node, false, true )
	
	if GUI.preview.background ~= nil then
		UIButtons.SetParent(SCUI.name_to_id["preview"], GUI.preview.background, UIEnums.Justify.MiddleCentre)
	end

end

function ShowPreview()

	GUI.current_view = GUI.photo_preview
	
	if GUI.preview.node ~= nil then
		UIButtons.SetActive( GUI.preview.node, true, true )
	end
	
	UIButtons.SetActive( SCUI.name_to_id["node_list"], false, true )
	
	local CurrentPictureIndex = (UIButtons.GetSelectionIndex( SCUI.name_to_id["node_list"] ))
	
	UIButtons.ChangeText( GUI.preview.bottom_text, "GAME_PHOTO_SEARCH_NAME_" .. CurrentPictureIndex )
	UIButtons.ChangeText( GUI.preview.top_text, "GAME_PHOTO_SEARCH_PREVIEW_TITLE_" .. CurrentPictureIndex )
	
	

end

function HidePreview()

	GUI.current_view = GUI.photo_select
	
	if GUI.preview.node ~= nil then
		UIButtons.SetActive( GUI.preview.node, false, true )
	end
	
	UIButtons.TimeLineActive( "Photo_Loaded", false, 0 )
	
	UIButtons.SetActive( SCUI.name_to_id["node_list"], true, true )	

end