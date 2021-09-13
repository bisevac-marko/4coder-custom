void 
set_current_mapid(Application_Links* app, Command_Map_ID mapid) 
{
    View_ID view = get_active_view(app, 0);
    Buffer_ID buffer = view_get_buffer(app, view, 0);
    Managed_Scope scope = buffer_get_managed_scope(app, buffer);
    Command_Map_ID* map_id_ptr = scope_attachment(app, scope, buffer_map_id, Command_Map_ID);
    *map_id_ptr = mapid;
}


CUSTOM_COMMAND_SIG(EnterEditMode)
{
    set_current_mapid(app, edit_map_id);
    active_color_table.arrays[defcolor_cursor].vals[0] = 0xffA75032;
}


CUSTOM_COMMAND_SIG(EnterNormalMode)
{
    set_current_mapid(app, normal_map_id);
    active_color_table.arrays[defcolor_cursor].vals[0] = 0xFF8EC363;
}