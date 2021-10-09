/* date = October 5th 2021 9:27 am */

#ifndef MARKO_MAP_H
#define MARKO_MAP_H

#define MAX_KEY_SEQUENCE 3
#define ModifiersEnd 0
#define KeySequenceEnd (1 << 16)


enum EditorMode
{
    EditorMode_Normal,
    EditorMode_Insert,
    EditorMode_Count,
};

struct TrieNode
{
    TrieNode* children[KeyCode_COUNT];
    b32 is_command;
};


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

#endif //MARKO_MAP_H
