GUI =
{
	finished = false,	
	carousel_branch = "Photos",
						
	unhide_carousel = true,
						
	CanExit = function( _GUI )
		return true
	end,
}

function Init()
	AddSCUI_Elements()
	Amax.ChangeUiCamera( "Sp_2", UIGlobals.CameraLerpTime, 0 )

	CarouselApp_SetScreenTimers()
		
	StoreInfoLine()
	SetupMenuOptions()
	
	LSP.SetInPhotoUI( true )
	LSP.Enable( true )
	
	GUI.MenuId = SCUI.name_to_id[ "node_list" ]
	
	local options = 
	{
		{ value = 0,	text = UIText.RBU_PHOTO_LOCAL_PHOTOS,		icon_name = "Local_Photos", help_text = UIText.SP_PHOTO_LOCAL_PHOTOS_HELP },
		{ value = 1,	text = UIText.RBU_PHOTO_MY_ONLINE_PHOTOS,	icon_name = "Online_Photos", help_text = UIText.SP_PHOTO_ONLINE_PHOTOS_HELP },
		--{ value = 2,	text = UIText.RBU_PHOTO_TOP_20_VOTED,		icon_name = "Highest_Voted", },
		--{ value = 3,	text = UIText.RBU_PHOTO_TOP_20_DOWNLOADED,	icon_name = "Most_downloaded", },
		{ value = 4,	text = UIText.RBU_PHOTO_SEARCH,				icon_name = "Search_Online", help_text = UIText.SP_PHOTO_SEARCH_ONLINE_PHOTOS_HELP },
	}
	
	if CanSave( Profile.GetPrimaryPad(), true ) == false then
		table.remove( options, 1 )
	end
	
	for i, v in ipairs( options ) do
		local node_id = UIButtons.CloneXtGadgetByName( "Photos\\Photos.lua", "_node" )
		
		local icon_id = UIButtons.FindChildByName( node_id, "_icon" )
		local text_id = UIButtons.FindChildByName( node_id, "_text" )
		local help_text_id = UIButtons.FindChildByName( node_id, "_help_text" )
		
		UIShape.ChangeObjectName( icon_id, v.icon_name )
		UIButtons.ChangeText( text_id, v.text )
		UIButtons.ChangeText( help_text_id, v.help_text )
		
		UIButtons.AddListItem( GUI.MenuId, node_id, v.value )
	end
	
	SetupScreenTitle( UIText.RBU_PHOTOS, SCUI.name_to_id["screen"], "photo" )
end

function PostInit()
	UIButtons.TimeLineActive( "Hide_Carousel", true )
	GUI.bottom_text_id = SetupBottomHelpBar( UIText.HLP_PHOTO, SCUI.name_to_id["screen"] )
end

function StartLoop( time )
	--
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if GUI.finished == true then
		return
	end

	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end
	
	if message == UIEnums.Message.MenuBack then
	
		GUI.unhide_carousel = true
	
	elseif message == UIEnums.Message.MenuNext and arg2 == true then
		PlaySfxNext( )	
		
		GUI.unhide_carousel = false	
		
		local menu_option  = UIButtons.GetSelection( GUI.MenuId )

		GUIBank.photo_viewing_from=-1

		if menu_option == 0 then
			StartAsyncLoadPhotos()
		elseif menu_option == 1 then
			if Profile.PadProfileOnline( Profile.GetPrimaryPad() ) == false then
				SetupCustomPopup( UIEnums.CustomPopups.MultiplayerOnlineConnectionLost )
			elseif LSP.IsConnected() == false then
				SetupCustomPopup( UIEnums.CustomPopups.ContentServerGeneralError )
			else
				UIGlobals.PhotoDoMyLIst = true
				GoScreen( "Photos\\MyOnlinePhotos.lua" )
			end
		elseif menu_option == 2 then
			if LSP.IsConnected() == false then
				SetupCustomPopup( UIEnums.CustomPopups.ContentServerGeneralError )
			else
				GoScreen( "Photos\\Top20VotedPhotos.lua" )
			end
		elseif menu_option == 3 then
			if LSP.IsConnected() == false then
				SetupCustomPopup( UIEnums.CustomPopups.ContentServerGeneralError )
			else
				GoScreen( "Photos\\Top20DownloadedPhotos.lua" )
			end
		elseif menu_option == 4 then
			if Profile.PadProfileOnline( Profile.GetPrimaryPad() ) == false then
				SetupCustomPopup( UIEnums.CustomPopups.MultiplayerOnlineConnectionLost )
			elseif LSP.IsConnected() == false then
				SetupCustomPopup( UIEnums.CustomPopups.ContentServerGeneralError )
			else
				GoScreen( "Photos\\PhotoSearchParams.lua" )
			end
		end
	end
end

function FrameUpdate( time )

end

function Render()
	--
end

function EnterEnd()

	if GUI.unhide_carousel == true then 
		UIButtons.TimeLineActive( "Hide_Carousel", false )
	end
	
	RestoreInfoLine()
end

function EndLoop( time )
	--
end

function End()
end
