internal inline bool 
MB_IsAlpha(char c)
{
    bool result = (((c >= 'a') && (c <= 'z')) ||
                   ((c >= 'A') && (c <= 'Z')));
    
    return(result);
}

internal inline bool 
MB_IsDigit(char c)
{
    return (c >= '0' && c <= '9');
}

internal inline bool 
MB_IsOther(char c)
{
    return ((c >= 58 && c <= 63) ||
            (c >= 33 && c <= 47) ||
            (c >= 91 && c <= 96) ||
            (c >= 123 && c <= 126));
}

internal bool 
MB_IsWhitespace(char c)
{
    return (c == ' ' || c == '\n');
}
