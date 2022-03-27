/*
File:   stdarg.h
Author: Taylor Robbins
Date:   03\21\2022
*/

#ifndef _STDARG_H
#define _STDARG_H

//This comes from LLVM's implementation of stdarg.h
typedef __builtin_va_list va_list;
#define _VA_LIST
#define va_start(ap, param) __builtin_va_start(ap, param)
#define va_end(ap)          __builtin_va_end(ap)
#define va_arg(ap, type)    __builtin_va_arg(ap, type)
#define va_copy(dest, src)  __builtin_va_copy(dest, src)


#endif //  _STDARG_H
