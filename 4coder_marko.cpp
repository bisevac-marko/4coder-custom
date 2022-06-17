//

String_ID mapid_shared;
String_ID mapid_normal;
String_ID mapid_insert;

void set_current_mapid( Application_Links* app, Command_Map_ID mapid ) {
  
  View_ID view = get_active_view( app, 0 );
  Buffer_ID buffer = view_get_buffer( app, view, 0 );
  Managed_Scope scope = buffer_get_managed_scope( app, buffer );
  Command_Map_ID* map_id_ptr = scope_attachment( app, scope, buffer_map_id, Command_Map_ID );
  *map_id_ptr = mapid;
}
CUSTOM_COMMAND_SIG(go_to_normal_mode) {
  
  set_current_mapid(app, mapid_normal);
  
  active_color_table.arrays[defcolor_cursor].vals[0] = 0xffff5533;
  active_color_table.arrays[defcolor_at_cursor].vals[0] = 0xff00aacc;
  active_color_table.arrays[defcolor_margin_active].vals[0] = 0xffff5533;
}

CUSTOM_COMMAND_SIG(go_to_insert_mode) {
  set_current_mapid(app, mapid_insert);
  active_color_table.arrays[defcolor_cursor].vals[0] = 0xff80ff80;
  active_color_table.arrays[defcolor_at_cursor].vals[0] = 0xff293134;
  active_color_table.arrays[defcolor_margin_active].vals[0] = 0xff80ff80;
  
}

CUSTOM_COMMAND_SIG(mb_page_down_center_view)
{
  page_down(app);
  center_view(app);
}

CUSTOM_COMMAND_SIG(mb_page_up_center_view)
{
  page_up(app);
  center_view(app);
}

CUSTOM_COMMAND_SIG(mb_end_of_line_and_insert_mode)
{
  seek_end_of_textual_line(app);
  go_to_insert_mode(app);
}

CUSTOM_COMMAND_SIG(mb_beginning_of_line_and_insert_mode)
{
  seek_beginning_of_textual_line(app);
  go_to_insert_mode(app);
}

CUSTOM_COMMAND_SIG(mb_cut_till_end_of_line)
{
  set_mark(app);
  seek_end_of_textual_line(app);
  delete_range(app);
  go_to_insert_mode(app);
}

CUSTOM_COMMAND_SIG(mb_insert_new_line_and_insert_mode)
{
  mb_end_of_line_and_insert_mode(app);
  write_text(app, string_u8_litexpr("\n"));
  auto_indent_line_at_cursor(app);
}

CUSTOM_COMMAND_SIG(mb_last_search_forward)
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

CUSTOM_COMMAND_SIG(mb_last_search_backward)
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

CUSTOM_COMMAND_SIG(mb_copy_line)
{
  View_ID view = get_active_view(app, Access_ReadVisible);
  i64 old_pos = view_get_cursor_pos(app, view);
  
  seek_beginning_of_textual_line(app);
  set_mark(app);
  seek_end_of_textual_line(app);
  move_right(app); // newline
  copy(app);
  
  view_set_cursor_and_preferred_x(app, view, seek_pos(old_pos));
}

CUSTOM_COMMAND_SIG(mb_close_brace)
{
  View_ID view = get_active_view(app, Access_ReadVisible);
  Buffer_ID buffer = view_get_buffer(app, view, Access_ReadVisible);
  i64 pos = view_get_cursor_pos(app, view);
  
  u8 c = buffer_get_char(app, buffer, pos);
  
  if (c == ')')
  {
    view_set_cursor_by_character_delta(app, view, 1);
    no_mark_snap_to_cursor_if_shift(app, view);
  }
  else
  {
    write_text(app, string_u8_litexpr(")"));
  }
}

CUSTOM_COMMAND_SIG(mb_open_brace)
{
  View_ID view = get_active_view(app, Access_ReadVisible);
  Buffer_ID buffer = view_get_buffer(app, view, Access_ReadVisible);
  i64 pos = view_get_cursor_pos(app, view);
  
  u8 c = buffer_get_char(app, buffer, pos);
  
  if (c == '(')
  {
    move_left(app);
  }
  else
  {
    write_text(app, string_u8_litexpr("()"));
    move_left(app);
  }
}

