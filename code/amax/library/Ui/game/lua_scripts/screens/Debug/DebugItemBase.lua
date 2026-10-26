------------------------------
-- Base Class for a debug item
------------------------------
DebugItem = 
{
	ID = -1,
	Gadget =
	{
		layer = UIEnums.Layer.LAYER_1,
		type = UIEnums.ButtonTypes.SPINNER_BUTTON,
		pos = {x = 70, y = 24 },
		size = { x = 320, y = 20 },
		colour_style = "!255 255 255 255",

		style_selected = { name = "MenuSelected", },
		style_deselected = { name = "MenuDeselected", },
		style_selected_unselectable = { name = "MenuSelectedUnselectable", },
		style_deselected_unselectable = { name = "MenuDeselectedUnselectable", },
		loop_list = true,		
		text = UIText.IDS_NOWT,
		justify = UIEnums.Justify.JUSTIFY_LEFT,
	},
	ObjectName = "",
}

function DebugItem:New(o)
	o = o or {}
	setmetatable(o,self)
	self.__index = self
	return o
end

function DebugItem:AddButton(heading,x_offset,y_offset,object_name)
	
	self.Gadget.pos.x=x_offset
	self.Gadget.pos.y=y_offset
	self.Gadget.text=heading
	self.ID = UIButtons.AddButton(self.Gadget)
	self.ObjectName=object_name
	self:Activate(false)
	return y_offset+20
end

function DebugItem:MoveButton(x,y)
	UIButtons.ChangePosition(self.ID,x,y)
	return y+20
end

function DebugItem:AddData()
	--
end

function DebugItem:Update()
	--
end

function DebugItem:SetSelection(table)
	if IsNumber( table[self.ObjectName] ) == true then
		UIButtons.SetSelection(self.ID, table[self.ObjectName])
	end
end

function DebugItem:Activate(should_activate)
	UIButtons.SetSelected(self.ID,should_activate)
end

function DebugItem:WriteToStructure(table)
	table[self.ObjectName]=UIButtons.GetSelection(self.ID)
end

function DebugItem:GetID()
	return self.ID
end


-------------------
-- Base Debug Items
-------------------
YesNoGadgetBase = DebugItem:New()
function YesNoGadgetBase:AddData()

	UIButtons.AddItem(self.ID,0,UIText.CMN_NO,false)
	UIButtons.AddItem(self.ID,1,UIText.CMN_YES,false)
end

OnOffGadgetBase = DebugItem:New()
function OnOffGadgetBase:AddData()

	UIButtons.AddItem(self.ID,0,UIText.CMN_OFF,false)
	UIButtons.AddItem(self.ID,1,UIText.CMN_ON,false)
end
