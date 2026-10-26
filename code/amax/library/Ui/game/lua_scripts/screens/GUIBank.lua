-- screen sizes
Screen =
{
	-- normal edges
	top		= 0,
	bottom	= 480,
	left	= SCREEN_LEFT,
	right	= SCREEN_RIGHT,
	width	= SCREEN_WIDTH,
	height	= 480,
	-- safe areas
	safe =
	{
		top		= 480 * 0.075,
		bottom	= 480 * 0.925,
		left	= SCREEN_LEFT + SCREEN_WIDTH * 0.075,
		right	= SCREEN_RIGHT - SCREEN_WIDTH * 0.075,
		width	= SCREEN_WIDTH * 0.85,
		height	= 480 * 0.85,
	},
}

GUITable = { }
GUIBank =
{
	major_avatar_usize = 1 / UIGlobals.NumMajorAvatarIcons,
	major_avatar_marker = 100,
	major_avatar_image =
	{
		filename = "member_icons",
		pos = { u = 0, v = 0 },
		size = { u = 1 / UIGlobals.NumMajorAvatarIcons, v = 1 },
	},
	
	minor_avatar_usize = 1 / UIGlobals.NumMinorAvatarIcons,
	minor_avatar_image =
	{
		filename = "npc_avatars",
		pos = { u = 0, v = 0 },
		size = { u = 1 / UIGlobals.NumMinorAvatarIcons, v = 1 },
	},
	
	blur_avatar_marker = 900,
	blur_avatar_image = 
	{
		filename = "raceworld_logo",
		pos = { u = 0, v = 0 },
		size = { u = 1, v = 1 },
	},

	CurrentContext = UIEnums.Context.Subscreen0, -- Current screen stack context
	
	-- Message functions
	MessageQueue	= {},
	MessageStart	= 1,
	MessageEnd		= 1,
	
	SelectedMessageIndex = -1,
	
	loading = false,
	load_timer = 0,
	load_next_text_change_at = 0,
	load_text_index = 0,
	load_start_delay = 2,
	load_start_delay_timer = 0,
	load_message_sent = false,
	load_finished = false,
	load_end_delay = 2,
	load_end_delay_timer = 0,
	end_effects_started = false,
	load_added_objectives = false,
	stay_on_current_screen = false,
	ready_sent = false,

	photo_viewing_download = false,

	PhotoViewFrom =
	{
		Top20Voted = 0,
		Top20Download = 1,
		Search = 2,
		MyOnline = 3,
	},

	photo_viewing_from = -1,

	EventFilters =
	{
		[UIEnums.MpEventFilter.QuickRace]		= { UIText.MP_QUICK_RACE,			UIText.MP_QUICK_RACE_DESC,			"quick_race",			"common_icons",	},
		[UIEnums.MpEventFilter.UserCreated]		= { UIText.MP_MY_EVENTS,			UIText.MP_MY_EVENTS_DESC,			"My_Groups",			"fe_icons",		},
		[UIEnums.MpEventFilter.Favourties]		= { UIText.MP_FAVOURITE_EVENTS,		UIText.MP_FAVOURITE_EVENTS_DESC,	"favourite_groups",		"fe_icons",		},
		[UIEnums.MpEventFilter.RecentlyPlayed]	= { UIText.MP_RECENTLY_PLAYED,		UIText.MP_RECENTLY_PLAYED_DESC,		"recent_Groups",		"fe_icons",		},
		[UIEnums.MpEventFilter.HighestRated]	= { UIText.MP_HIGHEST_RATED,		UIText.MP_HIGHEST_RATED_DESC,		"rated_groups",			"fe_icons",		},
		[UIEnums.MpEventFilter.Newest]			= { UIText.MP_NEWEST_EVENTS,		UIText.MP_NEWEST_EVENTS_DESC,		"new_groups",			"fe_icons",		},
	},
	
	IconOverides = 
	{
		[UIEnums.IconOverideType.Message] =			{ "fe_icons",		"message",			},
		[UIEnums.IconOverideType.Audio] =			{ "common_icons",	"mic",				},
		[UIEnums.IconOverideType.Photo] =			{ "common_icons",	"photo",			},
		[UIEnums.IconOverideType.Video] =			{ "common_icons",	"video",			},
		[UIEnums.IconOverideType.PriorityPhoto] =	{ "common_icons",	"photo",			},
		[UIEnums.IconOverideType.PriorityVideo] =	{ "common_icons",	"video",			},
		[UIEnums.IconOverideType.LinkToGarage] =	{ "fe_icons",		"garage_link",		},
		[UIEnums.IconOverideType.LinkToOptions] =	{ "fe_icons",		"options_link",		},
		[UIEnums.IconOverideType.LinkToShop] =		{ "fe_icons",		"upgrades_link",	},
		[UIEnums.IconOverideType.GiftCar] =			{ "common_icons",	"car",				},
		[UIEnums.IconOverideType.GiftCash] =		{ "fe_icons",		"rewards",			},
		[UIEnums.IconOverideType.GiftFans] =		{ "common_icons",	"fan",				},
		[UIEnums.IconOverideType.Event] =			{ "common_icons",	"events",			},
		[UIEnums.IconOverideType.CarChallenge] =	{ "fe_icons",		"car_challenges",	},
	},
}

-- Do here so we can catch them in the GUIBank reload
UISystem.LoadLuaScript( "Screens\\NetworkBank.lua" )	-- Network
UISystem.LoadLuaScript( "Screens\\SpGUIBank.lua" )		-- SinglePlayer
UISystem.LoadLuaScript( "Screens\\SsGUIBank.lua" )		-- Splitscreen
UISystem.LoadLuaScript( "Screens\\MpGUIBank.lua" )		-- Multiplayer
UISystem.LoadLuaScript( "Screens\\InfoLines.lua" )
--

function ShowGlobals( )
	local func_cnt = 0
	for k,v in pairs( _G ) do
		if type(v) ~= "function" then
			print(k .. " = ",v)
		else
			func_cnt = func_cnt + 1
		
		end
   	end

   	print("## function count", func_cnt )
end

function SetupFadeToBlack( fade_up, fade_down )
	if fade_up == true and fade_down == true then
		return UIButtons.CloneXtGadgetByName( "SCUIBank", "Gfx_FadeUpDown" )
	elseif fade_up == true then
		return UIButtons.CloneXtGadgetByName( "SCUIBank", "Gfx_FadeUp" )
	elseif fade_down == true then
		return UIButtons.CloneXtGadgetByName( "SCUIBank", "Gfx_FadeDown" )
	end
end

function ClearMultiplayerGlobals()
	UIGlobals.Multiplayer.PublicGame				= true
	UIGlobals.Multiplayer.SelectPlayerJoinRef		= -1
	UIGlobals.Multiplayer.ChallengePackViewing		= -1

	UIGlobals.Multiplayer.NotEnoughPlayersKick		= false
	UIGlobals.Multiplayer.InitialLobby				= false
	UIGlobals.Multiplayer.LobbyState				= UIEnums.MpLobbyState.FindingPlayers
	UIGlobals.Multiplayer.LobbyTimer				= 0
	UIGlobals.Multiplayer.LobbyUpdateLoadout		= false
	UIGlobals.Multiplayer.LobbyUpdateCar			= false
	UIGlobals.Multiplayer.VotingFinished			= false
	UIGlobals.Multiplayer.CountdownStarted			= false
	UIGlobals.Multiplayer.DisconnectRaceAsPartyHost = false

	UIGlobals.Multiplayer.ViewingResults			= false

	UIGlobals.Multiplayer.RaceFinished				= false
	UIGlobals.Multiplayer.PrimaryVehicleFinished	= false
end

function RemapMovieTracks( )
	 -- Default to English
	local vo_track_mono = 4
	local vo_track_stereo = 12
		
	if UIGlobals.CurrentLanguage == UIEnums.Language.French then
		vo_track_mono = 5
		vo_track_stereo = 13
	elseif UIGlobals.CurrentLanguage == UIEnums.Language.German then
		vo_track_mono = 6
		vo_track_stereo = 14
	elseif UIGlobals.CurrentLanguage == UIEnums.Language.Italian then
		vo_track_mono = 7
		vo_track_stereo = 15
	elseif UIGlobals.CurrentLanguage == UIEnums.Language.Spanish then
		vo_track_mono = 8
		vo_track_stereo = 16
	elseif UIGlobals.CurrentLanguage == UIEnums.Language.Japanese then
		vo_track_mono = 9
		vo_track_stereo = 17
	elseif UIGlobals.CurrentLanguage == UIEnums.Language.Russian then
		vo_track_mono = 10
		vo_track_stereo = 18
	elseif UIGlobals.CurrentLanguage == UIEnums.Language.Polish then
		vo_track_mono = 11
		vo_track_stereo = 19
	end
	
	UISystem.RemapMovieTracks( 0, 0, 1, 2, 3, vo_track_mono, vo_track_stereo )
end

function UpdateRouteShapes( route_name, num_routes, city_id, route_id, start_line_name )
	local start_line_id = SCUI.name_to_id[ start_line_name ]
	if IsNumber( start_line_id ) == true then
		UIButtons.SetActive( start_line_id, false )
	end
	
	for i = 1,num_routes,1 do
		local route_shape_id = SCUI.name_to_id[ route_name .. i ]
		if IsNumber( route_shape_id ) == true then
			UIButtons.SetActive( route_shape_id, false )
		end
	end
	
	if IsNumber( city_id ) == false or IsNumber( route_id ) == false then 
		print("id wrong",city_id,route_id)
		return
	end
	
	local city_table = GameData.GetCities()
	
	-- Find city
	local city
	for i,v in ipairs( city_table ) do
		if v.id == city_id then
			city = v
			break
		end
	end
	if IsTable(city) == false then
		print("City not found")
		return
	end
	
	-- Get city route	
	local routes = GameData.GetRoutes(city_id)
	if IsTable( routes ) == false then 
		print("route wrong")
		return
	end
	
	-- Find route
	local route
	for i,v in ipairs( routes ) do
		if v.id == route_id then
			route = v
			break
		end
	end
	if IsTable(route) == false then
		print("Route not found")
		return
	end
	
	-- print("Change route route", city.debug_tag, route.debug_tag )

	-- Change thumnail
	if IsNumber( SCUI.name_to_id[ "Thumbnail" ] ) == true then
		UIButtons.ChangeTexture( { filename = route.debug_tag }, 0, SCUI.name_to_id[ "Thumbnail" ] )
		GUI.route_texture_name = route.debug_tag
	end
	-- Change background
	if IsNumber( SCUI.name_to_id[ "Gfx_LoadingBG" ] ) == true then
		UIButtons.ChangeTexture( { filename = route.debug_tag }, 0, SCUI.name_to_id[ "Gfx_LoadingBG" ] )
		GUI.route_texture_name = route.debug_tag
	end
		
	-- Change routes
	for i = 1,num_routes,1 do
		local route_shape_id = SCUI.name_to_id[ route_name .. i ]
		if IsNumber( route_shape_id ) == true then
			if UIShape.ChangeSceneName( route_shape_id, city.debug_tag .. "_ui" ) == false then
				return
			end
			if UIShape.ChangeObjectName( route_shape_id, route.debug_tag .. "_ui" ) == false then
				if UIShape.ChangeObjectName( route_shape_id, route.debug_tag ) == false then
					return
				end
			end
		
			UIButtons.SetActive( route_shape_id, true )
		end
	end

	-- Change shape
	if IsNumber( start_line_id ) == true then
		if UIShape.ChangeSceneName( start_line_id, city.debug_tag .. "_ui" ) == true and UIShape.ChangeObjectName(start_line_id, route.debug_tag .. "_start_line" ) == true then
			UIButtons.SetActive( start_line_id, true )
		end
	end
		
	UIButtons.TimeLineActive( "show_map", true, 0, true )
end

-- timeline commands
function HandleCommand( command )
	-- store current
	local _GUI = GUI
	local _SCUI = SCUI
	
	local t = ContextTable[ UIScreen.Context() ]
	if IsTable( t ) == false then
		return
	end
	
	-- Set context globals
	GUI = t.GUI
	SCUI = t.SCUI
	
	-- run the command
	ProtectedLoadRunString( command )
	
	-- restore
	GUI = _GUI
	SCUI = _SCUI
end

function GetScreenOtions()
	local cs = UIScreen.GetCurrentScreen(-1)
	if UIGlobals.ScreenOptions[ cs ] == nil then
		UIGlobals.ScreenOptions[ cs ] = { }
	end
	
	return UIGlobals.ScreenOptions[ cs ]
end

-- Set selection from UIGlobals table
function RestoreScreenSelection( _GUI, name, use_index )
	local t = GetScreenOtions()
	if IsTable( t ) == false or IsTable(_GUI) == false or IsString( name ) == false then 
		print("Type error")
		return
	end
	
	local gadget_id = _GUI[ name ]
	if IsNumber( gadget_id ) == false then
		print( "Gadget doesn't exist in GUI" )
		return
	end
	
	-- Check if we've stored before
	local selection = t[ name ]
	if IsNumber( selection ) == false then
		return
	end
	
	-- Set selection
	if use_index == true then
		UIButtons.SetSelectionByIndex( gadget_id, selection )
	else
		UIButtons.SetSelection( gadget_id, selection )
	end
end

