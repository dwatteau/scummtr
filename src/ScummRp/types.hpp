/*
 * Copyright (c) 2003 Thomas Combeleran
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#ifndef __TYPES_HPP__
#define __TYPES_HPP__

#ifdef _MSC_VER
typedef unsigned __int8 byte;
typedef unsigned __int8 uint8;
typedef unsigned __int16 uint16;
typedef unsigned __int32 uint32;
typedef unsigned int uint;
typedef __int8 int8;
typedef __int16 int16;
typedef __int32 int32;
#elif defined(SCUMMRP_ORIGINAL_CONTENT_BUT_WRONG_TODAY)
typedef unsigned char byte;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned long uint32;
typedef unsigned int uint;
typedef char int8;
typedef short int16;
typedef long int32;
#else
// note: will do for almost any current 32-bit or 64-bit arch.
typedef unsigned char byte;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned int uint;
typedef signed char int8;
typedef short int16;
typedef int int32;
#endif

#define MKTAG2(a,b)	((uint16)((b) | ((a) << 8)))
#define MKTAG4(a,b,c,d)	((uint32)((d) | ((c) << 8) | ((b) << 16) | ((a) << 24)))

#endif // !__TYPES_HPP__
