/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (c) 2003-2004 Thomas Combeleran
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
 * Note: the last "original" binary was built on: 2003-10-05 23:40:55.
 * The #ifdef additions appear to have been done after that build, but they
 * were never released.
 */

// SCUMM charset format references:
//
// https://wiki.scummvm.org/index.php?title=SCUMM/Technical_Reference/Charset_resources
// https://wiki.scummvm.org/index.php/SCUMM/Technical_Reference/SCUMM_6_resource_files#3.2.20_CHAR
//
// BMP format references:
//
// https://formats.kaitai.io/bmp/index.html
// https://freeimage.sourceforge.io/fnet/html/4720264E.htm
// https://github.com/HexFiend/HexFiend/blob/v2.18.1/templates/Images/BMP.tcl
// https://github.com/synalysis/Grammars/blob/35e53acbad8d25faefefdf0863d2a08cc71918a8/bitmap.grammar

#include "common/types.hpp"
#include "common/file.hpp"
#include "common/toolbox.hpp"

#include <clocale>
#include <cstdarg>
#include <cstdio>
#include <cstring>

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

static const int PALETTE_SIZE = 4 * 256;
static byte glPalette[PALETTE_SIZE] =
{
	0xFF, 0x00, 0xFF, 0, 0xFF, 0xFF, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0xFF, 0x00, 0,
	0x00, 0x00, 0xFF, 0, 0x00, 0xFF, 0xFF, 0, 0x00, 0x7F, 0x7F, 0, 0x7F, 0x00, 0x00, 0,
	0x00, 0x7F, 0x00, 0, 0x00, 0x00, 0x7F, 0, 0x7F, 0x7F, 0x00, 0, 0x7F, 0x00, 0xFF, 0,
	0xFF, 0x00, 0x7F, 0, 0x00, 0x7F, 0xFF, 0, 0x00, 0xFF, 0x7F, 0, 0x7F, 0xFF, 0x00, 0,
/* {{{ */
	0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0,
	0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0,
	0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0,
	0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0,
	0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0,
	0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0,
	0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0,
	0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0,
	0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0,
	0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0,
	0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0,
	0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0,
	0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0,
	0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0,
	0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0,
	0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0,
	0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0,
	0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0,
	0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0,
	0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0,
	0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0,
	0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0,
	0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0,
	0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0,
	0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0,
	0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0,
	0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0,
	0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0,
	0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0,
	0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0,
	0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0,
	0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0,
	0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0,
	0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0,
	0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0,
	0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0,
	0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0,
	0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0,
	0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0,
	0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0,
	0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0,
	0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0,
	0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0,
	0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0,
	0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0,
	0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0,
	0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0,
	0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0,
	0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0,
	0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0,
	0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0,
	0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0,
	0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0,
	0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0,
	0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0,
	0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0,
	0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0,
	0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0,
/* }}} */
#ifdef SCUMMFONT_NEW_PALETTE_CHANGE_AFTER_RELEASE
	0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x10, 0x00, 0,
#else
	// note: starts at offset 00415450 through dumpbin in original scummfont.exe .data section
	0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0,
#endif
	0xFF, 0xBF, 0xFF, 0, 0xFF, 0xDF, 0xFF, 0, 0x00, 0x00, 0x00, 0, 0xDF, 0x00, 0xDF, 0
};
static byte *glFontBitmap = nullptr;
static int32 glWidth = 0;
static int32 glHeight = 0;

static int usage()
{
	std::cout << "ScummFont 0.6.0 (build " << SCUMMTR_BUILD_DATE << ") by Thomas Combeleran\n\n";

	std::cout << "usage:\n";
	std::cout << "  scummfont {i|o} font bitmap.bmp\n\n";
	std::cout << "  o: Export bitmap.bmp from font\n";
	std::cout << "  i: Import bitmap.bmp into font\n\n";
	std::cout << "\"font\" is either a CHAR block extracted with scummrp, or an LFL file\n";
	std::cout << "(usually 901.LFL/98.LFL, or higher numbers)\n\n";

	std::cout << "Using the GIMP image editor (www.gimp.org) with its \"Do not write\n";
	std::cout << "color space information\" compatibility option is highly recommended!\n\n";

	std::cout << "For questions about supported encodings, or missing characters for your\n";
	std::cout << "language, read the official ScummTR documentation.\n\n";

	printCommonDisclaimer();

	return 0;
}

