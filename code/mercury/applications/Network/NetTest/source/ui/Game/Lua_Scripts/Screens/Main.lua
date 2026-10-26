GUI =
{
}   

function Init()

	local networkPanel =  
	{
		panel = 1,
		ID = -1,
		layer = UIEnums.Layer.LAYER_1,
		type = UIEnums.ButtonTypes.NETWORKPANEL,
		width = 640-96, height = 480-64,
		colour = "255 255 255 255",
	}
   
   UIButtons.AddButton( networkPanel )
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
