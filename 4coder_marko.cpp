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
static Arena global_main_arena;

#include "marko_cpp_highlight.cpp"
#include "marko_modes.cpp"
#include "marko_cursor.cpp"

// HELLO

#if !defined(META_PASS)
#include "generated/managed_id_metadata.cpp"
#endif

CUSTOM_COMMAND_SIG(RunProgram)
{
    prj_exec_command_name(app, SCu8("run"));
    
}

CUSTOM_COMMAND_SIG(NewLineAndEditMode)
{
    EnterEditMode(app);
    seek_end_of_line(app);
    write_text(app, string_u8_litexpr("\n"));
}

CUSTOM_COMMAND_SIG(DeleteRange)
{
    View_ID view = get_active_view(app, Access_ReadWriteVisible);
    Buffer_ID buffer = view_get_buffer(app, view, Access_ReadWriteVisible);
    Range_i64 range = get_view_range(app, view);
    clipboard_post_buffer_range(app, 0, buffer, range);
    buffer_replace_range(app, buffer, range, string_u8_empty);
}

CUSTOM_COMMAND_SIG(DeleteLine)
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



CUSTOM_COMMAND_SIG(KillToEndOfLine)
{
    EnterEditMode(app);
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

CUSTOM_COMMAND_SIG(BackspaceDeleteAndEditMode)
{
    EnterEditMode(app);
    backspace_alpha_numeric_boundary(app);
}

CUSTOM_COMMAND_SIG(BackspaceDeleteWholeWord)
{
    EnterEditMode(app);
    snipe_backward_whitespace_or_token_boundary(app);
}



CUSTOM_COMMAND_SIG(BuildProject)
{
    build_in_build_panel(app);
    global_build_panel_open = true;
}

CUSTOM_COMMAND_SIG(ToggleBuildPannel)
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


#undef Bind

#include <windows.h>

#define Bind(F, K, ...) \
map_set_binding_l(m, map, BindFWrap_(F), InputEventKind_KeyStroke, (K), __VA_ARGS__, 0)

static b32
IsCodeFile(char* name, u64 len){
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

CUSTOM_COMMAND_SIG(Marko4CoderStartup)
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

CUSTOM_COMMAND_SIG(ReplayKeyboardMacro)
{
    keyboard_macro_replay(app);
}


CUSTOM_COMMAND_SIG(LastSearchForward)
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

CUSTOM_COMMAND_SIG(LastSearchBackward)
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
MarkoSetupBindings(Mapping* mapping, i64 global_id, i64 normal_id, i64 edit_id)
{
    MappingScope();
    SelectMapping(mapping);
    
    SelectMap(global_id);
    // Essential
    BindCore(Marko4CoderStartup, CoreCode_Startup);
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
    Bind(RunProgram,                           KeyCode_F5);
    
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
    
    Bind(move_up,                              KeyCode_K);
    Bind(move_down,                            KeyCode_J);
    Bind(move_left,                            KeyCode_H);
    Bind(move_right,                           KeyCode_L);
    Bind(CursorMoveLeft,                     KeyCode_B);
    Bind(CursorMoveRight,                    KeyCode_W);
    Bind(CursorJumpUp,                       KeyCode_K, KeyCode_Alt);
    Bind(CursorJumpDown, KeyCode_J, KeyCode_Alt);
    Bind(move_line_up,                         KeyCode_K, KeyCode_Control);
    Bind(move_line_down,                       KeyCode_J, KeyCode_Control);
    Bind(goto_line,                            KeyCode_G, KeyCode_Alt);
    Bind(EnterEditMode,                      KeyCode_I);
    Bind(search,                               KeyCode_ForwardSlash);
    Bind(search_identifier,                    KeyCode_ForwardSlash, KeyCode_Control);
    Bind(copy,                                 KeyCode_Y);
    Bind(cut,                                  KeyCode_X, KeyCode_Control);
    Bind(paste_and_indent,                     KeyCode_P);
    Bind(redo,                                 KeyCode_R, KeyCode_Control);
    Bind(undo,                                 KeyCode_U);
    Bind(GoBeginningOfLineAndEditMode,   KeyCode_I, KeyCode_Shift);
    Bind(GotEndOfLineAndEditMode,         KeyCode_A, KeyCode_Shift);
    Bind(kill_buffer,                          KeyCode_K, KeyCode_Control, KeyCode_Shift);
    Bind(comment_line_toggle,                  KeyCode_C, KeyCode_Alt);
    Bind(set_mark,                             KeyCode_S);
    Bind(save,                                 KeyCode_S, KeyCode_Alt);
    Bind(BackspaceDeleteWholeWord,             KeyCode_Backspace, KeyCode_Alt);
    Bind(BackspaceDeleteAndEditMode,           KeyCode_Backspace, KeyCode_Control);
    Bind(DeleteLine,                          KeyCode_D, KeyCode_Control);
    Bind(DeleteRange,                         KeyCode_D, KeyCode_Alt);
    Bind(delete_char,                          KeyCode_X);
    Bind(NewLineAndEditMode,               KeyCode_O);
    Bind(cursor_mark_swap,                     KeyCode_A, KeyCode_Alt);
    Bind(KillToEndOfLine,                  KeyCode_C, KeyCode_Shift);
    Bind(replace_in_range,                     KeyCode_R, KeyCode_Alt);
    Bind(replace_in_buffer,                    KeyCode_R, KeyCode_Alt, KeyCode_Control);
    Bind(duplicate_line,                       KeyCode_J, KeyCode_Alt, KeyCode_Control);
    Bind(ToggleBuildPannel,                   KeyCode_Tick);
    Bind(BuildProject,                        KeyCode_M, KeyCode_Alt);
    Bind(list_all_functions_all_buffers_lister,     KeyCode_P, KeyCode_Alt, KeyCode_Control);
    Bind(list_all_functions_current_buffer_lister,  KeyCode_P, KeyCode_Alt);
    Bind(ReplayKeyboardMacro,                 KeyCode_Period, KeyCode_Control);
    Bind(keyboard_macro_start_recording,       KeyCode_F1);
    Bind(keyboard_macro_finish_recording,      KeyCode_F2);
    Bind(keyboard_macro_finish_recording,      KeyCode_Escape);
    Bind(LastSearchForward,                        KeyCode_N);
    Bind(LastSearchBackward,                       KeyCode_N, KeyCode_Shift);
    
    //
    SelectMap(edit_id);
    ParentMap(global_id);
    
    
    
    Bind(EnterNormalMode,                      KeyCode_CapsLock);
    Bind(EnterNormalMode,                      KeyCode_Escape);
    Bind(backspace_alpha_numeric_boundary,     KeyCode_Backspace, KeyCode_Control);
    Bind(BackspaceDeleteWholeWord,             KeyCode_Backspace, KeyCode_Alt);
    Bind(move_line_up,                         KeyCode_K, KeyCode_Control);
    Bind(move_line_down,                       KeyCode_J, KeyCode_Control);
    Bind(goto_beginning_of_file,               KeyCode_Home);
    Bind(goto_end_of_file,                     KeyCode_End);
    Bind(move_left_whitespace_boundary,        KeyCode_Left, KeyCode_Control);
    Bind(move_right_whitespace_boundary,       KeyCode_Right, KeyCode_Control);
    Bind(move_up_to_blank_line_end,            KeyCode_K, KeyCode_Alt);
    Bind(move_down_to_blank_line_end,          KeyCode_J, KeyCode_Alt);
    Bind(backspace_char,                       KeyCode_Backspace);
    
    BindTextInput(write_text_and_auto_indent);
}

BUFFER_HOOK_SIG(MarkoBeginBuffer)
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
                    string_match(ext, string_u8_litexpr("cc"))){
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
    
    EnterNormalMode(app);
    
    // no meaning for return
    return(0);
}


void
custom_layer_init(Application_Links *app){
    Thread_Context *tctx = get_thread_context(app);
    Base_Allocator* allocator = get_base_allocator_system();
    global_main_arena = make_arena(allocator, 16384);
    
    // Base 4coder init
    default_framework_init(app);
    
    set_all_default_hooks(app);
    mapping_init(tctx, &framework_mapping);
    
    String_ID global_map_id = vars_save_string_lit("keys_global");
    normal_map_id = vars_save_string_lit("keys_file");
    edit_map_id = vars_save_string_lit("keys_edit");
    
    // setup_essential_mapping(&framework_mapping, global_map_id, file_map_id, normal_map_id);
	MarkoSetupBindings(&framework_mapping, global_map_id, normal_map_id, edit_map_id);
    
    set_custom_hook(app, HookID_BeginBuffer, MarkoBeginBuffer);
    
}

#endif //FCODER_DEFAULT_BINDINGS

// BOTTOM