static void changePaletteBpp1()
{
	glPalette[4] = glPalette[5] = glPalette[6] =  0x00;
	glPalette[8] = 0xFF;
	glPalette[9] = 0xFF;
	glPalette[10] = 0x00;
}

static void getFontInfo(int32 &baseOffset, File &file, int &version, int &bpp, int &maxHeight, int &maxWidth, int &bytesPerChar, int16 &numChars)
{
	int32 tag;
	int lineSpacing;

	file->getBE32(tag);
	baseOffset = (tag == MKTAG4('C','H','A','R')) ? 8 : 0;

	file.seekg(baseOffset + 0x04, std::ios::beg);
	if (file.get() != 'c')
		throw std::runtime_error("Not a scumm font");

	version = file.get();
	if (version != 1 && version != 3)
		throw std::runtime_error(xsprintf("Unsupported scumm font version: %i", version));

	if (version == 1)
	{
		bpp = 1;
		maxWidth = maxHeight = 8;
		numChars = (byte)file.get();
		bytesPerChar = file.get();
	}
	else
	{
#ifdef SCUMMFONT_MAKETABLE
		std::ofstream tableOutput("width-table.txt", std::ios::binary | std::ios::out | std::ios::trunc);
#endif
		bytesPerChar = 8;
		maxWidth = maxHeight = 0;

		file.seekg(baseOffset + 0x15, std::ios::beg);
		bpp = file.get();
		lineSpacing = file.get();
		file->getLE16(numChars);

		for (int i = 0; i < numChars; ++i)
		{
			int32 offset;
			int width, height;

			file.seekg(baseOffset + 0x19 + i * 4, std::ios::beg);
			file->getLE32(offset);
#ifdef SCUMMFONT_MAKETABLE
			if (offset == 0)
			{
				tableOutput << 0 << ", ";
			}
			else
			{
#endif
				file.seekg(baseOffset + 0x15 + offset, std::ios::beg);
				width = file.get();
				if (width > maxWidth)
					maxWidth = width;

				height = file.get();
				if (height > maxHeight)
					maxHeight = height;

#ifdef SCUMMFONT_MAKETABLE
				tableOutput << width << ", ";
			}
#endif
		}

		if (maxHeight < lineSpacing)
			maxHeight = lineSpacing;
	}

	if ((uint32)bytesPerChar < 8 || bpp == 0 || bpp == 3 || (uint32)bpp > 4
		|| (uint16)numChars > 0x100 || maxHeight < 0 || maxWidth < 0)
		throw std::runtime_error("Your font is strange...");
}

static const char *tmpPath(const char *path)
{
	static std::string s(path);

	s += "-new";

	return s.c_str();
}

static inline int roundTo4(int i)
{
	return (i + 3) & ~0x3;
}

static void saveBmp(const char *path)
{
	File file;
	uint32 udw;
	int32 sdw;
	uint16 w;
	byte *buf;

	file.open(path, std::ios::binary | std::ios::out | std::ios::trunc);
	if (!file.is_open())
		throw std::runtime_error("Cannot open BMP file");

	file.write("BM", 2);

	udw = 0x436 + roundTo4(glWidth) * glHeight;
	file->putLE32(udw);

	udw = 0;
	file->putLE32(udw);

	udw = 0x436;
	file->putLE32(udw);

	udw = 0x28;
	file->putLE32(udw);

	sdw = glWidth;
	file->putLE32(sdw);
	sdw = glHeight;
	file->putLE32(sdw);

	w = 1;
	file->putLE16(w);

	w = 8;
	file->putLE16(w);

	udw = 0;
	file->putLE32(udw);

	udw = roundTo4(glWidth) * glHeight;
	file->putLE32(udw);

	sdw = 0;
	file->putLE32(sdw);

	sdw = 0;
	file->putLE32(sdw);

	udw = 256;
	file->putLE32(udw);

	udw = 256;
	file->putLE32(udw);

	file.write((char *)glPalette, PALETTE_SIZE);

	buf = new byte[roundTo4(glWidth) * glHeight];
	memset(buf, 0, roundTo4(glWidth) * glHeight);
	for (int i = 0; i < glHeight; ++i)
		std::memcpy(buf + i * roundTo4(glWidth), glFontBitmap + (glHeight - i - 1) * glWidth, glWidth);

	file.write((char *)buf, roundTo4(glWidth) * glHeight);
	delete[] buf;

	file.close();
}

