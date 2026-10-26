GUI =
{
	--
}

function Init()


	local archer =
	{
		layer = UIEnums.Layer.LAYER_2,
		type = UIEnums.ButtonTypes.GRAPHIC,
		justify = UIEnums.Justify.MIDDLE_CENTRE,
		
		x = 100, y = 240,
		width = 256, height = 512,
		colour = "255 255 255 255",	
					
		-- Test data to load a shape
		textures = 
		{
			{
				name = UITexture.Textures.archer,
				u = 0, v = 0,
				usize = 1, vsize = 1,
			},
		},
	}
	UIButtons.AddButton( archer );
	
	
	local archerAdobe =
	{
		panel = UIEnums.Panel._3D0,
		layer = UIEnums.Layer.LAYER_2,
		type = UIEnums.ButtonTypes.SHAPE,
		justify = UIEnums.Justify.MIDDLE_CENTRE,
		
		--x = 500, y = 220,
		--width = 440, height = 440,
		
		x = 300, y = -380, z = 240,
		width = 380, height = 300,
		roty = 45,
		
		shader_index = UIEnums.ShaderIndex.PolygonFlat,
		colour = "255 255 255 255",	
					
		-- Test data to load a shape
		shape = UIShape.Archer,
		
		textures = 
		{
			{
				name = UITexture.Textures.null,
				u = 0, v = 0,
				usize = 1, vsize = 1,
			},
		},
	}
	UIButtons.AddButton( archerAdobe );	
	
	UIButtons.SetZEnable( false )
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
	UIButtons.SetZEnable( true )
	
--	UISystem.DestroyMovie(0)
end
