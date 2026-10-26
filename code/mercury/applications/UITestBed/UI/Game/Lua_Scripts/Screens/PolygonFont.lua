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
		x = 320, y = 85,
		width = 500, height = 80,
--		panel = UIEnums.Panel._3D0,
		text = "STR_0",
		do_wrapping = true,
		style = 
		{
			font = UIFont.courier_new_bold,
			italic = true,
			dropshadow = 2.0,
			dropshadow_proportional = false,
			dropshadow_colour = "128 255 0 128",
		},
		poly_shader_index = UIEnums.ShaderIndex.PolygonFlat,
		colour = "255 255 255 255",
	}
	UIButtons.StoreString( 0, "ABCDEFGHIJKLMN~~/n~~()&_^%\/@#[]$£¥€;:" )
	UIButtons.AddButton( text )
	
	text.y = text.y + text.height * 3.75
	text.style.font = UIFont.e_couriernewbold
	text.shader_index = UIEnums.ShaderIndex.IntensityAsAlpha
	text.poly_shader_index = nil
	UIButtons.AddButton( text )
	
	text.height = 20
	text.y = 190
	text.style.font = UIFont.courier_new_bold
	text.shader_index = nil
	text.poly_shader_index = UIEnums.ShaderIndex.PolygonFlat
	UIButtons.AddButton( text )
	
	text.y = text.y + text.height * 5
	text.style.font = UIFont.e_couriernewbold
	text.shader_index = UIEnums.ShaderIndex.IntensityAsAlpha
	text.poly_shader_index = nil
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
