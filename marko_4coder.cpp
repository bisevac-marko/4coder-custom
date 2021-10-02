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
static struct EditorState* global_editor;

#include "marko_cpp_highlight.cpp"
#include "marko_modes.cpp"
#include "marko_cursor.cpp"


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

struct TrieNode
{
    TrieNode* children[KeyCode_COUNT];
    b32 is_command;
};

const u32 MAX_KEY_SEQUENCE = 3;

struct KeyWithModifier
{
    Key_Code key;
    Key_Code modifiers[Input_MaxModifierCount];
    u8 mod_count;
};

struct KeySequence
{
    union
    {
        struct 
        {
            KeyWithModifier k1;
            KeyWithModifier k2;
            KeyWithModifier k3;
        };
        
        KeyWithModifier keys[3];
    };
};

struct EditorState
{
    Arena arena;
    TrieNode* root[EditorModeType_Count];
    EditorModeType current_mode;
    Table_Data_u64 command_map[EditorModeType_Count];
    u32 key_sequence_level;
    KeySequence key_sequence;
};


function void
MarkoInitEditor(Application_Links* app)
{
    Base_Allocator* allocator = get_base_allocator_system();
    global_editor = base_array(allocator, EditorState, 1);
    global_editor->arena = make_arena(allocator, MB(8));
    for (u8 mode = 0; mode < EditorModeType_Count; ++mode)
    {
		global_editor->command_map[mode] = make_table_Data_u64(allocator, 1024);
		global_editor->root[mode] = push_array_zero(&global_editor->arena, TrieNode, 1);
    }
}

function void 
SwapKeyCode(Key_Code* a, Key_Code* b)
{
    Key_Code t = *a;
    *a = *b;
    *b = t;
}

function i32 
Partition(Key_Code arr[], i32 low, i32 high)
{
    Key_Code pivot = arr[high];
    i32 i = (low - 1);
    
    for (i32 j = low; j <= high - 1; j++)
    {
        if (arr[j] > pivot)
        {
            i++;
            SwapKeyCode(&arr[i], &arr[j]);
        }
    }
    SwapKeyCode(&arr[i + 1], &arr[high]);
    return (i + 1);
}

function void 
QuickSortModifiers(Key_Code* mods, i32 low, i32 high)
{
    if (low < high)
    {
        i32 pi = Partition(mods, low, high);
        QuickSortModifiers(mods, low, pi - 1);
        QuickSortModifiers(mods, pi + 1, high);
    }
}

function void 
SortKeyModifiers(KeyWithModifier* key_mod)
{
    QuickSortModifiers(key_mod->modifiers, 0, key_mod->mod_count);
}

function b32 
IsKeyCodeModifier(Key_Code key)
{
    return ((key == KeyCode_Control) ||
            (key == KeyCode_Alt) ||
		    (key == KeyCode_Shift) ||
            (key == KeyCode_Command));
}

function void
TrieAddKeySequence(Arena* arena, TrieNode* root, KeySequence key_sequence)
{
    TrieNode* node = root;
    for (u32 i = 0; i < MAX_KEY_SEQUENCE; ++i)
    {
        KeyWithModifier key_mod = key_sequence.keys[i];
        if (key_mod.mod_count)
        {
            for (u8 m = 0; m < key_mod.mod_count; ++m)
            {
                Assert(IsKeyCodeModifier(key_mod.modifiers[m]));
                Key_Code mod = key_mod.modifiers[m];
                node->children[mod] = push_array_zero(arena, TrieNode, 1);
                node = node->children[mod];
            }
        }
        Key_Code key = key_mod.key;
        if (!node->children[key])
        {
            node->children[key] = push_array_zero(arena, TrieNode, 1);
        }
        if (i == (MAX_KEY_SEQUENCE - 1) || key_sequence.keys[i + 1].key == 0)
        {
            node->children[key]->is_command = true;
            break;
        }
        node = node->children[key];
    }
}