-- Get selection and store in UIGlobals table
function StoreScreenSelection( _GUI, name, use_index )
	local t = GetScreenOtions()
	if IsTable( t ) == false or IsTable(_GUI) == false or IsString( name ) == false then 
		print("Type error")
		return
	end
	
	local gadget_id = _GUI[ name ]
	if IsNumber( gadget_id ) == false then
		print( "Gadget doesn't exist in GUI" )
		return
	end
	
	-- Set selection
	if use_index == true then
		t[ name ] = UIButtons.GetSelectionIndex( gadget_id  )
	else
		t[ name ] = UIButtons.GetSelection( gadget_id )
	end
end

function Select( condition, option_true, option_false, option_nil )
	if condition == nil and option_nil ~= nil then
		return option_nil
	end
	
	if condition == true then
		return option_true
	end
	
	return option_false
end

function Clamp( v, l, h )
	if v < l then
		v = l
	elseif v > h then
		v = h
	end
	
	return v
end

function SmoothStep( t )
	local t2 = t * t
	local t3 = t2 * t
	return ( 3 * t2 - 2 * t3 )
end

function ReloadCommandScript()
	UISystem.LoadLuaScript( "Screens\\Commands.lua" )
end

function PlaySfxBack()
	UISystem.PlaySound( UIEnums.SoundEffect.Backward )
end

function PlaySfxNext()
	UISystem.PlaySound( UIEnums.SoundEffect.Forward )
end

function PlaySfxGraphicBack()
	UISystem.PlaySound( UIEnums.SoundEffect.GraphicBackward )
end

function PlaySfxGraphicNext()
	UISystem.PlaySound( UIEnums.SoundEffect.GraphicForward )
end

function PlaySfxToggle()
	UISystem.PlaySound( UIEnums.SoundEffect.Toggle )
end

function PlaySfxError()
	UISystem.PlaySound( UIEnums.SoundEffect.Error )
end

function PlaySfxUp()
	UISystem.PlaySound( UIEnums.SoundEffect.Up )
end

function PlaySfxDown()
	UISystem.PlaySound( UIEnums.SoundEffect.Down )
end

function PlaySfxLeft()
	UISystem.PlaySound( UIEnums.SoundEffect.Left )
end

function PlaySfxRight()
	UISystem.PlaySound( UIEnums.SoundEffect.Right )
end

-- can we save with the given profile?
function CanSave( profileIndex, ignore_do_not_save )
	if UIScreen.IsContextActive( UIEnums.Context.LoadSave ) == true then
		print( "CanSave() - already saving - bailing" )
		return false
	end
	
	-- if people have no device/corrupt profiles
	if ignore_do_not_save ~= true and UIGlobals.DoNotSave[ profileIndex ] == true then
		return false
	end
	
	-- only save out "proper" profiles
	if UIGlobals.ProfileState[ profileIndex ] ~= UIEnums.Profile.GamerProfile then
		return false
	end
	
	-- in splitscreen we can only save on the primary pad that we origianlly had
	-- things can go screwy if an original blagged host leaves and the control is given to a new pre-load profile
	if IsSplitScreen() == true and profileIndex ~= UIGlobals.splitscreen_primary_pad_original then
		return false
	end

	-- set the save index
	UIGlobals.ProfileSaveIndex = profileIndex
	
	return true
end


function AddListNode( bLocked, sValue )
	
	if bLocked == nil then
		bLocked = false
	end
	
	if sValue == nil then
		sValue = -1
	end
	
	local listNode =
	{
		layer = UIEnums.Layer.LAYER_1,
		type = UIEnums.ButtonTypes.NODE,
		pos = { x = 0, y = 0 },
		size = { x = 1, y = 1 },
		justify = UIEnums.Justify.TopLeft,	
		locked = bLocked,
		value = sValue,
	}
	return UIButtons.AddButton( listNode )
end

function AddListItem( nodeId, itemId )
	UIButtons.SetParent( itemId, nodeId, UIEnums.Justify.TopLeft )
end

function ScreenInit()
	--print( "ScreenInit()" )
	
	-- display profile states (debug)
	local text =
	{
		layer = UIEnums.Layer.TOP,
		type = UIEnums.ButtonTypes.TEXT,
		pos = { x = GUIBank.ScreenLeftEdge, y = GUIBank.ScreenTopEdge },
		size = { x = 0, y = 20 },
		justify = UIEnums.Justify.TopLeft,
		text_string = "",
	}
	
	-- profile names
	for i = 0, 3, 1 do
		text.text_string = "PROFILE_PAD" .. i .. "_NAME"
		--UIButtons.AddButton( text )
		text.pos.y = text.pos.y + text.size.y
	end
	
	text.pos.y = GUIBank.ScreenTopEdge
	text.pos.x = GUIBank.ScreenLeftEdge + 150
	
	-- profile status
	for i = 0, 3, 1 do
		text.text_string = "PROFILE_PAD" .. i .. "_STATUS"
		--UIButtons.AddButton( text )
		text.pos.y = text.pos.y + text.size.y
	end	
	
	-- call the normal screen Init
	Init()
end


function SetupFadedBackground( layer )

	if layer == nil then
		layer = UIEnums.Layer.LAYER_1
		print( "no layer was specified. Using layer 1" )
	end
	
	local background = 
	{
		layer = layer,
		type = UIEnums.ButtonTypes.GRAPHIC,
		pos = { x = 320, y = 240 },
		size = { x = SCREEN_WIDTH + 2, y = 482 },
		justify = UIEnums.Justify.MiddleCentre,
		colour_style = "!175 0 0 0",
		textures = 
		{
			{
				name = UITexture.Textures.null,
				pos = { u = 0, v = 0, },
				size = { u = 1, v = 1, },
			},
		},
	}
	UIButtons.AddButton( background )
end


-- Helpers
function IsString( v )
	return type( v ) == "string"
end

function IsFunction( v )
	return type( v ) == "function"
end

function IsTable( v )
	return type( v ) == "table"
end

function IsNumber( v )
	return type( v ) == "number"
end

function IsBoolean( v )
	return type( v ) == "boolean"
end

-- Run a string as a function ... carefully
function ProtectedLoadRunString( string )
	local f = loadstring( string )
	
	if f == nil then
		print( "ProtectedLoadRunString: ERROR: loadstring() failed", string )
	else
		local success, error = pcall( f )
		
		if success == false then
			print( "ProtectedLoadRunString: ERROR:", error )
		end
		
		return success
	end
	
	return false
end

function SCUI_MessageUpdate( message, arg1, arg2, arg3, arg4, overide )
	if IsTable( GUI ) == true then
		if GUI.finished == true then
			return true
		end
	end
	
	-- Overide table?
	local table = SCUI
	
	if IsTable( overide ) then 
		table = overide
	end
	
	if IsTable( table ) == false then
		return false
	end

	-- [Blur2PC] Retail GuiBank records the device of a menu action for split-screen sign-in.
	if ( message == UIEnums.Message.MenuNext or message == UIEnums.Message.MenuBack or
	     message == UIEnums.Message.ButtonX or message == UIEnums.Message.ButtonY ) and arg4 ~= nil then
		UIGlobals.LastInputDevice = arg4
	end

	if message == UIEnums.Message.MenuNext and IsString( table.on_next ) then
		return ProtectedLoadRunString( table.on_next )
	elseif message == UIEnums.Message.MenuBack and IsString( table.on_back ) then
		return ProtectedLoadRunString( table.on_back )
	end
	
	return false
end


function Lerp( a, b, f )
	return a + ( b - a ) * f
end

function SetNil( t )
	if t == nil then
		return
	end
	
	if IsTable( t ) then
		for i, v in pairs( t ) do
			if IsTable( t[ i ] ) then
				SetNil( t[ i ] )
			end
			
			t[ i ] = nil
		end
	end
	
	t = nil
end

-- make a deep copy of a table
function CloneTable( t, newT )
	if newT == nil then
		newT = {  }
	end
	
	if IsTable( t ) then
		for i, v in pairs( t ) do
			if IsTable( t[ i ] ) then
				newT[ i ] = CloneTable( t[ i ], newT[ i ] )
			else
				newT[ i ] = v
			end
		end
	end
	
	return newT
end

-- sorcery stuff
function ResolveFontHeight( v )
	if v.type ~= UIEnums.ButtonTypes.TEXT and 
	   v.type ~= UIEnums.ButtonTypes.VERTICAL_MENU and 
	   v.type ~= UIEnums.ButtonTypes.HORIZONTAL_MENU and 
	   v.type ~= UIEnums.ButtonTypes.SPINNER_BUTTON and
	   v.type ~= UIEnums.ButtonTypes.HUD_MESSAGES then
		return
	end
	
	local size = v.size.y
	
	-- overridden size are negative
	if size > -1 then
		return
	end
	
	size = size * -1
	
	if size > #UIGlobals.FontSize then
		-- outside of range - just use as is
		return
	end
	
	local amount = math.floor( size )
	
	if amount == #UIGlobals.FontSize then
		-- max - just set
		size = UIGlobals.FontSize[ amount ]
	else
		-- else lerp between heights
		local fract = size - amount
		
		size = Lerp( UIGlobals.FontSize[ amount ], UIGlobals.FontSize[ amount + 1 ], fract )
		
		-- set
		v.size.y = size
	end
	
	-- menu default size.x too
	if ( v.type == UIEnums.ButtonTypes.VERTICAL_MENU or v.type == UIEnums.ButtonTypes.HORIZONTAL_MENU ) and v.size.x == 0 then
		v.size.x = v.size.y
	end
end

function AddSCUI_Elements( overide )
--[[
	-- highjacking this function for means of lazyness
	if IsTable( overide ) == false then -- stops it being added multiple times for subscreens.
		AddInternalUseMessage()
	end
	
	]]--
	
	-- Overide table?
	local table = SCUI
	
	if IsTable( overide ) then 
		table = overide
	end
	
	-- Safety
	if IsTable( table ) == false or IsTable( table.elements ) == false then
		return
	end
	
	-- Create name_to_table if it doesn't exist
	if IsTable( table.name_to_id ) == false then
		table.name_to_id = { }
	end
	
	-- Create name_to_table if it doesn't exist
	if IsTable( table.name_to_index ) == false then
		table.name_to_index = { }
	end
	
	-- Add elephants		
	for i,v in pairs( table.elements ) do
		local addItem = true
		
		-- resolve font heights
		ResolveFontHeight( v )
		
		if IsString( v.name ) then
			-- Dont add anything with the specified prefix
			if string.sub( v.name, 1, 1 ) == "_" then
				addItem = false
			end
		end
	
		-- Do we add it?
		if addItem == true then
			v.ID = UIButtons.AddButton( v )
			table.name_to_id[ v.name ] = v.ID
		end
		
		if IsString( v.name ) then
			table.name_to_index[ v.name ] = i
		end
	end
end

function GoLoadingScreen( screen )
	if IsString( screen ) ~= true then
		print("GoLoadingScreen : invalid screen")
		return
	end

	StoreScreen( UIEnums.ScreenStorage.FE_GAME_LOADING, screen )
	GoScreen( "Loading\\PrepLoading.lua", UIEnums.Context.Main )
end

function GoScreen( next, ctx )
	if next == nil then
		return
	end
	
	local _GUI = GUI
	
	if ctx == nil then
		-- Popups automatically change context 0 unless stated otherwise
		ctx = UIScreen.Context()
		if ctx == 1 then
			ctx = 0
		end
	end
	if IsTable( ContextTable[ ctx ] ) ~= true or UIScreen.IsContextActive( ctx ) == false then
		if next == "EndScreen" then
			print("Trying to end a screen that's not been started on ctx",ctx)
		else
			UIScreen.SetNextScreen( next, ctx )
		end
		return
	end
	
	-- Check the finished flag
	_GUI = ContextTable[ ctx ].GUI
	
	if IsTable( _GUI ) == false then
		print("waaah - no local GUI for context", ctx )
		return
	end

	if _GUI.finished == nil then	
		print("Warning : Using GoScreen() without a 'finished' var in LocalGUI")
	end
	
	if _GUI.finished == true then
		print("trying to change a screen that's already finished")
		return
	end
	
	if UIScreen.Context() == ctx then
		_GUI.finished = true
	end
	
	if next == "EndScreen" then
		EndScreen( ctx )
	else
		UIScreen.SetNextScreen( next, ctx )
	end
end

function GoSubScreen( next )
	if next == nil then
		return
	end

	local _GUI = ContextTable[ UIEnums.Context.Main ].GUI
	if IsTable( _GUI ) == true then
		GoScreen( next, _GUI.active_context )
	end
end


function CloseCurrentApp( no_sound )
	local _GUI = ContextTable[ UIEnums.Context.Main ].GUI
	if IsTable( _GUI ) == false then
		return
	end

	if IsNumber( _GUI.active_context ) == false then
		print("active_context ~= NUMBER")
		return
	end
	
	if _GUI.active_context == -1 then
		print("No active context")
	end

	CloseApp( _GUI.active_selection, true, nil, no_sound )
end

