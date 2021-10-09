typedef void MoveCursorFunc(Application_Links* app);

void MB_SkipWhitespace(Application_Links* app, MoveCursorFunc* move_cursor)
{
    
    View_ID view = get_active_view(app, 0);
    Buffer_ID buffer = view_get_buffer(app, view, 0);
    
    i64 pos = view_get_cursor_pos(app, view);
    char c = buffer_get_char(app, buffer, pos);
    
    while (c == ' ' || c == '\n' || c == '\r\n')
    {
        move_cursor(app);
        c = buffer_get_char(app, buffer, pos);
    }
}


CUSTOM_COMMAND_SIG(MB_CursorMoveLeft)
{
    View_ID view = get_active_view(app, 0);
    Buffer_ID buffer = view_get_buffer(app, view, 0);
    i64 pos = view_get_cursor_pos(app, view);
    char c = buffer_get_char(app, buffer, pos);
    
    if (MB_IsAlpha(c) || c == '_' || MB_IsDigit(c))
    {
        while(MB_IsAlpha(c) || c == '_' || MB_IsDigit(c))
        {
            move_left(app);
            pos = view_get_cursor_pos(app, view);
            c = buffer_get_char(app, buffer, pos-1);
        }
    }
    else if (MB_IsOther(c))
    {
        move_left(app);
        pos = view_get_cursor_pos(app, view);
        c = buffer_get_char(app, buffer, pos);
        while (MB_IsAlpha(c) || c == '_' || MB_IsDigit(c))
        {
            move_left(app);
            pos = view_get_cursor_pos(app, view);
            c = buffer_get_char(app, buffer, pos-1);
        }
    }
    
    if (MB_IsWhitespace(c))
    {
        MB_SkipWhitespace(app, move_left);
    }
    
}



CUSTOM_COMMAND_SIG(CursorMoveRight)
{
    View_ID view = get_active_view(app, 0);
    Buffer_ID buffer = view_get_buffer(app, view, 0);
    
    i64 pos = view_get_cursor_pos(app, view);
    char c = buffer_get_char(app, buffer, pos);
    
    if (MB_IsAlpha(c) || c == '_' || MB_IsDigit(c))
    {
        while(MB_IsAlpha(c) || c == '_' || MB_IsDigit(c))
        {
            move_right(app);
            pos = view_get_cursor_pos(app, view);
            c = buffer_get_char(app, buffer, pos);
        }
    }
    else if (MB_IsOther(c))
    {
        while (MB_IsOther(c))
        {
            move_right(app);
            pos = view_get_cursor_pos(app, view);
            c = buffer_get_char(app, buffer, pos);
        }
    }
    
    if (MB_IsWhitespace(c))
    {
        MB_SkipWhitespace(app, move_right);
    }
    
}

CUSTOM_COMMAND_SIG(MB_GotEndOfLineAndEditMode)
{
    seek_end_of_line(app);
    MB_EnterEditMode(app);
}

CUSTOM_COMMAND_SIG(MB_GoBeginningOfLineAndEditMode)
{
    seek_beginning_of_line(app);
    MB_EnterEditMode(app);
}

CUSTOM_COMMAND_SIG(MB_CursorJumpUp)
{
    page_up(app);
    center_view(app);
}

CUSTOM_COMMAND_SIG(MB_CursorJumpDown)
{
    page_down(app);
    center_view(app);
}