function b32
TrieTryGetNode(EditorState* editor, TrieNode** out, KeyWithModifier key_mod)
{
    TrieNode* node = editor->root[editor->current_mode];
    for (u32 i = 0; i < editor->key_sequence_level; ++i)
    {
        KeyWithModifier next_key = editor->key_sequence.keys[i];
		for (u8 m = 0; m < next_key.mod_count; ++m)
		{
			Key_Code mod = next_key.modifiers[m];
			node = node->children[mod];
		}
        node = node->children[editor->key_sequence.keys[i].key];
    }
    // Skip modifiers
	for (u8 m = 0; m < key_mod.mod_count; ++m)
	{
		Key_Code mod = key_mod.modifiers[m];
        if (!node->children[mod])
        {
            return false;
        }
		node = node->children[mod];
	}
    if (node->children[key_mod.key])
    {
		*out = node->children[key_mod.key];
        return true;
    }
    return false;
}

function KeyWithModifier 
CreateKeyWithModifier(Key_Code key, ...)
{
    KeyWithModifier result = {};
    result.key = key;
    va_list vl;
    va_start(vl, key);
    Key_Code* mods = result.modifiers;
    for (;;)
    {
        Key_Code mod = va_arg(vl, Key_Code);
        if (mod == 0)
        {
            break;
        }
        *mods++ = mod;
        result.mod_count++;
    }
    return result;
}

function KeyWithModifier 
CreateKeyWithModifier(KeyWithModifier key_mod, ...)
{
    return key_mod;
}

#define KeyWrap(k, ...) CreateKeyWithModifier(k, __VA_ARGS__, 0)

#define IMap(F, k1, k2, k3) \
_Map(editor, EditorModeType_Insert, BindFWrap_(F), {KeyWrap((k1)), KeyWrap((k2)), KeyWrap((k3))})

#define NMap(F, k1, k2, k3) \
_Map(editor, EditorModeType_Normal, BindFWrap_(F), {KeyWrap((k1)), KeyWrap((k2)), KeyWrap((k3))})

function void
_Map(EditorState* editor, EditorModeType mode, Custom_Command_Function* command, KeySequence sequence)
{
    for (u8 i = 0; i < MAX_KEY_SEQUENCE; ++i)
    {
		SortKeyModifiers(&sequence.keys[i]);
    }
    TrieAddKeySequence(&editor->arena, editor->root[mode], sequence);
    KeySequence* key_sequence = push_array_write(&editor->arena, KeySequence, 1, &sequence);
    String_Const_u8 key = make_data(key_sequence, sizeof(KeySequence));
    table_insert(&editor->command_map[mode], key, (u64)PtrAsInt(command));
}

function String_ID
GetCurrentMapID(Application_Links* app)
{
    View_ID view = get_this_ctx_view(app, Access_Always);
    Command_Map_ID map_id = default_get_map_id(app, view);
    
    return map_id;
}


function KeyWithModifier
InputEventToKeyWithModifier(Input_Event* event)
{
    KeyWithModifier result = {};
    result.key = event->core.code;
    for (u8 i = 0; i < event->key.modifiers.count; ++i)
    {
        if (event->key.modifiers.mods[i] != result.key)
        {
			result.modifiers[i] = event->key.modifiers.mods[i];
            result.mod_count++;
        }
    }
    SortKeyModifiers(&result);
    return result;
}

CUSTOM_COMMAND_SIG(ViewInputHandler)
{
    Scratch_Block scratch(app);
    default_input_handler_init(app, scratch);
    
    View_ID view = get_this_ctx_view(app, Access_Always);
    Managed_Scope scope = view_get_managed_scope(app, view);
    
    for (;;){

        // NOTE(allen): Get input
        User_Input input = get_next_input(app, EventPropertyGroup_Any, 0);
        if (input.abort){
            break;
        }
        ProfileScopeNamed(app, "before view input", view_input_profile);
        // NOTE(allen): Mouse Suppression
        Event_Property event_properties = get_event_properties(&input.event);
        if (suppressing_mouse && (event_properties & EventPropertyGroup_AnyMouseEvent) != 0){
            continue;
        }
        // If input is sequence, wait for sequence
        if (input.event.kind == InputEventKind_KeyStroke && !IsKeyCodeModifier(input.event.core.code))
        {
            KeyWithModifier key_mod = InputEventToKeyWithModifier(&input.event);
			TrieNode* node;
            // TODO:
            EditorState* editor = global_editor;
			if (TrieTryGetNode(editor, &node, key_mod))
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
                    Custom_Command_Function* command = (Custom_Command_Function*)IntAsPtr(val);
					default_pre_command(app, scope);
					ProfileCloseNow(view_input_profile);
					command(app);
					ProfileScope(app, "after view input");
					default_post_command(app, scope);
                    editor->key_sequence_level = 0;
                    editor->key_sequence = {};
					continue;
				}
				
			}
        }
        // NOTE(allen): Get binding
        if (implicit_map_function == 0){
            implicit_map_function = default_implicit_map;
        }
        
        Implicit_Map_Result map_result = implicit_map_function(app, 0, 0, &input.event);
        
        if (map_result.command == 0)
        {
            leave_current_input_unhandled(app);
            
            continue;
        }
        // NOTE(allen): Run the command and pre/post command stuff
        default_pre_command(app, scope);
        ProfileCloseNow(view_input_profile);
        map_result.command(app);
        ProfileScope(app, "after view input");
        default_post_command(app, scope);
    }
}


