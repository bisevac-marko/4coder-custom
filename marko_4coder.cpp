/*
4coder_marko.cpp - Supplies the default bindings used for default 4coder behavior.
*/

// TOP
#if !defined(FCODER_DEFAULT_BINDINGS_CPP)
#define FCODER_DEFAULT_BINDINGS_CPP

#include "4coder_default_include.cpp"

static String_ID edit_map_id;
static String_ID normal_map_id;
static bool global_build_panel_open = false;
#include "marko_helpers.cpp"
#include "marko_cpp_highlight.cpp"
#include "marko_map.h"
#include "marko_map.cpp"
#include "marko_function_peek.cpp"
#include "marko_modes.cpp"
#include "marko_cursor.cpp"

#if !defined(META_PASS)
#include "generated/managed_id_metadata.cpp"
#endif

CUSTOM_COMMAND_SIG(MB_RunProgram)
{
    prj_exec_command_name(app, SCu8("run"));
    
}

CUSTOM_COMMAND_SIG(MB_NewLineAndEditMode)
{
    MB_EnterEditMode(app);
    seek_end_of_line(app);
    write_text(app, string_u8_litexpr("\n"));
}

CUSTOM_COMMAND_SIG(MB_DeleteRange)
{
    View_ID view = get_active_view(app, Access_ReadWriteVisible);
    Buffer_ID buffer = view_get_buffer(app, view, Access_ReadWriteVisible);
    Range_i64 range = get_view_range(app, view);
    clipboard_post_buffer_range(app, 0, buffer, range);
    buffer_replace_range(app, buffer, range, string_u8_empty);
}

CUSTOM_COMMAND_SIG(MB_DeleteLine)
{
    View_ID view = get_active_view(app, Access_ReadWriteVisible);
    Buffer_ID buffer = view_get_buffer(app, view, Access_ReadWriteVisible);
    i64 pos = view_get_cursor_pos(app, view);
    i64 line = get_line_number_from_pos(app, buffer, pos);
    Range_i64 range = get_line_pos_range(app, buffer, line);
    range.end += 1;
    i32 size = (i32)buffer_get_size(app, buffer);
    range.end = clamp_top(range.end, size);
    if (range_size(range) == 0 ||
        buffer_get_char(app, buffer, range.end - 1) != '\n'){
        range.start -= 1;
        range.first = clamp_bot(0, range.first);
    }
    
    clipboard_post_buffer_range(app, 0, buffer, range);
    
    buffer_replace_range(app, buffer, range, string_u8_litexpr(""));
}

CUSTOM_COMMAND_SIG(MB_KillToEndOfLine)
{
    MB_EnterEditMode(app);
    View_ID view = get_active_view(app, 0);
    
    i64 pos2 = view_get_cursor_pos(app, view);
    seek_end_of_line(app);
    i64 pos1 = view_get_cursor_pos(app, view);
    
    Range_i64 range = Ii64(pos1, pos2);
    if(pos1 == pos2)
    {
        range.max += 1;
    }
    
    Buffer_ID buffer = view_get_buffer(app, view, 0);
    clipboard_post_buffer_range(app, 0, buffer, range);
    buffer_replace_range(app, buffer, range, string_u8_empty);
}

CUSTOM_COMMAND_SIG(MB_BackspaceDeleteAndEditMode)
{
    MB_EnterEditMode(app);
    backspace_alpha_numeric_boundary(app);
}

CUSTOM_COMMAND_SIG(MB_BackspaceDeleteWholeWord)
{
    MB_EnterEditMode(app);
    snipe_backward_whitespace_or_token_boundary(app);
}

CUSTOM_COMMAND_SIG(MB_BuildProject)
{
    build_in_build_panel(app);
    global_build_panel_open = true;
}

CUSTOM_COMMAND_SIG(MB_ToggleBuildPannel)
{
    if (!global_build_panel_open)
    {
        View_ID view = get_or_open_build_panel(app);
        if (view != 0)
        {
            Buffer_ID buffer = get_buffer_by_name(app, string_u8_litexpr("*compilation*"), Access_Always);
            view_set_buffer(app, view, buffer, 0);
            //lock_jump_buffer(app, string_u8_litexpr("*compilation*"));
        }
        
    }
    else
    {
        close_build_panel(app);
    }
    global_build_panel_open = !global_build_panel_open;
}


static b32
MB_IsCodeFile(char* name, u64 len){
    b32 is_code = false;
    if (len >= 5){
        char* ext = &name[len - 4];
        if (ext[0] == '.' && ext[1] == 'c' && ext[2] == 'p' && ext[3] == 'p'){
            is_code = true;
        }
        else if (ext[0] == '.' && ext[1] == 'h' && ext[2] == 'p' && ext[3] == 'p'){
            is_code = true;
        }
    }
    if (len >= 4){
        char* ext = &name[len - 3];
        if (ext[0] == '.' && ext[1] == 'c' && ext[2] == 'c'){
            is_code = true;
        }
    }
    if (len >= 3){
        char* ext = &name[len - 2];
        if (ext[0] == '.' && ext[1] == 'h'){
            is_code = true;
        }
        else if (ext[0] == '.' && ext[1] == 'c'){
            is_code = true;
        }
    }
    return(is_code);
}

