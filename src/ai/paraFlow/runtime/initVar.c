/* initVar - handle variable initialization from tuples */
#include "common.h"
#include "runType.h"
#include "../compiler/pfPreamble.h"
#include "object.h"
#include "initVar.h"

static void _pf_class_cleanup(struct _pf_object *obj, int typeId)
/* Clean up all class fields, and then class itself. */
{
uglyf("_pf_class_cleanup (%d) - still needs work\n", typeId);
free(obj);
}

struct _pf_object *_pf_class_from_tuple(_pf_Stack *stack, int count, 
	int typeId, int memSize)
/* Convert tuple on stack to class. */
{
struct _pf_object *obj = needMem(memSize);
struct _pf_type *type = _pf_type_table[typeId], *fieldType;
struct _pf_base *base = type->base;
char *s = (char *)(obj);
_pf_Stack *field;
int i;

for (i=0, fieldType = base->fields; fieldType != NULL; ++i, fieldType = fieldType->next)
    {
    int offset = fieldType->offset;
    switch (fieldType->base->singleType)
	{
	case pf_stBit:
	    *((_pf_Bit *)(s + offset)) = stack->Bit;
	    break;
	case pf_stByte:
	    *((_pf_Byte *)(s + offset)) = stack->Byte;
	    break;
	case pf_stShort:
	    *((_pf_Short *)(s + offset)) = stack->Short;
	    break;
	case pf_stInt:
	    *((_pf_Int *)(s + offset)) = stack->Int;
	    break;
	case pf_stLong:
	    *((_pf_Long *)(s + offset)) = stack->Long;
	    break;
	case pf_stFloat:
	    *((_pf_Float *)(s + offset)) = stack->Float;
	    break;
	case pf_stDouble:
	    *((_pf_Double *)(s + offset)) = stack->Double;
	    break;
	case pf_stString:
	    *((_pf_String *)(s + offset)) = stack->String;
	    break;
	case pf_stArray:
	    *((_pf_Array *)(s + offset)) = stack->Array;
	    break;
	case pf_stList:
	    *((_pf_List *)(s + offset)) = stack->List;
	    break;
	case pf_stDir:
	    *((_pf_Dir *)(s + offset)) = stack->Dir;
	    break;
	case pf_stTree:
	    *((_pf_Tree *)(s + offset)) = stack->Tree;
	    break;
	case pf_stVar:
	    *((_pf_Var *)(s + offset)) = stack->Var;
	    break;
	case pf_stClass:
	    *((_pf_Object *)(s + offset)) = stack->Obj;
	    break;
	}
    stack += 1;
    }
obj->_pf_refCount = 1;
obj->_pf_cleanup = _pf_class_cleanup;
return obj;
}

static void _pf_array_cleanup(struct _pf_array *array, int id)
/* Clean up all elements of array, and then array itself. */
{
struct _pf_type *elType = _pf_type_table[array->elType];
uglyf("_pf_array_cleanup of %d elements\n", array->count);
if (elType->base->needsCleanup)
    {
    struct _pf_object **objs = (struct _pf_object **)(array->elements);
    int i;
    for (i=0; i<array->count; ++i)
	{
	struct _pf_object *obj = objs[i];
	if (obj != NULL && --obj->_pf_refCount <= 0)
	    obj->_pf_cleanup(obj, array->elType);
	}
    }
freeMem(array->elements);
free(array);
}

static _pf_Array array_of_type(int count, int allocated, 
	int arrayTypeId, int elTypeId, int elSize, void *elements)
/* Create an array of string initialized from tuple on stack. 
 * You still need to fill in array->elements. */
{
struct _pf_array *array;
AllocVar(array);
array->_pf_refCount = 1;
array->_pf_cleanup = _pf_array_cleanup;
array->_pf_typeId = arrayTypeId;
array->elements = elements;
array->count = count;
array->allocated = count;
array->elSize = elSize;
array->elType = elTypeId;
return array;
}

_pf_Array _pf_bit_array_from_tuple(_pf_Stack *stack, int count, 
	int typeId, int elTypeId)
/* Create an array of string initialized from tuple on stack. */
{
struct _pf_array *array;
_pf_Bit *elements = NULL;
int i;

if (count > 0) AllocArray(elements, count);
array = array_of_type(count, count, typeId, elTypeId, sizeof(elements[0]), elements);

for (i=0; i<count; ++i)
    elements[i] = stack[i].Bit;
return array;
}

_pf_Array _pf_byte_array_from_tuple(_pf_Stack *stack, int count, 
	int typeId, int elTypeId)
/* Create an array of string initialized from tuple on stack. */
{
struct _pf_array *array;
_pf_Byte *elements = NULL;
int i;

if (count > 0) AllocArray(elements, count);
array = array_of_type(count, count, typeId, elTypeId, sizeof(elements[0]), elements);

for (i=0; i<count; ++i)
    elements[i] = stack[i].Byte;
return array;
}