function void
MarkoSetupDefaultBindings(Mapping* mapping, i64 global_id, i64 normal_id, i64 edit_id)
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
    
    Bind(exit_4coder, KeyCode_W, KeyCode_K);
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
MarkoSetupBindings(EditorState* editor)
{
    // Normal mode mappings
    NMap(move_up, KeyCode_K, 0, 0);
    NMap(move_down,                            KeyCode_J, 0, 0);
    NMap(move_left,                            KeyCode_H, 0, 0);
    NMap(move_right,                           KeyCode_L, 0, 0);
    NMap(CursorMoveLeft,                       KeyCode_B, 0, 0);
    NMap(CursorMoveRight,                      KeyCode_W, 0, 0);
    /*
    NMap(CursorJumpUp,                         KeyWrap(KeyCode_K, KeyCode_Alt));
    NMap(CursorJumpDown, KeyCode_J, KeyCode_Alt);
    NMap(move_line_up,                         KeyWrap(KeyCode_K, KeyCode_Control));
    NMap(move_line_down,                       KeyWrap(KeyCode_J, KeyCode_Control));
    NMap(goto_line,                            KeyWrap(KeyCode_G, KeyCode_Alt));
    NMap(EnterEditMode,                        KeyCode_I);
    NMap(search,                               KeyCode_ForwardSlash);
    NMap(search_identifier,                    KeyWrap(KeyCode_ForwardSlash, KeyCode_Control));
    NMap(copy,                                 KeyCode_Y);
    NMap(cut,                                  KeyWrap(KeyCode_X, KeyCode_Control));
    NMap(paste_and_indent,                     KeyCode_P);
    NMap(redo,                                 KeyWrap(KeyCode_R, KeyCode_Control));
    NMap(undo,                                 KeyCode_U);
    NMap(GoBeginningOfLineAndEditMode,         KeyWrap(KeyCode_I, KeyCode_Shift));
    NMap(GotEndOfLineAndEditMode,              KeyWrap(KeyCode_A, KeyCode_Shift));
    NMap(kill_buffer,                          KeyWrap(KeyCode_K, KeyCode_Control, KeyCode_Shift));
    NMap(comment_line_toggle,                  KeyWrap(KeyCode_C, KeyCode_Alt));
    NMap(set_mark,                             KeyCode_S);
    NMap(save,                                 KeyWrap(KeyCode_S, KeyCode_Alt));
    NMap(BackspaceDeleteWholeWord,             KeyWrap(KeyCode_Backspace, KeyCode_Alt));
    NMap(BackspaceDeleteAndEditMode,           KeyWrap(KeyCode_Backspace, KeyCode_Control));
    NMap(DeleteLine,                           KeyWrap(KeyCode_D, KeyCode_Control));
    NMap(DeleteRange,                          KeyWrap(KeyCode_D, KeyCode_Alt));
    NMap(delete_char,                          KeyCode_X);
    NMap(NewLineAndEditMode,                   KeyCode_O);
    NMap(cursor_mark_swap,                     KeyWrap(KeyCode_A, KeyCode_Alt));
    NMap(KillToEndOfLine,                      KeyWrap(KeyCode_C, KeyCode_Shift));
    NMap(replace_in_range,                     KeyWrap(KeyCode_R, KeyCode_Alt));
    NMap(replace_in_buffer,                    KeyWrap(KeyCode_R, KeyCode_Alt, KeyCode_Control));
    NMap(duplicate_line,                       KeyWrap(KeyCode_J, KeyCode_Alt, KeyCode_Control));
    NMap(ToggleBuildPannel,                    KeyCode_Tick);
    NMap(BuildProject,                         KeyWrap(KeyCode_M, KeyCode_Alt));
    NMap(list_all_functions_all_buffers_lister,     KeyWrap(KeyCode_P, KeyCode_Alt, KeyCode_Control));
    NMap(list_all_functions_current_buffer_lister,  KeyWrap(KeyCode_P, KeyCode_Alt));
    NMap(ReplayKeyboardMacro,                       KeyWrap(KeyCode_Period, KeyCode_Control));
    NMap(keyboard_macro_start_recording,            KeyCode_F1);
    NMap(keyboard_macro_finish_recording,           KeyCode_F2);
    NMap(keyboard_macro_finish_recording,           KeyCode_Escape);
    NMap(LastSearchForward,                         KeyCode_N);
    NMap(LastSearchBackward,                        KeyWrap(KeyCode_N, KeyCode_Shift));
    
    // Insert mode mappings
    IMap(EnterNormalMode,                      KeyCode_CapsLock);
    IMap(EnterNormalMode,                      KeyCode_Escape);
    IMap(backspace_alpha_numeric_boundary,     KeyWrap(KeyCode_Backspace, KeyCode_Control));
    IMap(BackspaceDeleteWholeWord,             KeyWrap(KeyCode_Backspace, KeyCode_Alt));
    IMap(move_line_up,                         KeyWrap(KeyCode_K, KeyCode_Control));
    IMap(move_line_down,                       KeyCode_J, KeyCode_Control);
    IMap(goto_beginning_of_file,               KeyCode_Home);
    IMap(goto_end_of_file,                     KeyCode_End);
    IMap(move_left_whitespace_boundary,        KeyWrap(KeyCode_Left, KeyCode_Control));
    IMap(move_right_whitespace_boundary,       KeyWrap(KeyCode_Right, KeyCode_Control));
    IMap(move_up_to_blank_line_end,            KeyWrap(KeyCode_K, KeyCode_Alt));
    IMap(move_down_to_blank_line_end,          KeyWrap(KeyCode_J, KeyCode_Alt));
    IMap(backspace_char,                       KeyCode_Backspace);
*/
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
    
    EnterNormalMode(app);
    
	/*
    Rect_f32 rect = Rf32(0, 0, 100, 100);
    draw_rectangle_fcolor(app, rect, 0, 
                          
                          // no meaning for return
                          return(0);
						  */
	return 0;
}

