/*
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
 * Note: the last "official" binary was built on: 2003-10-05 23:40:55.
 * The #ifdef additions appear to have been done after that build, but they
 * were never released.
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <string>

#define MKTAG2(a,b)	((unsigned short)((b) | ((a) << 8)))
#define MKTAG4(a,b,c,d)	((unsigned int)((d) | ((c) << 8) | ((b) << 16) | ((a) << 24)))

static unsigned char glPalette[0x400] = {
	0xFF, 0x00, 0xFF, 0, 0xFF, 0xFF, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0xFF, 0x00, 0,
	0x00, 0x00, 0xFF, 0, 0x00, 0xFF, 0xFF, 0, 0x00, 0x7F, 0x7F, 0, 0x7F, 0x00, 0x00, 0,
	0x00, 0x7F, 0x00, 0, 0x00, 0x00, 0x7F, 0, 0x7F, 0x7F, 0x00, 0, 0x7F, 0x00, 0xFF, 0,
	0xFF, 0x00, 0x7F, 0, 0x00, 0x7F, 0xFF, 0, 0x00, 0xFF, 0x7F, 0, 0x7F, 0xFF, 0x00, 0,
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
	0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x00, 0x00, 0, 0x00, 0x10, 0x00, 0,
	0xFF, 0xBF, 0xFF, 0, 0xFF, 0xDF, 0xFF, 0, 0x00, 0x00, 0x00, 0, 0xDF, 0x00, 0xDF, 0 };
static unsigned char *glFontBitmap = 0;
static int glWidth = 0;
static int glHeight = 0;

static const char *xsprintf(const char *format, ...)
{
	static char errorMessage[256];
	va_list va;

	va_start(va, format);
	vsprintf(errorMessage, format, va);
	va_end(va);
	return errorMessage;
}

static int usage()
{
	std::cout << "usage:" << std::endl;
	std::cout << "  scummfont {i|o} font bitmap.bmp" << std::endl;
	std::cout << std::endl;
	std::cout << "  o: Export bitmap.bmp from font" << std::endl;
	std::cout << "  i: Import bitmap.bmp into font (result written in font-new)" << std::endl;
	std::cout << std::endl;
	std::cout << "\"font\" is either a CHAR block extracted with scummrp, or an LFL file" << std::endl;
	return 0;
}

static void getFontInfo(int &baseOffset, std::ifstream &file, int &version, int &bpp,
						int &maxHeight, int &maxWidth, int &bytesPerChar, short &numChars)
{
	int i;
	int tag;
	int lineSpacing;

	lineSpacing = 0;
	file.exceptions(std::ios::eofbit | std::ios::failbit | std::ios::badbit);
	file.read((char *)&tag, 4);
	baseOffset = tag == MKTAG4('R','A','H','C') ? 8 : 0;
	file.seekg(baseOffset + 0x04, std::ios::beg);
	if (file.get() != 'c')
		throw std::runtime_error("Not a scumm font");
	version = file.get();
	if (version != 1 && version != 3)
		throw std::runtime_error(xsprintf("Unsupported version: %i", version));
	if (version == 1)
	{
		bpp = 1;
		maxWidth = 8;
		maxHeight = 8;
		numChars = (short)file.get();
		bytesPerChar = file.get();
	}
	else
	{
#ifdef SCUMMFONT_MAKETABLE
		std::ofstream o("table", std::ios::binary | std::ios::out | std::ios::trunc);
#endif
		file.seekg(baseOffset + 0x15, std::ios::beg);
		bpp = file.get();
		lineSpacing = file.get();
		file.read((char *)&numChars, 2);
		bytesPerChar = 8;
		maxWidth = maxHeight = 0;
		for (i = 0; i < numChars; ++i)
		{
			int offset;
			int width, height;

#ifdef SCUMMFONT_MAKETABLE
			file.seekg(baseOffset + 0x19 + i * 4, std::ios::beg);
			file.read((char *)&offset, 4);
			if (offset == 0)
			{
				o << 0 << ", ";
			}
			else
			{
				file.seekg(baseOffset + 0x15 + offset, std::ios::beg);
				width = file.get();
				height = file.get();
				if (width > maxWidth)
					maxWidth = width;
				if (height > maxHeight)
					maxHeight = height;
				o << width << ", ";
			}
#else
			file.seekg(baseOffset + 0x19 + i * 4, std::ios::beg);
			file.read((char *)&offset, 4);
			file.seekg(baseOffset + 0x15 + offset, std::ios::beg);
			width = file.get();
			height = file.get();
			if (width > maxWidth)
				maxWidth = width;
			if (height > maxHeight)
				maxHeight = height;
#endif
		}
		if (maxHeight < lineSpacing)
			maxHeight = lineSpacing;
	}
	if ((unsigned int)bytesPerChar < 8 || bpp == 0 || bpp == 3 || (unsigned int)bpp > 4
		|| (unsigned short)numChars > 0x100 || maxHeight < 0 || maxWidth < 0)
		throw std::runtime_error("Your font is strange...");
}

static const char *tmpPath(const char *path)
{
	static std::string s(path);

	s += "-new";
	return s.c_str();
}

static int roundTo4(int i)
{
	if ((i & 3) != 0)
		return i + 4 - (i & 3);
	return i;
}

static void saveBmp(const char *path)
{
	std::ofstream file(path, std::ios::binary | std::ios::out | std::ios::trunc);
	unsigned int udw;
	int sdw;
	unsigned short w;
	unsigned char *buf;

	if (!file.is_open())
		throw std::runtime_error("Cannot open BMP file");
	file.write("BM", 2);
	udw = 0x436 + roundTo4(glWidth) * glHeight;
	file.write((char *)&udw, 4);
	udw = 0;
	file.write((char *)&udw, 4);
	udw = 0x436;
	file.write((char *)&udw, 4);
	udw = 0x28;
	file.write((char *)&udw, 4);
	sdw = glWidth;
	file.write((char *)&sdw, 4);
	sdw = glHeight;
	file.write((char *)&sdw, 4);
	w = 1;
	file.write((char *)&w, 2);
	w = 8;
	file.write((char *)&w, 2);
	udw = 0;
	file.write((char *)&udw, 4);
	udw = roundTo4(glWidth) * glHeight;
	file.write((char *)&udw, 4);
	sdw = 0;
	file.write((char *)&sdw, 4);
	sdw = 0;
	file.write((char *)&sdw, 4);
	udw = 256;
	file.write((char *)&udw, 4);
	udw = 256;
	file.write((char *)&udw, 4);
	file.write((char *)glPalette, 0x400);
	buf = new unsigned char[roundTo4(glWidth) * glHeight];
	memset(buf, 0, roundTo4(glWidth) * glHeight);
	for (int i = 0; i < glHeight; ++i)
		memcpy(buf + i * roundTo4(glWidth), glFontBitmap + (glHeight - i - 1) * glWidth, glWidth);
	file.write((char *)buf, roundTo4(glWidth) * glHeight);
}

static void loadBmp(const char *path)
{
	std::ifstream file(path, std::ios::binary | std::ios::in);
	unsigned int udw;
	int sdw;
	unsigned short w;
	unsigned char *buf;
	unsigned int off;

	if (!file.is_open())
		throw std::runtime_error("Cannot open BMP file");
	file.read((char *)&w, 2);
	if (w != MKTAG2('M','B'))
		throw std::runtime_error("This is not a BMP file");
	file.seekg(8, std::ios::cur);
	file.read((char *)&off, 4);
	if (off < 0x36)
		throw std::runtime_error("This is not a valid BMP file");
	file.read((char *)&udw, 4);
	if (udw != 0x28)
		throw std::runtime_error(xsprintf("This is not a BMPv3 file: version 0x%x was found", udw));
	file.read((char *)&sdw, 4);
	glWidth = sdw;
	file.read((char *)&sdw, 4);
	glHeight = sdw;
	if (glWidth <= 0 || glHeight <= 0)
		throw std::runtime_error(xsprintf("Unsupported \"%i\" per \"%i\" width/height", glWidth, glHeight));
	file.read((char *)&w, 2);
	if (w != 1)
		throw std::runtime_error(xsprintf("This is not a single-plane BMP file: %hu planes found", w));
	file.read((char *)&w, 2);
	if (w != 8)
		throw std::runtime_error(xsprintf("This is not an 8bpp BMP file: %hubpp found", w));
	file.read((char *)&udw, 4);
	if (udw != 0)
		throw std::runtime_error(xsprintf("This BMP file must be uncompressed, but \"%u\" compression was found", udw));
	file.seekg(off, std::ios::beg);
	buf = new unsigned char[roundTo4(glWidth) * glHeight];
	glFontBitmap = new unsigned char[glWidth * glHeight];
	file.read((char *)buf, roundTo4(glWidth) * glHeight);
	for (int i = 0; i < glHeight; ++i)
		memcpy(glFontBitmap + i * glWidth, buf + (glHeight - i - 1) * roundTo4(glWidth), glWidth);
}

static void saveFont(const char *path)
{
	std::ifstream file(path, std::ios::in | std::ios::binary);
	int i, j, k, version, bpp, maxHeight, maxWidth, bytesPerChar;
	short numChars;
	int newNumChars;
	int baseOffset, endOffset;

	if (!file.is_open())
		throw std::runtime_error("Cannot open font file");
	getFontInfo(baseOffset, file, version, bpp, maxHeight, maxWidth, bytesPerChar, numChars);
	if (glHeight < maxHeight * numChars || glWidth < maxWidth || (glWidth != maxWidth && version == 1)) // (1)
		throw std::runtime_error("Wrong resolution");
	if (glWidth > maxWidth)
		maxWidth = glWidth;
	newNumChars = glHeight / maxHeight;
	if (newNumChars > 0x100)
		newNumChars = version == 1 ? 0xFF : 0x100;
	{
		std::ofstream tmpFile(tmpPath(path), std::ios::binary | std::ios::out | std::ios::trunc);
		char buffer[0x440];

		if (!tmpFile.is_open())
			throw std::runtime_error("Cannot open new font file");
		file.seekg(0, std::ios::beg);
		memset(buffer, 0, sizeof buffer);
		if (version == 3)
			endOffset = baseOffset + 0x19 + 4 * numChars;
		else
			endOffset = baseOffset + 0x8 + numChars;
		file.read(buffer, endOffset);
		endOffset += (version == 3) ? (newNumChars - numChars) * 4 : newNumChars - numChars; // positive thanks to (1)
		tmpFile.write(buffer, endOffset);
		for (i = 0; i < newNumChars; ++i)
		{
			unsigned char b, p;
			int width, height;

			width = height = 0;
			for (k = maxWidth - 1; k >= 0; --k)
			{
				for (j = 0; j < maxHeight; ++j)
					if ((unsigned char)(glFontBitmap[k + maxWidth * (j + maxHeight * i)] + 1) > 1)
					{
						width = k + 1;
						break;
					}
				if (width != 0)
					break;
			}
			if (version > 1)
				for (j = maxHeight - 1; j >= 0; --j)
				{
					for (k = 0; k < width; ++k)
						if ((unsigned char)(glFontBitmap[k + maxWidth * (j + maxHeight * i)] + 1) > 1)
						{
							height = j + 1;
							break;
						}
					if (height != 0)
						break;
				}
			else
				height = maxHeight;
			if (version == 1)
			{
				tmpFile.seekp(baseOffset + 0x8 + i, std::ios::beg);
				tmpFile.put(width);
				width = maxWidth;
				tmpFile.seekp(baseOffset + 0x8 + newNumChars + i * bytesPerChar, std::ios::beg);
			}
			else
			{
				int offset;
				int x, y;

				if (width != 0 && height != 0)
				{
					x = y = 0;
					if (i < numChars)
					{
						file.seekg(baseOffset + 0x19 + i * 4, std::ios::beg);
						file.read((char *)&offset, 4);
						if (offset != 0)
						{
							file.seekg(baseOffset + 0x15 + offset + 2, std::ios::beg);
							x = file.get();
							y = file.get();
						}
					}
					offset = endOffset - baseOffset - 0x15;
					tmpFile.seekp(baseOffset + 0x19 + i * 4, std::ios::beg);
					tmpFile.write((char *)&offset, 4);
					tmpFile.seekp(endOffset, std::ios::beg);
					tmpFile.put(width);
					tmpFile.put(height);
					tmpFile.put(x);
					tmpFile.put(y);
					endOffset += 4;
				}
				else
				{
					offset = 0;
					tmpFile.seekp(baseOffset + 0x19 + i * 4, std::ios::beg);
					tmpFile.write((char *)&offset, 4);
					continue;
				}
			}
			b = p = 0;
			for (j = 0; j < height; ++j)
				for (k = 0; k < width; ++k)
				{
					int l = k + maxWidth * (j + maxHeight * i);

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
			tmpFile.put((char)((endOffset >> 0x18) & 0xFF));
			tmpFile.put((char)((endOffset >> 0x10) & 0xFF));
			tmpFile.put((char)((endOffset >> 0x8) & 0xFF));
			tmpFile.put((char)(endOffset & 0xFF));
		}
		if (version == 1)
		{
			if (numChars != newNumChars)
			{
				tmpFile.seekp(baseOffset, std::ios::beg);
				++endOffset;
				tmpFile.write((char *)&endOffset, 4);
				tmpFile.seekp(baseOffset + 0x6, std::ios::beg);
				tmpFile.put(newNumChars);
			}
		}
		else
		{
			tmpFile.seekp(baseOffset, std::ios::beg);
			endOffset -= 0xF + baseOffset;
			tmpFile.write((char *)&endOffset, 4);
			tmpFile.seekp(baseOffset + 0x17, std::ios::beg);
			numChars = newNumChars;
			tmpFile.write((char *)&numChars, 2);
		}
	}
}

static void loadFont(const char *path)
{
	std::ifstream file(path, std::ios::in | std::ios::binary);
	int i, j, k, version, bpp, maxHeight, maxWidth, bytesPerChar;
	short numChars;
	int baseOffset;

	if (!file.is_open())
		throw std::runtime_error("Cannot open font file");
	getFontInfo(baseOffset, file, version, bpp, maxHeight, maxWidth, bytesPerChar, numChars);
	delete[] glFontBitmap;
	glFontBitmap = 0;
#ifdef SCUMMFONT_256
	glFontBitmap = new unsigned char[128 * 128];
	memset(glFontBitmap, 0, 128 * 128);
#else
	glFontBitmap = new unsigned char[maxWidth * maxHeight * numChars];
	memset(glFontBitmap, 0, maxWidth * maxHeight * numChars);
#endif
	if (bpp == 1)
	{
		glPalette[4] = 0x00;
		glPalette[5] = 0x00;
		glPalette[6] = 0x00;
		glPalette[8] = 0xFF;
		glPalette[9] = 0xFF;
		glPalette[10] = 0x00;
	}
#ifdef SCUMMFONT_256
	glHeight = 128;
	glWidth = 128;
#else
	glHeight = maxHeight * numChars;
	glWidth = maxWidth;
#endif
	for (i = 0; i < numChars; ++i)
	{
		unsigned char b, mask, p;
		int width, height;

#ifdef SCUMMFONT_256
		for (j = 0; j < maxHeight; ++j)
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
			int offset;

			file.seekg(baseOffset + 0x19 + i * 4, std::ios::beg);
			file.read((char *)&offset, 4);
			if (offset == 0)
				continue;
			file.seekg(baseOffset + 0x15 + offset, std::ios::beg);
			width = file.get();
			height = file.get();
			file.seekg(2, std::ios::cur);
		}
		mask = b = p = 0;
		for (j = 0; j < height; ++j)
		{
			for (k = 0; k < width; ++k)
			{
				glFontBitmap[k + maxWidth * (j + maxHeight * i)] = 0xFD ^ (i & 1);
				mask >>= bpp;
				p += bpp;
				if (mask == 0)
				{
					mask = ((1 << bpp) - 1) << (8 - bpp);
					b = (unsigned char)file.get();
					p = 0;
				}
				if ((b & mask) != 0)
					glFontBitmap[k + maxWidth * (j + maxHeight * i)] = (b & mask) >> (8 - bpp - p);
			}
			if (version == 1)
				mask = b = p = 0;
		}
	}
}

int main(int argc, char **argv) try
{
	if (argc < 4)
		return usage();
	if (argv[1][0] == 'i')
	{
		loadBmp(argv[3]);
		saveFont(argv[2]);
	}
	else if (argv[1][0] == 'o')
	{
		loadFont(argv[2]);
		saveBmp(argv[3]);
	}
	else
		return usage();
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
