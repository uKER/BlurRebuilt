GUI =
{
	enumerating = false,
	purchasing = false,
}

function Init()
	AddSCUI_Elements()
	
	if UIEnums.CurrentPlatform ~= UIEnums.Platform.Xenon then
		return
	end
	
	-- start the enumeration
	GUI.enumerating = Market.StartEnumeration( Profile.GetPrimaryPad() )
	
	Marketplace_SetIcons()
end

function StartLoop( time )
	--
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end
	
	if message == UIEnums.Message.ButtonB then
		GoScreen( "Main.lua" )
		return
	end
	
	-- this is just a test screen and there happens to be two bits of content available
	if GUI.enumerating == false then
		local num_offers = Market.GetNumOffers()
		
		if Profile.GuideActive() == false then
			if message == UIEnums.Message.ButtonX and num_offers > 0 then
				GUI.purchasing = Market.StartPurchase( Profile.GetPrimaryPad(), 0 )
			elseif message == UIEnums.Message.ButtonY and num_offers > 1 then
				GUI.purchasing = Market.StartPurchase( Profile.GetPrimaryPad(), 1 )
			end
		end
	end	
end

function FrameUpdate( time )
	if GUI.enumerating == true then
		-- continue the enumeration
		local finished = Market.ContinueEnumeration()
		
		if finished == true then
			GUI.enumerating = false
			-- TODO - add stuff somewhere
		end
	end
	
	if GUI.purchasing == true then
		-- continue the purchase
		local finished = Market.ContinuePurchase()
		
		if finished == true then
			GUI.purchasing = false
			-- TODO - something?
		end
	end
	
	Marketplace_SetIcons()
end

function EndLoop( time )
	--
end

function End()
	Market.Cleanup()
end

function Marketplace_SetIcons()
	UIButtons.SetActive( SCUI.name_to_id[ "spinner" ], GUI.enumerating == true or GUI.purchasing == true )		
	UIButtons.SetActive( SCUI.name_to_id[ "spinner_download" ], Market.Downloading() == true )
end
