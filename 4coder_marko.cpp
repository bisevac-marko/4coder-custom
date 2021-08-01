/*
4coder_marko.cpp - Supplies the default bindings used for default 4coder behavior.
*/

// TOP

#if !defined(FCODER_DEFAULT_BINDINGS_CPP)
#define FCODER_DEFAULT_BINDINGS_CPP

#include "4coder_default_include.cpp"


CUSTOM_ID( colors, primitive_highlight_type );
CUSTOM_ID( colors, primitive_highlight_function );
CUSTOM_ID( colors, primitive_highlight_macro );
CUSTOM_ID( colors, primitive_highlight_4coder_command );

/* NOTE: end is the index of the last item, NOT one past the last item. */
function void primitive_highlight_quick_sort_hashes_notes( u64* hashes, Code_Index_Note** notes, u64 start, u64 end ) {

    if ( hashes && start < end ) {

        u64 pivot_index = ( start + end ) / 2;
        u64 pivot_hash = hashes[ pivot_index ];

        u64 i = start, j = end;

        while ( 1 ) {

            while ( hashes[ i ] < pivot_hash ) {
                i++;
            }

            while ( hashes[ j ] > pivot_hash ) {
                j--;
            }

            if ( i < j ) {

                u64 hash_temp = hashes[ i ];
                hashes[ i ] = hashes[ j ];
                hashes[ j ] = hash_temp;

                Code_Index_Note* temp = notes[ i ];
                notes[ i ] = notes[ j ];
                notes[ j ] = temp;

                i++;
                j--;

            } else {

                break;
            }
        }

        primitive_highlight_quick_sort_hashes_notes( hashes, notes, start, j );
        primitive_highlight_quick_sort_hashes_notes( hashes, notes, j + 1, end );
    }
}

typedef struct primitive_highlight_hashes_notes_t {
    u64* hashes;
    Code_Index_Note** notes;
    i32 count;
} primitive_highlight_hashes_notes_t;

function primitive_highlight_hashes_notes_t primitive_highlight_create_big_note_array( Application_Links* app, Arena* arena ) {

    ProfileScope( app, "create_big_note_array" );

    primitive_highlight_hashes_notes_t hashes_notes = { 0 };

    Buffer_ID buffer_it = get_buffer_next( app, 0, Access_Always );

#if 0
    /* NOTE: locking the index only in this function seems to cost more than locking in the primitive_highlight_draw_cpp_token_colors function. */
    code_index_lock( );
#endif

    while ( buffer_it ) {

        Code_Index_File *file = code_index_get_file( buffer_it );

        if ( file ) {
            hashes_notes.count += file->note_array.count;
        }

        buffer_it = get_buffer_next( app, buffer_it, Access_Always );
    }

    hashes_notes.hashes = push_array( arena, u64, hashes_notes.count );
    hashes_notes.notes = push_array( arena, Code_Index_Note*, hashes_notes.count );

    i32 count = 0;

    {
        ProfileScope( app, "create hashes" );

        buffer_it = get_buffer_next( app, 0, Access_Always );

        while ( buffer_it ) {

            Code_Index_File *file = code_index_get_file( buffer_it );

            if ( file ) {

                for ( i32 i = 0; i < file->note_array.count; i++ ) {
                    hashes_notes.notes[ count ] = file->note_array.ptrs[ i ];
                    hashes_notes.hashes[ count ] = table_hash_u8( hashes_notes.notes[ count ]->text.str, hashes_notes.notes[ count ]->text.size );
                    count++;
                }
            }

            buffer_it = get_buffer_next( app, buffer_it, Access_Always );
        }
    }

#if 0
    code_index_unlock( );
#endif

    if ( count ) {
        ProfileScope( app, "quick_sort_hashes_notes" );
        primitive_highlight_quick_sort_hashes_notes( hashes_notes.hashes, hashes_notes.notes, 0, count - 1 );
    }

    return hashes_notes;
}