CUSTOM_COMMAND_SIG(MB_Marko4CoderStartup)
{
    
    ProfileScope(app, "marko 4coder startup");
    User_Input input = get_current_input(app);
    
    if (match_core_code(&input, CoreCode_Startup)){
        String_Const_u8_Array file_names = input.event.core.file_names;
        load_themes_default_folder(app);
        default_4coder_initialize(app, file_names);
        default_4coder_side_by_side_panels(app, file_names);
        b32 auto_load = def_get_config_b32(vars_save_string_lit("automatically_load_project"));
        if (auto_load){
            load_project(app);
        }
    }
    
    {
        def_audio_init();
    }
    
    {
        def_enable_virtual_whitespace = def_get_config_b32(vars_save_string_lit("enable_virtual_whitespace"));
        clear_all_layouts(app);
    }
    /*
    String_Const_u8 project_dir = def_get_config_string(&main_arena, vars_save_string_lit("project_directory"));

    WIN32_FIND_DATA find_data;

    HANDLE file_handle = FindFirstFileA((char*)project_dir.str, &find_data);

    if (file_handle == INVALID_HANDLE_VALUE)
    {
        OutputDebugStringA((char*)project_dir.str);
        return;
    }


    // Remove the '*'
    project_dir.str[project_dir.size-1] = '\0';

    //set_hot_directory(app, full_file_name);

    Scratch_Block scratch(app);

    while (FindNextFileA(file_handle, &find_data) != 0)
    {
        if (IsCodeFile(find_data.cFileName, cstring_length(find_data.cFileName)))
        {
            String_Const_u8 full_file_name =
                push_u8_stringf(scratch, "%.*s%s", string_expand(project_dir), find_data.cFileName);

            Buffer_Create_Flag flags = BufferCreate_NeverNew;
            create_buffer(app, full_file_name, flags);
            // OutputDebugStringA(find_data.cFileName);
        }
    }
    */
}

CUSTOM_COMMAND_SIG(MB_ReplayKeyboardMacro)
{
    keyboard_macro_replay(app);
}


CUSTOM_COMMAND_SIG(MB_LastSearchForward)
{
    View_ID view = get_active_view(app, Access_ReadVisible);
    Buffer_ID buffer = view_get_buffer(app, view, Access_ReadVisible);
    
    i64 new_pos = 0;
    String_Const_u8 last_search = SCu8(previous_isearch_query, cstring_length(previous_isearch_query));
    i64 pos = view_get_cursor_pos(app, view) + cstring_length(previous_isearch_query);
    
    seek_string_forward(app, buffer, pos - 1, 0, last_search, &new_pos);
    
    if (new_pos != buffer_get_size(app, buffer))
    {
        view_set_cursor_and_preferred_x(app, view, seek_pos(new_pos));
    }
}

CUSTOM_COMMAND_SIG(MB_LastSearchBackward)
{
    View_ID view = get_active_view(app, Access_ReadVisible);
    Buffer_ID buffer = view_get_buffer(app, view, Access_ReadVisible);
    i64 pos = view_get_cursor_pos(app, view);
    
    i64 new_pos = 0;
    String_Const_u8 last_search = SCu8(previous_isearch_query, cstring_length(previous_isearch_query));
    seek_string_backward(app, buffer, pos - 1, 0, last_search, &new_pos);
    
    if (new_pos != -1)
    {
        view_set_cursor_and_preferred_x(app, view, seek_pos(new_pos));
    }
}


function void
MB_MarkoInitEditor(Application_Links* app)
{
    Base_Allocator* allocator = get_base_allocator_system();
    global_editor = base_array(allocator, EditorState, 1);
    global_editor->arena = make_arena(allocator, MB(8));
    global_editor->function_peek.arena = make_arena(allocator, KB(2));
    for (u8 mode = 0; mode < EditorMode_Count; ++mode)
    {
        global_editor->command_map[mode] = make_table_Data_u64(allocator, 1024);
        global_editor->root[mode] = push_array_zero(&global_editor->arena, TrieNode, 1);
    }
}


function String_ID
MB_GetCurrentMapID(Application_Links* app)
{
    View_ID view = get_this_ctx_view(app, Access_Always);
    Command_Map_ID map_id = default_get_map_id(app, view);
    
    return map_id;
}