function CloseApp( selection, do_not_call_can_exit, arg1, no_sound )
	-- Make safe to be called from elsewhere
	local _GUI = ContextTable[ UIEnums.Context.Main ].GUI
	local _SCUI = ContextTable[ UIEnums.Context.Main ].SCUI
	local can_exit = true
	
	if UIScreen.GetScreenActive( _GUI.active_context ) == false then
		return false
	end
	
	if IsNumber( _GUI.active_context ) == true then
		if IsTable( ContextTable[ _GUI.active_context ] ) == true then
			local ActiveGUI = ContextTable[ _GUI.active_context ].GUI
		
			if IsTable( ActiveGUI ) and IsFunction( ActiveGUI.CanExit ) and do_not_call_can_exit ~= true then
				can_exit = ActiveGUI.CanExit( ActiveGUI, arg1 )
			end
			
			if can_exit == true then
				ActiveGUI.finished = true
			end
		end
	end
	
	if can_exit == true then
		EndActive()
		UIButtons.TimeLineActive( "HelpFade", true, 0 )
		if Amax.GetGameMode() == UIEnums.GameMode.SinglePlayer then
			CarouselSetApp( _GUI, _SCUI, selection, true )
		
		elseif UIGlobals.IsIngame == false then
			LeaveMultiplayerMode()
		end
		
		if no_sound ~= true then
			PlaySfxBack()
		end
	end
	
	if can_exit == true and IsTable( _GUI.node_id ) == true then
		UIButtons.PrivateTimeLineActive( _GUI.node_id[ selection ], "Open", false, 2 )
		UIButtons.PrivateTimeLineActive( _GUI.bottom_help_id, "Hide_BottomHelp", false )
	end
	
	return can_exit
end

function EndActive()
	local _GUI = ContextTable[ UIEnums.Context.Main ].GUI
	local active = UIScreen.GetScreenActive( _GUI.active_context )
	
	if active == true then
		-- End current selection
		UIButtons.TimeLineActive( "Outro", true, nil, nil, _GUI.active_context )

		-- Switch active context
		_GUI.inactive_context = _GUI.active_context
	else
		EndScreen( _GUI.active_context )
	end

	-- Give control back to the carousel
	if _GUI.do_tutorial ~= true and IsControllerLocked() == false then
		UIButtons.SetActive( _GUI.carousel_id, true )
		UIButtons.SetSelected( _GUI.carousel_id, true )
	end
			
	UIButtons.TimeLineActive( "CarouselSetback", false )
	
	-- Clear
	_GUI.active_selection = -1
end

function EndInactive()
	if UIScreen.Context() ~= 0 then
		return
	end
	local _GUI = ContextTable[ UIEnums.Context.Main ].GUI
	
	if IsNumber( _GUI.inactive_context ) == true then
		EndScreen( _GUI.inactive_context )
		_GUI.inactive_context = nil
	end
end

function CarouselApp_SetScreenTimers()
	UIScreen.SetScreenTimers( 0.3, 0.15 )
end

function EndScreen( context )
	if context == nil then
		context = UIScreen.Context()
	end
	
	if IsTable( ContextTable[ context ] ) == true and IsTable( ContextTable[ context ].GUI ) then
		ContextTable[ context ].GUI.finished = true
	end
	
	UIScreen.EndScreen( context )
end

function PushScreen( screen )
	if screen == nil then
		return
	end
	
	if GUI.finished == nil then	
		print("Warning : Using PushScreen() without a 'finished' var in LocalGUI")
	end
	
	if GUI.finished == true then
		return
	end
	
	if GUIBank.CurrentContext > UIEnums.Context.Subscreen2 then
		print("PushScreen : Trying to push but there's nowhere to go")
		return
	end
	
	-- Push screen
	UIScreen.SetNextScreen( screen, GUIBank.CurrentContext )
	GUIBank.CurrentContext = GUIBank.CurrentContext + 1
end


function PopScreen()
	if GUIBank.CurrentContext <= UIEnums.Context.Subscreen0 then
		print("PopScreen : Trying to pop but you should stop")
		return
	end
	
	-- Pop current screen
	GUIBank.CurrentContext = GUIBank.CurrentContext - 1
	EndScreen( GUIBank.CurrentContext )

	if IsTable( ContextTable[ GUIBank.CurrentContext ] ) == true then
		ContextTable[ GUIBank.CurrentContext ].GUI.finished = true
	end
end


function FlushSubscreens()
	-- Remove the extra subscreens
	EndScreen( UIEnums.Context.Subscreen0 )
	EndScreen( UIEnums.Context.Subscreen1 )
	EndScreen( UIEnums.Context.Subscreen2 )
	EndScreen( UIEnums.Context.Blurb )
	
	GUIBank.CurrentContext = UIEnums.Context.Subscreen0
end


function SubScreenActive()
	return ( 
		UIScreen.IsContextActive( UIEnums.Context.Subscreen0 ) == true or 
		UIScreen.IsContextActive( UIEnums.Context.Subscreen1 ) == true or 
		UIScreen.IsContextActive( UIEnums.Context.Subscreen2 ) == true )
end

function StoreScreen( index, name )
	if name == nil then
		name = UIScreen.GetCurrentScreen()
	end

	if IsNumber( index ) == false or IsString( name ) == false then
		print("Invalid params : StoreScreen( number, string ) : ", type( index ), type( name ) )
		return
	end

	UIGlobals.ScreenStore[ index ] = name
end

function GetStoredScreen( index )
	if IsNumber( index ) == false then
		print("Invalid params : GetStoredScreen( number, string ) : ", type( index ) )
		return "INVALID_SCREEN"
	end
	
	local screen = UIGlobals.ScreenStore[ index ]
	if IsString( screen ) == false then
		print( "Invalid index", index )
		return "INVALID_SCREEN"
	end
	
	return screen
end

function SetupMovie( slot, name, in_memory, memory_size, renderWorld, backgroundMusic, volume )
	-- Clear the screen
	local black = 
	{
		layer = UIEnums.Layer.LAYER_1,
		type = UIEnums.ButtonTypes.GRAPHIC,
		pos = { x = 320, y = 240 },
		size = { x = SCREEN_WIDTH + 2, y = 482 },
		justify = UIEnums.Justify.MiddleCentre,
		colour_style = "!255 0 0 0",
		textures = 
		{
			{
				name = UITexture.Textures.null,
				pos = { u = 0, v = 0, },
				size = { u = 1, v = 1, },
			},
		},
	}
	UIButtons.AddButton(black)
	
	-- Use defaults if not defined.
	if in_memory == nil then
		in_memory = false
	end
		
    local volume = Amax.GetMovieVolume()

	--[[
    -- tie FMV volume to sfx volume. sam. fixes bug #23628. 
    if UIGlobals.ProfileSinglePlayerPad ~= nil and UIGlobals.ProfileSinglePlayerPad ~= -1 then
       volume = UIGlobals.Profile[ UIGlobals.ProfileSinglePlayerPad + 1 ].Preferences.SoundOptions.SFXVolume * 32768    
    end
    ]]--
	
	UISystem.InitMovie(slot, name, in_memory, memory_size, renderWorld, backgroundMusic, volume)

	local movie_image =
	{
		layer = UIEnums.Layer.LAYER_1,
		type = UIEnums.ButtonTypes.GRAPHIC,
		pos = { x = SCREEN_LEFT, y = 0, },
		size = { x = SCREEN_WIDTH + 2, y = 480 + 2, },
		colour_style = "!255 255 255 255",
		textures = 
		{
			{
				name = UIEnums.TextureType.MOVIE0,
				pos = {	u = 0, v = 0, },
				size = { u = 1, v = 1, },
			},
		},
		blend_mode = UIEnums.BlendMode.Solid,
		effect_index = UIEnums.EffectIndex.Movie,
	}
	if UIEnums.CurrentPlatform ~= UIEnums.Platform.PC and UIHardware.WideScreen() == false then	-- [Blur2PC] retail PC: no 4:3 movie adjustment
		local temp = 480 - (640 * 9 / 16)
		movie_image.pos.y = temp * 0.5
		movie_image.size.y = 480 - temp
	end
	
	UIButtons.AddButton(movie_image)
end

function LocalGUI( ctx )
	print("Deprecated")
	return GUI
end

function LocalGUIName( ctx )
	print("Deprecated")
end

function LocalSCUI( ctx )
	print("Deprecated")
	return SCUI
end

function AddInternalUseMessage()

	-- TODO : return on certain builds?
	-- if ?? then
	--		return
	-- end
	
	local text = 
	{
		type = UIEnums.ButtonTypes.TEXT,
		layer = UIEnums.Layer.TOP,
		justify = UIEnums.Justify.BottomCentre,
		parent_attach = UIEnums.Justify.BottomCentre,
		pos = { x = 0, y = -20, z = 0 },
		size = { x = 0, y = 14, z = 0 },
		colour_style = "!96 255 255 255",
	
		text = UIText.CMN_INTERNAL_USE,
	}
	UIButtons.AddButton( text )
end

-- Make sure the things we 'assume' are globals are cleared
function ClearScreenTemps()
	Init			= nil
	PostInit		= nil
	StartLoop		= nil
	FrameUpdate		= nil
	MessageUpdate	= nil
	EnterEnd		= nil
	EndLoop			= nil
	End				= nil
	
	GUI				= nil
	SCUI			= nil
end

-- Table that contains each contexts' globals + functions.
ContextTable = { }

function PreLoadCallFile()
	ContextTable[ UIScreen.Context() ] = { } -- Clear context table

	ClearScreenTemps()
end

function PostLoadCallFile()
	local t = ContextTable[ UIScreen.Context() ]
	
	-- Store the screens globals
	t.GUI			= GUI
	t.Init			= Init
	t.PostInit		= PostInit
	t.StartLoop		= StartLoop
	t.FrameUpdate	= FrameUpdate
	t.MessageUpdate = MessageUpdate
	t.EnterEnd		= EnterEnd
	t.EndLoop		= EndLoop	
	t.End			= End
	
	-- And then clear them
	ClearScreenTemps()
end

function SetContextTable( ctx )
	local t = ContextTable[ ctx ]
	if IsTable( t ) == false then
		return nil
	end
	
	-- Store
	ContextTable_BackupGUI = GUI
	ContextTable_BackupSCUI = SCUI

	-- Set context globals
	GUI = t.GUI
	SCUI = t.SCUI
	
	return t
end

function ResetContextTable()
	-- Restore
	GUI		= ContextTable_BackupGUI
	SCUI	= ContextTable_BackupSCUI
	
	-- Don't keep copy so it can be garbage collected
	ContextTable_BackupSCUI = nil
	ContextTable_BackupGUI = nil
end

-- Context thread wrappers
function Init_Wrapper()
	local t = ContextTable[ UIScreen.Context() ]
	if IsTable( t ) == false then
		return
	end
	
	if UIScreen.Context() == InfoLineContext then
		InfoLineClearCommands()
	end
	
	-- Clear this
	GUIBank.render_buffer_id = nil
			
	-- Store the SCUI table (as this is build just before Init is called)
	t.SCUI	= SCUI
	
	-- Set context globals
	GUI = t.GUI
	
	-- for debugging
	_G[ "GUI" .. UIScreen.Context() ] = GUI
	_G[ "SCUI" .. UIScreen.Context() ] = SCUI
			
	if IsFunction( t.Init ) then
		t.Init()
	end
	
	if UIScreen.Context() == 0 then
		if IsTable( SCUIBank ) == false then
			print( "SCUIBank doesnt exists" )
			
			ResetContextTable()
			return
		end
	end
	
	if UIScreen.Context() == InfoLineContext then
		Friends_CheckForEnable()
	end
	
	ResetContextTable()
end

function PostInit_Wrapper()
	local t = SetContextTable( UIScreen.Context() )
	if IsTable( t ) == false then
		return
	end
	
	if IsFunction( t.PostInit ) then
		t.PostInit()
	end

	-- [Blur2PC] PC menus use the whole viewport; console letterboxing is unchanged.
	if UIEnums.CurrentPlatform ~= UIEnums.Platform.PC and UIGlobals.ScaleCamera == true then
		AddFourThreeBars()
	end
	
	-- need to force the panels if the popup is active
	if (UIScreen.IsPopupActive() == true or UIGlobals.AsyncSave_FailMessageActive == true) and UIScreen.Context() ~= UIEnums.Context.LoadSave then
		UIButtons.TimeLineActive( "Popup_Active", true )
	end
	
	ResetContextTable()
end

function StartLoop_Wrapper( time )
	local t = SetContextTable( UIScreen.Context() )
	if IsTable( t ) == false then
		return
	end
		
	if IsFunction( t.StartLoop ) then
		t.StartLoop( time )
	end

	ResetContextTable()
end

function FrameUpdate_Wrapper( time )
	if UIScreen.Context() == 0 then
		FrameUpdateLoading( time )
	end
	
	local t = SetContextTable( UIScreen.Context() )
	if IsTable( t ) == false then
		return
	end
	
	if GUI.finished ~= true then
		if IsFunction( t.FrameUpdate ) then
			t.FrameUpdate( time )
		end
	end
		
	if UIScreen.Context() == 0 then
		if UIGlobals.LaunchMode == UIEnums.LaunchMode.Automation then
			if GUI.automation_start == false then
				if Amax.IsGamePaused() == true then
					UIGlobals.automation_timer = UIGlobals.automation_length
				else
					UIGlobals.automation_timer = UIGlobals.automation_timer - time
					if UIGlobals.automation_timer < 0 then
						Amax.SendMessage(UIEnums.GameFlowMessage.QuitRace)
						GoScreen("Loading\\LoadingUi.lua")
					end
				end
			end
		end
	end
	
	if UIScreen.Context() == InfoLineContext then
		Friends_CheckForEnable()
		InfoLineFrameUpdate()
	end
	
	ResetContextTable()
