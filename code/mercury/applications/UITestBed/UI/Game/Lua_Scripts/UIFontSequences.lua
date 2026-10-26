FontSequences =
{
	-- XBox - A button
	{
		name		= "{A}",
		colour		= "255 255 255 255",
		texture		= UITexture.Textures.xbox360_buttons,
		u			= 0.5,
		v			= 0.875,
		usize		= 0.125,
		vsize		= 0.125,
		dropshadow	= true,
	},
	-- XBox - B button
	{
		name		= "{B}",
		colour		= "255 255 255 255",
		texture		= UITexture.Textures.xbox360_buttons,
		u			= 0.625,
		v			= 0.875,
		usize		= 0.125,
		vsize		= 0.125,
		dropshadow	= true,
	},
	
	-----------------
	-- Shapes
	-----------------
	-- XBox - A button
	{
		name		= "{A}",
		colour		= "255 255 255 255",
		scene_name	= "XboxButtons",
		group_name  = "A",
		dropshadow	= true,
	},
	-- XBox - B button
	{
		name		= "{B}",
		colour		= "255 255 255 255",
		scene_name	= "XboxButtons",
		group_name  = "B",
		dropshadow	= true,
	},
	-- XBox - X button
	{
		name		= "{X}",
		colour		= "255 255 255 255",
		scene_name	= "XboxButtons",
		group_name  = "X",
		dropshadow	= true,
	},
	-- XBox - Y button
	{
		name		= "{Y}",
		colour		= "255 255 255 255",
		scene_name	= "XboxButtons",
		group_name  = "Y",
		dropshadow	= true,
	},
}

-- default font style
FontStyle =
{
	font = UIFont.verdana,
	--font = UIFont.e_couriernewbold,
	
	render_passes =
	{
		{	-- normal
			type = UIEnums.FontStyle.Default,
			effect_index = UIEnums.EffectIndex.PolygonFont,
			--effect_index = UIEnums.EffectIndex.IntensityAsAlpha,
		},
	},
}
