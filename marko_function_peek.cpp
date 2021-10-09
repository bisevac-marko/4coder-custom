global b32 global_peek_open;

struct FunctionDefinition
{
    FunctionDefinition* next;
    String_u8 string;
    u32* argument_idxs;
    u8 argument_count;
    u8 current_argument_idx;
    u32 arg_start_idx;
};

struct FunctionPeek
{
    Arena arena;
    FunctionDefinition* function_definitions;
    String_u8 current_function_name;
    i64 prev_cursor_line;
};

internal f32
MB_GetStringGlyphAdvance(Face_Advance_Map advance_map, Face_Metrics metrics, String_Const_u8 string)
{
    f32 result = 0.0f;
    for (i32 i = 0; i < string.size; ++i)
        result += font_get_glyph_advance(&advance_map, &metrics, string.str[i], 4);
    return result;
}

internal void
MB_RenderFunctionPeek(Application_Links* app, View_ID view_id, Text_Layout_ID layout, FunctionPeek* peek)
{
    i64 pos = view_get_cursor_pos(app, view_id);
    Face_ID face_id = get_face_id(app, 0);
    Face_Metrics face_metrics = get_face_metrics(app, face_id);
    Face_Advance_Map face_advance_map = get_face_advance_map(app, face_id);
    Buffer_Cursor cursor = view_compute_cursor(app, view_id, seek_pos(pos));
    FunctionDefinition* definition = peek->function_definitions;
    Rect_f32 rect = text_layout_character_on_screen(app, layout, pos);
    rect.y0 += face_metrics.line_height;
    Vec2_f32 text_pos = V2f32(rect.x0, rect.y0);
    
    rect.y1 = rect.y0 + 20.0f;
    rect.x1 = rect.x0 + MB_GetStringGlyphAdvance(face_advance_map, face_metrics, SCu8(definition->string));
    draw_rectangle_fcolor(app, rect, 0, fcolor_id(defcolor_highlight_white));
    draw_rectangle_outline_fcolor(app, rect, 0, 2.0f, fcolor_id(defcolor_text_default));
    draw_string(app, face_id, 
                string_substring(SCu8(definition->string), Ii64(0, definition->arg_start_idx)), 
                text_pos, fcolor_id(defcolor_text_default));
    
    
    i32 start_index = 0;
    i32 end_index = definition->arg_start_idx;
    for (u8 argument_idx = 0; argument_idx < definition->argument_count; ++argument_idx)
    {
        i32 next_end_index = definition->argument_idxs[argument_idx]; 
        text_pos.x += MB_GetStringGlyphAdvance(face_advance_map, face_metrics, string_substring(SCu8(definition->string), Ii64(start_index, end_index)));
        String_Const_u8 substring = string_substring(SCu8(definition->string), Ii64(end_index, next_end_index));
        
        if (definition->current_argument_idx == argument_idx)
            draw_string(app, face_id, substring, text_pos, fcolor_id(defcolor_cursor));
        else
            draw_string(app, face_id, substring, text_pos, fcolor_id(defcolor_text_default));
        start_index = end_index;
        end_index = next_end_index;
    }
}

internal b32
MB_GetFunctionDefinitionMatch(Application_Links* app, String_Const_u8 match, FunctionPeek* peek)
{
    View_ID view = get_active_view(app, Access_Visible);
    b32 success = false;
    
    if (view != 0){
        Scratch_Block scratch(app);
        code_index_lock();
        for (Buffer_ID buffer = get_buffer_next(app, 0, Access_Always);
             buffer != 0 && !success;
             buffer = get_buffer_next(app, buffer, Access_Always)){
            Code_Index_File *file = code_index_get_file(buffer);
            if (file != 0){
                for (i32 i = 0; i < file->note_array.count; i += 1){
                    Code_Index_Note *note = file->note_array.ptrs[i];
                    if (note->note_kind == CodeIndexNote_Function && 
                        string_match(note->text, match))
                    {
                        //point_stack_push_view_cursor(app, view);
                        //jump_to_location(app, view, buffer, note->pos.first);
                        FunctionDefinition* definition = push_array_zero(&peek->arena, FunctionDefinition, 1);
                        definition->string = push_string_u8(&peek->arena, 256);
                        sll_stack_push(peek->function_definitions, definition);
                        definition->argument_idxs = push_array(&peek->arena, u32, 20);
                        i64 max_tries = note->pos.first + 512;
                        for (i64 pos = note->pos.first; pos < max_tries; ++pos)
                        {
                            char next_c = buffer_get_char(app, buffer, pos);
                            string_append_character(&definition->string, next_c);
                            if (next_c == ')')
                            {
                                break;
                            }
                            else if (next_c == '(')
                            {
                                definition->arg_start_idx = (u32)definition->string.size;
                            }
                            else if (next_c == ',')
                            {
                                u32* idx = definition->argument_idxs + definition->argument_count;
                                *idx = (u32)definition->string.size;
                                definition->argument_count++;
                            }
                        }
                        if (definition->argument_count)
                        {
                            // num arguments = (num of ',') + 1
                            u32* idx = definition->argument_idxs + definition->argument_count;
                            *idx = (u32)definition->string.size;
                            definition->argument_count++;
                        }
                        success = true;
                    }
                }
                if (success)
                    break;
            }
        }
        code_index_unlock();
    }
    return success;
}


