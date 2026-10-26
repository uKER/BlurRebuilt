-- TODO - REMOVE ME - NOT USED ANYMORE
GUI =
{
	finished = false,	

	CanExit = function( _GUI )
		return false
	end,
}

function Init()
	UIGlobals.FileParams = {  }
	
	AddSCUI_Elements()
	
	StoreInfoLine()
	SetupInfoLine()
	
	if CanSave( Profile.GetPrimaryPad(), true ) == true then
		UIGlobals.FileParams.FinishedSuccess=false
		StartAsyncLoadPhotos()
	end
end

function PostInit()

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
	
	if message == UIEnums.Message.MenuBack and arg2 == true then 
		PlaySfxBack()
		GoScreen("Photos\\Photos.lua" )
	end
end

function FrameUpdate( time )

	-- enumerated load finished
	if UIGlobals.FileParams.FinishedSuccess == true then
		GUIBank.photo_viewing_download=false
		GoScreen( "Photos\\ViewPhoto.lua" )
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
