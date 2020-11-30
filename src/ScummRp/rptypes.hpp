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

#ifndef SCUMMRP_RPTYPES_HPP
#define SCUMMRP_RPTYPES_HPP

enum BlockFormat { BFMT_NULL = 0, BFMT_NOHEADER, BFMT_SIZEONLY,
				   BFMT_SHORTTAG, BFMT_LONGTAG, BFMT_LONGTAG_ALTSIZE };
enum GlobalTocFormat { GTCFMT_NULL = 0, GTCFMT_16SEP32, GTCFMT_8SEP16, GTCFMT_8MIX32, GTCFMT_16MIX32 };
enum GameId { GID_NULL = 0, GID_MANIAC, GID_ZAK, GID_INDY3, GID_LOOM, GID_MONKEY, GID_MONKEY2,
			  GID_INDY4, GID_TENTACLE, GID_SAMNMAX, GID_FT, GID_DIG, GID_CMI };

enum { GF_NULL         = 0,
	   GF_NES          = 1 << 0,
	   GF_FMTOWNS      = 1 << 1,
	   GF_MACINTOSH    = 1 << 2,
	   GF_OLD_BUNDLE   = 1 << 3 };

enum { INF_NULL    = 0,
	   INF_GLOBAL  = 1 << 0,
	   INF_DETAIL  = 1 << 1,
	   INF_LISTING = 1 << 2,
	   INF_ALL     = -1 };

struct GameDefinition
{
	const char *shortName;
	const char *name;
	const char *indexFileName;
	const char *dataFileName;
	int id;
	int version;
	byte indexXorKey;
	byte dataXorKey;
	GlobalTocFormat globalTocFormat;
	BlockFormat blockFormat;
	int32 blockHeaderSize;
	int features;
};

#endif
