// $Id: Types.h 9431 2010-03-29 13:10:28Z schep $

/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Deadly Rooms of Death.
 *
 * The Initial Developer of the Original Code is
 * Caravel Software.
 * Portions created by the Initial Developer are Copyright (C) 2002, 2005
 * Caravel Software. All Rights Reserved.
 *
 * Contributor(s):
 *
 * ***** END LICENSE BLOCK ***** */

//Type declarations.

#ifndef TYPES_H
#define TYPES_H

//Make sure I have the basics, like NULL.  I don't know how portable this 
//include is.
#  include <time.h>
#  include <limits.h>
#  include <stdio.h>

#  ifndef _WINDOWS_ //If <windows.h> wasn't included.

#		ifndef __APPLE__

typedef unsigned char      BYTE;
typedef unsigned short     USHORT;
typedef unsigned int       UINT;
#ifndef ULONG
typedef unsigned long      ULONG;  //32-bit on Win/amd64, 64-bit on Linux/amd64
#endif

#		else
#			ifdef __GNUC__  // if so, then inttypes.h is probably available
#				include <inttypes.h>  // use the C99 typedefs
				// Typedefs to integral types with exact, specified sizes.  (On 32-bit
				// architectures, at least.)  Note that UINT and DWORD are both
				// typedefs to an unsigned, 32 bit value.  However, for overloading
				// purposes, these must not both refer to the same fundamental type.
				// How to achieve this on compilers where unsigned int and unsigned
				// long are not the same size is beyond me.  (I.e., it requires compiler
				// support one way or another.)
				typedef uint8_t BYTE;
				typedef uint8_t UCHAR;  // I have no idea what the difference is.
				typedef uint16_t USHORT;
				typedef unsigned int UINT;  // (currently, must match Wchar.h)
				typedef unsigned long DWORD;
				typedef int32_t SDWORD;
				typedef uint32_t ULONG;
#			else
#				error FIXME:  provide BYTE, etc. typedefs
#			endif
#		endif //...#ifndef __APPLE__

#  endif //...#ifndef _WINDOWS_

#ifdef WIN32
typedef unsigned __int64 ULONGLONG;
#elif (defined __linux__) || (defined __FreeBSD__) || (defined __APPLE__)
typedef unsigned long long ULONGLONG;
#else
//!!FIX: for other OSs
typedef unsigned long ULONGLONG;
#endif
typedef ULONGLONG QWORD;

#ifndef __APPLE__
	//good random number picking (rely more on high order bits)
	#define RAND(a)   (UINT)(((ULONGLONG)rand() * (a)) / ((ULONGLONG)RAND_MAX+1))
	//random float value in the range [0,a]
	#define fRAND(a) ((a) * (rand() / (float)RAND_MAX))
	//uniform random value in the range [-a,+a]
	#define fRAND_MID(a) ((2*(a) * (rand() / (float)RAND_MAX)) - (a))
#else  // Mac's rand() is spectacularly bad.
	// good random number picking (rely more on a better generator)
	#define RAND(a) ((a) ? random() % (a) : 0)
	// random float value in the range [0,a]
	#define fRAND(a) (1.0 * random() / (1ULL<<32) * (a))
	// uniform random value in the range [-a,+a]
	#define fRAND_MID(a) (fRAND(2.0*(a)) - (a))
#endif

#endif //...#ifndef TYPES_H
