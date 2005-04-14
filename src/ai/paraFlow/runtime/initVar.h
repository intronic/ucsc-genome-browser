/* initVar - handle variable initialization from tuples */

struct _pf_string *_pf_string_from_const(char *s);
/* Wrap string around constant. */

struct _pf_object *_pf_class_from_tuple(_pf_Stack *stack, int typeId, _pf_Stack **retStack);
/* Convert tuple on stack to class. */

_pf_Array _pf_bit_array_from_tuple(_pf_Stack *stack, int count, int typeId, int elTypeId);
_pf_Array _pf_byte_array_from_tuple(_pf_Stack *stack, int count, int typeId, int elTypeId);
_pf_Array _pf_short_array_from_tuple(_pf_Stack *stack, int count, int typeId, int elTypeId);
_pf_Array _pf_int_array_from_tuple(_pf_Stack *stack, int count, int typeId, int elTypeId);
_pf_Array _pf_long_array_from_tuple(_pf_Stack *stack, int count, int typeId, int elTypeId);
_pf_Array _pf_float_array_from_tuple(_pf_Stack *stack, int count, int typeId, int elTypeId);
_pf_Array _pf_double_array_from_tuple(_pf_Stack *stack, int count, int typeId, int elTypeId);
_pf_Array _pf_string_array_from_tuple(_pf_Stack *stack, int count, int typeId, int elTypeId);
_pf_Array _pf_class_array_from_tuple(_pf_Stack *stack, int count, int typeId, int elTypeId);
_pf_Array _pf_var_array_from_tuple(_pf_Stack *stack, int count, int typeId, int elTypeId);

_pf_List _pf_bit_list_from_tuple(_pf_Stack *stack, int count, int typeId, int elTypeId);
_pf_List _pf_byte_list_from_tuple(_pf_Stack *stack, int count, int typeId, int elTypeId);
_pf_List _pf_short_list_from_tuple(_pf_Stack *stack, int count, int typeId, int elTypeId);
_pf_List _pf_int_list_from_tuple(_pf_Stack *stack, int count, int typeId, int elTypeId);
_pf_List _pf_long_list_from_tuple(_pf_Stack *stack, int count, int typeId, int elTypeId);
_pf_List _pf_float_list_from_tuple(_pf_Stack *stack, int count, int typeId, int elTypeId);
_pf_List _pf_double_list_from_tuple(_pf_Stack *stack, int count, int typeId, int elTypeId);
_pf_List _pf_string_list_from_tuple(_pf_Stack *stack, int count, int typeId, int elTypeId);
_pf_List _pf_class_list_from_tuple(_pf_Stack *stack, int count, int typeId, int elTypeId);

_pf_Tree _pf_bit_tree_from_tuple(_pf_Stack *stack, int count, int typeId, int elTypeId);
_pf_Tree _pf_byte_tree_from_tuple(_pf_Stack *stack, int count, int typeId, int elTypeId);
_pf_Tree _pf_short_tree_from_tuple(_pf_Stack *stack, int count, int typeId, int elTypeId);
_pf_Tree _pf_int_tree_from_tuple(_pf_Stack *stack, int count, int typeId, int elTypeId);
_pf_Tree _pf_long_tree_from_tuple(_pf_Stack *stack, int count, int typeId, int elTypeId);
_pf_Tree _pf_float_tree_from_tuple(_pf_Stack *stack, int count, int typeId, int elTypeId);
_pf_Tree _pf_double_tree_from_tuple(_pf_Stack *stack, int count, int typeId, int elTypeId);
_pf_Tree _pf_string_tree_from_tuple(_pf_Stack *stack, int count, int typeId, int elTypeId);
_pf_Tree _pf_class_tree_from_tuple(_pf_Stack *stack, int count, int typeId, int elTypeId);

_pf_Dir _pf_bit_dir_from_tuple(_pf_Stack *stack, int count, int typeId, int elTypeId);
_pf_Dir _pf_byte_dir_from_tuple(_pf_Stack *stack, int count, int typeId, int elTypeId);
_pf_Dir _pf_short_dir_from_tuple(_pf_Stack *stack, int count, int typeId, int elTypeId);
_pf_Dir _pf_int_dir_from_tuple(_pf_Stack *stack, int count, int typeId, int elTypeId);
_pf_Dir _pf_long_dir_from_tuple(_pf_Stack *stack, int count, int typeId, int elTypeId);
_pf_Dir _pf_float_dir_from_tuple(_pf_Stack *stack, int count, int typeId, int elTypeId);
_pf_Dir _pf_double_dir_from_tuple(_pf_Stack *stack, int count, int typeId, int elTypeId);
_pf_Dir _pf_string_dir_from_tuple(_pf_Stack *stack, int count, int typeId, int elTypeId);
_pf_Dir _pf_class_dir_from_tuple(_pf_Stack *stack, int count, int typeId, int elTypeId);

