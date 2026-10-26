GUI =
{
	--
	test = 
	{
		name = "test",
		type = UIEnums.ButtonTypes.GRAPHIC,
		size = 
		{ 
			x = 640, 
			y = 480, 
		},
		pos = 
		{ 
			x = 320, 
			y = 240, 
		},
		textures = 
		{
			{	
				texture = UITexture.Textures.null,
			},
		},
		justify	= UIEnums.Justify.MIDDLE_CENTRE,
	},
	
	object = 
	{
		name = "object",
		type = UIEnums.ButtonTypes.GRAPHIC,
		pos  =
		{
			x = 0, 
			y = 50,
		},
		size = 
		{
			x = 140, 
			y = 140,
		},
		textures =
		{
			{
				name = UITexture.Textures.test_1,
				pos = { u = 0, v = 0 },
				size = {u = 1, v = 1 },
			},
		},
		justify	= UIEnums.Justify.MIDDLE_CENTRE,
	},
	
	text = 
	{
		name = "text",
		type = UIEnums.ButtonTypes.TEXT,
		pos  =
		{
			x = 0, 
			y = -50,
		},
		size = 
		{
			x = 140, 
			y = 40,
		},
		text_string = "STR_0",
		justify	= UIEnums.Justify.MIDDLE_CENTRE,
	},
	
	shape = 
	{
		name = "shape",
		type = UIEnums.ButtonTypes.SHAPE,
		pos  =
		{
			x = 0, 
			y = 50,
		},
		size = 
		{
			x = 120, 
			y = 120,
		},
		scene_name = "Shapes",
		justify	= UIEnums.Justify.MIDDLE_CENTRE,
	},
	
	grafitti = 
	{
		name = "graffiti",
		type = UIEnums.ButtonTypes.DYNAMIC_SHAPE,
		pos  =
		{
			x = 0, 
			y = 50,
		},
		size = 
		{
			x = 120, 
			y = 120,
		},
		scene_name = "Graffiti",
	},
	
	FrameId = -1,
	CanvasId = -1,
	TextId = -1,
	
	ItemsAdded = false,	
	
	text = 
	{
		layer = UIEnums.Layer.Layer1,
		name = "testText",
		type = UIEnums.ButtonTypes.TEXT,
		pos  =
		{
			x = 0, 
			y = 0,
		},
		size = 
		{
			x = 0, 
			y = 32,
		},	
		text_id = UIText.IDS_MAGIC,
		styles =
		{
			name = "D",
		},
	}
}

function Init()
	AddSCUI_Elements( )
end

function StartLoop( time )
	return true
end

function MessageUpdate( message, arg1, arg2 )
	if SCUI_MessageUpdate( message, arg1, arg2 ) == true then
		return
	end
end

function FrameUpdate( time )
	--
	
	--[[
	if GUI.ItemsAdded == false then
		for var = 0, 10, 1 do
		
			local id = UIButtons.AddButton( GUI.text )
		
			UIWindows.AddItem( GUI.FrameId, id )
		end
		GUI.ItemsAdded = true
	end
	]]--
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