static void loadBmp(const char *path)
{
	byte paletteCheck[PALETTE_SIZE];
	File file;
	uint32 udw;
	int32 sdw;
	uint16 w;
	byte *buf;
	uint32 off;

	file.open(path, std::ios::in | std::ios::binary);
	if (!file.is_open())
		throw std::runtime_error("Cannot open BMP file");

	file->getBE16(w);
	if (w != MKTAG2('B','M'))
		throw std::runtime_error("This is not a BMP file");

	file.seekg(8, std::ios::cur);
	file->getLE32(off);
	if (off < 0x36)
		throw std::runtime_error("This is not a valid BMP file");

	file->getLE32(udw);
	if (udw != 40)
		throw std::runtime_error(xsprintf("A 40-byte BITMAPINFOHEADER was expected, but %i bytes were found instead", udw));

	file->getLE32(sdw);
	glWidth = sdw;
	file->getLE32(sdw);
	glHeight = sdw;
	if (glWidth <= 0 || glHeight <= 0)
		throw std::runtime_error(xsprintf("%i per %i width/height detected, but negative values are not supported", glWidth, glHeight));

	file->getLE16(w);
	if (w != 1)
		throw std::runtime_error(xsprintf("This is not a single-plane BMP file: %hu planes found", w));

	file->getLE16(w);
	if (w != 8)
		throw std::runtime_error(xsprintf("This is not an 8-bpp BMP file: %hu bpp found", w));

	file->getLE32(udw);
	if (udw != 0)
		throw std::runtime_error(xsprintf("This BMP file must be uncompressed, but \"%u\" compression was found", udw));

	file.seekg(12, std::ios::cur);
	file->getLE32(udw);
	if (udw != 0 && udw != 256)
		throw std::runtime_error(xsprintf("Palette must have exactly 256 colors, but %u colors were found", udw));

	file.seekg(4, std::ios::cur);
	file.read((char *)paletteCheck, PALETTE_SIZE);
	if (std::memcmp(glPalette, paletteCheck, PALETTE_SIZE) != 0)
	{
		changePaletteBpp1();

		if (std::memcmp(glPalette, paletteCheck, PALETTE_SIZE) != 0)
			throw std::runtime_error("This file doesn't contain an original ScummFont palette");
	}

	file.seekg(off, std::ios::beg);
	buf = new byte[roundTo4(glWidth) * glHeight];
	glFontBitmap = new byte[glWidth * glHeight];
	file.read((char *)buf, roundTo4(glWidth) * glHeight);
	for (int i = 0; i < glHeight; ++i)
		std::memcpy(glFontBitmap + i * glWidth, buf + (glHeight - i - 1) * roundTo4(glWidth), glWidth);
	delete[] buf;

	file.close();
}