function Code_Index_Note* primitive_highlight_get_note( Application_Links* app, primitive_highlight_hashes_notes_t* hashes_notes, String_Const_u8 name ) {

    u8 type_weight[ 4 ];
    type_weight[ CodeIndexNote_4coderCommand ] = 1;
    type_weight[ CodeIndexNote_Function ] = 2;
    type_weight[ CodeIndexNote_Type ] = 3;
    type_weight[ CodeIndexNote_Macro ] = 4;

    ProfileScope( app, "get_note" );

    Code_Index_Note* result = 0;

    u64 name_hash = table_hash_u8( name.str, name.size );

    i32 start = 0;
    i32 end = hashes_notes->count - 1;

    while ( start <= end ) {

        i32 midle = ( start + end ) / 2;

        u64 note_hash = hashes_notes->hashes[ midle ];

        if ( name_hash < note_hash ) {
            end = midle - 1;
        } else if ( name_hash > note_hash ) {
            start = midle + 1;
        } else {

            ProfileBlockNamed( app, "solve collisions", solve_collisions );

            while ( midle - 1 >= start && hashes_notes->hashes[ midle - 1 ] == name_hash ) {
                midle--;
            }

            u8 current_weight = 0;

            while ( midle <= end && hashes_notes->hashes[ midle ] == name_hash ) {

                Code_Index_Note* note = hashes_notes->notes[ midle ];
                // Assert( note->kind < ArrayCount( type_weight ) );

                if ( type_weight[ note->note_kind ] > current_weight ) {

                    if ( string_compare( name, note->text ) == 0 ) {

                        current_weight = type_weight[ note->note_kind ];
                        result = note;

                        if ( current_weight == 4 ) {
                            break;
                        }
                    }
                }

                midle++;
            }

            ProfileCloseNow( solve_collisions );

            break;
        }
    }

    return result;
}

/* NOTE: This funciton is a modification of 'draw_cpp_token_colors' from '4coder_draw.cpp'. */
function void primitive_highlight_draw_cpp_token_colors( Application_Links *app, Text_Layout_ID text_layout_id, Token_Array *array, Buffer_ID buffer ) {

    Range_i64 visible_range = text_layout_get_visible_range( app, text_layout_id );
    i64 first_index = token_index_from_pos( array, visible_range.first );
    Token_Iterator_Array it = token_iterator_index( 0, array, first_index );

    /* NOTE: Start modification. */
    Scratch_Block scratch( app );

#if 1
    code_index_lock( );
#endif

    Temp_Memory notes_temp = begin_temp( scratch );
    primitive_highlight_hashes_notes_t hashes_notes = primitive_highlight_create_big_note_array( app, scratch );

    ProfileBlockNamed( app, "token loop", token_loop );
    /* NOTE: End modification. */

    for ( ; ; ) {

        Token *token = token_it_read( &it );

        if ( token->pos >= visible_range.one_past_last ){
            break;
        }

        /* NOTE: Start modification. */
        FColor color = fcolor_id( defcolor_text_default );
        b32 colored = false;

        if ( token->kind == TokenBaseKind_Identifier ) {

            Temp_Memory temp = begin_temp( scratch );
            String_Const_u8 lexeme = push_token_lexeme( app, scratch, buffer, token );
            Code_Index_Note* note = primitive_highlight_get_note( app, &hashes_notes, lexeme );
            end_temp( temp );

            if ( note ) {

                switch ( note->note_kind ) {
                    case CodeIndexNote_Type: {
                        color = fcolor_id( primitive_highlight_type );
                    } break;

                    case CodeIndexNote_Function: {
                        color = fcolor_id( primitive_highlight_function );
                    } break;

                    case CodeIndexNote_Macro: {
                        color = fcolor_id( primitive_highlight_macro );
                    } break;

                    case CodeIndexNote_4coderCommand: {
                        /* NOTE: 4coder doesn't create those notes as of 4.1.6. */
                        color = fcolor_id( primitive_highlight_4coder_command );
                    } break;
                }

                colored = true;

#if 1
                if ( note->note_kind == CodeIndexNote_Type ) {

                    Token_Iterator_Array dot_arrow_it = it;

                    if ( token_it_dec_non_whitespace( &dot_arrow_it ) ) {

                        Token* dot_arrow = token_it_read( &dot_arrow_it );

                        if ( dot_arrow->kind == TokenBaseKind_Operator && ( dot_arrow->sub_kind == TokenCppKind_Dot || dot_arrow->sub_kind == TokenCppKind_Arrow ) ) {
                            colored = false;
                        }
                    }
                }
#endif
            }
        }

        if( !colored ) {
            color = get_token_color_cpp( *token );
        }
        /* NOTE: End modification. */

        ARGB_Color argb = fcolor_resolve( color );
        paint_text_color( app, text_layout_id, Ii64_size( token->pos, token->size ), argb );

        if ( !token_it_inc_all( &it ) ){
            break;
        }
    }

#if 1
    code_index_unlock( );
#endif

    /* NOTE: Start modification. */
    ProfileCloseNow( token_loop );
    end_temp( notes_temp );
    /* NOTE: End modification. */
}

