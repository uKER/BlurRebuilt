PuGUI =
{
	--
	test = 
	{
		name = "test",
		type = UIEnums.ButtonTypes.GRAPHIC,
		size = 
		{ 
			x = 64, 
			y = 48, 
		},
		pos = 
		{ 
			x = 320, 
			y = 240, 
		},
		colour = "255 128 255 0",
		colour1 =  "255 128 255 255",
		colour2 =  "255 128 0 0",
		textures = 
		{
			{	
				texture = UITexture.Textures.null,
			},
		},
		justify	= UIEnums.Justify.MIDDLE_CENTRE,
	},
}

function PopupInit()
	UIScreen.SetPopupTimers( 0.3, 0.3 )

	UIButtons.AddButton( PuGUI.test )
end

function PopupStartLoop( time )
	return true
end

function PopupMessageUpdate( message, arg1, arg2 )
end

function PopupFrameUpdate( time )
	--
end

function PopupRender()
	--
end

function PopupEndLoop( time )
	--
end

function PopupEnd()
	--
end