static void saveFont(const char *path)
{
	File file;
	int version, bpp, maxHeight, maxWidth, bytesPerChar;
	int16 numChars;
	int newNumChars;
	int32 baseOffset, endOffset;

	file.open(path, std::ios::in | std::ios::binary);
	if (!file.is_open())
		throw std::runtime_error("Cannot open font file");

	getFontInfo(baseOffset, file, version, bpp, maxHeight, maxWidth, bytesPerChar, numChars);

	if (glHeight < maxHeight * numChars || glWidth < maxWidth || (glWidth != maxWidth && version == 1)) // (1)
		throw std::runtime_error("Wrong resolution");
	if (maxHeight == 0)
		throw std::runtime_error("maxHeight can't be equal to zero");

	if (glWidth > maxWidth)
		maxWidth = glWidth;

	newNumChars = glHeight / maxHeight;
	if (newNumChars > 0x100)
		newNumChars = (version == 1) ? 0xFF : 0x100;

	{
		File tmpFile;
		char buffer[0x440];
		const char *tmpfilepath = tmpPath(path);

		tmpFile.open(tmpfilepath, std::ios::binary | std::ios::out | std::ios::trunc);
		if (!tmpFile.is_open())
			throw std::runtime_error("Cannot open new font file");

		file.seekg(0, std::ios::beg);
		memset(buffer, 0, sizeof buffer);

		endOffset = (version == 3) ? baseOffset + 0x19 + 4 * numChars : baseOffset + 0x8 + numChars;
		file.read(buffer, endOffset);
		endOffset += (version == 3) ? (newNumChars - numChars) * 4 : newNumChars - numChars; // positive thanks to (1)
		tmpFile.write(buffer, endOffset);

		for (int i = 0; i < newNumChars; ++i)
		{
			byte b, p;
			int width, height;

			width = height = 0;
			for (int k = maxWidth - 1; k >= 0; --k)
			{
				for (int j = 0; j < maxHeight; ++j)
				{
					if ((byte)(glFontBitmap[k + maxWidth * (j + maxHeight * i)] + 1) > 1)
					{
						width = k + 1;
						break;
					}
				}

				if (width != 0)
					break;
			}

			if (version > 1)
			{
				for (int j = maxHeight - 1; j >= 0; --j)
				{
					for (int k = 0; k < width; ++k)
					{
						if ((byte)(glFontBitmap[k + maxWidth * (j + maxHeight * i)] + 1) > 1)
						{
							height = j + 1;
							break;
						}
					}

					if (height != 0)
						break;
				}
			}
			else
			{
				height = maxHeight;
			}

			if (version == 1)
			{
				tmpFile.seekp(baseOffset + 0x8 + i, std::ios::beg);
				tmpFile.put((char)width);
				width = maxWidth;
				tmpFile.seekp(baseOffset + 0x8 + newNumChars + i * bytesPerChar, std::ios::beg);
			}
			else
			{
				int32 offset;
				byte x, y;

				if (width != 0 && height != 0)
				{
					x = y = 0;
					if (i < numChars)
					{
						file.seekg(baseOffset + 0x19 + i * 4, std::ios::beg);
						file->getLE32(offset);
						if (offset != 0)
						{
							file.seekg(baseOffset + 0x15 + offset + 2, std::ios::beg);
							x = file.get();
							y = file.get();
						}
					}
					offset = endOffset - baseOffset - 0x15;
					tmpFile.seekp(baseOffset + 0x19 + i * 4, std::ios::beg);
					tmpFile->putLE32(offset);
					tmpFile.seekp(endOffset, std::ios::beg);
					tmpFile.put((char)width);
					tmpFile.put((char)height);
					tmpFile.put(x);
					tmpFile.put(y);
					endOffset += 4;
				}
				else
				{
					offset = 0;
					tmpFile.seekp(baseOffset + 0x19 + i * 4, std::ios::beg);
					tmpFile->putLE32(offset);
					continue;
				}
			}

			b = p = 0;
			for (int j = 0; j < height; ++j)
			{
				for (int k = 0; k < width; ++k)
				{
					const int l = k + maxWidth * (j + maxHeight * i);

					if ((glFontBitmap[l] & ((1 << bpp) - 1)) == glFontBitmap[l])
						b |= glFontBitmap[l] << (8 - bpp - p);

					p += bpp;
					if (p >= 8)
					{
						tmpFile.put((char)b);
						++endOffset;
						b = p = 0;
					}
				}
			}

			if (p != 0)
			{
				tmpFile.put((char)b);
				++endOffset;
			}
			else if (version == 3)
			{
				tmpFile.put((char)b); // FIXME is it really some junk?
				++endOffset;
			}
		}

		if (baseOffset == 8) // block header
		{
			tmpFile.seekp(0x4, std::ios::beg);
			tmpFile->putBE32(endOffset);
		}

		if (version == 1)
		{
			if (numChars != newNumChars)
			{
				tmpFile.seekp(baseOffset, std::ios::beg);
				++endOffset;
				tmpFile->putLE32(endOffset);
				tmpFile.seekp(baseOffset + 0x6, std::ios::beg);
				tmpFile.put((char)newNumChars);
			}
		}
		else
		{
			tmpFile.seekp(baseOffset, std::ios::beg);
			endOffset -= 0xF + baseOffset;
			tmpFile->putLE32(endOffset);
			tmpFile.seekp(baseOffset + 0x17, std::ios::beg);
			numChars = (int16)newNumChars;
			tmpFile->putLE16(numChars);
		}
		tmpFile.close();
		file.close();

		xremove(path);
		xrename(tmpfilepath, path);
	}
}

