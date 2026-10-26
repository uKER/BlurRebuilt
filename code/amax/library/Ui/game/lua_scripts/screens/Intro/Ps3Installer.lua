GUI =
{
	finished = false,
}

function Ps3Installer_ChangePicture( id )
	print( Ps3Installer_Images[ Ps3Installer_ImageIndex ] )
	-- Change
	UIButtons.ChangeTexture( { filename = Ps3Installer_Images[ Ps3Installer_ImageIndex ] }, 0,  id )
	-- Inc
	Ps3Installer_ImageIndex = Ps3Installer_ImageIndex + 1
	if Ps3Installer_ImageIndex == #Ps3Installer_Images then
		Ps3Installer_ImageIndex = 1
	end
end

function Init()
	AddSCUI_Elements( )

	Ps3Installer_ImageIndex = 1
	Ps3Installer_Images = 
	{ 
		"install_00",
		"install_01",
		"install_02",
		"install_03",
		"install_04",
		"install_05",
		"install_06",
		"install_07",
		"install_08",
		"install_09",
		"install_10",
		"install_11",
		"install_12",
		"install_13",
	}
		
	--[[
	Ps3Installer_Images = { }

	-- Build a list of valid routes
	local citys = GameData.GetCities()
	for i, v in pairs( citys ) do
		local routes = GameData.GetRoutes(v.id)
		for j, k in pairs( routes ) do
			if IsTable( k ) == true then
				if IsString( k.debug_tag ) == true then
					-- Exclude object test and route 0
					if string.find( k.debug_tag, "OBJ") == nil and string.find( k.debug_tag, "_0") == nil then
						Ps3Installer_Images[ #Ps3Installer_Images+1 ] = k.debug_tag
					end
				end
			end
		end
	end
	]]--
	
	StopFrontendMusic()

	UIScreen.SetScreenTimers( 0.3, 0.3 )
	UIButtons.CloneXtGadgetByName( "SCUIBank", "Gfx_FadeUpDown" )
	
	-- Progress bar
	GUI.progress_bar_bra_id = UIButtons.CloneXtGadgetByName( "SCUIBank", "Bra_Installer" )
	GUI.progress_bar_id = UIButtons.FindChildByName( GUI.progress_bar_bra_id, "Gfx_ProgressBar" )
	
	local message_id =    UIButtons.FindChildByName( GUI.progress_bar_bra_id, "SavingContentMessage" )
	local message_bar_mid_id = UIButtons.FindChildByName( GUI.progress_bar_bra_id, "MessageBarMid" )
			
	local sx, sy = UIButtons.GetSize( message_id )
	local num_lines = UIButtons.GetStaticTextHeight( message_id ) / sy
	UIButtons.ChangeScale( message_bar_mid_id, 1.0, num_lines )		
	UIButtons.ChangeScale( GUI.progress_bar_id, 0, 1.0 )
end

function StartLoop( time )
	--
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end
end

function FrameUpdate( time )
	PS3_CheckForInstallingMessage()
	
	UIButtons.ChangeScale( GUI.progress_bar_id, Amax.Ps3InstallerProgress(), 1.0 )
	
	if UIGlobals.Ps3Installing ~= true then
		UIButtons.SetActive(  GUI.progress_bar_id, false, true )
		Amax.InstallTrophies()
		Amax.SendMessage( UIEnums.GameFlowMessage.LoadUIScene )
		-- Run legal until Showroom has loaded
		GoScreen( "Intro\\Legal.lua")
	end		
end

function Render()
	--
end

function EndLoop( time )
	--
end

function End()

	Ps3Installer_ImageIndex = nil
	Ps3Installer_Images = nil
	Ps3Installer_ChangePicture = nil
end