CUSTOM_COMMAND_SIG(mb_open_bracket)
{
  View_ID view = get_active_view(app, Access_ReadVisible);
  Buffer_ID buffer = view_get_buffer(app, view, Access_ReadVisible);
  
  i64 cursor_pos = view_get_cursor_pos(app, view);
  i64 line_start = get_start_of_line_at_cursor(app, view, buffer);
  
  
  if (cursor_pos == line_start)
    open_long_braces(app);
  else
  {
    u8 c = buffer_get_char(app, buffer, cursor_pos);
    
    if (c == '{')
    {
      move_left(app);
    }
    else
    {
      write_text(app, string_u8_litexpr("{}"));
      move_left(app);
    }
  }
}

CUSTOM_COMMAND_SIG(mb_close_bracket)
{
  View_ID view = get_active_view(app, Access_ReadVisible);
  Buffer_ID buffer = view_get_buffer(app, view, Access_ReadVisible);
  i64 pos = view_get_cursor_pos(app, view);
  
  u8 c = buffer_get_char(app, buffer, pos);
  
  if (c == '}')
  {
    move_right(app);
  }
  else
  {
    write_text(app, string_u8_litexpr("}"));
  }
  
}

CUSTOM_COMMAND_SIG(mb_cut_line)
{
  seek_beginning_of_textual_line(app);
  set_mark(app);
  seek_end_of_textual_line(app);
  cut(app);
}

CUSTOM_COMMAND_SIG(mb_cut)
{
  cut(app);
  go_to_insert_mode(app);
}

function bool char_is_newline(u8 c)
{
  return (c == '\n');
}

