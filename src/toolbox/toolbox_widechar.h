//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
//						 
// toolbox_widechar.h
//					
// Copyright (c) 2010 Francois Oligny-Lemieux
//				
// Maintained by : Francois Oligny-Lemieux
//          Date : 08.Aug.2010
//      Modified : 
//
//  Description: 
//      Defines for UTF16 (wide char) functions for linux/windows
//
//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

#ifndef __C_TOOLBOX_WIDE_CHAR__
#define __C_TOOLBOX_WIDE_CHAR__

#include "toolbox-basic-types.h"  // for uint64_t

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_MSC_VER)

#define wfopen _wfopen
#define wfread _wfread
// not needed in wide string, use fwrite, #define wfwrite _wfwrite

#else

#define wfopen wfopen
#ifndef wchar_t
# define wchar_t int // dont use that, just so it compiles
#endif

#endif

#ifdef __cplusplus
}
#endif

#endif