function void
MB_MarkoSetupDefaultBindings(Mapping* mapping, i64 global_id, i64 normal_id, i64 edit_id)
{
    MappingScope();
    SelectMapping(mapping);
    
    SelectMap(global_id);
    // Essential
    BindCore(MB_Marko4CoderStartup, CoreCode_Startup);
    BindCore(default_try_exit, CoreCode_TryExit);
    BindCore(clipboard_record_clip, CoreCode_NewClipboardContents);
    BindMouseWheel(mouse_wheel_scroll);
    BindMouseWheel(mouse_wheel_change_face_size, KeyCode_Control);
    ////////////
    
    Bind(change_active_panel,           KeyCode_W, KeyCode_Alt);
    Bind(exit_4coder,                   KeyCode_F4, KeyCode_Alt);
    Bind(command_lister,                KeyCode_X, KeyCode_Alt);
    Bind(interactive_new,               KeyCode_N, KeyCode_Alt);
    Bind(interactive_open_or_new,       KeyCode_F, KeyCode_Alt);
    Bind(interactive_switch_buffer,            KeyCode_B, KeyCode_Alt);
    Bind(quick_swap_buffer,                    KeyCode_B, KeyCode_Control);
    Bind(move_up,                              KeyCode_Up);
    Bind(move_down,                            KeyCode_Down);
    Bind(move_left,                            KeyCode_Left);
    Bind(move_right,                           KeyCode_Right);
    Bind(goto_next_jump,                       KeyCode_E, KeyCode_Alt);
    Bind(goto_prev_jump,                       KeyCode_Q, KeyCode_Alt);
    Bind(load_theme_current_buffer,            KeyCode_F8);
    Bind(MB_RunProgram,                           KeyCode_F5);
    Bind(decrease_face_size,                   KeyCode_Minus, KeyCode_Control);
    Bind(increase_face_size,                   KeyCode_Equal, KeyCode_Control);
    
    // Essential
    BindMouse(click_set_cursor_and_mark,        MouseCode_Left);
    BindMouseRelease(click_set_cursor,          MouseCode_Left);
    BindCore(click_set_cursor_and_mark,         CoreCode_ClickActivateView);
    BindMouseMove(click_set_cursor_if_lbutton);
    ////////////
    Bind(write_todo,                           KeyCode_T, KeyCode_Control);
    Bind(goto_beginning_of_file,               KeyCode_Home);
    Bind(goto_end_of_file,                     KeyCode_End);
    
    Bind(word_complete_drop_down,              KeyCode_Tab, KeyCode_Control);
    Bind(word_complete,                        KeyCode_Tab);
    
    SelectMap(normal_id);
    ParentMap(global_id);
    
    SelectMap(edit_id);
    ParentMap(global_id);
    
    BindTextInput(write_text_and_auto_indent);
    
    
    // Revert back to normal mode
}

