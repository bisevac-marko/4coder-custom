/*
4coder_marko.cpp - Supplies the default bindings used for default 4coder behavior.
*/

// TOP

#if !defined(FCODER_DEFAULT_BINDINGS_CPP)
#define FCODER_DEFAULT_BINDINGS_CPP

#include "4coder_default_include.cpp"

// NOTE(allen): Users can declare their own managed IDs here.

#if !defined(META_PASS)
#include "generated/managed_id_metadata.cpp"
#endif

bool global_edit_mode = true;
String_ID edit_map_id;
String_ID normal_map_id;

void 
set_current_mapid(Application_Links* app, Command_Map_ID mapid) 
{
    View_ID view = get_active_view(app, 0);
    Buffer_ID buffer = view_get_buffer(app, view, 0);
    Managed_Scope scope = buffer_get_managed_scope(app, buffer);
    Command_Map_ID* map_id_ptr = scope_attachment(app, scope, buffer_map_id, Command_Map_ID);
    *map_id_ptr = mapid;
}

CUSTOM_COMMAND_SIG(enter_edit_mode)
{
    set_current_mapid(app, edit_map_id);
    // print_message(app, string_u8_litexpr("Test"));
}

CUSTOM_COMMAND_SIG(enter_normal_mode)
{
    set_current_mapid(app, normal_map_id);
}

CUSTOM_COMMAND_SIG(go_end_of_line_and_edit_mode)
{
    seek_end_of_line(app);
    enter_edit_mode(app);
}

CUSTOM_COMMAND_SIG(go_beginning_of_line_and_edit_mode)
{
    seek_beginning_of_line(app);
    enter_edit_mode(app);
}

function void
marko_setup_bindings(Mapping* mapping, i64 global_id, i64 file_id, i64 normal_id, i64 edit_id)
{
    MappingScope();
    SelectMapping(mapping);

    SelectMap(global_id);

    // Essential
    BindCore(default_startup, CoreCode_Startup);
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

    SelectMap(file_id);
    ParentMap(global_id);

    // Essential
    BindMouse(click_set_cursor_and_mark, MouseCode_Left);
    BindMouseRelease(click_set_cursor, MouseCode_Left);
    BindCore(click_set_cursor_and_mark, CoreCode_ClickActivateView);
    BindMouseMove(click_set_cursor_if_lbutton);
    ////////////

    //
    SelectMap(normal_id);
    ParentMap(file_id);

    Bind(move_up,                KeyCode_K);
    Bind(move_down,              KeyCode_J);
    Bind(move_left,              KeyCode_H);
    Bind(move_right,             KeyCode_L);
    Bind(move_left_whitespace_boundary,    KeyCode_B);
    Bind(move_right_whitespace_boundary,   KeyCode_W);
    Bind(move_up_to_blank_line_end,        KeyCode_K, KeyCode_Alt);
    Bind(move_down_to_blank_line_end,      KeyCode_J, KeyCode_Alt);
    Bind(move_line_up,                     KeyCode_K, KeyCode_Control);
    Bind(move_line_down,                   KeyCode_J, KeyCode_Control);
    Bind(goto_line,                        KeyCode_G, KeyCode_Alt);
    Bind(enter_edit_mode,        KeyCode_I);
    Bind(backspace_alpha_numeric_boundary, KeyCode_Backspace, KeyCode_Control);
    Bind(search,                 KeyCode_ForwardSlash);
    Bind(copy,                   KeyCode_Y);
    Bind(cut,                    KeyCode_X, KeyCode_Control);
    Bind(paste_and_indent,       KeyCode_P);
    Bind(redo,                   KeyCode_R);
    Bind(undo,                   KeyCode_U);
    Bind(goto_beginning_of_file, KeyCode_Home);
    Bind(goto_end_of_file,       KeyCode_End);
    Bind(go_beginning_of_line_and_edit_mode, KeyCode_I, KeyCode_Shift);
    Bind(go_end_of_line_and_edit_mode, KeyCode_A, KeyCode_Shift);
    Bind(kill_buffer,                  KeyCode_K, KeyCode_Control, KeyCode_Shift);
    Bind(comment_line_toggle,          KeyCode_C, KeyCode_Alt);
    Bind(word_complete,                KeyCode_Tab);
    Bind(set_mark,                     KeyCode_S);
    Bind(save,                         KeyCode_S, KeyCode_Alt);
    Bind(snipe_backward_whitespace_or_token_boundary, KeyCode_Backspace, KeyCode_Alt);
    Bind(delete_line,                  KeyCode_D, KeyCode_Alt);
    Bind(delete_char,                  KeyCode_X);

    //
    SelectMap(edit_id);
    ParentMap(file_id);

    Bind(enter_normal_mode,             KeyCode_CapsLock);
    Bind(enter_normal_mode,             KeyCode_Escape);
    Bind(backspace_alpha_numeric_boundary, KeyCode_Backspace, KeyCode_Control);
    Bind(snipe_backward_whitespace_or_token_boundary, KeyCode_Backspace, KeyCode_Alt);
    Bind(move_line_up,                     KeyCode_K, KeyCode_Control);
    Bind(move_line_down,                   KeyCode_J, KeyCode_Control);
    Bind(goto_beginning_of_file, KeyCode_Home);
    Bind(goto_end_of_file,       KeyCode_End);
    Bind(word_complete,                KeyCode_Tab);
    Bind(move_left_whitespace_boundary,    KeyCode_Left, KeyCode_Control);
    Bind(move_right_whitespace_boundary,   KeyCode_Right, KeyCode_Control);
    Bind(move_up_to_blank_line_end,        KeyCode_K, KeyCode_Alt);
    Bind(move_down_to_blank_line_end,      KeyCode_J, KeyCode_Alt);
    Bind(backspace_char,         KeyCode_Backspace);
    Bind(move_up,                KeyCode_Up);
    Bind(move_down,              KeyCode_Down);
    Bind(move_left,              KeyCode_Left);
    Bind(move_right,             KeyCode_Right);

    BindTextInput(write_text_and_auto_indent);
}

BUFFER_HOOK_SIG(marko_begin_buffer)
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
    
    // no meaning for return
    return(0);
}


void
custom_layer_init(Application_Links *app){
    Thread_Context *tctx = get_thread_context(app);
    
    // Base 4coder init
    default_framework_init(app);
    
    set_all_default_hooks(app);
    mapping_init(tctx, &framework_mapping);

    String_ID global_map_id = vars_save_string_lit("keys_global");
    String_ID file_map_id = vars_save_string_lit("keys_file");
    normal_map_id = vars_save_string_lit("keys_normal");
    edit_map_id = vars_save_string_lit("keys_edit");

    // setup_essential_mapping(&framework_mapping, global_map_id, file_map_id, normal_map_id);
	marko_setup_bindings(&framework_mapping, global_map_id, file_map_id, normal_map_id, edit_map_id);

    set_custom_hook(app, HookID_BeginBuffer, marko_begin_buffer);
    // set_current_mapid(app, normal_map_id);
}

#endif //FCODER_DEFAULT_BINDINGS

// BOTTOM