end

function MessageUpdate_Wrapper( message, arg1, arg2, arg3, arg4, arg5 )
	if UIScreen.Context() == 0 then
		MessageUpdateLoading( message, arg1, arg2, arg3, arg4, arg5 )
	end
	
	local t = SetContextTable( UIScreen.Context() )
	if IsTable( t ) == false then
		return
	end
	
	-- Respond to global messages
	if UIScreen.Context() == 0 then
		-- This can no occur anywhere
		if message == UIEnums.GameFlowMessage.UILoaded then
			if UIGlobals.IsLoading == true then
				print("Ui Loaded")
				
				-- only come out of loading state if we are not doing a quick restart
				if UIGlobals.IsQuickRestart == false then
					UIGlobals.IsLoading = false
				end
			else
				print("Error - got UILoaded message when we're not flagged as loading")
			end
		end
		
		-- need to check for this here in case we get dragged into a game
		if UIGlobals.MpModShopKeyboardCheck ~= -1 then
			if message == UIEnums.Message.KeyboardFinished then
				Multiplayer.SetNameOfLoadOut( UIGlobals.MpModShopKeyboardCheck )
				UIGlobals.MpModShopKeyboardCheck = -1
				
			elseif message == UIEnums.Message.KeyboardCancelled then
				UIGlobals.MpModShopKeyboardCheck = -1
			end
		end
	end
	
	if GUI.finished ~= true then
		if IsFunction( t.MessageUpdate ) then
			t.MessageUpdate( message, arg1, arg2, arg3, arg4, arg5 )
		end
	end
	
	ResetContextTable()
end

function  EnterEnd_Wrapper()
	local t = SetContextTable( UIScreen.Context() )
	if IsTable( t ) == false then
		return
	end
		
	if IsFunction( t.EnterEnd ) then
		t.EnterEnd()
	end
	
	if UIScreen.Context() == InfoLineContext then
		SetupInfoLine()	-- remove completely as we're changing screen anyway and don't need to restore
	end
	
	ResetContextTable()
end


function EndLoop_Wrapper( time )
	local t = SetContextTable( UIScreen.Context() )
	if IsTable( t ) == false then
		return
	end
	
	if IsFunction( t.EndLoop ) then
		t.EndLoop( time )
	end
	
	ResetContextTable()
end

function End_Wrapper()
	local t = SetContextTable( UIScreen.Context() )
	if IsTable( t ) == false then
		return
	end
	
	if IsFunction( t.End ) then
		t.End()
	end
	
--	print( "->GUI" )
	SetNil( _G[ "GUI" .. UIScreen.Context() ] )
--	print( "->SCUI" )
	SetNil( _G[ "SCUI" .. UIScreen.Context() ] )
--	print( "->Context" )
	SetNil( ContextTable[ UIScreen.Context() ] )
--	print( "->Done" )

	-- [Blur2PC] retail: the aspect-ratio bars belong to the main context's screen
	if UIScreen.Context() == 0 then
		GUIBank.FourThreeBars_BottomId = nil
		GUIBank.FourThreeBars_TopId = nil
	end

	ResetContextTable()
end

function CarouselAddLockedShapes( )
	if IsTable( GUI.locked_table ) == false then
		return
	end

	local table = SCUI.elements[ SCUI.name_to_index[ "_Lock" ] ]
	if IsTable( table ) == false then
		print("No locked item found")
		return
	end
	
	if IsTable( GUI ) == false or IsTable( GUI.branch_name ) == false then
		print("No branch table")
		return
	end
		
	for i,v in ipairs( GUI.locked_table ) do
		local dummy_name = GUI.branch_name[ v ] .. "Dummy"
		local id = UIButtons.AddButton( table )
		UIButtons.SetParent( id, SCUI.name_to_id[ dummy_name ], UIEnums.Justify.BottomRight )
	end
end

function CarouselIsAppLocked( selection )
	if IsTable( GUI.locked_table ) == false then
		return false
	end
	
	for i,v in ipairs( GUI.locked_table ) do
		if selection == v then
			return true
		end
	end
	
	return false
end

function CarouselSetApp( _GUI, _SCUI, selection, set )
	if selection < 0 then
		return
	end
	if IsTable( _GUI ) == false or IsTable( _SCUI ) == false then
		return
	end
	
	local name = _GUI.branch_name[ selection ]
	
	if IsString( name ) == false then
		return
	end
	
	local frame = _SCUI.name_to_id[ name .. "Frame" ]
	local title = _SCUI.name_to_id[ name .. "Title" ]
	local shape = _SCUI.name_to_id[ name .. "Shape" ]
	
	if IsTable( _GUI.node_id ) == true then
		frame = UIButtons.FindChildByName( _GUI.node_id[ selection ], "Frame" )
		title = UIButtons.FindChildByName( _GUI.node_id[ selection ], "Title" )
		shape = UIButtons.FindChildByName( _GUI.node_id[ selection ], "Shape" )
	end	
	
	local col_selected = "Main_1"
	local col_unselected = "Main_2"
	
	local frame_col = col_unselected
	local title_col = col_unselected
	local shape_col = col_unselected
	
	if CarouselIsAppLocked( selection ) == false then
		if set == true then
			frame_col = col_selected
			title_col = "Main_0"
			shape_col = col_selected
		end
	else
		frame_col = "Main_Black"
		title_col = "Main_Black"
		shape_col = "Main_Black"
	end
	
	if UIGlobals.IsIngame == true or ( UIGlobals.IsIngame == false and selection ~= UIEnums.RwBranch.Facebook ) then
		UIButtons.ChangeColour( frame, frame_col )
		UIButtons.ChangeColour( title, title_col )
		UIButtons.ChangeColour( shape, shape_col )
	end
end

function StartProfileNew( arg1 )
	UIGlobals.ProfilePressedStart = arg1
	Profile.SetPrimaryPad( UIGlobals.ProfilePressedStart )

	Profile.LockToPad( arg1 )

	GameProfile.InitPrimary()

	Amax.SetGameMode( UIEnums.GameMode.SinglePlayer )
end

-- Active context helpers
function GetActiveContext()
	local main_GUI = ContextTable[ UIEnums.Context.Main ].GUI

	-- Sanity
	if IsTable( main_GUI ) == false then
		print("GUI (0) == nil")
		return
	end
	
	return ContextTable[ main_GUI.active_context ]
end

function GetActiveGUI()
	local ctx = GetActiveContext()
	if IsTable( ctx ) == false then
		return
	end
	
	return ctx.GUI
end

function GetActiveSCUI()
	local ctx = GetActiveContext()
	if IsTable( ctx ) == false then
		return
	end
	
	return ctx.SCUI
end

function CarouselLoadGame( stay_on_current_screen )
	print("##### SinglePlayer. Nowhere to go ####")
	print("##### SinglePlayer. Please fix-up ####")
end

function StartTimelineActive( _name, _active )
	for i,v0 in pairs( SCUI.elements ) do
		if IsTable( v0 ) == true and IsTable( v0.time_lines ) == true then
			for k,v1 in pairs( v0.time_lines ) do
				if v1.label == _name then
--					print(v0.name, _name, _active)
					v1.start_active = _active
				end
			end
		end
	end
end


function StartPrivateTimelineActive( _gadget_name, _name, _active )

	local index	= SCUI.name_to_index[ _gadget_name ]
	if index == nil then 
		return
	end
	
	local gadget = SCUI.elements[index]

	if IsTable( gadget ) == true and IsTable( gadget.time_lines ) == true then 
		for i,v in pairs( gadget.time_lines ) do
			if v.label == _name then
				v.start_active = _active
			end
		end
	end
end


function SetTimelineStartEnd( _name, _start, _end )
	for i,v0 in pairs( SCUI.elements ) do
		if IsTable( v0 ) == true and IsTable( v0.time_lines ) == true then
			for k,v1 in pairs( v0.time_lines ) do
				if v1.label == _name then
					if IsNumber( _end ) == true then
						v1.time["start"] = _start
						v1.time["end"] = _end
					else
						-- Maintain length
						v1.time["end"] = _start + (v1.time["end"] - v1.time["start"])
						v1.time["start"] = _start
					end
				end
			end
		end
	end
end

-- Switch composite static graphic gadget on / off depending on if the render buffer texture is valid
function CheckRenderBuffer( )
	local valid = UISystem.TextureValid( "RENDER_BUFFER" )
	local id = GUIBank.render_buffer_id
	if IsNumber( id ) == false then 
		return 
	end
	
	if valid == true then
		local texture = 
		{
			name = "RENDER_BUFFER",
			pos = { u = 0.0, v = 0 },
			size = { u = 1, v = 1 },
		}
		UIButtons.ChangeTexture( texture, 0, id )
	end
	
	UIButtons.SetActive( id, valid )
end

function LockController( )
	UIGlobals.ControllerLocked = true
end

function UnlockController( )
	UIGlobals.ControllerLocked = false
end

function IsControllerLocked( )
	return UIGlobals.ControllerLocked
end

function IsGarageMiniMultiplayer()
	return ( Amax.GetGameMode() == UIEnums.GameMode.Online or Amax.GetGameMode() == UIEnums.GameMode.SystemLink )
end

function IsMultiplayerMode( )
	return Amax.IsGameModeMultiplayer()
end

function IsNetworkMultiplayerMode( )
	return Amax.IsGameModeNetworked()
end

function StartUiLoad( )
	if UIGlobals.IsLoading == true then
		print("Warning - StartUiLoad : Already loaded")
		return
	end
	print("StartUiLoad")
	
	Debug.IncrementMemoryTag("ui")

	Amax.SendMessage(UIEnums.GameFlowMessage.StopGameRendering)
	Amax.SendMessage(UIEnums.GameFlowMessage.StartUILoad)
		
	if UIGlobals.ReturnMode ~= UIEnums.ReturnMode.QuitSpGame and UIGlobals.ReturnMode ~= UIEnums.ReturnMode.QuitMpGame then
		if Amax.GetGameMode() == UIEnums.GameMode.SinglePlayer then
			Amax.SetUICarToMultiplayer(false)
		else
			Amax.SetUICarToMultiplayer(true)
		end
		Amax.SendMessage(UIEnums.GameFlowMessage.LoadUIScene)
	end

	UIGlobals.IsLoading = true
	
	net_LockoutFriendsOverlay( true )
end

function StartGameLoad( first_time, stay_on_current_screen )
	if stay_on_current_screen == nil then
		stay_on_current_screen = false
	end
	
	GUIBank.stay_on_current_screen = stay_on_current_screen
	
	print( "StartGameLoad" )
	UIGlobals.IsLoading = true
	
	-- disable friends on ps3/pc while racing.
	net_LockoutFriendsOverlay( true ) 

	GUIBank.loading = true
	GUIBank.load_timer = 0
	GUIBank.load_next_text_change_at = 0
	GUIBank.load_text_index = 0
	GUIBank.load_start_delay_timer = 0
	GUIBank.load_message_sent = false
	GUIBank.load_finished = false
	GUIBank.load_end_delay_timer = 0
	GUIBank.ready_sent = false
	
	if first_time == true then
		GUIBank.load_first_time = true
	else
		GUIBank.load_first_time = false
	end
	
	-- end timelines
	GUIBank.end_effects_started = false
	
	if GUIBank.stay_on_current_screen == false then
		UIButtons.TimeLineActive( "start_load", true )
	end
	
	-- stop music
	StopFrontendMusic()
	
	GUIBank.load_added_objectives = false
end

