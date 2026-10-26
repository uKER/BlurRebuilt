GUI =
{
	finished = false,
}

function Init()
	AddSCUI_Elements()
end

function PostInit()
	-- disable avatar dialogue at start
	UIButtons.SetActive( SCUI.name_to_id[ "avatar_parent" ], false, true )
end

function StartLoop( time )
	--
end

function MessageUpdate( message, arg1, arg2, arg3, arg4, arg5 )
	if SCUI_MessageUpdate( message, arg1, arg2 ) == true then
		return
	end
	
	if GUI.finished == true then
		return
	end
	
	if message == UIEnums.GameMessage.Dialogue then
		local avatar_id = arg1
		local loc_id = arg2
		
		if avatar_id == nil or loc_id == nil then
			return
		end
		
		AvatarTexture( SCUI.name_to_id[ "avatar_image" ], avatar_id, true )
		Amax.ChangeText( SCUI.name_to_id[ "avatar_text" ], loc_id )
		
		UIButtons.TimeLineActive( "avatar_dialogue", true, 0 )
	end
end

function FrameUpdate( time )
	--
end

function EndLoop( time )
	--
end

function End()
	--
end