function void
mb_set_bindings(Mapping *mapping)
{
  String_ID global_map_id = vars_save_string_lit("keys_global");
  String_ID file_map_id = vars_save_string_lit("keys_file");
  String_ID code_map_id = vars_save_string_lit("keys_code");
  
  mapid_shared = vars_save_string_lit("mapid_shared");
  mapid_normal = vars_save_string_lit("mapid_normal");
  mapid_insert = vars_save_string_lit("mapid_insert");
  
  MappingScope();
  SelectMapping(mapping);
  
  SelectMap(global_map_id);
  
  // SHARED MODE
  SelectMap(mapid_shared);
  BindCore(mb_startup, CoreCode_Startup);
  BindCore(default_try_exit, CoreCode_TryExit);
  Bind(exit_4coder,          KeyCode_F4, KeyCode_Alt);
  BindMouseWheel(mouse_wheel_scroll);
  BindMouseWheel(mouse_wheel_change_face_size, KeyCode_Control);
  Bind(command_lister, KeyCode_X, KeyCode_Alt);
  Bind(change_active_panel, KeyCode_W, KeyCode_Alt);
  Bind(build_in_build_panel, KeyCode_F5);
  Bind(snipe_backward_whitespace_or_token_boundary, KeyCode_Backspace, KeyCode_Control);
  Bind(backspace_char, KeyCode_Backspace);
  Bind(word_complete, KeyCode_Tab);
  Bind(list_all_locations_of_type_definition, KeyCode_D, KeyCode_Alt);
  Bind(save, KeyCode_S, KeyCode_Control);
  Bind(save_all_dirty_buffers, KeyCode_Control, KeyCode_Shift, KeyCode_S);
  
  // NORMAL MODE
  SelectMap(mapid_normal);
  ParentMap(mapid_shared);
  Bind(go_to_insert_mode, KeyCode_I);
  Bind(copy, KeyCode_Y);
  Bind(mb_copy_line, KeyCode_Y, KeyCode_Shift);
  Bind(mb_cut_line, KeyCode_X, KeyCode_Shift);
  Bind(mb_cut, KeyCode_C);
  Bind(paste_and_indent, KeyCode_P);
  Bind(move_left, KeyCode_H);
  Bind(move_right, KeyCode_L);
  Bind(move_up, KeyCode_K);
  Bind(move_down, KeyCode_J);
  Bind(mb_page_up_center_view, KeyCode_K, KeyCode_Alt);
  Bind(mb_page_down_center_view, KeyCode_J, KeyCode_Alt);
  Bind(cursor_mark_swap, KeyCode_V, KeyCode_Shift);
  Bind(move_right_token_boundary, KeyCode_W);
  Bind(move_left_token_boundary, KeyCode_B);
  Bind(set_mark, KeyCode_V);
  Bind(search_identifier, KeyCode_F, KeyCode_Control);
  Bind(f4_search, KeyCode_ForwardSlash);
  Bind(mb_last_search_backward, KeyCode_N, KeyCode_Shift);
  Bind(mb_last_search_forward, KeyCode_N);
  Bind(undo, KeyCode_U);
  Bind(redo, KeyCode_R, KeyCode_Shift);
  Bind(f4_go_to_definition_same_panel, KeyCode_G, KeyCode_Shift);
  Bind(f4_code_peek, KeyCode_P, KeyCode_Shift);
  Bind(f4_go_to_definition, KeyCode_G, KeyCode_Control);
  Bind(list_all_functions_all_buffers_lister, KeyCode_P, KeyCode_Shift, KeyCode_Alt);
  Bind(comment_line_toggle, KeyCode_T, KeyCode_Shift);
  Bind(replace_in_range, KeyCode_R, KeyCode_Control);
  Bind(list_all_functions_current_buffer_lister, KeyCode_P, KeyCode_Alt);
  Bind(replace_in_all_buffers, KeyCode_R, KeyCode_Control, KeyCode_Shift);
  Bind(delete_char, KeyCode_X);
  Bind(delete_line, KeyCode_D, KeyCode_Shift);
  Bind(delete_range, KeyCode_D, KeyCode_Control);
  Bind(mb_end_of_line_and_insert_mode, KeyCode_A, KeyCode_Shift);
  Bind(mb_beginning_of_line_and_insert_mode, KeyCode_I, KeyCode_Shift);
  Bind(mb_cut_till_end_of_line, KeyCode_C, KeyCode_Shift);
  
  Bind(keyboard_macro_start_recording, KeyCode_F1);
  Bind(keyboard_macro_finish_recording, KeyCode_F2);
  Bind(keyboard_macro_replay, KeyCode_Period);
  Bind(interactive_new, KeyCode_N, KeyCode_Control);
  Bind(interactive_kill_buffer, KeyCode_K, KeyCode_Control);
  Bind(interactive_open, KeyCode_O, KeyCode_Control);
  Bind(kill_buffer, KeyCode_K, KeyCode_Control, KeyCode_Shift);
  Bind(f4_toggle_enclosure_side, KeyCode_5, KeyCode_Shift);
  Bind(mb_insert_new_line_and_insert_mode, KeyCode_O);
  Bind(interactive_switch_buffer, KeyCode_F, KeyCode_Alt);
  Bind(quick_swap_buffer, KeyCode_B, KeyCode_Control);
  Bind(f4_toggle_compilation_expand, KeyCode_Tick);
  Bind(goto_line, KeyCode_Semicolon, KeyCode_Shift);
  Bind(goto_next_jump, KeyCode_E, KeyCode_Alt);
  Bind(goto_prev_jump, KeyCode_Q, KeyCode_Alt);
  Bind(goto_beginning_of_file, KeyCode_Home);
  Bind(goto_end_of_file, KeyCode_End);
  Bind(move_line_down, KeyCode_J, KeyCode_Shift);
  Bind(move_line_up, KeyCode_K, KeyCode_Shift);
  
  // INSERT MODE
  SelectMap(mapid_insert);
  ParentMap(mapid_shared);
  BindTextInput(write_text_and_auto_indent);
  Bind(mb_open_brace, KeyCode_9, KeyCode_Shift);
  Bind(mb_close_brace, KeyCode_0, KeyCode_Shift);
  Bind(mb_close_bracket, KeyCode_RightBracket, KeyCode_Shift);
  Bind(mb_open_bracket, KeyCode_LeftBracket, KeyCode_Shift);
  Bind(go_to_normal_mode, KeyCode_Escape);
  
  /* This is to make sure that the default bindings on the buffers will be mapid_normal. */
  SelectMap(file_map_id);
  ParentMap(mapid_normal);
  
  SelectMap(code_map_id);
  ParentMap(mapid_normal);
}

