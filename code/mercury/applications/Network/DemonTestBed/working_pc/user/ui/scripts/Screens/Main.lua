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
		width = 400, height = 400,
		justify = UIEnums.Justify.MIDDLE_CENTRE,
		colour = "255 128 128 128",
		textures =
		{
			{
				name = UITexture.Textures.null,
				u = 0, v = 0,
				usize = 1, vsize = 1,
			},
		},
		--[[
		effects =
		{
			{
				type = UIEnums.Effect.ROTATE,
				fromx = 0, fromy = 0, fromz = 0,
				tox = 0, toy = 360, toz = 0,
				start = 0, duration = 1.0,
				when = UIEnums.EffectTime.MAINLOOP,
				timer = 0,
			},
		},
		timers =
		{
			{
				type = UIEnums.TimerType.WRAP,
				timer = 1.0,
			},
		},
		]]--
		
		--clipx = 320,
		--clipy = 240,
		--clipsizex = 420,
		--clipsizey = 340,
	}
	
	UIButtons.AddButton( quad )
	
	--[[
	-- render target test
	quad.panel = UIEnums.Panel._RT0
	quad.x = quad.x - quad.width
	quad.y = quad.y - quad.height
	UIButtons.AddButton( quad )
	
	quad.panel = -1
	quad.textures[ 1 ].name = UIEnums.TextureType.RENDER_TARGET0
	quad.x = quad.x + quad.width * 2
	quad.y = quad.y + quad.height * 2
	UIButtons.AddButton( quad )
	]]--
	
	-- 3d test
	quad.panel = UIEnums.Panel._3D0
	quad.x = 0
	quad.y = 0
	quad.z = 0
	quad.width = 20
	quad.height = 10
	quad.roty = 180
	
	--UIButtons.AddButton( quad )
	
	--[[
	for i = 5, 0, -1 do
		quad.layer = i
		quad.colour = "255 " .. math.random( 255 ) .. " " .. math.random( 255 ) .. " " .. math.random( 255 )
		quad.x = 50 + 50 * i
		quad.y = 50 + 50 * i
		UIButtons.AddButton( quad )
	end
	]]--
	
	local tex =
	{
		layer = UIEnums.Layer.BACKGROUND,
		type = UIEnums.ButtonTypes.GRAPHIC,
		x = 0, y = 0, z = -10,
		width = SCREEN_WIDTH, height = 480,
		panel = UIEnums.Panel._3D0,
		justify = UIEnums.Justify.MIDDLE_CENTRE,
		colour = "255 0 64 0",
		textures =
		{
			{
				--name = UIEnums.TextureType.GRADIENT,
				name = UITexture.Textures.null,
				u = 0, v = 0,
				usize = 1, vsize = 1,
			},
		},
		--[[
		effects =
		{
			{
				type = UIEnums.Effect.ROTATE,
				fromx = 0, fromy = 0, fromz = 0,
				tox = 0, toy = 0, toz = 360,
				start = 0, duration = 10,
				when = UIEnums.EffectTime.MAINLOOP,
				timer = 0,
			},
		},
		timers =
		{
			{
				type = UIEnums.TimerType.WRAP,
				timer = 10,
			},
		},
		]]--	
	}
	--UIButtons.AddButton( tex )
	
