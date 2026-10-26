GUI =
{
	--
}

function Init()
	--
	local text =
	{
		layer = UIEnums.Layer.LAYER_2,
		type = UIEnums.ButtonTypes.TEXT,
		justify = UIEnums.Justify.MIDDLE_CENTRE,
		x = 320, y = 125,
		width = 640, height = 80,
--		panel = UIEnums.Panel._3D0,
		text = "STR_0",
		--do_wrapping = true,
		style = 
		{
			font = UIFont.courier_new_bold,
			dropshadow = 2.0,
			dropshadow_proportional = false,
			dropshadow_colour = "128 255 0 128",
		},
		poly_shader_index = UIEnums.ShaderIndex.PolygonFlat,
		colour = "255 255 255 255",
	}
	text.style.font = UIFont.comic_sans_ms
	text.text = "STR_0"
	UIButtons.StoreString( 0, "Comic Sans MS" )
	UIButtons.AddButton( text )
	
	text.y = text.y + text.height * 1.1
	text.style.font = UIFont.times_new_roman
	text.text = "STR_1"
	UIButtons.StoreString( 1, "Times New Roman" )
	UIButtons.AddButton( text )
	
	text.y = text.y + text.height * 1.1
	text.style.font = UIFont.impact
	text.text = "STR_2"
	UIButtons.StoreString( 2, "Impact" )
	UIButtons.AddButton( text )
	text.y = text.y + text.height * 1.1
	text.style.font = UIFont.melanie
	text.text = "STR_3"
	UIButtons.StoreString( 3, "Melanie" )
	UIButtons.AddButton( text )
end

function StartLoop( time )
	return true
end

function MessageUpdate( message, arg1, arg2 )
	if message == "BUTTON_BACK" then
		UIScreen.SetNextScreen( "Main.lua" )
	end
end

function FrameUpdate( time )
	--
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
