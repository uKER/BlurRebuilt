GUI =
{
	finished = false,
	started = false,
	delay = 0,
	timeout = 0,
	failed  = false,
	complete = false,
	
	carousel_branch = "Group",

	CanExit = function( _GUI )
		return false
	end,
}

function Init()
	StoreInfoLine()
	SetupInfoLine()
	
	UIScreen.SetScreenTimers( 0.3, 0.3, UIScreen.Context() )

	-- no game invites during lsp server connection
	net_EnableGlobalUpdate( false )

	AddSCUI_Elements( )
	
	GUI.connecting_online_txt = SCUI.name_to_id[ "Connecting_Online" ]
	GUI.connecting_offline_txt = SCUI.name_to_id[ "Connecting_Offline" ]
	GUI.failed_online_txt = SCUI.name_to_id[ "Failed_Online" ]
	GUI.failed_offline_txt = SCUI.name_to_id[ "Failed_Offline" ]

	SetCorrectText( )
end

function PostInit()
end

function StartLoop( time )
	--
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end
	
	if GUI.finished == true then
		return
	end
	
	if message == UIEnums.Message.MenuNext and arg2 == true and GUI.complete == true then	
		PlaySfxNext( )
		MoveOn( )
	end
end

function FrameUpdate( time )
	if GUI.finished == false then
		
		SetCorrectText( )
	
		if GUI.complete == true then
			GUI.timeout = GUI.timeout + time
			if GUI.timeout > 5 then
				MoveOn( )
			end
		else
			if GUI.started == false then
				GUI.delay = GUI.delay + time
				if GUI.delay >= 1 then
					Begin( )	
				end
			else
				Monitor( )
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
	UIGlobals.server_connection.active = false
end

function Begin( )

	-- get current connection status.
	local state = UIEnums.Network.Router.eConnectionStateNotActive
	local result = UIEnums.Network.Router.eConnectionResultNone
	local online = false
	state, result, online = NetServices.ConnectionStatus()
	
	-- we can keep the online connection active if we only want an offline connection, no point disconnecting.
	if state == UIEnums.Network.Router.eConnectionStateComplete and online == true and UIGlobals.server_connection.online_mode == false then
		CompleteConnect( )
	else
		GUI.started = true
		net_FlushEverything( true ) -- clean networking but ignore current rich presence states
		if NetServices.Connect( UIGlobals.server_connection.online_mode ) == false then
			FailConnect( )
		end
	end
end

function Monitor( )
	-- get current connection status.
	local state = UIEnums.Network.Router.eConnectionStateNotActive
	local result = UIEnums.Network.Router.eConnectionResultNone
	local online = false
	state, result, online = NetServices.ConnectionStatus()	
	
	if state == UIEnums.Network.Router.eConnectionStateComplete then
		CompleteConnect( )
	elseif state == UIEnums.Network.Router.eConnectionStateFailed or state == UIEnums.Network.Router.eConnectionStateNotActive then
		if result == UIEnums.Network.Router.eConnectionResultNoServiceProvider then
			-- error would have been shown by the ps3 overlay, no need to show our own..
			GUI.failed = true
			GUI.complete = true
			net_CloseServiceConnection( )
			MoveOn( )
		elseif result == UIEnums.Network.Router.eConnectionResultInvalidUserName then
			-- todo show correct message for pc
			FailConnect( )
		elseif result == UIEnums.Network.Router.eConnectionResultInvalidPassword then
			-- todo show correct message for pc
			FailConnect( )
		else
			FailConnect( )
		end
	end
end

function MoveOn( )
	net_EnableGlobalUpdate( true )
	
	if GUI.failed == false then
		GoScreen( GetStoredScreen( UIEnums.ScreenStorage.FE_NEXT ) )
	else
		GoScreen( GetStoredScreen( UIEnums.ScreenStorage.FE_BACK ) )
	end
end

function SetCorrectText( )
	RemoveAllText( )
	if GUI.failed == true then
		if UIGlobals.server_connection.online_mode == true then	
			UIButtons.SetActive( GUI.failed_online_txt, true )
		else
			UIButtons.SetActive( GUI.failed_offline_txt, true )
		end
	else
		if UIGlobals.server_connection.online_mode == true then	
			UIButtons.SetActive( GUI.connecting_online_txt, true )
		else
			UIButtons.SetActive( GUI.connecting_offline_txt, true )
		end
	end
end

function RemoveAllText( )
	UIButtons.SetActive( GUI.connecting_online_txt, false )
	UIButtons.SetActive( GUI.connecting_offline_txt, false )
	UIButtons.SetActive( GUI.failed_online_txt, false )
	UIButtons.SetActive( GUI.failed_offline_txt, false )
end

function FailConnect( )
	GUI.failed = true
	GUI.complete = true
	net_CloseServiceConnection( )
	SetCorrectText( )
end

function CompleteConnect( )

	-- on ps3 and pc we re read the primary players profile since the code does not handle going to an online profile from an offline one.
	-- whould always have a valid offline profile on these platforms.
	if UIEnums.CurrentPlatform == UIEnums.Platform.PS3 or UIEnums.CurrentPlatform == UIEnums.Platform.PC then
		Profile.ReadPadProfile( UIGlobals.ProfilePressedStart )
	end

	-- start the async bandwidth evaluation.
	NetServices.StartBandwidthEvaluation( )

	GUI.failed = false
	GUI.complete = true
	MoveOn( )
end
