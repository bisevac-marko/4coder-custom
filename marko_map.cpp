function void 
MB_SwapKeyCode(Key_Code* a, Key_Code* b)
{
    Key_Code t = *a;
    *a = *b;
    *b = t;
}

function i32 
MB_Partition(Key_Code arr[], i32 low, i32 high)
{
    Key_Code pivot = arr[high];
    i32 i = (low - 1);
    
    for (i32 j = low; j <= high - 1; j++)
    {
        if (arr[j] > pivot)
        {
            i++;
            MB_SwapKeyCode(&arr[i], &arr[j]);
        }
    }
    MB_SwapKeyCode(&arr[i + 1], &arr[high]);
    return (i + 1);
}

function void 
MB_QuickSortModifiers(Key_Code* mods, i32 low, i32 high)
{
    if (low < high)
    {
        i32 pi = MB_Partition(mods, low, high);
        MB_QuickSortModifiers(mods, low, pi - 1);
        MB_QuickSortModifiers(mods, pi + 1, high);
    }
}

function void 
MB_SortKeyModifiers(KeyWithModifier* key_mod)
{
    MB_QuickSortModifiers(key_mod->modifiers, 0, key_mod->mod_count);
}

function b32 
MB_IsKeyCodeModifier(Key_Code key)
{
    return ((key == KeyCode_Control) ||
            (key == KeyCode_Alt) ||
		    (key == KeyCode_Shift) ||
            (key == KeyCode_Command));
}

function void
MB_TrieAddKeySequence(Arena* arena, TrieNode* root, KeySequence* key_sequence)
{
    TrieNode* node = root;
    for (u32 i = 0; i < MAX_KEY_SEQUENCE; ++i)
    {
        KeyWithModifier key_mod = key_sequence->keys[i];
        if (key_mod.mod_count)
        {
            for (u8 m = 0; m < key_mod.mod_count; ++m)
            {
                Assert(MB_IsKeyCodeModifier(key_mod.modifiers[m]));
                Key_Code mod = key_mod.modifiers[m];
                if (!node->children[mod])
                {
					node->children[mod] = push_array_zero(arena, TrieNode, 1);
                }
                node = node->children[mod];
            }
        }
        Key_Code key = key_mod.key;
        if (!node->children[key])
        {
            node->children[key] = push_array_zero(arena, TrieNode, 1);
        }
        if (i == (MAX_KEY_SEQUENCE - 1) || key_sequence->keys[i + 1].key == 0)
        {
            node->children[key]->is_command = true;
            break;
        }
        node = node->children[key];
    }
}

function b32
MB_TrieTryGetNode(TrieNode* node, KeySequence* key_sequence, u32 sequence_level, TrieNode** out, KeyWithModifier key_mod)
{
    for (u32 i = 0; i < sequence_level; ++i)
    {
        KeyWithModifier next_key = key_sequence->keys[i];
		for (u8 m = 0; m < next_key.mod_count; ++m)
		{
			Key_Code mod = next_key.modifiers[m];
			node = node->children[mod];
		}
        node = node->children[key_sequence->keys[i].key];
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

function void
_Map(TrieNode* root, Arena* arena, Table_Data_u64* command_map, Custom_Command_Function* command, Key_Code key, ...)
{
    KeySequence* key_sequence = push_array_zero(arena, KeySequence, 1);
    va_list vl;
    va_start(vl, key);
    key_sequence->keys[0].key = key;
	Key_Code next_key = va_arg(vl, Key_Code); 
	if (MB_IsKeyCodeModifier(next_key))
	{
		for (;;)
		{
			Assert(MB_IsKeyCodeModifier(next_key));
			key_sequence->keys[0].modifiers[key_sequence->keys[0].mod_count++] = next_key;
			next_key = va_arg(vl, Key_Code); 
            if (next_key == ModifiersEnd)
            {
				next_key = va_arg(vl, Key_Code); 
				break;
            }
		}
	}
    u8 key_idx = 1;
    for (;;)
    {
        if (next_key == KeySequenceEnd)
            break;
		key_sequence->keys[key_idx].key = next_key;
		next_key = va_arg(vl, Key_Code); 
        if (MB_IsKeyCodeModifier(next_key))
        {
			for (;;)
			{
				Assert(MB_IsKeyCodeModifier(next_key));
				key_sequence->keys[key_idx].modifiers[key_sequence->keys[key_idx].mod_count++] = next_key;
				next_key = va_arg(vl, Key_Code); 
                if (next_key == ModifiersEnd)
                {
					next_key = va_arg(vl, Key_Code); 
					break;
                }
			}
        }
        // NOTE: if there are not modifiers it doesn't sort
        MB_SortKeyModifiers(&key_sequence->keys[key_idx]);
        key_idx++;
    }
    va_end(vl);
    MB_TrieAddKeySequence(arena, root, key_sequence);
    String_Const_u8 map_key = make_data(key_sequence, sizeof(KeySequence));
    table_insert(command_map, map_key, (u64)PtrAsInt(command));
}

function KeyWithModifier
MB_InputEventToKeyWithModifier(Input_Event* event)
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
    MB_SortKeyModifiers(&result);
    return result;
}

/////////////////////////////

// NOTE: KeyCombo macro appends 0 after all the modifiers to indicate a boundary.
#define MB_KeyCombo(k, ...) k, __VA_ARGS__, 0

#define IMap(F, K, ...) \
_Map(editor->root[EditorMode_Insert], \
&editor->arena, &editor->command_map[EditorMode_Insert], \
BindFWrap_(F), K, __VA_ARGS__, KeySequenceEnd);

#define NMap(F, K, ...) \
_Map(editor->root[EditorMode_Normal], \
&editor->arena, &editor->command_map[EditorMode_Normal], \
BindFWrap_(F), K, __VA_ARGS__, KeySequenceEnd);