//~ NOTE(rjf): @f4_startup Whenever 4coder's core is ready for the custom layer to start up,
// this is called.

// TODO(rjf): This is only being used to check if a font file exists because
// there's a bug in try_create_new_face that crashes the program if a font is
// not found. This function is only necessary until that is fixed.
function b32
IsFileReadable(String_Const_u8 path)
{
  b32 result = 0;
  FILE *file = fopen((char *)path.str, "r");
  if(file)
  {
    result = 1;
    fclose(file);
  }
  return result;
}

CUSTOM_COMMAND_SIG(mb_startup)
CUSTOM_DOC("Marko's startup event")
{
  ProfileScope(app, "default startup");
  
  User_Input input = get_current_input(app);
  if(!match_core_code(&input, CoreCode_Startup))
  {
    return;
  }
  
  //~ Default 4coder initialization.
  String_Const_u8_Array file_names = input.event.core.file_names;
  load_themes_default_folder(app);
  default_4coder_initialize(app, file_names);
  
  //~ Open special buffers.
  {
    // Open compilation buffer.
    {
      Buffer_ID buffer = create_buffer(app, string_u8_litexpr("*compilation*"),
                                       BufferCreate_NeverAttachToFile |
                                       BufferCreate_AlwaysNew);
      buffer_set_setting(app, buffer, BufferSetting_Unimportant, true);
      buffer_set_setting(app, buffer, BufferSetting_ReadOnly, true);
    }
    
    // Open lego buffer.
    {
      Buffer_ID buffer = create_buffer(app, string_u8_litexpr("*lego*"),
                                       BufferCreate_NeverAttachToFile |
                                       BufferCreate_AlwaysNew);
      buffer_set_setting(app, buffer, BufferSetting_Unimportant, true);
      buffer_set_setting(app, buffer, BufferSetting_ReadOnly, true);
    }
    
    // Open calc buffer.
    {
      Buffer_ID buffer = create_buffer(app, string_u8_litexpr("*calc*"),
                                       BufferCreate_NeverAttachToFile |
                                       BufferCreate_AlwaysNew);
      buffer_set_setting(app, buffer, BufferSetting_Unimportant, true);
    }
    
    // Open peek buffer.
    {
      Buffer_ID buffer = create_buffer(app, string_u8_litexpr("*peek*"),
                                       BufferCreate_NeverAttachToFile |
                                       BufferCreate_AlwaysNew);
      buffer_set_setting(app, buffer, BufferSetting_Unimportant, true);
    }
    
    // Open LOC buffer.
    {
      Buffer_ID buffer = create_buffer(app, string_u8_litexpr("*loc*"),
                                       BufferCreate_NeverAttachToFile |
                                       BufferCreate_AlwaysNew);
      buffer_set_setting(app, buffer, BufferSetting_Unimportant, true);
    }
  }
  
  //~ Initialize panels
  {
    Buffer_Identifier comp = buffer_identifier(string_u8_litexpr("*compilation*"));
    Buffer_Identifier left  = buffer_identifier(string_u8_litexpr("*calc*"));
    Buffer_Identifier right = buffer_identifier(string_u8_litexpr("*messages*"));
    Buffer_ID comp_id = buffer_identifier_to_id(app, comp);
    Buffer_ID left_id = buffer_identifier_to_id(app, left);
    Buffer_ID right_id = buffer_identifier_to_id(app, right);
    
    // Left Panel
    View_ID view = get_active_view(app, Access_Always);
    new_view_settings(app, view);
    view_set_buffer(app, view, left_id, 0);
    
    // Bottom panel
    View_ID compilation_view = 0;
    {
      compilation_view = open_view(app, view, ViewSplit_Bottom);
      new_view_settings(app, compilation_view);
      Buffer_ID buffer = view_get_buffer(app, compilation_view, Access_Always);
      Face_ID face_id = get_face_id(app, buffer);
      Face_Metrics metrics = get_face_metrics(app, face_id);
      view_set_split_pixel_size(app, compilation_view, (i32)(metrics.line_height*4.f));
      view_set_passive(app, compilation_view, true);
      global_compilation_view = compilation_view;
      view_set_buffer(app, compilation_view, comp_id, 0);
    }
    
    view_set_active(app, view);
    
    // Right Panel
    open_panel_vsplit(app);
    
    View_ID right_view = get_active_view(app, Access_Always);
    view_set_buffer(app, right_view, right_id, 0);
    
    // Restore Active to Left
    view_set_active(app, view);
  }
  
  //~ Auto-Load Project.
  {
    b32 auto_load = def_get_config_b32(vars_save_string_lit("automatically_load_project"));
    if (auto_load)
    {
      load_project(app);
    }
  }
  
  //~ Set misc options.
  {
    global_battery_saver = def_get_config_b32(vars_save_string_lit("f4_battery_saver"));
  }
  
  //~ Initialize audio.
  {
    def_audio_init();
  }
  
  //~ Initialize bindings.
  {
    mb_set_bindings(&framework_mapping);
  }
  
  //~ Initialize stylish fonts.
  {
    Scratch_Block scratch(app);
    String_Const_u8 bin_path = system_get_path(scratch, SystemPath_Binary);
    
    // Fallback font.
    Face_ID face_that_should_totally_be_there = get_face_id(app, 0);
    
    // Title font.
    {
      Face_Description desc = {0};
      {
        desc.font.file_name =  push_u8_stringf(scratch, "%.*sfonts/RobotoCondensed-Regular.ttf", string_expand(bin_path));
        desc.parameters.pt_size = 18;
        desc.parameters.bold = 0;
        desc.parameters.italic = 0;
        desc.parameters.hinting = 0;
      }
      
      if(IsFileReadable(desc.font.file_name))
      {
        global_styled_title_face = try_create_new_face(app, &desc);
      }
      else
      {
        global_styled_title_face = face_that_should_totally_be_there;
      }
    }
    
    // Label font.
    {
      Face_Description desc = {0};
      {
        desc.font.file_name =  push_u8_stringf(scratch, "%.*sfonts/RobotoCondensed-Regular.ttf", string_expand(bin_path));
        desc.parameters.pt_size = 10;
        desc.parameters.bold = 1;
        desc.parameters.italic = 1;
        desc.parameters.hinting = 0;
      }
      
      if(IsFileReadable(desc.font.file_name))
      {
        global_styled_label_face = try_create_new_face(app, &desc);
      }
      else
      {
        global_styled_label_face = face_that_should_totally_be_there;
      }
    }
    
    // Small code font.
    {
      Face_Description normal_code_desc = get_face_description(app, get_face_id(app, 0));
      
      Face_Description desc = {0};
      {
        desc.font.file_name =  push_u8_stringf(scratch, "%.*sfonts/Inconsolata-Regular.ttf", string_expand(bin_path));
        desc.parameters.pt_size = normal_code_desc.parameters.pt_size - 1;
        desc.parameters.bold = 1;
        desc.parameters.italic = 1;
        desc.parameters.hinting = 0;
      }
      
      if(IsFileReadable(desc.font.file_name))
      {
        global_small_code_face = try_create_new_face(app, &desc);
      }
      else
      {
        global_small_code_face = face_that_should_totally_be_there;
      }
    }
  }
  
  {
    def_enable_virtual_whitespace = def_get_config_b32(vars_save_string_lit("enable_virtual_whitespace"));
    clear_all_layouts(app);
  }
  
  // NOTE(marko): Go to normal mode on startup
  go_to_normal_mode(app);
}
