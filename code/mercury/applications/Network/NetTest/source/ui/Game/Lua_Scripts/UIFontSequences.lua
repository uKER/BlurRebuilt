UIEnums.Colour = {  }
UIEnums.Colour.White = "255 255 255 255"

FontSequences =
{
	-- XBox - Jewel button
	{
		name		= "{J}",
		colour		= UIEnums.Colour.White,
		texture		= UITexture.Textures.xbox360_buttons,
		u			= 0.625,
		v			= 0.75,
		usize		= 0.125,
		vsize		= 0.125,
	},		
	-- XBox - A button
	{
		name		= "{A}",
		colour		= UIEnums.Colour.White,
		texture		= UITexture.Textures.xbox360_buttons,
		u			= 0.5,
		v			= 0.875,
		usize		= 0.125,
		vsize		= 0.125,
	},
	-- XBox - B button
	{
		name		= "{B}",
		colour		= UIEnums.Colour.White,
		texture		= UITexture.Textures.xbox360_buttons,
		u			= 0.625,
		v			= 0.875,
		usize		= 0.125,
		vsize		= 0.125,
	},
	-- XBox - left trigger
	{
		name		= "{LT}",
		colour		= UIEnums.Colour.White,
		texture		= UITexture.Textures.xbox360_buttons,
		u			= 0.0,
		v			= 0.125,
		usize		= 0.125,
		vsize		= 0.125,
	},
	-- XBox - right trigger
	{
		name		= "{RT}",
		colour		= UIEnums.Colour.White,
		texture		= UITexture.Textures.xbox360_buttons,
		u			= 0.125,
		v			= 0.125,
		usize		= 0.125,
		vsize		= 0.125,
	},
	-- XBox - left button
	{
		name		= "{LB}",
		colour		= UIEnums.Colour.White,
		texture		= UITexture.Textures.xbox360_buttons,
		u			= 0.25,
		v			= 0.125,
		usize		= 0.125,
		vsize		= 0.125,
	},
	-- XBox - right button
	{
		name		= "{RB}",
		colour		= UIEnums.Colour.White,
		texture		= UITexture.Textures.xbox360_buttons,
		u			= 0.375,
		v			= 0.125,
		usize		= 0.125,
		vsize		= 0.125,
	},
	-- XBox - X Button
	{
		name		= "{X}",
		colour		= UIEnums.Colour.White,
		texture		= UITexture.Textures.xbox360_buttons,
		u			= 0.75,
		v			= 0.875,
		usize		= 0.125,
		vsize		= 0.125,
	},
	-- XBox - Y Button
	{
		name		= "{Y}",
		colour		= UIEnums.Colour.White,
		texture		= UITexture.Textures.xbox360_buttons,
		u			= 0.875,
		v			= 0.875,
		usize		= 0.125,
		vsize		= 0.125,
	},
	-- XBox - DPad
	{
		name		= "{DP}",
		colour		= UIEnums.Colour.White,
		texture		= UITexture.Textures.xbox360_buttons,
		u			= 0.0,
		v			= 0.25,
		usize		= 0.125,
		vsize		= 0.125,
	},
	-- XBox - Start button
	{
		name		= "{S}",
		colour		= UIEnums.Colour.White,
		texture		= UITexture.Textures.xbox360_buttons,
		u			= 0.125,
		v			= 0.0,
		usize		= 0.125,
		vsize		= 0.125,
	},
	-- XBox - Back button
	{
		name		= "{BK}",
		colour		= UIEnums.Colour.White,
		texture		= UITexture.Textures.xbox360_buttons,
		u			= 0.0,
		v			= 0.0,
		usize		= 0.125,
		vsize		= 0.125,
	},
	-- XBox - Right stick
	{
		name		= "{RS}",
		colour		= UIEnums.Colour.White,
		texture		= UITexture.Textures.xbox360_buttons,
		u			= 3/8,
		v			= 6/8,
		usize		= 0.125,
		vsize		= 0.125,
	},
	-- XBox - Left stick
	{
		name		= "{LS}",
		colour		= UIEnums.Colour.White,
		texture		= UITexture.Textures.xbox360_buttons,
		u			= 2/8,
		v			= 6/8,
		usize		= 0.125,
		vsize		= 0.125,
	},
	-- XBox - Speaker
	{
		name		= "{spk}",
		colour		= UIEnums.Colour.White,
		texture		= UITexture.Textures.xbox360_buttons,
		u			= 0.25,
		v			= 0.875,
		usize		= 0.125,
		vsize		= 0.125,
	},
	-- XBox - Speaker muted
	{
		name		= "{spkm}",
		colour		= UIEnums.Colour.Red,
		texture		= UITexture.Textures.xbox360_buttons,
		u			= 0.25,
		v			= 0.875,
		usize		= 0.125,
		vsize		= 0.125,
	},
	-- infinity
	{
		name		= "{infinity}",
		colour		= UIEnums.Colour.White,
		texture		= UITexture.Textures.xbox360_buttons,
		u			= 0.375,
		v			= 0.875,
		usize		= 0.125,
		vsize		= 0.125,
	},
}

-- default font style
_FontStyle =
{
	--font = UIFont.e_couriernewbold,
	font = UIFont.e_impact_outline,
	render_passes =
	{
		{
			 -- normal
			type = UIEnums.FontStyle.Default,
			effect_index = UIEnums.EffectIndex.IntensityAsAlpha,
		},
	},
}

 

