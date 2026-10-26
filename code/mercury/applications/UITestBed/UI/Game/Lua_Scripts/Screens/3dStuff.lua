GUI =
{
	--
}

function Init()
	local quad =
	{
		layer = UIEnums.Layer.BACKGROUND,
		type = UIEnums.ButtonTypes.GRAPHIC,
		x = 320, y = 240, z = 0,
		width = SCREEN_WIDTH, height = 480,
		justify = UIEnums.Justify.MIDDLE_CENTRE,
		colour = "255 128 128 128",
		textures =
		{
			{
				name = UITexture.Textures.test_card,
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
	}
	
	--UIButtons.AddButton( quad )
	
	-- 3d test
	quad.panel = UIEnums.Panel._3D1
	quad.x = 0
	quad.y = 0
	quad.z = 0
	quad.width = 100
	quad.height = 50
--	quad.roty = -10
	quad.colour = "255 128 128 128"
	
	UIButtons.AddButton( quad )
	
	quad.x = 175
	quad.z = 100
--	quad.rotx = -10
	quad.rotz = -90
	quad.textures[ 1 ].name = -2
	UIButtons.AddButton( quad )
	
	local text =
	{
		layer = UIEnums.Layer.LAYER_2,
		type = UIEnums.ButtonTypes.TEXT,
		justify = UIEnums.Justify.MIDDLE_LEFT,
		
		pivotx = 80, pivoty = -5, pivotz = 0,
		x = 80, y = -5,
		roty = 90, rotx = 180,
		width = 640, height = 30,
		panel = UIEnums.Panel._3D1,
		text = "STR_0",
		--do_wrapping = true,
		style = 
		{
			font = UIFont.impact,
			dropshadow = 2.0,
			dropshadow_proportional = false,
			dropshadow_colour = "128 255 0 128",
		},
		--[[
		effects =
		{
			{
				type = UIEnums.Effect.ROTATE,
				fromx = 0, fromy = 0, fromz = 0,
				tox = 0, toy = 90, toz = 0,
				start = 0, duration = 5,
				when = UIEnums.EffectTime.MAINLOOP,
				timer = 0,
			},
		},
		timers =
		{
			{
				type = UIEnums.TimerType.WRAP,
				timer = 5,
			},
		},
		]]--
					
		poly_shader_index = UIEnums.ShaderIndex.PolygonFlat,
		colour = "255 128 128 255",
	}
	UIButtons.StoreString( 0, "Some Text String" )
	UIButtons.AddButton( text )
	
	-- camera test
	local timer = 3.5
	local camera = 
	{
		type = UIEnums.ButtonTypes.CAMERA,
		x = 0,
		y = 0,
		z = -75,
		rotx = 0,
		roty = 0,
		rotz = 0,
		effects =
		{
			{
				type = UIEnums.Effect.MOVEZ,
				from = -75, to = 25,
				start = 0, duration = timer,
				when = UIEnums.EffectTime.MAINLOOP,
				timer = 0,
			},
			{
				type = UIEnums.Effect.MOVEX,
				from = 0, to = 175,
				start = 0, duration = timer,
				when = UIEnums.EffectTime.MAINLOOP,
				timer = 1,
			},
			{
				type = UIEnums.Effect.ROTATE,
				fromx = 0, fromy = 0, fromz = 0,
				tox = 0, toy = 0, toz = 90,
				start = 0, duration = timer,
				when = UIEnums.EffectTime.MAINLOOP,
				timer = 2,
			},
		},
		timers =
		{
			{
				type = UIEnums.TimerType.WRAP,
				timer = timer + 5,
				curve = UIEnums.Curve.CameraZTest,
			},
			{
				type = UIEnums.TimerType.WRAP,
				timer = timer + 5,
				curve = UIEnums.Curve.CameraXTest,
			},
			{
				type = UIEnums.TimerType.WRAP,
				timer = timer + 5,
				curve = UIEnums.Curve.CameraRotZTest,
			},
		},
	}
	
	UIButtons.AddButton( camera )
	
	-- random shapes in background
	local shape =
	{
		panel = UIEnums.Panel._3D1,
		type = UIEnums.ButtonTypes.SHAPE,
		justify = UIEnums.Justify.MIDDLE_CENTRE,
		shader_index = UIEnums.ShaderIndex.PolygonFlat,
		colour = "255 255 255 255", 
		width = 5,
		height = 5,
		
		textures =
		{
			{
				name = UITexture.Textures.null,
				u = 0, v = 0, 
				usize = 0, vsize = 0,
			},
		},
	}
	
	--UIButtons.AddButton( shape )
	
	for i = 0, 150, 1 do
		local s = math.random( 3 )
		
		shape.shape = UIShape.Cross
		
		shape.colour = "255 255 255 " .. math.random( 255 )
		
		shape.x = math.random( 800 ) - 100
		shape.y = math.random( 800 ) - 400
		shape.z = math.random( 200 ) + 20
		
		UIButtons.AddButton( shape )
	end
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