function void
MB_MarkoSetupBindings(EditorState* editor)
{
    // Normal mode mappings
    NMap(move_up,                              KeyCode_K);
    NMap(move_down,                            KeyCode_J);
    NMap(move_left,                            KeyCode_H);
    NMap(move_right,                           KeyCode_L);
    NMap(move_left_whitespace_or_token_boundary,        KeyCode_B);
    NMap(move_right_whitespace_or_token_boundary,       KeyCode_W);
    NMap(move_up_to_blank_line,                MB_KeyCombo(KeyCode_K, KeyCode_Alt));
    NMap(move_down_to_blank_line,              MB_KeyCombo(KeyCode_J, KeyCode_Alt));
    NMap(move_line_up,                         MB_KeyCombo(KeyCode_K, KeyCode_Control));
    NMap(move_line_down,                       MB_KeyCombo(KeyCode_J, KeyCode_Control));
    NMap(goto_line,                            MB_KeyCombo(KeyCode_G, KeyCode_Alt));
    NMap(MB_EnterEditMode,                        KeyCode_I);
    NMap(search,                               KeyCode_ForwardSlash);
    NMap(search_identifier,                    MB_KeyCombo(KeyCode_ForwardSlash, KeyCode_Control));
    NMap(copy,                                 KeyCode_Y);
    NMap(cut,                                  MB_KeyCombo(KeyCode_X, KeyCode_Control));
    NMap(paste_and_indent,                     KeyCode_P);
    NMap(redo,                                 MB_KeyCombo(KeyCode_R, KeyCode_Control));
    NMap(undo,                                 KeyCode_U);
    NMap(MB_GoBeginningOfLineAndEditMode,         MB_KeyCombo(KeyCode_I, KeyCode_Shift));
    NMap(MB_GotEndOfLineAndEditMode,              MB_KeyCombo(KeyCode_A, KeyCode_Shift));
    NMap(kill_buffer,                          MB_KeyCombo(KeyCode_K, KeyCode_Control, KeyCode_Shift));
    NMap(comment_line_toggle,                  MB_KeyCombo(KeyCode_C, KeyCode_Alt));
    NMap(set_mark,                             KeyCode_S);
    NMap(save,                                 MB_KeyCombo(KeyCode_S, KeyCode_Alt));
    NMap(MB_BackspaceDeleteWholeWord,             MB_KeyCombo(KeyCode_Backspace, KeyCode_Alt));
    NMap(MB_BackspaceDeleteAndEditMode,           MB_KeyCombo(KeyCode_Backspace, KeyCode_Control));
    NMap(MB_DeleteLine,                           MB_KeyCombo(KeyCode_D, KeyCode_Control));
    NMap(MB_DeleteRange,                          MB_KeyCombo(KeyCode_D, KeyCode_Alt));
    NMap(delete_char,                          KeyCode_X);
    NMap(MB_NewLineAndEditMode,                   KeyCode_O);
    NMap(cursor_mark_swap,                     MB_KeyCombo(KeyCode_A, KeyCode_Alt));
    NMap(MB_KillToEndOfLine,                      MB_KeyCombo(KeyCode_C, KeyCode_Shift));
    NMap(replace_in_range,                     MB_KeyCombo(KeyCode_R, KeyCode_Alt));
    NMap(replace_in_buffer,                    MB_KeyCombo(KeyCode_R, KeyCode_Alt, KeyCode_Control));
    NMap(duplicate_line,                       MB_KeyCombo(KeyCode_J, KeyCode_Alt, KeyCode_Control));
    NMap(MB_ToggleBuildPannel,                    KeyCode_Tick);
    NMap(MB_BuildProject,                         MB_KeyCombo(KeyCode_M, KeyCode_Alt));
    NMap(list_all_functions_all_buffers_lister,     MB_KeyCombo(KeyCode_P, KeyCode_Alt, KeyCode_Control));
    NMap(list_all_functions_current_buffer_lister,  MB_KeyCombo(KeyCode_P, KeyCode_Alt));
    NMap(MB_ReplayKeyboardMacro,                       MB_KeyCombo(KeyCode_Period, KeyCode_Control));
    NMap(keyboard_macro_start_recording,            KeyCode_F1);
    NMap(keyboard_macro_finish_recording,           KeyCode_F2);
    NMap(keyboard_macro_finish_recording,           KeyCode_Escape);
    NMap(MB_LastSearchForward,                         KeyCode_N);
    NMap(MB_LastSearchBackward,                        MB_KeyCombo(KeyCode_N, KeyCode_Shift));
    NMap(jump_to_definition_at_cursor,              MB_KeyCombo(KeyCode_I, KeyCode_Alt));
    
    // Insert mode mappings
    IMap(MB_EnterNormalMode,                      KeyCode_CapsLock);
    IMap(MB_EnterNormalMode,                      KeyCode_Escape);
    IMap(backspace_alpha_numeric_boundary,     MB_KeyCombo(KeyCode_Backspace, KeyCode_Control));
    IMap(MB_BackspaceDeleteWholeWord,             MB_KeyCombo(KeyCode_Backspace, KeyCode_Alt));
    IMap(move_line_up,                         MB_KeyCombo(KeyCode_K, KeyCode_Control));
    IMap(move_line_down,                       MB_KeyCombo(KeyCode_J, KeyCode_Control));
    IMap(goto_beginning_of_file,               KeyCode_Home);
    IMap(goto_end_of_file,                     KeyCode_End);
    IMap(move_left_whitespace_boundary,        MB_KeyCombo(KeyCode_Left, KeyCode_Control));
    IMap(move_right_whitespace_boundary,       MB_KeyCombo(KeyCode_Right, KeyCode_Control));
    IMap(backspace_char,                       KeyCode_Backspace);
}

