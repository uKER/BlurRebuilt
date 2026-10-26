GUI =
{
	--
}

function Init()
	--
	local quad =
	{
		layer = UIEnums.Layer.LAYER_1,
		type = UIEnums.ButtonTypes.GRAPHIC,
		x = 320, y = 240, z = 0,
		width = 200, height = 200,
		justify = UIEnums.Justify.MIDDLE_CENTRE,
		colour = "255 64 64 64",
		textures =
		{
			{
				name = UITexture.Textures.sunny,
				u = 0, v = 0,
				usize = 1, vsize = 1,
			},
		},
	}
	
	UIButtons.AddButton( quad )
	
	-- Test Text	
	local text =
	{
		layer = UIEnums.Layer.LAYER_2,
		type = UIEnums.ButtonTypes.TEXT,
		justify = UIEnums.Justify.MIDDLE_CENTRE,
		x = 320, y = 240,
		width = 640, height = 120,
		text = "STR_0",
		do_wrapping = true,
		style = 
		{
			font = UIFont.arial,
			italic = true,
			dropshadow = 8.0,
			dropshadow_colour = "200 0 0 0",
		},
		
		colour= "255 255 255 255",
		
	}
	UIButtons.StoreString( 0, "monkey arse ~~{B}~~" )
	--UIButtons.StoreString( 0, "12345Ã67890 bcdefghijhklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ ()&_^%\/@#[]$£¥€;:" )
	UIButtons.AddButton( text )
end

function StartLoop( time )
	return true
end

function MessageUpdate( message, arg1, arg2 )
	--
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
