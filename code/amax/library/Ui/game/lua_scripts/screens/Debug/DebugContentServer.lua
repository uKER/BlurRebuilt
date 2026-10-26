GUI =
{
	finished = false,
	Pumping = false,

	TaskType = "Nothing",
}

function Init()
	
	AddSCUI_Elements( )

	NetServices.Disconnect()
	NetServices.Connect( true )
end

function StartLoop( time )
	--
end

function MessageUpdate( message, arg1, arg2 )
	if SCUI_MessageUpdate( message, arg1, arg2 ) == true then
		return
	end

	if GUI.finished == true then
		return
	end

	if message == UIEnums.Message.MenuBack and arg2 == true then
		GoScreen("Intro\\StartScreen.lua")
	end
	
	if net_RouterAvailable( true ) then
		if message == UIEnums.Message.MenuNext and arg2 == true then
			
			GUI.TaskType="Uploading"

			LSP.SetFileName("Amax1.bin")
			LSP.SetTags(3,1,2,3)
			LSP.SetUserIndex(0)
			GUI.Pumping = LSP.UploadData()

		elseif message == UIEnums.Message.ButtonX and arg2 == true then

			GUI.TaskType="Downloading"

			LSP.SetFileName("Amax.bin")
			LSP.SetUserIndex(0)
			GUI.Pumping = LSP.DownloadData()

		elseif message == UIEnums.Message.ButtonY and arg2 == true then

			GUI.TaskType="Searching"

			LSP.SetUserIndex(0)
			GUI.Pumping = LSP.ListFileByOwner(0,false)

			LSP.SetTags(3,1,2,3)
			GUI.Pumping = LSP.SearchByTags()

		end
	end
end

function FrameUpdate( time )
	local error
	if GUI.Pumping == true then

		GUI.Pumping,error = LSP.PumpCurrentTask()

		if GUI.Pumping == false then
			if error == false then
				if GUI.TaskType=="Downloading" then

					LSP.Debug_PrintDownloadData()

				elseif GUI.TaskType=="Searching" then

					local search_table=LSP.GetSearchResults()
					show_table(search_table,true)

				end
			end
		end
	end
end

function Render()
	--
end

function EndLoop( time )
	--
end

function End()
	--
end