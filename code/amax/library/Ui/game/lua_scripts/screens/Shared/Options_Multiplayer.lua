GUI =
{
	finished = false,	
	carousel_branch = "Options",

	CanExit = function( _GUI )
		return false
	end,

	nat_data =
	{
		[UIEnums.NatType.Open]		= { text = UIText.MP_NAT_TYPE_OPEN,		colour = "Support_0", count = 3 },
		[UIEnums.NatType.Moderate]	= { text = UIText.MP_NAT_TYPE_MODERATE, colour = "Support_4", count = 2 },
		[UIEnums.NatType.Strict]	= { text = UIText.MP_NAT_TYPE_STRICT,	colour = "Support_3", count = 1 },
	},

	region_data =
	{
		[UIEnums.MatchingRegion.Any]			= UIText.MP_REGION_ANY,
		[UIEnums.MatchingRegion.Asia]			= UIText.MP_REGION_ASIA,
		[UIEnums.MatchingRegion.Australia]		= UIText.MP_REGION_AUSTRALIA,
		[UIEnums.MatchingRegion.Europe]			= UIText.MP_REGION_EUROPE,
		[UIEnums.MatchingRegion.NorthAmerica]	= UIText.MP_REGION_NORTH_AMERICA,
		[UIEnums.MatchingRegion.SouthAmerica]	= UIText.MP_REGION_SOUTH_AMERICA,
	},
}

function Init()
	AddSCUI_Elements()
	
	StoreInfoLine()

	GUI.region_mode_id = SCUI.name_to_id[ "region_rule_slider" ]
	UIButtons.AddItem( GUI.region_mode_id, UIEnums.MatchingRule.Any, UIText.MP_REGION_ANY, false )
	UIButtons.AddItem( GUI.region_mode_id, UIEnums.MatchingRule.Preferred, UIText.MP_REGION_PREFERRED, false )
	UIButtons.AddItem( GUI.region_mode_id, UIEnums.MatchingRule.Enforced, UIText.MP_REGION_REQUIRED, false )

	-- set current
	local rule = Amax.GetMultiplayerOptions()
	UIButtons.SetSelection( GUI.region_mode_id, rule )

	SetupInfoLineL( UIText.INFO_B_BACK )
end

function PostInit()
	-- system settings
	if net_RouterAvailable( true ) == true then
		local nat_type = NetServices.GetNatType()
		UIButtons.ChangeText( SCUI.name_to_id[ "nat_type" ], GUI.nat_data[nat_type].text )

		for i = 1, 3 do
			if i <= GUI.nat_data[nat_type].count then
				UIButtons.ChangeColour( SCUI.name_to_id[ "nat_gfx_" .. i ], GUI.nat_data[nat_type].colour )
			else
				UIButtons.ChangeColour( SCUI.name_to_id[ "nat_gfx_" .. i ], "Main_Black" )
			end
		end

		UIButtons.ChangeText( SCUI.name_to_id[ "region" ], GUI.region_data[NetServices.GetSystemRegion()] )
	else
		UIButtons.SetActive( SCUI.name_to_id[ "unavailible_msg" ], true )
		UIButtons.SetActive( SCUI.name_to_id[ "settings" ], false )
	end
end

function StartLoop( time )
	--
end

function MessageUpdate( message, arg1, arg2, arg3, arg4 )
	if SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4 ) == true then
		return
	end

	if message == UIEnums.Message.MenuBack and arg2 == true then
		
		-- update player options and network settings
		Amax.SetMultiplayerOptions( UIButtons.GetSelection( GUI.region_mode_id ) )
		NetServices.SetMatchingRegionInfo()

		PlaySfxBack()
		GoScreen("Shared\\Options.lua")
	end
end

function FrameUpdate( time )
	--
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
end