function FrameUpdateLoading( time )

	if GUIBank.loading == false then
		return
	end
	
	if GUIBank.load_message_sent == false then
		GUIBank.load_start_delay_timer = GUIBank.load_start_delay_timer + time
		
		if GUIBank.load_start_delay_timer > GUIBank.load_start_delay then
			GUIBank.load_message_sent = true
			-- start the load
			print( "FrameUpdateLoading : StartGameLoad" )
			Debug.IncrementMemoryTag( "game" )
			if UIGlobals.IsQuickRestart == false then
				Amax.SendMessage( UIEnums.GameFlowMessage.StartRaceLoad )
			else
				Amax.SendMessage(UIEnums.GameFlowMessage.UnPause)
				Amax.SendMessage(UIEnums.GameFlowMessage.RestartRace)
				Amax.SendMessage(UIEnums.GameFlowMessage.RestartLevel)
			end
		end
	end
	
	if GUIBank.load_finished == true and Amax.WaitForStreamer() == false then

		-- incase a game invite is pending
		if NetServices.GameInvitePending() == true then
			-- this will allow networkbank to run the game invite logic
			UIGlobals.IsIngame = true
			UIGlobals.IsLoading = false 
			GUIBank.loading = false
			return
		end

		local can_proceed = true

		-- need to tell the host were ready and then wait until everybody else is ready before continuing
		if Amax.IsGameModeNetworked() == true then
	
			if GUIBank.ready_sent == false then
				NetRace.EnterSyncAfterRaceLoad()
				GUIBank.ready_sent = true
				can_proceed = false

			elseif NetRace.RaceStartSynchronised() == false then
				can_proceed = false
			end
		end

		if can_proceed == false then
			return -- not ready just yet
		end

		-- start timelime
		if GUIBank.end_effects_started == false then
			if GUIBank.stay_on_current_screen == false then
				UIButtons.TimeLineActive( "end_load", true )
			end
			
			GUIBank.end_effects_started = true
			
			if GUIBank.stay_on_current_screen == true then
				-- ditch the subscreens here
				EndScreen( UIEnums.Context.CarouselApp )
			end
		end
		
		GUIBank.load_end_delay_timer = GUIBank.load_end_delay_timer + time
		
		if GUIBank.load_end_delay_timer > GUIBank.load_end_delay then
			-- stop music
			local has, dummy = Amax.HasStartCutScene()
			if has == true and dummy == false then
				print( "SendMessage : VideoPresent" )
				UIGlobals.VideoPresent = true
				Amax.SendMessage( UIEnums.GameFlowMessage.VideoPresent )
			else
				UIGlobals.VideoPresent = nil
			end
			
			Amax.SendMessage( UIEnums.GameFlowMessage.StartGameRendering )
			StartIngameMusic()
			
			if GUIBank.stay_on_current_screen == false then
				if GUIBank.load_first_time == false then
					UIButtons.TimeLineActive( "end_load_fade", true )
				end
			end
			
			-- go go go
			GUIBank.loading = false
			
			UIGlobals.IsLoading = false
			UIGlobals.IsIngame = true
			UIGlobals.IsQuickRestart = false
			--UIScreen.SetScreenTimers( 0, 0 )

			if Amax.IsGameModeNetworked() == true then
				NetRace.StartPlay( )
				GoScreen( "Multiplayer\\Ingame\\MpRollingStart.lua" )
			else
				if Amax.GetGameMode() == UIEnums.GameMode.SinglePlayer then
					GoScreen( "Ingame\\IntroVista.lua" )
				else
					Amax.SendMessage( UIEnums.GameFlowMessage.RaceStarted )

					if IsSplitScreen() == true then
						Amax.SendMessage( UIEnums.GameFlowMessage.RaceLoaded )
						GoScreen( "Multiplayer\\Ingame\\MpRollingStart.lua" )
					else
						GoScreen( "Ingame\\HUD.lua" )
					end
				end
			end
		
			-- print( "LOAD FINISHED - ", GUIBank.load_timer )
		end
	end
end

function MessageUpdateLoading( message, arg1, arg2, arg3, arg4, arg5 )
	if GUIBank.loading == false then
		return
	end
	
	if message == UIEnums.GameFlowMessage.RaceLoaded and UIScreen.Context() == 0 then
		Amax.SetGlobalFade(1)
		Amax.FillStreamer()
--		UIButtons.SetActive( GUIBank.fade_to_black_id, true )
		GUIBank.load_finished = true
	end	
end

-- return false if you wish to skip the FrameUpdate() MessageUpdate() functions on the current screen..
function GlobalUpdate()

	-- are we waiting to return
	if UIGlobals.UserKickBackActive == true and UIGlobals.IsIngame == false and UIGlobals.UserKickBackMode ~= UIEnums.UserKickBackMode.None then
		-- Only once all other screens have successfully ended
		if UIScreen.IsPopupActive() == false and SubScreenActive() == false and 
			UIScreen.IsContextActive( UIEnums.Context.LoadSave ) == false and
			UIScreen.IsContextActive( UIEnums.Context.CarouselApp ) == false then
			-- [Bone] #35999 : Environment state not reset when kicked out
			Amax.ChangeEnvironmentSettings( false )

			GoScreen( UIGlobals.UserKickBackScreen, 0 )
			UIGlobals.UserKickBackActive = false
		end
	end
	
	if UIGlobals.ActUponFriendChallenges == true and UIGlobals.PendingFriendChallengeInfo.Pending == true and UIGlobals.IsLoading == false and UIScreen.GetCurrentScreen() ~= "loading\\loadingui.lua" then
		ActUponPendingFriendChallenge()
		-- Only once all other screens have successfully ended
		if UIScreen.IsPopupActive() == false and UIScreen.IsContextActive( UIEnums.Context.LoadSave ) == false and SubScreenActive() == false then
			Amax.ChangeEnvironmentSettings( false )
			
			ReturnToSpMain()
			UIGlobals.PendingFriendChallengeInfo = {}
		end
	end

	net_CheckFriendsAvailability( )
	
	if net_GlobalUpdate( ) == false then
		return false
	end
	
	return true
end


function RemoveBlaggedProfiles( )
	for i = 0, 3, 1 do
		if UIGlobals.ProfileState[ i ] == UIEnums.Profile.Blagged then
			if Profile.GetPrimaryPad( ) ~= i then	-- don't remove primary profile
				Profile.ClearProfile( i )
				GameProfile.ClearGameProfile( i, true )
				UIGlobals.ProfileState[ i ] = UIEnums.Profile.None
				UIGlobals.LoadProfile[ i ] = false
			end
		end
	end
 
end


 --[[****************************************************************************************
 
	SCUI Functions : Keep this at the bottom so we can find it. Maybe stick it in its own
					 file once i get time.
 
 ****************************************************************************************]]--
function AddCutSceneBars( arg ) -- pass a goolie boolie
	local top_id = UIButtons.CloneXtGadgetByName( "SCUIBank", "TopCutSceneBar" )
	local bot_id = UIButtons.CloneXtGadgetByName( "SCUIBank", "BotCutSceneBar" )

	if arg == nil then
		UIButtons.PrivateTimeLineActive( top_id, "move", true, 1 )
		UIButtons.PrivateTimeLineActive( bot_id, "move", true, 1 )

	elseif arg == 0 then -- move out
		UIButtons.PrivateTimeLineActive( top_id, "move", false, 1 )
		UIButtons.PrivateTimeLineActive( bot_id, "move", false, 1 )

	elseif arg == 1 then -- move in
		UIButtons.PrivateTimeLineActive( top_id, "move", true, 0 )
		UIButtons.PrivateTimeLineActive( bot_id, "move", true, 0 )
	end
end


function DarkenScreen( arg )
	local id = UIButtons.CloneXtGadgetByName( "SCUIBank", "Gfx_Darken" )

	if arg == nil then
		UIButtons.PrivateTimeLineActive( id, "fade", true, 1 )

	elseif arg == 0 then -- fade out
		UIButtons.PrivateTimeLineActive( id, "fade", false, 1 )

	elseif arg == 1 then -- fade in
		UIButtons.PrivateTimeLineActive( id, "fade", true, 0 )
	end
end

function LocalGamerPicture( id, pad )
	local texture = nil

	if pad ~= nil then
		texture = 
		{
			filename = "GAMERPIC_" .. pad,
			name = "GAMERPIC_" .. pad,
			pos = { u = 0, v = 0 },
			size = { u = 1, v = 1 },
		}
	else
		texture = 
		{
			filename = "default_gamerpic",
			name = "default_gamerpic",
			pos = { u = 0, v = 0 },
			size = { u = 1, v = 1 },
		}
	end

	UIButtons.ChangeTexture( texture, 1, id )
end

function RemoteGamerPicture( id, slot )
	local texture = nil

	if slot ~= nil then
		texture = 
		{
			filename = "REMOTE_GAMERPIC_" .. slot,
			name = "REMOTE_GAMERPIC_" .. slot,
			pos = { u = 0, v = 0 },
			size = { u = 1, v = 1 },
		}
	else
		texture = 
		{
			filename = "default_gamerpic",
			name = "default_gamerpic",
			pos = { u = 0, v = 0 },
			size = { u = 1, v = 1 },
		}
	end

	UIButtons.ChangeTexture( texture, 1, id )
end

function AIGamerPicture( id, index )
	local size = 1 / UIGlobals.NumMajorAvatarIcons
	local texture = 
	{
		filename = "member_icons",
		pos = { u = size * index, v = 0 },
		size = { u = size, v = 1 },
	}
	UIButtons.ChangeTexture( texture, 1, id )
end

function AddLoadingSegs( fX, fY, fSizeX, fSizeY, uPanel )
	-- defaults
	fX = fX or ( Screen.safe.right - 15 )
	fY = fY or ( Screen.safe.bottom - 15 )
	fSizeX = fSizeX or 30
	fSizeY = fSizeY or 30
	uPanel = uPanel or UIEnums.Panel._2DAA
	
	local root_id = UIButtons.CloneXtGadgetByName( "SCUIBank", "Dmy_LoadingSegs" )

	-- Make comparable size to transmitter
	if IsNumber( fSizeX ) == true and IsNumber( fSizeY ) == true then
		UIButtons.ChangeScale( root_id, fSizeX * 0.5, fSizeY * 0.5 )
	end
	
	UIButtons.ChangePosition( root_id, fX, fY )

	if IsNumber( uPanel ) == true then
		UIButtons.ChangePanel( root_id, uPanel )
		
		-- Flip the rotations of the segments?
		local flip_rot = false
		if UISystem.IsPanel3D(uPanel) == true then
			flip_rot = true
		end
		-- Update all the segments
		for i = 1, 8 do			
			UIButtons.ChangePanel( UIButtons.FindChildByName( root_id, "Gfx_Seg" .. i ), uPanel )
						
			if flip_rot == true then
				UIButtons.ChangeOrientation( UIButtons.FindChildByName( root_id, "Gfx_Seg" .. i ), 0, 0, 180 )
			end
		end
	end
	
	return root_id
end

function AddTransmitter( fX, fY, fSizeX, fSizeY, uPanel, layer, do_effect, disable, text, text_wrap_size )
	local root_id = UIButtons.CloneXtGadgetByName( "SCUIBank", "Gfx_BleepBleep" )

	local mast_id = UIButtons.FindChildByName( root_id, "Mast" )
	local one_id = UIButtons.FindChildByName( root_id, "One" )
	local two_id = UIButtons.FindChildByName( root_id, "Two" )
	local three_id= UIButtons.FindChildByName( root_id, "Three" )
	local text_id= UIButtons.FindChildByName( root_id, "BleepBleepText" )
	
	if text_wrap_size == nil then
		text_wrap_size = 0
	end
	
	UIButtons.ChangeSize( mast_id, fSizeX, fSizeY )
	UIButtons.ChangeSize( one_id, fSizeX, fSizeY )
	UIButtons.ChangeSize( two_id, fSizeX, fSizeY )
	UIButtons.ChangeSize( three_id, fSizeX, fSizeY )
	UIButtons.ChangeSize( text_id, text_wrap_size, fSizeY )

	UIButtons.ChangePosition( root_id, fX, fY )

	if uPanel ~= nil then
		UIButtons.ChangePanel( root_id, uPanel )
		UIButtons.ChangePanel( mast_id, uPanel )
		UIButtons.ChangePanel( one_id, uPanel )
		UIButtons.ChangePanel( two_id, uPanel )
		UIButtons.ChangePanel( three_id, uPanel )
		UIButtons.ChangePanel( text_id, uPanel )
	end
	
	if layer ~= nil then
		UIButtons.ChangeLayer( root_id, layer )
		UIButtons.ChangeLayer( mast_id, layer )
		UIButtons.ChangeLayer( one_id, layer )
		UIButtons.ChangeLayer( two_id, layer )
		UIButtons.ChangeLayer( three_id, layer )
		UIButtons.ChangeLayer( text_id, layer )
	end
	
	if text ~= nil then
		UIButtons.SetActive( text_id, true )
		UIButtons.ChangeText( text_id, text )
	end
	
	if do_effect ~= false then
		TransmitterOn( root_id )
	end
	
	if disable == true then
		UIButtons.SetActive( root_id, false, true )
	end
	
	return root_id
end

function TransmitterOn( root_id )
	local mast_id = UIButtons.FindChildByName( root_id, "Mast" )
	local one_id = UIButtons.FindChildByName( root_id, "One" )
	local two_id = UIButtons.FindChildByName( root_id, "Two" )
	local three_id= UIButtons.FindChildByName( root_id, "Three" )
	
	UIButtons.PrivateTimeLineActive( mast_id, "bleep_bleep", true, 0 )
	UIButtons.PrivateTimeLineActive( one_id, "bleep_bleep", true, 0 )
	UIButtons.PrivateTimeLineActive( two_id, "bleep_bleep", true, 0 )
	UIButtons.PrivateTimeLineActive( three_id, "bleep_bleep", true, 0 )
end

function TransmitterOff( root_id )
	local mast_id = UIButtons.FindChildByName( root_id, "Mast" )
	local one_id = UIButtons.FindChildByName( root_id, "One" )
	local two_id = UIButtons.FindChildByName( root_id, "Two" )
	local three_id= UIButtons.FindChildByName( root_id, "Three" )
	
	UIButtons.PrivateTimeLineActive( mast_id, "bleep_bleep", false )
	UIButtons.PrivateTimeLineActive( one_id, "bleep_bleep", false )
	UIButtons.PrivateTimeLineActive( two_id, "bleep_bleep", false )
	UIButtons.PrivateTimeLineActive( three_id, "bleep_bleep", false )
end

function InitMpGlobals( )
	MpGlobals = 
	{
		group_id = 0,
		user_created_group = false,
		number_of_drivers = 20,
		teams = 0,
		number_of_teams = 2,
		number_of_races = 1,
		current_race_index = 1,
		each_race_the_same = 0,
		public_game = 1,

		race_setup = { },
	}