--	UISystem.InitMovie(0,"Movies\\Club_Splash.bik",false)
	
	local text =
	{
		layer = UIEnums.Layer.LAYER_2,
		type = UIEnums.ButtonTypes.TEXT,
		justify = UIEnums.Justify.MIDDLE_CENTRE,
		x = 320, y = 240,
		width = 640, height = 60,
		text = "STR_0",
		do_wrapping = true,
		style = 
		{
			font = UIFont.e_impact_outline,
			italic = true,
			dropshadow_proportional = true,
			dropshadow = 0.05,
			--dropshadow_colour = "128 255 0 128",
		},
		poly_shader_index = UIEnums.ShaderIndex.PolygonFlat,
		poly_gradient_angle = 90,
		poly_gradient_index = 1,
		
--		colour0 = "255 255 255 0",
--		colour2 = "255 255 255 255",
--		colour3 = "255 255 255 255",
--		colour = "255 0 0 255",
		colour= "255 0 255 255",
		
		textures = 
		{
			{
				name = UITexture.Textures.null,
				name = UIEnums.TextureType.GRADIENT,
--				name = UIEnums.TextureType.MOVIE00,
--				name = UITexture.Textures.mesh4,
				u = 0, v = 0,
				usize = 1, vsize = 1,
			},
	
		},
		--[[
		effects =
		{
			{
				type = UIEnums.Effect.SCALE,
				fromx = 0.1, fromy = 0.1, fromz = 1,
				tox = 4, toy = 4, toz = 1,
				start = 0, duration = 10,
				when = UIEnums.EffectTime.MAINLOOP,
				timer = 0,
			},
			{
				type = UIEnums.Effect.ROTATE,
				fromx = 0, fromy = 0, fromz = 0,
				tox = 0, toy = 0, toz = 360,
				start = 0, duration = 10,
				when = UIEnums.EffectTime.MAINLOOP,
				timer = 0,
			},
		},
		timers =
		{
			{
				type = UIEnums.TimerType.BOUNCE,
				timer = 10,
			},
		},
		]]--
		
		--clipx1 = 320,
		--clipy1 = 240,
		--clipx2 = 420,
		--clipy2 = 340,
	}
	--UIButtons.StoreString( 0, "UITestBed - Hello World!" )
	UIButtons.StoreString( 0, "12345Ã67890 bcdefghijhklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ ()&_^%\/@#[]$£¥€;:" )
	--UIButtons.AddButton( text )
	--text.style.font = UIFont.e_impact_outline
	--text.y = text.y - 164
	--text.layer = UIEnums.Layer.LAYER_1
	--UIButtons.AddButton( text )
	
	-- 3d test
	text.panel = UIEnums.Panel._3D0
	text.x = 0
	text.y = 0
	text.z = 0
	text.height = 10
	text.roty = 180
	
	--UIButtons.AddButton( text )
	
	local testShapes =
	{
		layer = UIEnums.Layer.LAYER_2,
		type = UIEnums.ButtonTypes.SHAPE,
		justify = UIEnums.Justify.TOP_LEFT,
		x = SCREEN_LEFT, y = 240,
		width = 340/480, height = 340/480,
		shader_index = UIEnums.ShaderIndex.PolygonFlat,
		colour = "255 255 255 255",	
		rotz = 180,
		
		-- Test data to load a shape
		shape = UIShape.Shapes.Gradient,
	}
	--UIButtons.AddButton( testShapes );
	
	local City =
	{
		panel = UIEnums.Panel._3D0,
		layer = UIEnums.Layer.LAYER_2,
		type = UIEnums.ButtonTypes.SHAPE,
		justify = UIEnums.Justify.JUSTIFY_LEFT,
		x = 0, y = 0, z = 150,
		width = 150/480, height = 150/480,
		shader_index = UIEnums.ShaderIndex.PolygonFlat,
		colour = "255 255 255 255",	
		rotY = 180,
		rotz = 180,
		
		-- Test data to load a shape
		shape = UIShape.Shapes.Gradient,
	}
	--UIButtons.AddButton( City );
	
	local shape =
	{
		panel = UIEnums.Panel._3D0,
		layer = UIEnums.Layer.LAYER_2,
		type = UIEnums.ButtonTypes.SHAPE,
		justify = UIEnums.Justify.JUSTIFY_LEFT,
		x = 0, y = 0,
		width = 50, height = 50,
		shader_index = UIEnums.ShaderIndex.PolygonFlat,
		colour = "255 255 255 255",	
		rotY = 180,
		rotz = 180,
		
		-- Test data to load a shape
		shape = UIShape.Shapes.Archer,
	}
--	UIButtons.AddButton( shape );
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
--	UISystem.DestroyMovie(0)
end