#include <time.h>

void
custom_layer_init(Application_Links *app){
    
    Thread_Context *tctx = get_thread_context(app);
    // Base 4coder init
    default_framework_init(app);
    
    MarkoInitEditor(app);
    
    set_custom_hook(app, HookID_BufferViewerUpdate, default_view_adjust);
    
    set_custom_hook(app, HookID_ViewEventHandler, ViewInputHandler);
    set_custom_hook(app, HookID_Tick, default_tick);
    set_custom_hook(app, HookID_RenderCaller, default_render_caller);
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
    set_custom_hook(app, HookID_BeginBuffer, MarkoBeginBuffer);
    
    set_custom_hook(app, HookID_Layout, layout_unwrapped);
    
    mapping_init(tctx, &framework_mapping);
    
    String_ID global_map_id = vars_save_string_lit("keys_global");
    normal_map_id = vars_save_string_lit("keys_file");
    edit_map_id = vars_save_string_lit("keys_edit");
    
    // setup_essential_mapping(&framework_mapping, global_map_id, file_map_id, normal_map_id);
    MarkoSetupDefaultBindings(&framework_mapping, global_map_id, normal_map_id, edit_map_id);
    MarkoSetupBindings(global_editor);
}

#endif //FCODER_DEFAULT_BINDINGS

// BOTTOM