end

-- main profile data
function StartAsyncLoad()
	UIGlobals.FileParams = 
	{
		SetInterface	= GameProfile.SetSaveInfoInterfaceProfile,
		FinishedMessage	= UIEnums.GameFlowMessage.SaveGameLoaded,
	}
	
	StoreScreen( UIEnums.ScreenStorage.LOAD_NEXT, "EndScreen" )
	UIScreen.SetNextScreen( "Profile\\AsyncLoad.lua", UIEnums.Context.LoadSave )
	
	return true
end

-- main profile data
function StartAsyncSave()
	if CanSave( Profile.GetPrimaryPad() ) == false then
		return false
	end

	UIGlobals.FileParams = 
	{
		SetInterface	= GameProfile.SetSaveInfoInterfaceProfile,
	}
	
	StoreScreen( UIEnums.ScreenStorage.SAVE_NEXT, "EndScreen" )
	UIScreen.SetNextScreen( "Profile\\AsyncSave.lua", UIEnums.Context.LoadSave )
	
	return true
end

-- photos
function StartAsyncLoadPhotos()
	if CanSave( Profile.GetPrimaryPad(), true ) == false then
		return false
	end
	
	UIGlobals.FileParams = 
	{
		SetInterface	= GameProfile.SetSaveInfoInterfacePhotos,
		FinishedSuccess	= false,
	}
	
	UIGlobals.ProfileEnumerateIndex = Profile.GetPrimaryPad()
	
	GUIBank.photo_viewing_download = false
	StoreScreen( UIEnums.ScreenStorage.SAVE_NEXT, "Photos\\ViewPhoto.lua" )
	
	--GoScreen( "Profile\\AsyncEnumerate.lua" )
	--UIScreen.SetNextScreen( "Profile\\AsyncEnumerate.lua", UIEnums.Context.LoadSave )
	
	return true
end

function StartAsyncSavePhotos()
	if CanSave( Profile.GetPrimaryPad(), true ) == false then
		return false
	end
	
	UIGlobals.FileParams = 
	{
		SetInterface	= GameProfile.SetSaveInfoInterfacePhotos,
	}
	
	StoreScreen( UIEnums.ScreenStorage.SAVE_NEXT, "EndScreen" )
	UIScreen.SetNextScreen( "Profile\\AsyncSave.lua", UIEnums.Context.LoadSave )
	
	return true
end

-- Call the add SCUIBank code
UIScreen.AddXtRecord( "SCUIBank", false )
if IsTable( SCUIBank ) == true then
	SCUIBank.name_to_index = { }
	for i,v in pairs( SCUIBank.elements ) do
		if IsString( v.name ) then
			SCUIBank.name_to_index[ v.name ] = i
		end
	end
end

function AddSCUIBankElement( name )
	return UIButtons.CloneXtGadgetByName( "SCUIBank", name )
end

function CreateGroupOverview( group, index )

	local root_id = UIButtons.CloneXtGadgetByName( "SCUIBank", "node_group" )

	local name_id		= UIButtons.FindChildByName( root_id, "name" )
	local desc_id		= UIButtons.FindChildByName( root_id, "desc" )
	local owner_id		= UIButtons.FindChildByName( root_id, "owner" )

	-- change any data
	if group ~= nil then
		UIButtons.ChangeText( name_id, "MPL_GROUP_NAME" .. index )
		UIButtons.ChangeText( desc_id, "MPL_GROUP_DESC" .. index )
		UIButtons.ChangeText( owner_id, "MPL_GROUP_OWNER" .. index )

		-- display rating
		local rating = 1 -- TODO : link up from group data
		for i = 1, rating do 
			UIButtons.ChangeColour( UIButtons.FindChildByName( root_id, "rating_" .. i ), "Support_4" )
			UIButtons.ChangeAlpha( UIButtons.FindChildByName( root_id, "rating_" .. i ), 1.0 )
		end
	else
		UIButtons.SetActive( owner_id, false )

		UIButtons.ChangeColour( name_id, "Main_Black" )
		UIButtons.ChangeColour( desc_id, "Main_Black" )
		UIButtons.ChangeColour( owner_id, "Main_Black" )
		--UIButtons.ChangeColour( gamerpic_id, "Main_Black" )

		UIButtons.ChangeColour( UIButtons.FindChildByName( root_id, "rating" ), "Main_Black" )
		UIButtons.ChangeColour( UIButtons.FindChildByName( root_id, "frame" ), "Main_Black" )
	end
	return root_id
end

function ForceCloseScreens()
	EndScreen( UIEnums.Context.LoadSave )
	
	-- set the screen timers to 0 so they close instantly
	UIScreen.SetScreenTimers( 0, 0, UIEnums.Context.CarouselApp )
	UIScreen.SetScreenTimers( 0, 0, UIEnums.Context.Subscreen0 )
	UIScreen.SetScreenTimers( 0, 0, UIEnums.Context.Subscreen1 )
	UIScreen.SetScreenTimers( 0, 0, UIEnums.Context.Subscreen2 )

	-- nuke them all
	EndScreen( UIEnums.Context.CarouselApp )
	FlushSubscreens()
end

function ReturnToStartScreen( eUserKickBackMode )
	-- make sure we dont try and kick back again
	if UserKickBackActive() == false and UIGlobals.UserKickBackMode == UIEnums.UserKickBackMode.None then
		if UIGlobals.IsIngame == true and UIScreen.GetCurrentScreen() ~= "loading\\loadingui.lua" then
			Amax.SendMessage( UIEnums.GameFlowMessage.UnPause )
			Amax.SendMessage( UIEnums.GameFlowMessage.QuitRace )
			
			StoreScreen( UIEnums.ScreenStorage.FE_RETURN, UIGlobals.UserKickBackScreen )
			GoScreen( "Loading\\LoadingUi.lua", UIEnums.Context.Main )
		end
		
		-- clear the popup
		UIScreen.CancelPopup()
		
		ForceCloseScreens()
		
		if IsTable( PopupGUI ) == true then
			PopupGUI.spawn_next = nil
		end

		-- this is safe to call from a non-network game
		net_FlushEverything()
	end
	
	-- need to reset this otherwise the start screen will think we have returned normally
	UIGlobals.ReturnMode = UIEnums.ReturnMode.None

	UIGlobals.UserKickBackMode = eUserKickBackMode
	UIGlobals.UserKickBackActive = true

	print( "Returning to start screen because of ", eUserKickBackMode )
	show_table( UIEnums.UserKickBackMode, 1 )
end

function UserKickBackActive()
	if UIGlobals.UserKickBackMode ~= UIEnums.UserKickBackMode.None then
		return true
	end
	
	return false	
end

function ReturnToSpMain()
	-- only if we arnt on our way back to the start screen
	if UserKickBackActive() == true then
		return
	end

	if UIGlobals.IsIngame == true and UIScreen.GetCurrentScreen() ~= "loading\\loadingui.lua" then
		Amax.SendMessage( UIEnums.GameFlowMessage.UnPause )
		Amax.SendMessage( UIEnums.GameFlowMessage.QuitRace )
		
		StoreScreen( UIEnums.ScreenStorage.FE_RETURN, "SinglePlayer\\SpMain.lua" )
		GoScreen( "Loading\\LoadingUi.lua" )
	elseif UIGlobals.IsIngame == false then
		GoScreen( "SinglePlayer\\SpMain.lua" )
	end

	-- clear the popup
	UIScreen.CancelPopup()
	PopupGUI.spawn_next = nil

	-- this is safe to call from a non-network game
	net_FlushSessionEnumerator()
	net_CloseAllSessions()
end

function ProfileSignedInOut( signed_in, index )
	if signed_in == true then
		print( "ProfileSignedIn", index )
	else
		print( "ProfileSignedOut", index )
	end
	
	local is_active_pad = false
	
	if IsSplitScreen() == true then
		is_active_pad = ( ( UIGlobals.ProfileState[ index ] == UIEnums.Profile.Blagged ) or ( UIGlobals.ProfileState[ index ] == UIEnums.Profile.GamerProfile ) )
	elseif Amax.IsGameModeNetworked() == true then
		is_active_pad = ( index == Profile.GetPrimaryPad() )
	else	
		is_active_pad = ( index == Profile.GetPrimaryPad() )
	end

	if is_active_pad == false then
		if signed_in == true then
			-- setup new profile for pre-load
			Profile.Setup( index )
			UIGlobals.ProfilesFound = UIGlobals.ProfilesFound + 1
			UIGlobals.DoNotSave[ index ] = false
			UIGlobals.ProfileState[ index ] = UIEnums.Profile.PreLoad
		else
			-- clear profile
			UIGlobals.ProfileState[ index ] = UIEnums.Profile.None
			Profile.ClearProfile( index )
			UIGlobals.ProfilesFound = UIGlobals.ProfilesFound - 1
		end
	else
		-- kick
		print( "kicking you ... BYE!" )
		
		Amax.KickedOutOfGame();

		-- eeek, we have managed to get the disconnect from live first, it will have already started the load so ignore it
		-- and show the sign out message instead. Hooooway
		ReturnToStartScreen( UIEnums.UserKickBackMode.UserChanged )
	end
end

function StartLoad()
	-- disable loading
	if GUI.DisableLoading ~= true then

		Debug.IncrementMemoryTag("game")
		
		-- disable friends on ps3/pc while racing.
		net_LockoutFriendsOverlay( true ) 

		print( "LoadingGame : StartGameLoad" )
		UIGlobals.IsLoading = true

		Amax.SendMessage(UIEnums.GameFlowMessage.StopGameRendering)
		Amax.SendMessage(UIEnums.GameFlowMessage.StartRaceLoad)
	end
end

function StartLobbyNetworkLoad()
	if UIScreen.IsPopupActive( ) == true then
		UIScreen.CancelPopup( )
	end

	print( "StartLobbyNetworkLoad" )
	Debug.IncrementMemoryTag("game")

	NetRace.EnterRaceLoad() -- change players states so peers know we are loading.

	UIGlobals.Multiplayer.LaunchScreen = UIEnums.MpLaunchScreen.MultiplayerLobby
	UIGlobals.IsLoading = true
	UIGlobals.IsQuickRestart = false

	Amax.SendMessage(UIEnums.GameFlowMessage.StopGameRendering)
	
	StopFrontendMusic()
	GoLoadingScreen("Loading\\LoadingMpGame.lua", 0 )
end

function UpdateSpStatusBar()
	if IsTable( GUIBank.SpStatusBar ) == false then
		return
	end

	UIButtons.SetActive( GUIBank.SpStatusBar.branch_id, true )
	UIButtons.ChangeScale( GUIBank.SpStatusBar.progress_id, SinglePlayer.GameInfo().rankProgress, 1, 1 )
end

function UpdateMpStatusBar()
	if IsTable( GUIBank.MpStatusBar ) == false then
		return
	end
	
	local is_online = Amax.IsGameModeOnline()
	
	UIButtons.SetActive( GUIBank.MpStatusBar.branch_id, true )
	UIButtons.SetActive( GUIBank.MpStatusBar.branch_online_id, is_online )
	
	if is_online == true then
		local progression = Multiplayer.ProfileProgression()
		
		if IsTable( progression ) == true then
			Mp_RankIcon( GUIBank.MpStatusBar.level_icon_id, progression.level - 1, progression.legend )	
			UIButtons.ChangeText( GUIBank.MpStatusBar.level_text_id, "GAME_NUM_" .. progression.level )

			-- set progress bar size
			Mp_SetupPlayerProgressBar( GUIBank.MpStatusBar.branch_online_id )
		end
	end
end

function AddStatusBar( multiplayer )
	local parent_id = UIButtons.CloneXtGadgetByName( "SCUIBank", "Dmy_StatusBar" )
	local bar_id = UIButtons.FindChildByName( parent_id, "Gfx_StatusBarInner" )
	local icon_id = UIButtons.FindChildByName( parent_id, "Shp_DlgLogo" )
	
	GUIBank.SpStatusBar = nil
	GUIBank.MpStatusBar = nil

	local sb_active = false
	if multiplayer ~= nil then
		UIShape.ChangeObjectName( icon_id, "groups" )

		UIButtons.ChangeColour( icon_id, "Support_1" )
		UIButtons.ChangeColour( UIButtons.FindChildByName( parent_id, "Gfx_StatusBar" ), "Support_1" )
--		UIButtons.ChangeColour( UIButtons.FindChildByName( parent_id, "Gfx_StatusBarInner" ), "Support_1" )
		
		GUIBank.MpStatusBar = 
		{
			branch_id			= UIButtons.FindChildByName( parent_id, "Bra_MpStatusBar" ),
			
			branch_online_id	= UIButtons.FindChildByName( parent_id, "Bra_MpOnlineBar" ),
			level_text_id		= UIButtons.FindChildByName( parent_id, "Txt_MpLevel" ),
			level_icon_id		= UIButtons.FindChildByName( parent_id, "Gfx_MpLevel" ),
		}
		UpdateMpStatusBar()
	else
		GUIBank.SpStatusBar = 
		{
			branch_id		= UIButtons.FindChildByName( parent_id, "Bra_SpStatusBar" ),
			progress_id		= UIButtons.FindChildByName( parent_id, "progress" ),
		}
		
		UpdateSpStatusBar()
	end

	return parent_id