static void loadFont(const char *path)
{
	File file;
	int version, bpp, maxHeight, maxWidth, bytesPerChar;
	int16 numChars;
	int32 baseOffset;

	file.open(path, std::ios::in | std::ios::binary);
	if (!file.is_open())
		throw std::runtime_error("Cannot open font file");

	getFontInfo(baseOffset, file, version, bpp, maxHeight, maxWidth, bytesPerChar, numChars);

#ifdef SCUMMFONT_256
	glFontBitmap = new byte[128 * 128];
	memset(glFontBitmap, 0, 128 * 128);
#else
	glFontBitmap = new byte[maxWidth * maxHeight * numChars];
	memset(glFontBitmap, 0, maxWidth * maxHeight * numChars);
#endif
	if (bpp == 1)
		changePaletteBpp1();
#ifdef SCUMMFONT_256
	glHeight = 128;
	glWidth = 128;
#else
	glHeight = maxHeight * numChars;
	glWidth = maxWidth;
#endif
	for (int i = 0; i < numChars; ++i)
	{
		byte b, mask, p;
		int width, height;

#ifdef SCUMMFONT_256
		for (int j = 0; j < maxHeight; ++j)
			memset(glFontBitmap + ((i % 128) * maxWidth) + j * 128, (i & 1) ? 0xFF : 0x00, maxWidth);
#else
		memset(glFontBitmap + i * maxWidth * maxHeight, (i & 1) ? 0xFF : 0x00, maxWidth * maxHeight);
#endif
		if (version == 1)
		{
			height = maxHeight;
			file.seekg(baseOffset + 0x8 + i, std::ios::beg);
			width = file.get();

			file.seekg(baseOffset + 0x8 + numChars + i * bytesPerChar, std::ios::beg);
		}
		else
		{
			int32 offset;

			file.seekg(baseOffset + 0x19 + i * 4, std::ios::beg);
			file->getLE32(offset);
			if (offset == 0)
				continue;

			file.seekg(baseOffset + 0x15 + offset, std::ios::beg);
			width = file.get();
			height = file.get();

			file.seekg(2, std::ios::cur);
		}

		mask = b = p = 0;
		for (int j = 0; j < height; ++j)
		{
			for (int k = 0; k < width; ++k)
			{
				glFontBitmap[k + maxWidth * (j + maxHeight * i)] = 0xFD ^ (i & 1);

				mask >>= bpp;
				p += bpp;
				if (mask == 0)
				{
					mask = (byte)(((1 << bpp) - 1) << (8 - bpp));
					b = (byte)file.get();
					p = 0;
				}

				if ((b & mask) != 0)
					glFontBitmap[k + maxWidth * (j + maxHeight * i)] = (b & mask) >> (8 - bpp - p);
			}

			if (version == 1)
				mask = b = p = 0;
		}
	}
	file.close();
}

int main(int argc, char **argv) try
{
	std::setlocale(LC_CTYPE, "");

	if (argc < 4)
		return usage();

	if (argv[1][0] == 'i')
	{
		loadBmp(argv[3]);
		saveFont(argv[2]);
		delete[] glFontBitmap;
	}
	else if (argv[1][0] == 'o')
	{
		loadFont(argv[2]);
		saveBmp(argv[3]);
		delete[] glFontBitmap;
	}
	else
	{
		return usage();
	}

	return 0;
}
catch (std::exception &e)
{
	delete[] glFontBitmap;
	std::cerr << "ERROR: " << e.what() << std::endl;
}
catch (...)
{
	delete[] glFontBitmap;
	throw;
}