CUSTOM_COMMAND_SIG(MB_ViewInputHandler)
{
    Scratch_Block scratch(app);
    default_input_handler_init(app, scratch);
    View_ID view = get_this_ctx_view(app, Access_Always);
    Managed_Scope scope = view_get_managed_scope(app, view);
    for (;;)
    {
        // NOTE(allen): Get input
        User_Input input = get_next_input(app, EventPropertyGroup_Any, 0);
        if (input.abort){
            break;
        }
        ProfileScopeNamed(app, "before view input", view_input_profile);
        // NOTE(mb): Function peek logic
        MB_FunctionPeekOnInput(app, view, &input, &global_editor->function_peek);
        // NOTE(allen): Mouse Suppression
        Event_Property event_properties = get_event_properties(&input.event);
        Custom_Command_Function* command = 0;
        if (suppressing_mouse && (event_properties & EventPropertyGroup_AnyMouseEvent) != 0){
            continue;
        }
        // If input is sequence, wait for sequence
        if (input.event.kind == InputEventKind_KeyStroke && 
            !MB_IsKeyCodeModifier(input.event.core.code))
        {
            KeyWithModifier key_mod = MB_InputEventToKeyWithModifier(&input.event);
            TrieNode* node = 0;
            EditorState* editor = global_editor;
            if (MB_TrieTryGetNode(editor->root[editor->current_mode], 
                                  &editor->key_sequence,
                                  editor->key_sequence_level,
                                  &node, key_mod))
            {
                editor->key_sequence.keys[editor->key_sequence_level] = key_mod;
                if (!node->is_command)
                {
                    leave_current_input_unhandled(app);
                    editor->key_sequence_level++;
                    continue;
                }
                String_Const_u8 key = make_data(&editor->key_sequence, sizeof(KeySequence));
                Table_Lookup lookup = table_lookup(&editor->command_map[editor->current_mode], key);
                if (lookup.found_match)
                {
                    u64 val = {};
                    table_read(&editor->command_map[editor->current_mode], lookup, &val);
                    command = (Custom_Command_Function*)IntAsPtr(val);
                    editor->key_sequence_level = 0;
                    editor->key_sequence = {};
                    goto finish_command;
                }
            }
        }
        
        if (implicit_map_function == 0) {
            implicit_map_function = default_implicit_map;
        }
        
        Implicit_Map_Result map_result = implicit_map_function(app, 0, 0, &input.event);
        
        if (map_result.command == 0)
        {
            leave_current_input_unhandled(app);
            continue;
        }
        command = map_result.command;
        // GOTO
        finish_command:
        default_pre_command(app, scope);
        ProfileCloseNow(view_input_profile);
        command(app);
        ProfileScope(app, "after view input");
        default_post_command(app, scope);
    }
}