end

function FindRemotePictureIndex( join_ref, pictures )
	local picture_index = 0
	for i, v in ipairs( pictures ) do
		if join_ref == v.join_ref then
			picture_index = i - 1
			break
		end
	end
	return picture_index
end

function DialogueBox_Resize( dlg_box_id, body )
	local t = SCUIBank.elements[ SCUIBank.name_to_index[ "Txt_DlgBoxMessage" ] ]
	if IsTable( t ) == false then
		return
	end
	
	local nl = UISystem.GetNumStringLines( body, t.size.y, t.size.x, t.styles )
	local ny = 4 + (t.size.y * nl)
	local x,y,z = UIButtons.GetSize( UIButtons.FindChildByName( dlg_box_id, "Box_DlgBox" ) )
	if ny > y then
		UIButtons.ChangeSize( UIButtons.FindChildByName(dlg_box_id, "Box_DlgBox" ), x, ny )
	end
end

function StopFrontendMusic()
	print("[StopFrontendMusic]")
	Amax.SendMessage( UIEnums.GameFlowMessage.StopFrontendAudio )
end

function StartIngameMusic()
	print("[StartIngameMusic]")
	Amax.SendMessage( UIEnums.GameFlowMessage.StartIngameAudio )
end

function StopIngameMusic()
	print("[StopIngameMusic]")
	Amax.SendMessage( UIEnums.GameFlowMessage.StopIngameAudio )
end

function GarageCheat_Reload()
	UIGlobals.GarageCheatReload = true
end

function Friends_CheckForEnable()
	if UIEnums.CurrentPlatform ~= UIEnums.Platform.PS3 and UIEnums.CurrentPlatform ~= UIEnums.Platform.PC then
		return
	end
	
	if Profile.IsFriendsEnabled() == true and Profile.IsFriendsDisplayed() == false then
		if GUI.friends_enable_id == nil then
			GUI.friends_enable_id = UIButtons.CloneXtGadgetByName( "SCUIBank", "_info_lineR", InfoLineContext )
			UIButtons.ChangeText( GUI.friends_enable_id, UIText.INFO_BK_FRIENDS )
			UIButtons.ChangeJustification( GUI.friends_enable_id, UIEnums.Justify.BottomRight )
			UIButtons.ChangePosition( GUI.friends_enable_id, Screen.safe.right, Screen.safe.bottom )
			UIButtons.ReplaceTimeLineLabel( GUI.friends_enable_id, "SetupInfoLineR", "FriendsActive" )
		else
			if GUI.friends_active ~= true then
				UIButtons.SetActive( GUI.friends_enable_id, true )
				UIButtons.TimeLineActive( "FriendsActive", true, 0 )
				GUI.friends_active = true
			end
		end
	else
		if GUI.friends_enable_id ~= nil then
			UIButtons.SetActive( GUI.friends_enable_id, false )
			GUI.friends_active = false
		end
	end
end

function XtToScreenSpaceX( x, world_camera )
	x = x / (720 / 480)
	
	if world_camera == true then
		x = x - 320
	end
	
	x = x + (((640 - (640 * (4 / 3)) + 0.5) / 2) - 0.25) -- SCREEN_LEFT
	
	if world_camera == true then
		x = x * 0.1
	end
	
	return x
end
	
function XtToScreenSpaceY( y, world_camera )
	y = y / (720 / 480)
	
	if world_camera == true then
		y = y - 240
	end
	
	if world_camera == true then
		y = y * -0.1
	end
	
	return y
end

function AddFourThreeBars()
	local four_three_bb =
	{
		layer = UIEnums.Layer.BACKGROUND,
		panel  = UIEnums.Panel._2DAA,
		type = UIEnums.ButtonTypes.GRAPHIC,
		pos = { x = SCREEN_LEFT, y = 0, },
		size = { x = SCREEN_WIDTH + 2, y = 0, },
		colour_style = "!255 0 0 0",
		textures = 
		{
			{
				pos = {	u = 0, v = 0, },
				size = { u = 1, v = 1, },
			},
		},
--		blend_mode = UIEnums.BlendMode.Solid,
		effect_index = UIEnums.EffectIndex.OneColour,
	}
	
	local temp = 480 - (640 * 9 / 16)
	
	-- Top
	four_three_bb.size.y = temp/2
	UIButtons.AddButton(four_three_bb)
	
	-- Bottom
	four_three_bb.pos.y = 480 - temp/2
	UIButtons.AddButton(four_three_bb)
end

-- [Blur2PC] PC layout follows the actual back buffer: 480 virtual units high,
-- with width derived from the viewport. The UI camera already uses that aspect.
-- No additional frontend zoom or aspect-ratio mask is needed.

-- retail IsWideScreen: 16:9 or wider (PC); consoles ask the hardware
function IsWideScreen()
	if UIEnums.CurrentPlatform ~= UIEnums.Platform.PC then
		return UIHardware.WideScreen()
	end
	return UIGlobals.resolution_aspect >= 16 / 9
end

-- retail CalcScreenBounds: the Screen table for the current SCREEN_* values
function CalcScreenBounds()
	Screen =
	{
		top		= 0,
		bottom	= 480,
		left	= SCREEN_LEFT,
		right	= SCREEN_RIGHT,
		width	= SCREEN_WIDTH,
		height	= 480,
		safe =
		{
			top		= 36,
			bottom	= 444,
			left	= SCREEN_LEFT + SCREEN_WIDTH * 0.075,
			right	= SCREEN_RIGHT - SCREEN_WIDTH * 0.075,
			width	= SCREEN_WIDTH * 0.85,
			height	= 408,
		},
	}
end

-- retail ChangeResolution, called with Amax.GetScreenResolution() when this file loads and after the
-- graphics options change
function ChangeResolution( width, height )
	if IsNumber( width ) ~= true or IsNumber( height ) ~= true then
		print( "ChangeResolution : Invalid number" )
		return
	end

	if width <= 0 or height <= 0 then
		print( "ChangeResolution : Invalid dimension", width, height )
		return
	end

	UIGlobals.resolution_width = width
	UIGlobals.resolution_height = height
	UIGlobals.resolution_aspect = width / height

	if UIEnums.CurrentPlatform ~= UIEnums.Platform.PC then
		return
	end

	SCREEN_WIDTH = width * ( 480 / height )
	SCREEN_LEFT = ( ( 640 - SCREEN_WIDTH ) / 2 ) - 0.25
	SCREEN_RIGHT = SCREEN_LEFT + SCREEN_WIDTH
	CalcScreenBounds()
	Camera_UseFrontend()
end

function Camera_UseFrontend()
	if UIEnums.CurrentPlatform == UIEnums.Platform.PC then
		-- [Blur2PC] Use the viewport-derived projection directly. Retail's extra
		-- width-based zoom makes menus taller than the viewport on wide displays.
		UISystem.SetCameraScale( true, 0, 1 )
		return
	end

	if UIHardware.WideScreen() == true then
		return
	end
	
	UIGlobals.ScaleCamera = true
	UISystem.SetCameraScale( true, 0, 3/4 )
end

function Camera_UseInGame()
	if UIEnums.CurrentPlatform == UIEnums.Platform.PC then
		-- [Blur2PC] retail
		UISystem.SetCameraScale( true, 0, 1 )
		return
	end

	if UIHardware.WideScreen() == true then
		return
	end
	
	UISystem.SetCameraScale( true, 0, 1 )
	UIGlobals.ScaleCamera = false
end

function PS3_CheckForInstallingMessage()
	if UIGlobals.Ps3Installing == true then
		if Amax.Ps3PollInstaller() == true then
			print("PS3 : Finished install")
			UIGlobals.Ps3Installing = nil
			Amax.CleanPS3Install()
		end
	end
end

function PlaySPMovieFullScreen( VideoEnum, overrideCanPlay )

	if overrideCanPlay == nil then
		overrideCanPlay = false
	end
	
	-- Dont Want to queue these movies up so just add them to [1]
	if Amax.CanPlayGameModeVideo( VideoEnum ) == true or overrideCanPlay == true then
	
		if overrideCanPlay == false then
			--dont save if we are just playing the video from the carousel
			UIGlobals.SpMovieScreenShouldSave = true
		end

		--loop through and add movies to the array so we can queue them up
		for i, m in ipairs( UIGlobals.ActiveSPMovie ) do
			if  m.Filename == -1 then

				print("Adding Item to array in position %d", i)
	
				m.Filename, m.Skipable, m.FullScreen = Amax.GetGameModeVideoFile( VideoEnum )

				print(m.Filename)
				print(m.Skipable)
				print(m.FullScreen)
				
				if overrideCanPlay == true then
					m.FullScreen = true
					m.Skipable = true
				else
					Amax.SetVideoPlayedBitInProfile( VideoEnum )
				end

				if i == 1 then
					PushScreen( "SinglePlayer\\SpMovieScreen.lua" )
				end

				break
			end
		end

		--UIGlobals.ActiveSPMovie[1].Filename, UIGlobals.ActiveSPMovie[1].Skipable, UIGlobals.ActiveSPMovie[1].FullScreen = Amax.GetGameModeVideoFile( VideoEnum )
		
		--print(UIGlobals.ActiveSPMovie[1].Filename)
		--print(UIGlobals.ActiveSPMovie[1].Skipable)
		--print(UIGlobals.ActiveSPMovie[1].FullScreen)

		--Amax.SetVideoPlayedBitInProfile( VideoEnum )
		--PushScreen( "SinglePlayer\\SpMovieScreen.lua" )

		return true
	else
		return false
	end
end

function PlaySPUnlockMovieFullScreen( VideoEnum, tier )

	--loop through and add movies to the array so we can queue them up
	UIGlobals.SpMovieScreenShouldSave = true
	
	for i, m in ipairs( UIGlobals.ActiveSPMovie ) do
		if  m.Filename == -1 then

			print("Adding Item to array in position %d", i)

			m.Filename, m.Skipable, m.FullScreen = Amax.GetUnlocksVideoFile( VideoEnum, tier )

			print(m.Filename)
			print(m.Skipable)
			print(m.FullScreen)
			
			if i == 1 then
				PushScreen( "SinglePlayer\\SpMovieScreen.lua" )
			end

			break
		end
	end
	
	
	return true
end

function SetupScreenTitle( textId, parent_id, icon_name, filename, layer, justify, do_no_offset, box_alpha, panel, enable_background, parent_attach )
	local dummy_id = UIButtons.CloneXtGadgetByName( "SCUIBank", "ScreenTitle_Dummy" )
	local box_dummy_id = UIButtons.FindChildByName( dummy_id, "box_dummy" )
	local outer_id = UIButtons.FindChildByName( dummy_id, "BoxFrame_Title" )
	local inner_id = UIButtons.FindChildByName( dummy_id, "BoxInner_Title" )
	local icon_id = UIButtons.FindChildByName( dummy_id, "icon" )
	local title_id = UIButtons.FindChildByName( dummy_id, "title" )
	
	UIButtons.ChangeText( title_id, textId )
	local x_padding = UIButtons.GetSize( outer_id )
	
	local box_length = UIButtons.GetStaticTextLength( title_id ) + x_padding
	UIButtons.ChangeSizeX( dummy_id, box_length )
	UIButtons.ChangeSizeX( outer_id, box_length )
	UIButtons.ChangeSizeX( inner_id, box_length )
	
	if icon_name == nil then
		UIButtons.SetActive( icon_id, false )
	else	
		if filename ~= nil then
			UIShape.ChangeSceneName( icon_id, filename )
		end
		
		UIShape.ChangeObjectName( icon_id, icon_name )
	end
	
	if panel ~= nil then
		UIButtons.ChangePanel( dummy_id, panel, true )
	end
	
	if layer ~= nil then
		UIButtons.ChangeLayer( dummy_id, layer, true )
		UIButtons.ChangeLayer( title_id, layer + 1, true )
	end
	
	if justify ~= nil then
		UIButtons.ChangeJustification( dummy_id, justify )
	end
	
	if do_no_offset == true then
		local x, _, z = UIButtons.GetPosition( dummy_id )
		UIButtons.ChangePosition( dummy_id, x, 0, z )
	end
	
	if box_alpha ~= nil then
		UIButtons.SetAlphaIntensity( box_dummy_id, box_alpha, nil )
	end

	if Amax.GetGameMode() ~= UIEnums.GameMode.SinglePlayer then
		UIButtons.SwapColour( dummy_id, "Main_2", "Support_1" )
		UIButtons.SwapColour( dummy_id, "Main_1", "Support_1" )
	end
	
	if parent_attach ~= nil then
		UIButtons.SetParent( dummy_id, parent_id, parent_attach )
	else
		UIButtons.SetParent( dummy_id, parent_id, UIEnums.Justify.MiddleCentre )
	end
	
	return dummy_id
end