internal void
MB_EndFunctionPeek(FunctionPeek* peek)
{
    global_peek_open = false;
    pop_array(&peek->arena, u8, peek->current_function_name.cap);
    for (FunctionDefinition* definition = peek->function_definitions;
         definition;
         definition = definition->next)
    {
        pop_array(&peek->arena, u8, definition->string.cap);
        pop_array(&peek->arena, FunctionDefinition, 1);
    }
    peek->function_definitions = 0;
    peek->current_function_name = {};
}

internal void
MB_StartFunctionPeek(Application_Links*app, View_ID view, FunctionPeek* peek)
{
    
    i64 pos = view_get_cursor_pos(app, view);
    Buffer_Cursor cursor = view_compute_cursor(app, view, seek_pos(pos));
    peek->prev_cursor_line = cursor.line;
    //peek->current_function_name = string_u8_push(&peek->arena, function_name.size);
    //string_append(&peek->current_function_name, function_name);
    global_peek_open = true;
    //print_message(app, string_u8_litexpr("Function definition:"));
    //print_message(app, SCu8(peek->function_definitions->string));
}

internal void
MB_FunctionPeekOnInput(Application_Links* app, View_ID view, User_Input* input, FunctionPeek* peek)
{
    Scratch_Block scratch(app);
    Buffer_ID buffer = view_get_buffer(app, view, 0);
    if (global_peek_open)
    {
        i64 pos = view_get_cursor_pos(app, view);
        Buffer_Cursor cursor = view_compute_cursor(app, view, seek_pos(pos));
        if (cursor.line != peek->prev_cursor_line)
        {
            MB_EndFunctionPeek(peek);
        }
    }
    if (input->event.core.code == KeyCode_Backspace)
    {
        i64 pos = view_get_cursor_pos(app, view);
        char c = buffer_get_char(app, buffer, pos-1);
        if (c == ')')
        {
            i32 max_tries = 256;
            for (i32 i = 0; i < max_tries; ++i)
            {
                c = buffer_get_char(app, buffer, pos-i);
                if (c == '(')
                {
                    pos = pos - i - 1; // this is pos before '('
                    String_Const_u8 function_name = push_token_or_word_under_pos(app, scratch, buffer, pos - 1);
                    if (MB_GetFunctionDefinitionMatch(app, function_name, peek))
                    {
                        peek->function_definitions->current_argument_idx = peek->function_definitions->argument_count - 1;
                        MB_StartFunctionPeek(app, view, peek);
                    }
                    break;
                }
            }
        }
        else if (c == '(')
        {
            if (global_peek_open)
                MB_EndFunctionPeek(peek);
        }
        else if (c == ',')
        {
            peek->function_definitions->current_argument_idx--;
        }
    }
    else if (input->event.kind == InputEventKind_TextInsert)
    {
        String_Const_u8 insert = to_writable(input);
        if ((string_match(insert, string_u8_litexpr("(")) && !global_peek_open))
        {
            i64 pos = view_get_cursor_pos(app, view);
            String_Const_u8 function_name = push_token_or_word_under_pos(app, scratch, buffer, pos - 1);
            if (MB_GetFunctionDefinitionMatch(app, function_name, peek))
            {
                MB_StartFunctionPeek(app, view, peek);
            }
        }
        else if (string_match(insert, string_u8_litexpr(")")) && global_peek_open)
        {
            MB_EndFunctionPeek(peek);
        }
        else if (string_match(insert, string_u8_litexpr(",")) && global_peek_open)
        {
            peek->function_definitions->current_argument_idx++;
        }
    }
}

