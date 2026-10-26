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
		
	--------------------------------------------------------------
	---------------CHANGE TO WHAT YOU WANT TO LOAD----------------
	--------------------------------------------------------------
	
	
	local graffiti =
	{
		layer = UIEnums.Layer.LAYER_2,
		type = UIEnums.ButtonTypes.DYNAMIC_SHAPE,
		justify = UIEnums.Justify.MIDDLE_CENTRE,
		
		x = 320, y = 260,
		width = 480, height = 340,
		
		shader_index = UIEnums.ShaderIndex.PolygonFlat,
		colour = "255 255 255 255",	
					
		-- Test data to load a shape
		shape = UIShape.Graffiti,
		
		textures = 
		{
			{
				name = UITexture.Textures.swipe,
				u = 0, v = 0,
				usize = 1, vsize = 1,
			},
		},
		
		render_effect = 
		{
			type = UIEnums.RenderEffect.Swipe,
			speed = 1,		-- Seconds
			angle = 330,	-- Degrees
		},
	}
	UIButtons.AddButton( graffiti );
	
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
