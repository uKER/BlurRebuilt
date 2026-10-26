GUI =
{
	--
}

function Init()
	--
	local quad =
	{
		layer = UIEnums.Layer.BACKGROUND,
		type = UIEnums.ButtonTypes.GRAPHIC,
		x = 320, y = 240, z = -2,
		width = SCREEN_WIDTH, height = 480,
		justify = UIEnums.Justify.MIDDLE_CENTRE,
		colour = "255 64 64 64",
		textures =
		{
			{
				name = 0,--UITexture.Textures.test_card,
				u = 0, v = 0,
				usize = 1, vsize = 1,
			},
		},
	}
	
	UIButtons.AddButton( quad )
		
	
	local outline =
	{
		layer = UIEnums.Layer.LAYER_2,
		type = UIEnums.ButtonTypes.SHAPE,
		justify = UIEnums.Justify.MIDDLE_CENTRE,
		
		x = 320, y = 240,
		width = 640, height = 480,
		
		shader_index = UIEnums.ShaderIndex.PolygonFlat,
		colour = "255 255 255 255",	
					
		-- Test data to load a shape
		shape = UIShape.CityOutline,
		
		textures = 
		{
			{
				name = UITexture.Textures.null,
				u = 0, v = 0,
				usize = 1, vsize = 1,
			},
		},
		
		render_effect = 
		{
			type = UIEnums.RenderEffect.Sketch,
			speed = 0.02,	-- Seconds
			angle = 0,		-- Degrees
		},
	}
	UIButtons.AddButton( outline );
		
end

function StartLoop( time )
	return true
end

function MessageUpdate( message, arg1, arg2 )
	--
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
--	UISystem.DestroyMovie(0)
end