function SetupBottomHelpBar( textId, parent_id, layer, do_not_offset, bar_alpha, num_lines )
	local bottom_bar_dummy_id = UIButtons.CloneXtGadgetByName( "SCUIBank", "bottom_bar_help_dummy" )
	local bottom_bar_text_id = UIButtons.FindChildByName( bottom_bar_dummy_id, "bottom_bar_help_text" )
	local bottom_bar_mid_id = UIButtons.FindChildByName( bottom_bar_dummy_id, "bottom_bar_mid" )
	
	if IsNumber( textId ) == true then
		UIButtons.ChangeText( bottom_bar_text_id, textId )
	end
	
	-- PR - took this out - wrap size already on means we shouldn't need to do this
	--[[
	local min_backing_size_x = UIButtons.GetSize( bottom_bar_mid_id )
	local text_size = UIButtons.GetStaticTextLength( bottom_bar_text_id ) 
	
	if min_backing_size_x < text_size then
		min_backing_size_x = text_size
	end
	
	UIButtons.ChangeSizeX( bottom_bar_mid_id, min_backing_size_x )
	--]]
	
	if IsNumber( layer ) then
		UIButtons.ChangeLayer( bottom_bar_mid_id, layer - 1, true )
		UIButtons.ChangeLayer( bottom_bar_text_id, layer )
	end
	
	-- Scale up to accommodate extra lines
	if IsNumber( num_lines ) == true then
		UIButtons.ChangeScale( bottom_bar_mid_id, 1.0, num_lines )
	elseif IsNumber( textId ) == true then
		local _, sy = UIButtons.GetSize( bottom_bar_text_id )
		if IsNumber( sy ) == true then
			local num_lines = UIButtons.GetStaticTextHeight( bottom_bar_text_id ) / sy
			UIButtons.ChangeScale( bottom_bar_mid_id, 1.0, num_lines )
		end
	end
	
	--[[
	if do_not_offset == true then
		local x, _, z = UIButtons.GetPosition( bottom_bar_dummy_id )
		UIButtons.ChangePosition( bottom_bar_dummy_id, x, 0, z )
	end
	]]--
	
	if IsNumber( bar_alpha ) == true then
		UIButtons.SetAlphaIntensity( bottom_bar_mid_id, bar_alpha, nil )
	end
	
	if Amax.GetGameMode() ~= UIEnums.GameMode.SinglePlayer then
		UIButtons.SwapColour( bottom_bar_dummy_id, "Main_2", "Support_1" )
		UIButtons.SwapColour( bottom_bar_dummy_id, "Main_1", "Support_1" )
	end
	
	if IsNumber( parent_id ) == true then
		UIButtons.SetParent( bottom_bar_dummy_id, parent_id, UIEnums.Justify.BottomCentre )
	end
	
	return bottom_bar_text_id, bottom_bar_dummy_id
end

function BoolToNumber( bool )
	if bool == true then
		return 1
	else
		return 0
	end
end

function NumberToBool( num )
	if num == 1 then
		return true
	else
		return false
	end
end

function CreatePhotoPreviewPage( parent_id )

	local node_id = UIButtons.CloneXtGadgetByName( "SpSCUI", "photo_preview_dummy" )
	
	UIButtons.SetParent( node_id, parent_id, UIEnums.Justify.MiddleCentre )
	
	local ref = 
	{
		node = node_id,
		background = UIButtons.FindChildByName( node_id, "background" ),
		bottom_text = UIButtons.FindChildByName( node_id, "bottom_bar_text" ),
		top_text = UIButtons.FindChildByName( node_id, "top_bar_text" ),
	}
	
	return ref
	
end

function CustomActionButtonPressed(pad, action, friend_index)

	UIGlobals.PendingFriendChallengeInfo.Pending		= true
	UIGlobals.PendingFriendChallengeInfo.Pad			= pad
	UIGlobals.PendingFriendChallengeInfo.Action			= action
	UIGlobals.PendingFriendChallengeInfo.FriendIndex	= friend_index
	UIGlobals.PendingFriendChallengeInfo.Act			= true

end

function ActUponPendingFriendChallenge()

	if UIGlobals.ActUponFriendChallenges == false or UIGlobals.PendingFriendChallengeInfo.Pending ~= true then
		return 
	end
	
	if Profile.GetPrimaryPad() ~= UIGlobals.PendingFriendChallengeInfo.Pad then
		-- Error - don't cary on
		SetupCustomPopup( UIEnums.CustomPopups.FriendDemandCustomAttemptPadError )
		UIGlobals.PendingFriendChallengeInfo = {}
	
	else
		if UIGlobals.PendingFriendChallengeInfo.Act == true then
			-- Close current screens ready for ReturnToSpMain()
			UIGlobals.FriendDemandFilterFriend=UIGlobals.PendingFriendChallengeInfo.FriendIndex
			UIGlobals.FriendDemandAttemptFromMessage = true

			ForceCloseScreens()

			UIGlobals.PendingFriendChallengeInfo.Act=nil
		end
	end
end

function EnterSpRaceBook(goto_screen)
	-- Begin A Network session for single player
	Amax.StartCreateStatsOnlyMatchingSession()
	UIGlobals.NetworkSessionStarted = false
	--print("STARTING NETWORK SESSION")
	
	Amax.SetUICarToMultiplayer(false)		
	Amax.SendMessage(UIEnums.GameFlowMessage.LoadSPProfileCar)
	
	if goto_screen == true then
		GoScreen( "SinglePlayer\\SpMain.lua" )
		FriendDemand.RetrieveFromServer(true)
	end
	
	Amax.SetLoadStateIntoUI()
	LSP.GetLinkCode()
	
	-- PR - flush gamerpics to ditch any stored from multiplayer
	Profile.FlushRemotePictures()
end

function SetupSelectionBox( parent, text, layer, panel, box_justify, box_parent_attach, text_justify, get_box_size_from_parent, non_highlight )
	if parent == nil then
		return
	end
	
	text_justify = text_justify or UIEnums.Justify.MiddleCentre
	box_parent_attach = box_parent_attach or UIEnums.Justify.MiddleCentre
	text_parent_justify = text_justify	-- would we ever need anything different?
	
	local box_id = UIButtons.CloneXtGadgetByName( "SCUIBank", Select( non_highlight == true, "selection_box_no_highlight", "selection_box" ) )
	UIButtons.SetParent( box_id, parent, box_parent_attach )
	
	local box_id_inner = UIButtons.FindChildByName( box_id, "selection_box_inner" )
	
	if box_justify ~= nil then
		UIButtons.ChangeJustification( box_id, box_justify )
	end
	
	local text_id = UIButtons.CloneXtGadgetByName( "SCUIBank", Select( non_highlight == true, "selection_box_text_no_highlight", "selection_box_text" ) )
	
	UIButtons.SetParent( text_id, box_id, text_parent_justify )
	
	if text ~= nil then
		UIButtons.ChangeText( text_id, text )
	end
	
	local offset_scale = 0.8
	local sx_t, sy_t, sz_t = UIButtons.GetSize( text_id )
	
	if text_justify ~= nil then
		UIButtons.ChangeJustification( text_id, text_justify )
		
		local offset_x = 0
		
		if text_justify == UIEnums.Justify.TopLeft or text_justify == UIEnums.Justify.MiddleLeft or text_justify == UIEnums.Justify.BottomLeft then
			offset_x = sy_t * offset_scale
		elseif text_justify == UIEnums.Justify.TopRight or text_justify == UIEnums.Justify.MiddleRight or text_justify == UIEnums.Justify.BottomRight then
			offset_x = -sy_t * offset_scale
		end
		
		if offset ~= 0 then
			local px, py, pz = UIButtons.GetPosition( text_id )
			UIButtons.ChangePosition( text_id, px + offset_x, py, pz )
		end
	end
	
	if layer ~= nil then
		UIButtons.ChangeLayer( box_id, layer, true )
		UIButtons.ChangeLayer( box_id_inner, math.max( layer - 1, 0 ) )
	end
	
	if panel ~= nil then
		UIButtons.ChangeLayer( box_id, panel, true )
	end
	
	-- size box	
	if get_box_size_from_parent == true then
		local sx, sy, sz = UIButtons.GetSize( parent )
		UIButtons.ChangeSize( box_id, sx, sy, sz )
		UIButtons.ChangeSize( box_id_inner, sx, sy, sz )
		UIButtons.ChangeSize( text_id, sx - ( sy_t * offset_scale * 2 ), sy_t, sz_t )	-- wrap text to box
		
	elseif text_id ~= nil then
		-- size to text - assume no wrapping - would we ever need it?
		local sx, sy, sz = UIButtons.GetSize( box_id )
		local gap_x = 9
		local text_length = UIButtons.GetStaticTextLength( text_id )
		local length = text_length + gap_x
		UIButtons.ChangeSize( box_id, length, sy, sz )
		UIButtons.ChangeSize( box_id_inner, length, sy, sz )
	end
	
	return box_id, text_id
end

function GetResultColour( result )
	if IsString( result ) == true then
		if result == "gold" then
			return "Pos_First"
		elseif result == "silver" then
			return "Pos_Second"
		elseif result == "bronze" then
			return "Pos_Third"
		end
	elseif IsNumber( result ) == true then
		if result == 1 then
			return "Pos_First"
		elseif result == 2 then
			return "Pos_Second"
		elseif result == 3 then
			return "Pos_Third"
		end
	end

	return "Pos_None"
end

function DeferCam_Init( type, timer, lerp_time, viewport )
	-- Set defaults
	if IsNumber( timer ) == false then
		timer = 0.1
	end
	if IsNumber( lerp_time ) == false then
		lerp_time = 0.3
	end
	if IsNumber( viewport ) == false then
		viewport = 0
	end
	
	GUI.camera = 
	{
		type		= type,
		timer		= timer,
		lerp_time	= lerp_time,
		viewport	= viewport,
	}
end

function DeferCam_Update( time )
	--
	if IsTable( GUI.camera ) == false then
		return false
	end
	
	GUI.camera.timer = GUI.camera.timer - time
	if GUI.camera.timer <= 0 then
		if IsString( GUI.camera.type ) == true then
			-- Simple
			Amax.ChangeUiCamera( GUI.camera.type, GUI.camera.lerp_time, GUI.camera.viewport )
		else
			-- Extended
			if GUI.camera.type == UIEnums.ExtendedCameraType.IntoGarage then
				Amax.ChangeUiSplineCamera( GUI.camera.lerp_time, GUI.camera.viewport, nil, "_ShowRoomCamera", nil, nil, nil, 2 )
			else
				print("Unknown camera type", GUI.camera.type )
			end
		end
		GUI.camera = nil
		
		return true
	end
	
	return false
end

function UiPanelBloom( id, use_bloom, include_children, light )
	if IsNumber( id ) == false then
		return
	end
	
	-- Default setting
	if include_children ~= false then
		include_children = true
	end
	
	local panel = Select( use_bloom, Select( (light == true), UIEnums.Panel._3DAA_LIGHT, UIEnums.Panel._3DAA_0), UIEnums.Panel._3D )
	
	UIButtons.ChangePanel( id, panel, include_children )
	UIButtons.SetAlphaIntensity( id, nil, Select( use_bloom, 0.4, 1 ) )
end


function WorldPanelBloom( id, use_bloom, include_children )
	if IsNumber( id ) == false then
		return
	end
	
	-- Default setting
	if include_children ~= false then
		include_children = true
	end
	
	local panel = Select( use_bloom, UIEnums.Panel._3DAA_WORLD, UIEnums.Panel._3D_WORLD )
	
	UIButtons.ChangePanel( id, panel, include_children )
	UIButtons.SetAlphaIntensity( id, nil, Select( use_bloom, 0.4, 1 ) )
end

-- called from Lua_NetServices.cpp
function Presence_IsPlayingSplitscreen( pad )
	return ( UIGlobals.ProfileState[ pad ] == UIEnums.Profile.GamerProfile )
end

function FadeUpLoading()
	if IsNumber( UIGlobals.FadeUpLoading ) ~= true then
		return
	end
	
	if UIGlobals.FadeUpLoading == 0 then
		local fade_id = UIButtons.CloneXtGadgetByName( "SCUIBank", "FadeUpLoading" )
		if UIEnums.CurrentPlatform == UIEnums.Platform.PC then
			-- [Blur2PC] Keep the return-to-menu fade full-width, including its first frame.
			UIButtons.ChangeSizeX( fade_id, SCREEN_WIDTH )
		end
	elseif UIGlobals.FadeUpLoading == 1 then
		local bg_id = UIButtons.CloneXtGadgetByName( "Loading\\LoadingUi.lua", "SpBgStuff" )
		UIButtons.PrivateTimeLineActive( bg_id, "fade_bg", true )
		UIButtons.ChangePanel( bg_id, UIEnums.Panel._2D, true )
	else
		print("Invalid fade up mode",UIGlobals.FadeUpLoading)
	end

	-- Clear
	UIGlobals.FadeUpLoading = nil
end

function ClearGlobals()
	if IsTable( _Globals ) ~= true then
		return
	end
	
	print( "[Clear]" )
	for i,v in pairs( _G ) do
		if _Globals[ i ] == nil then
			_G[ i ] = nil
--			print( i, "   ", v )
		end
	end
	print( "[Done]" )
	collectgarbage()
	print( "[GC]" )
end

-- [Blur2PC] retail GuiBank ends by sizing the UI for the current back buffer
if UIEnums.CurrentPlatform == UIEnums.Platform.PC then
	SCREEN_WIDTH = 640
	SCREEN_LEFT = 0
	SCREEN_RIGHT = SCREEN_WIDTH
	CalcScreenBounds()
	ChangeResolution( Amax.GetScreenResolution() )
end
