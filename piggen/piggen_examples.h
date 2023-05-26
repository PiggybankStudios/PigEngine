/*
File:   piggen_examples.h
Author: Taylor Robbins
Date:   05\09\2023
Description:
	** This file serves as a set of example PigGen inputs that we can test with and
	** also serve as some documentation of the format
*/

#ifndef _PIGGEN_EXAMPLES_H
#define _PIGGEN_EXAMPLES_H

#if PIGGEN
Serializable: ItemStack_t
{
	id:        u64
	flags:     u8
	itemId:    ItemId_t/u64
	count:     u64
	uiRec:     rec
	position:  v3?
}
Serializable: AnotherStruct_t
{
	id:        u64
}
#else
#include "piggen_examples_h_0.h"
#endif

#endif //  _PIGGEN_EXAMPLES_H