function void
MB_RenderBuffer(Application_Links *app, View_ID view_id, Face_ID face_id,
                Buffer_ID buffer, Text_Layout_ID text_layout_id,
                Rect_f32 rect){
    ProfileScope(app, "render buffer");
    
    View_ID active_view = get_active_view(app, Access_Always);
    b32 is_active_view = (active_view == view_id);
    Rect_f32 prev_clip = draw_set_clip(app, rect);
    
    Range_i64 visible_range = text_layout_get_visible_range(app, text_layout_id);
    
    // NOTE(allen): Cursor shape
    Face_Metrics metrics = get_face_metrics(app, face_id);
    u64 cursor_roundness_100 = def_get_config_u64(app, vars_save_string_lit("cursor_roundness"));
    f32 cursor_roundness = metrics.normal_advance*cursor_roundness_100*0.01f;
    f32 mark_thickness = (f32)def_get_config_u64(app, vars_save_string_lit("mark_thickness"));
    
    // NOTE(allen): Token colorizing
    Token_Array token_array = get_token_array_from_buffer(app, buffer);
    if (token_array.tokens != 0){
        primitive_highlight_draw_cpp_token_colors( app, text_layout_id, &token_array, buffer );
        // draw_cpp_token_colors(app, text_layout_id, &token_array);
        
        
        // NOTE(allen): Scan for TODOs and NOTEs
        b32 use_comment_keyword = def_get_config_b32(vars_save_string_lit("use_comment_keyword"));
        if (use_comment_keyword){
            Comment_Highlight_Pair pairs[] = {
                {string_u8_litexpr("NOTE"), finalize_color(defcolor_comment_pop, 0)},
                {string_u8_litexpr("TODO"), finalize_color(defcolor_comment_pop, 1)},
            };
            draw_comment_highlights(app, buffer, text_layout_id, &token_array, pairs, ArrayCount(pairs));
        }
        
#if 0
        // TODO(allen): Put in 4coder_draw.cpp
        // NOTE(allen): Color functions
        
        Scratch_Block scratch(app);
        ARGB_Color argb = 0xFFFF00FF;
        
        Token_Iterator_Array it = token_iterator_pos(0, &token_array, visible_range.first);
        for (;;){
            if (!token_it_inc_non_whitespace(&it)){
                break;
            }
            Token *token = token_it_read(&it);
            String_Const_u8 lexeme = push_token_lexeme(app, scratch, buffer, token);
            Code_Index_Note *note = code_index_note_from_string(lexeme);
            if (note != 0 && note->note_kind == CodeIndexNote_Function){
                paint_text_color(app, text_layout_id, Ii64_size(token->pos, token->size), argb);
            }
        }
#endif
    }
    else{
        paint_text_color_fcolor(app, text_layout_id, visible_range, fcolor_id(defcolor_text_default));
    }
    
    i64 cursor_pos = view_correct_cursor(app, view_id);
    view_correct_mark(app, view_id);
    
    // NOTE(allen): Scope highlight
    b32 use_scope_highlight = def_get_config_b32(vars_save_string_lit("use_scope_highlight"));
    if (use_scope_highlight){
        Color_Array colors = finalize_color_array(defcolor_back_cycle);
        draw_scope_highlight(app, buffer, text_layout_id, cursor_pos, colors.vals, colors.count);
    }
    
    b32 use_error_highlight = def_get_config_b32(vars_save_string_lit("use_error_highlight"));
    b32 use_jump_highlight = def_get_config_b32(vars_save_string_lit("use_jump_highlight"));
    if (use_error_highlight || use_jump_highlight){
        // NOTE(allen): Error highlight
        String_Const_u8 name = string_u8_litexpr("*compilation*");
        Buffer_ID compilation_buffer = get_buffer_by_name(app, name, Access_Always);
        if (use_error_highlight){
            draw_jump_highlights(app, buffer, text_layout_id, compilation_buffer,
                                 fcolor_id(defcolor_highlight_junk));
        }
        
        // NOTE(allen): Search highlight
        if (use_jump_highlight){
            Buffer_ID jump_buffer = get_locked_jump_buffer(app);
            if (jump_buffer != compilation_buffer){
                draw_jump_highlights(app, buffer, text_layout_id, jump_buffer,
                                     fcolor_id(defcolor_highlight_white));
            }
        }
    }
    
    // NOTE(allen): Color parens
    b32 use_paren_helper = def_get_config_b32(vars_save_string_lit("use_paren_helper"));
    if (use_paren_helper){
        Color_Array colors = finalize_color_array(defcolor_text_cycle);
        draw_paren_highlight(app, buffer, text_layout_id, cursor_pos, colors.vals, colors.count);
    }
    
    // NOTE(allen): Line highlight
    b32 highlight_line_at_cursor = def_get_config_b32(vars_save_string_lit("highlight_line_at_cursor"));
    if (highlight_line_at_cursor && is_active_view){
        i64 line_number = get_line_number_from_pos(app, buffer, cursor_pos);
        draw_line_highlight(app, text_layout_id, line_number, fcolor_id(defcolor_highlight_cursor_line));
    }
    
    // NOTE(allen): Whitespace highlight
    b64 show_whitespace = false;
    view_get_setting(app, view_id, ViewSetting_ShowWhitespace, &show_whitespace);
    if (show_whitespace){
        if (token_array.tokens == 0){
            draw_whitespace_highlight(app, buffer, text_layout_id, cursor_roundness);
        }
        else{
            draw_whitespace_highlight(app, text_layout_id, &token_array, cursor_roundness);
        }
    }
    
    // NOTE(allen): Cursor
    switch (fcoder_mode){
        case FCoderMode_Original:
        {
            draw_original_4coder_style_cursor_mark_highlight(app, view_id, is_active_view, buffer, text_layout_id, cursor_roundness, mark_thickness);
        }break;
        case FCoderMode_NotepadLike:
        {
            draw_notepad_style_cursor_highlight(app, view_id, buffer, text_layout_id, cursor_roundness);
        }break;
    }
    
    // NOTE(allen): Fade ranges
    paint_fade_ranges(app, text_layout_id, buffer);
    
    // NOTE(allen): put the actual text on the actual screen
    draw_text_layout_default(app, text_layout_id);
    
    draw_set_clip(app, prev_clip);
}

