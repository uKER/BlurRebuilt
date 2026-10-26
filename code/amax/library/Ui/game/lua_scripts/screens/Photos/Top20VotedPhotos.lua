GUI =
{
	finished = false,	

	PhotoTaskLeaderboard = 0,
	PhotoTaskLeaderboardFinished = 1,
	PhotoTaskGetMetaData = 2,
	PhotoTaskDownload = 3,
	PhotoTaskFinished = 4,

	PhotoTask = 0,

	doing_photo_task = false,
	CanExit = function( _GUI )
		return false
	end,
	
	leaderboardIndex = 0,
	
	ids = {  },
	node_list_id = -1,
	
	num_items = 20,
}

function Init()
	AddSCUI_Elements()
	
	StoreInfoLine()
	SetupInfoLine( UIText.INFO_OPEN_A, UIText.INFO_B_BACK )
	
	GUI.node_list_id = SCUI.name_to_id[ "node_list" ]
end

function PostInit()

	GUI.PhotoTask = GUI.PhotoTaskLeaderboard

	LSP.Enable(true)
	LSP.SetUserIndex(Profile.GetPrimaryPad())

	LSP.GetTop20Leaderboard(1,true)

	GUI.doing_photo_task=true
	
	for i = 1, GUI.num_items, 1 do
		-- clone new node
		local node_id = UIButtons.CloneXtGadgetByName( "Photos\\Top20VotedPhotos.lua", "_node" )
		UIButtons.AddListItem( GUI.node_list_id, node_id, i - 1 )
		
		GUI.ids[ #GUI.ids + 1 ] = 
		{
			node = node_id,
			text = UIButtons.FindChildByName( node_id, "_text" ),
		}
	end
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
		GoScreen("Photos\\Photos.lua" )
	elseif message == UIEnums.Message.MenuNext and arg2 == true then 

		GUI.leaderboardIndex = UIButtons.GetSelection( GUI.node_list_id )

		GUI.doing_photo_task=true

		LSP.GetMetaDataFromLeaderboardIndex(GUI.leaderboardIndex, true)

		GUI.PhotoTask = GUI.PhotoTaskGetMetaData

	elseif message == UIEnums.Message.ButtonX and arg2 == true then

		local leaderboardIndex = UIButtons.GetSelection( GUI.node_list_id )	
		Amax.ViewPhotoTop20GamerCard( Profile.GetPrimaryPad(), leaderboardIndex, true )

	end
end

function FrameUpdate( time )
	if GUI.doing_photo_task == true then

		still_pumping,error = LSP.PumpCurrentTask()

		if still_pumping == false then

			GUI.PhotoTask = GUI.PhotoTask + 1

			if GUI.PhotoTask == GUI.PhotoTaskLeaderboardFinished then

				GUI.doing_photo_task = false

				local searches_table = LSP.GetReturnedLeaderboardData(true)

				if LSP.IsStatsConnected() == false or #searches_table == 0 then
					SetupCustomPopup( UIEnums.CustomPopups.ContentServerLeaderboardError )
				else
					for i, v in ipairs( searches_table ) do
						UIButtons.SetNodeItemLocked( GUI.node_list_id, i - 1, false )
						UIButtons.ChangeText( GUI.ids[ i ].text, "GAME_PHOTO_LEADERBOARD_VOTE_" .. v.index )
					end
					
					for i = #searches_table + 1, #GUI.ids, 1 do
						UIButtons.SetNodeItemLocked( GUI.node_list_id, i - 1, true )
					end

					if NetServices.CanViewGamerCard( Profile.GetPrimaryPad() ) == true then
						SetupInfoLine( UIText.INFO_X_SHOW_GAMERCARD, UIText.INFO_OPEN_A, UIText.INFO_B_BACK )
					end
				
				end

			elseif GUI.PhotoTask == GUI.PhotoTaskGetMetaData then
				

			elseif GUI.PhotoTask == GUI.PhotoTaskDownload then

				if error == 0 then
					SetupCustomPopup( UIEnums.CustomPopups.ContentServerData )
					-- the meta data is put into slot 0 as its the only one we get
					LSP.DownloadPhoto(0)
				else
					GUI.doing_photo_task = false
					GUI.PhotoTask = GUI.PhotoTaskFinished
					SetupCustomPopup( UIEnums.CustomPopups.ContentServerGeneralError )
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

					GUIBank.photo_viewing_from = GUIBank.PhotoViewFrom.Top20Voted

					GUIBank.photo_viewing_download=true
					GoScreen( "Photos\\ViewPhoto.lua" )
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