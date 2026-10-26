GUI =
{
	finished = false,
		
	CanExit = function( _GUI )
		return false
	end,
	
	is_online = false,
	voting = false,
	
	stars = {  },
	rating = 3,
}

function Init()
	AddSCUI_Elements()
	
	StoreInfoLine()
	SetupInfoLine( UIText.INFO_PHOTO_VOTE_A, UIText.INFO_B_BACK )
	
	for i = 1, 5, 1 do
		GUI.stars[ i ] = SCUI.name_to_id[ "star_" .. i ]
				
		if i > GUI.rating then
			UIButtons.ChangeColour( GUI.stars[ i ], "Main_Black" )
		end
	end
end

function StartLoop( time )
	--
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if GUI.voting == true then	
		return
	end

	if GUI.finished == true then
		return
	end
	
	if message == UIEnums.Message.ButtonLeft then
		if GUI.rating > 0 then
			UIButtons.ChangeColour( GUI.stars[ GUI.rating ], "Main_Black" )
			GUI.rating = GUI.rating - 1
		end
	
	elseif message == UIEnums.Message.ButtonRight then
		if GUI.rating < 5 then
			GUI.rating = GUI.rating + 1
			UIButtons.ChangeColour( GUI.stars[ GUI.rating ], "Support_4" )
		end
		
	elseif message == UIEnums.Message.MenuBack then
		GoScreen( "Photos\\ViewPhoto.lua" )

	elseif message == UIEnums.Message.MenuNext then
		local vote = GUI.rating * 2
		local is_ok_to_vote=LSP.Vote(vote)

		if is_ok_to_vote == true then
			GUI.voting=true
		else
			SetupCustomPopup( UIEnums.CustomPopups.AlreadyVoted )
		end

	end

end

function FrameUpdate( time )
	-- continue the upload
	if GUI.voting == true then	
		still_pumping, error = LSP.PumpCurrentTask()

		if still_pumping == false then
			GUI.voting = false

			if error == 0 then
				UIGlobals.FinishedVoting=false
				SetupCustomPopup( UIEnums.CustomPopups.ThankyouForVoting )
			else
				SetupCustomPopup( UIEnums.CustomPopups.ContentServerGeneralError )
			end
		end
	end

	if UIGlobals.FinishedVoting == true then
		StartAsyncSave()
		UIGlobals.FinishedVoting = false
		
		if GUIBank.photo_viewing_from == GUIBank.PhotoViewFrom.Top20Voted then
			GoScreen( "Photos\\Top20VotedPhotos.lua" )
		elseif GUIBank.photo_viewing_from == GUIBank.PhotoViewFrom.Top20Download then
			GoScreen( "Photos\\Top20DownloadedPhotos.lua" )
		elseif GUIBank.photo_viewing_from == GUIBank.PhotoViewFrom.Search then
			GoScreen( "Photos\\PhotoSearch.lua" )
		elseif GUIBank.photo_viewing_from == GUIBank.PhotoViewFrom.MyOnline then
			UIGlobals.PhotoDoMyLIst = false
			GoScreen( "Photos\\MyOnlinePhotos.lua" )
		else
			GoScreen( "Photos\\Photos.lua" )
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