function void
MB_RenderCaller(Application_Links *app, Frame_Info frame_info, View_ID view_id){
    ProfileScope(app, "default render caller");
    View_ID active_view = get_active_view(app, Access_Always);
    b32 is_active_view = (active_view == view_id);
    
    Rect_f32 region = draw_background_and_margin(app, view_id, is_active_view);
    Rect_f32 prev_clip = draw_set_clip(app, region);
    
    Buffer_ID buffer = view_get_buffer(app, view_id, Access_Always);
    Face_ID face_id = get_face_id(app, buffer);
    Face_Metrics face_metrics = get_face_metrics(app, face_id);
    f32 line_height = face_metrics.line_height;
    f32 digit_advance = face_metrics.decimal_digit_advance;
    
    // NOTE(allen): file bar
    b64 showing_file_bar = false;
    if (view_get_setting(app, view_id, ViewSetting_ShowFileBar, &showing_file_bar) && showing_file_bar){
        Rect_f32_Pair pair = layout_file_bar_on_top(region, line_height);
        draw_file_bar(app, view_id, buffer, face_id, pair.min);
        region = pair.max;
    }
    
    Buffer_Scroll scroll = view_get_buffer_scroll(app, view_id);
    
    Buffer_Point_Delta_Result delta = delta_apply(app, view_id,
                                                  frame_info.animation_dt, scroll);
    if (!block_match_struct(&scroll.position, &delta.point)){
        block_copy_struct(&scroll.position, &delta.point);
        view_set_buffer_scroll(app, view_id, scroll, SetBufferScroll_NoCursorChange);
    }
    if (delta.still_animating){
        animate_in_n_milliseconds(app, 0);
    }
    
    // NOTE(allen): query bars
    region = default_draw_query_bars(app, region, view_id, face_id);
    
    // NOTE(allen): FPS hud
    if (show_fps_hud){
        Rect_f32_Pair pair = layout_fps_hud_on_bottom(region, line_height);
        draw_fps_hud(app, frame_info, face_id, pair.max);
        region = pair.min;
        animate_in_n_milliseconds(app, 1000);
    }
    
    // NOTE(allen): layout line numbers
    b32 show_line_number_margins = def_get_config_b32(vars_save_string_lit("show_line_number_margins"));
    Rect_f32 line_number_rect = {};
    if (show_line_number_margins){
        Rect_f32_Pair pair = layout_line_number_margin(app, buffer, region, digit_advance);
        line_number_rect = pair.min;
        region = pair.max;
    }
    
    // NOTE(allen): begin buffer render
    Buffer_Point buffer_point = scroll.position;
    Text_Layout_ID text_layout_id = text_layout_create(app, buffer, region, buffer_point);
    
    // NOTE(allen): draw line numbers
    if (show_line_number_margins){
        draw_line_number_margin(app, view_id, buffer, face_id, text_layout_id, line_number_rect);
    }
    
    // NOTE(allen): draw the buffer
    MB_RenderBuffer(app, view_id, face_id, buffer, text_layout_id, region);
    
    // NOTE(mb): rendering
    if (global_peek_open)
    {
        MB_RenderFunctionPeek(app, view_id, text_layout_id, &global_editor->function_peek);
    }
    
    text_layout_free(app, text_layout_id);
    draw_set_clip(app, prev_clip);
}

BUFFER_HOOK_SIG(MB_MarkoBeginBuffer)
{
    ProfileScope(app, "begin buffer");
    
    Scratch_Block scratch(app);
    
    b32 treat_as_code = false;
    String_Const_u8 file_name = push_buffer_file_name(app, scratch, buffer_id);
    if (file_name.size > 0){
        String_Const_u8 treat_as_code_string = def_get_config_string(scratch, vars_save_string_lit("treat_as_code"));
        String_Const_u8_Array extensions = parse_extension_line_to_extension_list(app, scratch, treat_as_code_string);
        String_Const_u8 ext = string_file_extension(file_name);
        for (i32 i = 0; i < extensions.count; ++i){
            if (string_match(ext, extensions.strings[i])){
                
                if (string_match(ext, string_u8_litexpr("cpp")) ||
                    string_match(ext, string_u8_litexpr("h")) ||
                    string_match(ext, string_u8_litexpr("c")) ||
                    string_match(ext, string_u8_litexpr("hpp")) ||
                    string_match(ext, string_u8_litexpr("cc")) ||
                    string_match(ext, string_u8_litexpr("glsl")) ||
                    string_match(ext, string_u8_litexpr("mdesk"))) {
                    treat_as_code = true;
                }
                
#if 0
                treat_as_code = true;
                
                if (string_match(ext, string_u8_litexpr("cs"))){
                    if (parse_context_language_cs == 0){
                        init_language_cs(app);
                    }
                    parse_context_id = parse_context_language_cs;
                }
                
                if (string_match(ext, string_u8_litexpr("java"))){
                    if (parse_context_language_java == 0){
                        init_language_java(app);
                    }
                    parse_context_id = parse_context_language_java;
                }
                
                if (string_match(ext, string_u8_litexpr("rs"))){
                    if (parse_context_language_rust == 0){
                        init_language_rust(app);
                    }
                    parse_context_id = parse_context_language_rust;
                }
                if (string_match(ext, string_u8_litexpr("cpp")) ||
                    string_match(ext, string_u8_litexpr("h")) ||
                    string_match(ext, string_u8_litexpr("c")) ||
                    string_match(ext, string_u8_litexpr("hpp")) ||
                    string_match(ext, string_u8_litexpr("cc"))){
                    if (parse_context_language_cpp == 0){
                        init_language_cpp(app);
                    }
                    parse_context_id = parse_context_language_cpp;
                }
                
                // TODO(NAME): Real GLSL highlighting
                if (string_match(ext, string_u8_litexpr("glsl"))){
                    if (parse_context_language_cpp == 0){
                        init_language_cpp(app);
                    }
                    parse_context_id = parse_context_language_cpp;
                }
                
                // TODO(NAME): Real Objective-C highlighting
                if (string_match(ext, string_u8_litexpr("m"))){
                    if (parse_context_language_cpp == 0){
                        init_language_cpp(app);
                    }
                    parse_context_id = parse_context_language_cpp;
                }
#endif
                
                break;
            }
        }
    }
    
    
    // TODO ///////////
    // String_ID code_map_id = vars_save_string_lit("keys_code");
    // Command_Map_ID map_id = (treat_as_code)?(code_map_id):(file_map_id);
    // String_ID normal_map_id = vars_save_string_lit("keys_normal");
    Command_Map_ID map_id = normal_map_id;
    Managed_Scope scope = buffer_get_managed_scope(app, buffer_id);
    Command_Map_ID *map_id_ptr = scope_attachment(app, scope, buffer_map_id, Command_Map_ID);
    *map_id_ptr = map_id;
    
    Line_Ending_Kind setting = guess_line_ending_kind_from_buffer(app, buffer_id);
    Line_Ending_Kind *eol_setting = scope_attachment(app, scope, buffer_eol_setting, Line_Ending_Kind);
    *eol_setting = setting;
    
    // NOTE(allen): Decide buffer settings
    b32 wrap_lines = true;
    b32 use_lexer = false;
    if (treat_as_code){
        wrap_lines = def_get_config_b32(vars_save_string_lit("enable_code_wrapping"));
        use_lexer = true;
    }
    
    String_Const_u8 buffer_name = push_buffer_base_name(app, scratch, buffer_id);
    if (buffer_name.size > 0 && buffer_name.str[0] == '*' && buffer_name.str[buffer_name.size - 1] == '*'){
        wrap_lines = def_get_config_b32(vars_save_string_lit("enable_output_wrapping"));
    }
    
    if (use_lexer){
        ProfileBlock(app, "begin buffer kick off lexer");
        Async_Task *lex_task_ptr = scope_attachment(app, scope, buffer_lex_task, Async_Task);
        *lex_task_ptr = async_task_no_dep(&global_async_system, do_full_lex_async, make_data_struct(&buffer_id));
    }
    
    {
        b32 *wrap_lines_ptr = scope_attachment(app, scope, buffer_wrap_lines, b32);
        *wrap_lines_ptr = wrap_lines;
    }
    
    if (use_lexer){
        buffer_set_layout(app, buffer_id, layout_virt_indent_index_generic);
    }
    else{
        if (treat_as_code){
            buffer_set_layout(app, buffer_id, layout_virt_indent_literal_generic);
        }
        else{
            buffer_set_layout(app, buffer_id, layout_generic);
        }
    }
    
    MB_EnterNormalMode(app);
    
    /*
    Rect_f32 rect = Rf32(0, 0, 100, 100);
    draw_rectangle_fcolor(app, rect, 0, 
                          */
    return 0;
}

