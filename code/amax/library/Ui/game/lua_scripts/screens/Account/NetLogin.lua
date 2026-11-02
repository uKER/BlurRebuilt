-- [Blur2PC] Recovered retail Screens/Account/NetLogin.lua, T261.
-- AMAX accounts are entered here; the obsolete DW registration flow is omitted.
GUI = { finished = false, saving = false, save_complete = false,
        option = { username = 1, password = 2, login = 3, skip = 4 } }

function Init()
    AddSCUI_Elements()
    StoreInfoLine()
    UIGlobals.NetAccountActive = true
    UIScreen.BlockInputToContext(true)
    UIScreen.Suspend(UIEnums.Context.Main)
end

function PostInit()
    GUI.menu_id = SCUI.name_to_id.menu
    for _, name in ipairs({"username", "password", "login", "skip"}) do
        UIButtons.AddListItem(GUI.menu_id, SCUI.name_to_id[name .. "_node"], GUI.option[name])
    end
    UIButtons.SetActive(SCUI.name_to_id.create_node, false)
    GUI.username_id = UIButtons.FindChildByName(SCUI.name_to_id.username_node, "textbox")
    GUI.password_id = UIButtons.FindChildByName(SCUI.name_to_id.password_node, "textbox")
    local saved = NetServices.ReadOnlineAccountDetails(GUI.username_id, GUI.password_id)
    UIButtons.SetSelection(GUI.menu_id, saved and GUI.option.login or GUI.option.username)
end

function StartLoop(time) end
function EndLoop(time) end

function MessageUpdate(message, arg1, arg2, arg3, arg4)
    if GUI.finished or GUI.saving then return end
    if SCUI_MessageUpdate(message, arg1, arg2, arg3, arg4) == true then return end
    if message == UIEnums.Message.MenuNext and arg2 == true then
        local selected = UIButtons.GetSelection(GUI.menu_id)
        if selected == GUI.option.login then NetLogin_Login()
        elseif selected == GUI.option.skip then NetLogin_Skip() end
    elseif message == UIEnums.Message.Tab and (arg2 == true or arg2 == 1) then
        local selected = UIButtons.GetSelection(GUI.menu_id)
        UIButtons.SetSelection(GUI.menu_id, selected % GUI.option.skip + 1)
    elseif message == UIEnums.Message.MenuBack and arg2 == true then
        NetLogin_Skip()
    end
end

function FrameUpdate(time)
    if GUI.save_complete then
        GUI.save_complete = false
        GUI.saving = false
        if GUI.save_success then
            net_StartServiceConnection(true, UIGlobals.server_connection.callback)
        end
    end
end

function End()
    RestoreInfoLine()
    UIGlobals.NetAccountActive = false
    UIScreen.Resume(UIEnums.Context.Main)
end

function NetLogin_Login()
    if not NetServices.StoreOnlineAccountDetails(GUI.username_id, GUI.password_id) then
        UIButtons.SetSelection(GUI.menu_id, GUI.option.username)
        return
    end
    if StartAsyncSave() then
        GUI.saving = true
        local login = GUI
        UIGlobals.FileParams.LocalOnly = true
        UIGlobals.FileParams.FinishedCallback = function(success)
            login.save_success = success
            login.save_complete = true
        end
    end
end

function NetLogin_Skip()
    GUI.finished = true
    UIGlobals.server_connection.active = false
    if IsFunction(UIGlobals.server_connection.callback) then
        UIGlobals.server_connection.callback(false)
    end
    EndScreen(UIEnums.Context.Subscreen2)
end