_pf_Array _pf_short_array_from_tuple(_pf_Stack *stack, int count, 
	int typeId, int elTypeId)
/* Create an array of string initialized from tuple on stack. */
{
struct _pf_array *array;
_pf_Short *elements = NULL;
int i;

if (count > 0) AllocArray(elements, count);
array = array_of_type(count, count, typeId, elTypeId, sizeof(elements[0]), elements);

for (i=0; i<count; ++i)
    elements[i] = stack[i].Short;
return array;
}

_pf_Array _pf_int_array_from_tuple(_pf_Stack *stack, int count, 
	int typeId, int elTypeId)
/* Create an array of string initialized from tuple on stack. */
{
struct _pf_array *array;
_pf_Int *elements = NULL;
int i;

if (count > 0) AllocArray(elements, count);
array = array_of_type(count, count, typeId, elTypeId, sizeof(elements[0]), elements);

for (i=0; i<count; ++i)
    elements[i] = stack[i].Int;
return array;
}

_pf_Array _pf_long_array_from_tuple(_pf_Stack *stack, int count, 
	int typeId, int elTypeId)
/* Create an array of string initialized from tuple on stack. */
{
struct _pf_array *array;
_pf_Long *elements = NULL;
int i;

if (count > 0) AllocArray(elements, count);
array = array_of_type(count, count, typeId, elTypeId, sizeof(elements[0]), elements);

for (i=0; i<count; ++i)
    elements[i] = stack[i].Long;
return array;
}

_pf_Array _pf_float_array_from_tuple(_pf_Stack *stack, int count, 
	int typeId, int elTypeId)
/* Create an array of string initialized from tuple on stack. */
{
struct _pf_array *array;
_pf_Float *elements = NULL;
int i;

if (count > 0) AllocArray(elements, count);
array = array_of_type(count, count, typeId, elTypeId, sizeof(elements[0]), elements);

for (i=0; i<count; ++i)
    elements[i] = stack[i].Float;
return array;
}

_pf_Array _pf_double_array_from_tuple(_pf_Stack *stack, int count, 
	int typeId, int elTypeId)
/* Create an array of string initialized from tuple on stack. */
{
struct _pf_array *array;
_pf_Double *elements = NULL;
int i;

if (count > 0) AllocArray(elements, count);
array = array_of_type(count, count, typeId, elTypeId, sizeof(elements[0]), elements);

for (i=0; i<count; ++i)
    elements[i] = stack[i].Double;
return array;
}

_pf_Array _pf_string_array_from_tuple(_pf_Stack *stack, int count, 
	int typeId, int elTypeId)
/* Create an array of string initialized from tuple on stack. */
{
struct _pf_array *array;
_pf_String *strings = NULL;
int i;

if (count > 0) AllocArray(strings, count);
array = array_of_type(count, count, typeId, elTypeId, sizeof(strings[0]), strings);

for (i=0; i<count; ++i)
    strings[i] = stack[i].String;
return array;
}

_pf_Array _pf_var_array_from_tuple(_pf_Stack *stack, int count, 
	int typeId, int elTypeId)
/* Create an array of string initialized from tuple on stack. */
{
struct _pf_array *array;
_pf_Var *elements = NULL;
int i;

if (count > 0) AllocArray(elements, count);
array = array_of_type(count, count, typeId, elTypeId, sizeof(elements[0]), elements);

for (i=0; i<count; ++i)
    elements[i] = stack[i].Var;
return array;
}

_pf_Array _pf_class_array_from_tuple(_pf_Stack *stack, int count, 
	int typeId, int elTypeId)
/* Create an array of string initialized from tuple on stack. */
{
struct _pf_array *array;
struct _pf_object **elements = NULL;
int i;

if (count > 0) AllocArray(elements, count);
array = array_of_type(count, count, typeId, elTypeId, sizeof(elements[0]), elements);

for (i=0; i<count; ++i)
    {
    struct _pf_object *obj = stack[i].Obj;
    elements[i] = obj;
    }
return array;
}

#ifdef TEMPLATE
_pf_Array _pf_xyz_array_from_tuple(_pf_Stack *stack, int count, 
	int typeId, int elTypeId)
/* Create an array of string initialized from tuple on stack. */
{
struct _pf_array *array;
_pf_Xyz *elements = NULL;
int i;

if (count > 0) AllocArray(elements, count);
array = array_of_type(count, count, typeId, elTypeId, sizeof(elements[0]), elements);

for (i=0; i<count; ++i)
    elements[i] = stack[i].Xyz;
return array;
}

#endif /* TEMPLATE */