void
custom_layer_init(Application_Links *app){
    Thread_Context *tctx = get_thread_context(app);
    // Base 4coder init
    default_framework_init(app);
    
    MB_MarkoInitEditor(app);
    
    set_custom_hook(app, HookID_BufferViewerUpdate, default_view_adjust);
    
    set_custom_hook(app, HookID_ViewEventHandler, MB_ViewInputHandler);
    set_custom_hook(app, HookID_Tick, default_tick);
    set_custom_hook(app, HookID_RenderCaller, MB_RenderCaller);
    set_custom_hook(app, HookID_WholeScreenRenderCaller, default_whole_screen_render_caller);
    
    set_custom_hook(app, HookID_DeltaRule, fixed_time_cubic_delta);
    set_custom_hook_memory_size(app, HookID_DeltaRule,
                                delta_ctx_size(fixed_time_cubic_delta_memory_size));
    
    set_custom_hook(app, HookID_BufferNameResolver, default_buffer_name_resolution);
    
    set_custom_hook(app, HookID_EndBuffer, end_buffer_close_jump_list);
    set_custom_hook(app, HookID_NewFile, default_new_file);
    set_custom_hook(app, HookID_SaveFile, default_file_save);
    set_custom_hook(app, HookID_BufferEditRange, default_buffer_edit_range);
    set_custom_hook(app, HookID_BufferRegion, default_buffer_region);
    set_custom_hook(app, HookID_ViewChangeBuffer, default_view_change_buffer);
    set_custom_hook(app, HookID_BeginBuffer, MB_MarkoBeginBuffer);
    
    set_custom_hook(app, HookID_Layout, layout_unwrapped);
    
    mapping_init(tctx, &framework_mapping);
    
    String_ID global_map_id = vars_save_string_lit("keys_global");
    normal_map_id = vars_save_string_lit("keys_file");
    edit_map_id = vars_save_string_lit("keys_edit");
    
    // setup_essential_mapping(&framework_mapping, global_map_id, file_map_id, normal_map_id);
    MB_MarkoSetupDefaultBindings(&framework_mapping, global_map_id, normal_map_id, edit_map_id);
    MB_MarkoSetupBindings(global_editor);
}

#endif //FCODER_DEFAULT_BINDINGS

// BOTTOM

