struct EditorState
{
    Arena arena;
    TrieNode* root[EditorMode_Count];
    EditorMode current_mode;
    Table_Data_u64 command_map[EditorMode_Count];
    u32 key_sequence_level;
    KeySequence key_sequence;
    
    FunctionPeek function_peek;
};

static EditorState* global_editor;

function void 
MB_SetCurrentMapID(Application_Links* app, Command_Map_ID mapid) 
{
    View_ID view = get_active_view(app, 0);
    Buffer_ID buffer = view_get_buffer(app, view, 0);
    Managed_Scope scope = buffer_get_managed_scope(app, buffer);
    Command_Map_ID* map_id_ptr = scope_attachment(app, scope, buffer_map_id, Command_Map_ID);
    *map_id_ptr = mapid;
}


CUSTOM_COMMAND_SIG(MB_EnterEditMode)
{
    MB_SetCurrentMapID(app, edit_map_id);
    global_editor->current_mode = EditorMode_Insert;
    active_color_table.arrays[defcolor_cursor].vals[0] = 0xffA75032;
}


CUSTOM_COMMAND_SIG(MB_EnterNormalMode)
{
    MB_SetCurrentMapID(app, normal_map_id);
    global_editor->current_mode = EditorMode_Normal;
    active_color_table.arrays[defcolor_cursor].vals[0] = 0xFF8EC363;
}