/*
internal b32
MB_GetFunctionDefinitionIfMatch(Application_Links* app, Buffer_ID buffer, Function_Positions *positions_array, i64 positions_count,  String_Const_u8 function_name, String_u8* out)
{
    b32 found = false;
    Scratch_Block scratch(app);
    
    for (i32 i = 0; i < positions_count; ++i)
    {
        Function_Positions *positions = &positions_array[i];
        
        i64 start_index = positions->sig_start_index;
        i64 end_index = positions->sig_end_index;
        //i64 open_paren_pos = positions->open_paren_pos;
        //i64 line_number = get_line_number_from_pos(app, buffer, open_paren_pos);
        
        Assert(end_index > start_index);
        Token_Array array = get_token_array_from_buffer(app, buffer);
        
        if (array.tokens != 0)
        {
            Token prev_token = {};
            Token_Iterator_Array it = token_iterator_index(buffer, &array, start_index);
            for (;;)
            {
                Token *token = token_it_read(&it);
                if (!HasFlag(token->flags, TokenBaseFlag_PreprocessorBody) &&
                    token->kind != TokenBaseKind_Comment &&
                    token->kind != TokenBaseKind_Whitespace)
                {
                    
                    if ((prev_token.sub_kind == TokenCppKind_Identifier ||
                         prev_token.sub_kind == TokenCppKind_Star ||
                         prev_token.sub_kind == TokenCppKind_Comma ||
                         prev_token.kind == TokenBaseKind_Keyword) &&
                        !(token->sub_kind == TokenCppKind_ParenOp ||
                          token->sub_kind == TokenCppKind_ParenCl ||
                          token->sub_kind == TokenCppKind_Comma))
                    {
                        string_append_character(out, ' ');
                    }
                    Temp_Memory token_temp = begin_temp(scratch);
                    String_Const_u8 token_str = push_token_lexeme(app, scratch, buffer, token);
                    if (!found)
                    {
                        if (string_match(function_name, token_str))
                        {
                            string_append(out, token_str);
                            found = true;
                        }
                    }
                    else
                    {
                        string_append(out, token_str);
                        if (token->kind == TokenBaseKind_ParentheticalClose)
                        {
                            goto exit_search;
                        }
                    }
                    end_temp(token_temp);
                    
                    prev_token = *token;
                }
                if (!token_it_inc(&it)){
                    break;
                }
                i64 index = token_it_index(&it);
                if (index > end_index){
                    break;
                }
            }
        }
    }
    exit_search:
    return found;
}

internal b32
MB_TryGetFunctionDefinitionMatch(Application_Links* app, FunctionPeek* peek, String_Const_u8 function_name)
{
    b32 found = false;
    Scratch_Block scratch(app);
    
    i32 positions_max = KB(4)/sizeof(Function_Positions);
    Function_Positions *positions_array = push_array(scratch, Function_Positions, positions_max);
    
    for (Buffer_ID buffer_it = get_buffer_next(app, 0, Access_Always);
         buffer_it != 0;
         buffer_it = get_buffer_next(app, buffer_it, Access_Always))
    {
        Token_Array array = get_token_array_from_buffer(app, buffer_it);
        if (array.tokens != 0)
        {
            i64 token_index = 0;
            b32 still_looping = false;
            Token prev_token = {};
            do
            {
                Get_Positions_Results get_positions_results = get_function_positions(app, buffer_it, token_index, positions_array, positions_max);
                
                i64 positions_count = get_positions_results.positions_count;
                token_index = get_positions_results.next_token_index;
                still_looping = get_positions_results.still_looping;
                
                if (MB_GetFunctionDefinitionIfMatch(app, buffer_it, positions_array, positions_count,  function_name, &peek->functions.definition_string))
                {
                    found = true;
                    break;
                }
                
            }while(still_looping);
        }
    }
    return found;
}
*/
