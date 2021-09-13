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

inline char GetChar(Application_Links* app, View_ID view, Buffer_ID buffer, i32 offset = 0)
{
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
    
    View_ID view = get_active_view(app, 0);
    Buffer_ID buffer = view_get_buffer(app, view, 0);
    
    char c = GetChar(app, view, buffer);
    while (c == ' ' || c == '\n' || c == '\r\n')
    {
        move_cursor(app);
        c = GetChar(app, view, buffer);
    }
}


CUSTOM_COMMAND_SIG(CursorMoveLeft)
{
    View_ID view = get_active_view(app, 0);
    Buffer_ID buffer = view_get_buffer(app, view, 0);
    
    u8 c = GetChar(app, view, buffer);
    
    if (IsAlpha(c) || c == '_' || IsDigit(c))
    {
        while(IsAlpha(c) || c == '_' || IsDigit(c))
        {
            move_left(app);
            c = GetChar(app, view, buffer, -1);
        }
    }
    else if (IsOther(c))
    {
        move_left(app);
        c = GetChar(app, view, buffer);
        
        while (IsAlpha(c) || c == '_' || IsDigit(c))
        {
            move_left(app);
            c = GetChar(app, view, buffer, -1);
        }
    }
    
    if (IsWhitespace(c))
    {
        SkipWhitespace(app, move_left);
    }
    
}



CUSTOM_COMMAND_SIG(CursorMoveRight)
{
    View_ID view = get_active_view(app, 0);
    Buffer_ID buffer = view_get_buffer(app, view, 0);
    
    u8 c = GetChar(app, view, buffer);
    
    if (IsAlpha(c) || c == '_' || IsDigit(c))
    {
        while(IsAlpha(c) || c == '_' || IsDigit(c))
        {
            move_right(app);
            c = GetChar(app, view, buffer);
        }
    }
    else if (IsOther(c))
    {
        while (IsOther(c))
        {
            move_right(app);
            c = GetChar(app, view, buffer);
        }
    }
    
    if (IsWhitespace(c))
    {
        SkipWhitespace(app, move_right);
    }
    
}

CUSTOM_COMMAND_SIG(GotEndOfLineAndEditMode)
{
    seek_end_of_line(app);
    EnterEditMode(app);
}

CUSTOM_COMMAND_SIG(GoBeginningOfLineAndEditMode)
{
    seek_beginning_of_line(app);
    EnterEditMode(app);
}

CUSTOM_COMMAND_SIG(CursorJumpUp)
{
    page_up(app);
    center_view(app);
}

CUSTOM_COMMAND_SIG(CursorJumpDown)
{
    page_down(app);
    center_view(app);
}