// NOTE(allen): Users can declare their own managed IDs here.

#if !defined(META_PASS)
#include "generated/managed_id_metadata.cpp"
#endif

static String_ID edit_map_id;
static String_ID normal_map_id;
static bool IsBuildPanelOpen = false;

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
    active_color_table.arrays[defcolor_cursor].vals[0] = 0xffff5533;
}


CUSTOM_COMMAND_SIG(enter_normal_mode)
{
    set_current_mapid(app, normal_map_id);
    active_color_table.arrays[defcolor_cursor].vals[0] = 0xff80ff80;
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

CUSTOM_COMMAND_SIG(new_line_and_edit_mode)
{
    
    seek_end_of_line(app);
    enter_edit_mode(app);
    write_text(app, string_u8_litexpr("\n"));
}

CUSTOM_COMMAND_SIG(delete_range_and_edit)
{
    delete_range(app);
    enter_edit_mode(app);
}

CUSTOM_COMMAND_SIG(kill_to_end_of_line)
{
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
    buffer_replace_range(app, buffer, range, string_u8_empty);
    enter_edit_mode(app);
}

inline bool IsAlpha(char c)
{
    bool result = (((c >= 'a') && (c <= 'z')) ||
                   ((c >= 'A') && (c <= 'Z')));
    
    return(result);
}

inline bool IsDigit(char c)
{
    return (c >= '0' && c <= '9');
}

inline char GetChar(Application_Links* app, i64 offset = 0)
{
    View_ID view = get_active_view(app, 0);
    Buffer_ID buffer = view_get_buffer(app, view, 0);

    i64 pos = view_get_cursor_pos(app, view);
    char c = buffer_get_char(app, buffer, pos + offset);
    return c;
}

inline bool IsOther(char c)
{
    return ((c >= 58 && c <= 63) ||
            (c >= 33 && c <= 47) ||
            (c >= 91 && c <= 96) ||
            (c >= 123 && c <= 126));
}

bool IsWhitespace(char c)
{
    return (c == ' ' || c == '\n');
}

typedef void MoveCursorFunc(Application_Links* app);

void SkipWhitespace(Application_Links* app, MoveCursorFunc* move_cursor)
{
    char c = GetChar(app);
    while (c == ' ' || c == '\n')
    {
        move_cursor(app);
        c = GetChar(app);
    }
}

CUSTOM_COMMAND_SIG(cursor_move_left)
{
    u8 c = GetChar(app);

    if (IsAlpha(c) || c == '_' || IsDigit(c))
    {
        while(IsAlpha(c) || c == '_' || IsDigit(c))
        {
            move_left(app);
            c = GetChar(app, -1);
        }
    }
    else if (IsOther(c))
    {
        move_left(app);
        c = GetChar(app);

        while (IsAlpha(c) || c == '_' || IsDigit(c))
        {
            move_left(app);
            c = GetChar(app, -1);
        }
    }
    else if (IsWhitespace(c))
    {
        SkipWhitespace(app, move_left);
    }

}



CUSTOM_COMMAND_SIG(cursor_move_right)
{
    u8 c = GetChar(app);

    if (IsAlpha(c) || c == '_' || IsDigit(c))
    {
        while(IsAlpha(c) || c == '_' || IsDigit(c))
        {
            move_right(app);
            c = GetChar(app);
        }
    }
    else if (IsOther(c))
    {
        while (IsOther(c))
        {
            move_right(app);
            c = GetChar(app);
        }
    }

    if (IsWhitespace(c))
    {
        SkipWhitespace(app, move_right);
    }

}


CUSTOM_COMMAND_SIG(backspace_delete_whole_word)
{
    snipe_backward_whitespace_or_token_boundary(app);
    enter_edit_mode(app);
}



CUSTOM_COMMAND_SIG(build_project)
{
    build_in_build_panel(app);
    IsBuildPanelOpen = true;
}

CUSTOM_COMMAND_SIG(toggle_build_panel)
{
    if (IsBuildPanelOpen)
    {
        close_build_panel(app);
    }
    else
    {
        change_to_build_panel(app);
    }
    IsBuildPanelOpen = !IsBuildPanelOpen;
}

CUSTOM_COMMAND_SIG(list_current_buffer_functions_other_panel)
{
    View_ID view = get_active_view(app, Access_ReadVisible);
    Buffer_ID buffer = view_get_buffer(app, view, Access_ReadVisible);

    change_to_build_panel(app);
    IsBuildPanelOpen = true;

    list_all_functions(app, buffer);
}

CUSTOM_COMMAND_SIG(list_all_buffers_functions_other_panel)
{
    change_to_build_panel(app);
    IsBuildPanelOpen = true;

    list_all_functions_all_buffers(app);
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
    Bind(interactive_switch_buffer,            KeyCode_B, KeyCode_Alt);
    Bind(quick_swap_buffer,                    KeyCode_B, KeyCode_Control);
    Bind(move_up,                              KeyCode_Up);
    Bind(move_down,                            KeyCode_Down);
    Bind(move_left,                            KeyCode_Left);
    Bind(move_right,                           KeyCode_Right);
    Bind(goto_next_jump,                       KeyCode_E, KeyCode_Alt);
    Bind(goto_prev_jump,                       KeyCode_Q, KeyCode_Alt);
    
    SelectMap(file_id);
    ParentMap(global_id);
    
    // Essential
    BindMouse(click_set_cursor_and_mark,        MouseCode_Left);
    BindMouseRelease(click_set_cursor,          MouseCode_Left);
    BindCore(click_set_cursor_and_mark,         CoreCode_ClickActivateView);
    BindMouseMove(click_set_cursor_if_lbutton);
    ////////////

    
    //
    SelectMap(normal_id);
    ParentMap(file_id);
    
    Bind(move_up,                              KeyCode_K);
    Bind(move_down,                            KeyCode_J);
    Bind(move_left,                            KeyCode_H);
    Bind(move_right,                           KeyCode_L);
    Bind(cursor_move_left,                     KeyCode_B);
    Bind(cursor_move_right,                    KeyCode_W);
    Bind(move_up_to_blank_line_end,            KeyCode_K, KeyCode_Alt);
    Bind(move_down_to_blank_line_end,          KeyCode_J, KeyCode_Alt);
    Bind(move_line_up,                         KeyCode_K, KeyCode_Control);
    Bind(move_line_down,                       KeyCode_J, KeyCode_Control);
    Bind(goto_line,                            KeyCode_G, KeyCode_Alt);
    Bind(enter_edit_mode,                      KeyCode_I);
    Bind(backspace_alpha_numeric_boundary,     KeyCode_Backspace, KeyCode_Control);
    Bind(search,                               KeyCode_ForwardSlash);
    Bind(copy,                                 KeyCode_Y);
    Bind(cut,                                  KeyCode_X, KeyCode_Control);
    Bind(paste_and_indent,                     KeyCode_P);
    Bind(redo,                                 KeyCode_R, KeyCode_Control);
    Bind(undo,                                 KeyCode_U);
    Bind(goto_beginning_of_file,               KeyCode_Home);
    Bind(goto_end_of_file,                     KeyCode_End);
    Bind(go_beginning_of_line_and_edit_mode,   KeyCode_I, KeyCode_Shift);
    Bind(go_end_of_line_and_edit_mode,         KeyCode_A, KeyCode_Shift);
    Bind(kill_buffer,                          KeyCode_K, KeyCode_Control, KeyCode_Shift);
    Bind(comment_line_toggle,                  KeyCode_C, KeyCode_Alt);
    Bind(word_complete,                        KeyCode_Tab);
    Bind(set_mark,                             KeyCode_S);
    Bind(save,                                 KeyCode_S, KeyCode_Alt);
    Bind(backspace_delete_whole_word,          KeyCode_Backspace, KeyCode_Alt);
    Bind(delete_line,                          KeyCode_D, KeyCode_Alt);
    Bind(delete_char,                          KeyCode_X);
    Bind(new_line_and_edit_mode,               KeyCode_O);
    Bind(cursor_mark_swap,                     KeyCode_A, KeyCode_Alt);
    Bind(delete_range_and_edit,                KeyCode_C, KeyCode_Control);
    Bind(kill_to_end_of_line,                  KeyCode_C, KeyCode_Shift);
    Bind(replace_in_range,                     KeyCode_R, KeyCode_Alt);
    Bind(replace_in_buffer,                    KeyCode_R, KeyCode_Alt, KeyCode_Control);
    Bind(duplicate_line,                       KeyCode_J, KeyCode_Alt, KeyCode_Control);
    Bind(write_todo,                           KeyCode_T, KeyCode_Control);
    Bind(toggle_build_panel,                   KeyCode_Tick);
    Bind(build_project,                        KeyCode_M, KeyCode_Alt);
    Bind(list_current_buffer_functions_other_panel, KeyCode_P, KeyCode_Alt);
    Bind(list_all_buffers_functions_other_panel,    KeyCode_P, KeyCode_Alt, KeyCode_Control);
    
    //
    SelectMap(edit_id);
    ParentMap(file_id);
    
    Bind(enter_normal_mode,                    KeyCode_CapsLock);
    Bind(enter_normal_mode,                    KeyCode_Escape);
    Bind(backspace_alpha_numeric_boundary,     KeyCode_Backspace, KeyCode_Control);
    Bind(backspace_delete_whole_word,          KeyCode_Backspace, KeyCode_Alt);
    Bind(move_line_up,                         KeyCode_K, KeyCode_Control);
    Bind(move_line_down,                       KeyCode_J, KeyCode_Control);
    Bind(goto_beginning_of_file,               KeyCode_Home);
    Bind(goto_end_of_file,                     KeyCode_End);
    Bind(word_complete,                        KeyCode_Tab);
    Bind(move_left_whitespace_boundary,        KeyCode_Left, KeyCode_Control);
    Bind(move_right_whitespace_boundary,       KeyCode_Right, KeyCode_Control);
    Bind(move_up_to_blank_line_end,            KeyCode_K, KeyCode_Alt);
    Bind(move_down_to_blank_line_end,          KeyCode_J, KeyCode_Alt);
    Bind(backspace_char,                       KeyCode_Backspace);
    
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
                if (string_match(ext, string_u8_litexpr("cs")) ||
                    string_match(ext, string_u8_litexpr("mdesk"))) 
                {
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

