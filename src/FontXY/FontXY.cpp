/*
 * SPDX-License-Identifier: MIT
 *
 * Copyright (c) 2006 Thomas Combeleran
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
 * Note: the last "original" binary was built on: 2006-02-19 16:25:03.
 */

#include "common/types.hpp"
#include "common/file.hpp"
#include "common/toolbox.hpp"

#include <clocale>
#include <cstdlib>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <list>
#include <string>
#include <vector>

int main(int argc, char **argv)
{
	std::setlocale(LC_CTYPE, "");

	if (argc != 3 || argv[1][0] == '\0' || argv[1][1] != '\0' || (argv[1][0] != 'i' && argv[1][0] != 'o'))
	{
		std::cout << "FontXY 0.6.0 (build " << SCUMMTR_BUILD_DATE << ") by Thomas Combeleran\n\n";
		std::cout << "Usage: FontXY {i|o} <CHAR file>\n\n";
		std::cout << "Examples:\tFontXY o CHAR_0002\n";
		std::cout << "         \tFontXY i CHAR_0003\n\n";
		printCommonDisclaimer();
		return 0;
	}

	bool bImport = argv[1][0] == 'i';
	char *pszChar = argv[2];
	char szTxt[] = "XY.txt"; // TODO: param

	if (bImport)
	{
		char szLine[1024];
		std::ifstream fTxt(szTxt, std::ifstream::in);
		File fChar;

		if (!fTxt.is_open())
		{
			std::cerr << "Error: cannot open " << szTxt << std::endl;
			return EXIT_FAILURE;
		}

		fChar.open(pszChar, std::ios::in | std::ios::out | std::ios::binary);
		if (!fChar.is_open())
		{
			std::cerr << "Error: cannot open " << pszChar << std::endl;
			return EXIT_FAILURE;
		}

		// Seek after the header
		fChar.seekg(8 + 0x17, std::ios::beg);

		int16 snNumChars; // <= 0x100
		fChar->getLE16(snNumChars);
		int nNumChars = snNumChars;
		if (nNumChars > 0x100 || nNumChars <= 0)
		{
			std::cerr << "Error: invalid number of chars: " << nNumChars << std::endl;
			return EXIT_FAILURE;
		}

		for (int i = 0; i < nNumChars; ++i)
		{
			uint32 uOffset;
			fChar.seekg(8 + 0x19 + i * 4, std::ios::beg);
			fChar->getLE32(uOffset);

			if (uOffset == 0)
			{
				fTxt.getline(szLine, sizeof szLine, '\n');
			}
			// Read the "x,y" line and update the CHAR file
			else
			{
				int nLeft, nTop;
				int8 byLeft, byTop;
				fTxt >> nLeft;
				fTxt.getline(szLine, sizeof szLine, ';');
				fTxt >> nTop;
				fTxt.getline(szLine, sizeof szLine, '\n');
				byLeft = (int8)nLeft;
				byTop = (int8)nTop;
				fChar.seekp(8 + 0x15 + uOffset + 2, std::ios::beg);
				fChar.write((char *)&byLeft, 1);
				fChar.write((char *)&byTop, 1);
			}
		}
		fTxt.close();
		fChar.close();
	}
	else
	{
		File fChar;
		std::ofstream fTxt(szTxt, std::ofstream::out | std::ofstream::trunc);

		if (!fTxt.is_open())
		{
			std::cerr << "Error: cannot open " << szTxt << std::endl;
			return EXIT_FAILURE;
		}

		fChar.open(pszChar, std::ios::in | std::ios::binary);
		if (!fChar.is_open())
		{
			std::cerr << "Error: cannot open " << pszChar << std::endl;
			return EXIT_FAILURE;
		}

		// Seek after the header
		fChar.seekg(8 + 0x17, std::ios::beg);

		int16 snNumChars; // <= 0x100
		fChar->getLE16(snNumChars);
		int nNumChars = snNumChars;
		if (nNumChars > 0x100 || nNumChars <= 0)
		{
			std::cerr << "Error: invalid number of chars: " << nNumChars << std::endl;
			return EXIT_FAILURE;
		}

		for (int i = 0; i < nNumChars; ++i)
		{
			uint32 uOffset;
			fChar.seekg(8 + 0x19 + i * 4, std::ios::beg);
			fChar->getLE32(uOffset);

			// Just add an empty line
			if (uOffset == 0)
			{
				fTxt << "\n";
			}
			// Read the values, and write them in the TXT file
			else
			{
				int8 byLeft;
				int8 byTop;
				fChar.seekg(8 + 0x15 + uOffset + 2, std::ios::beg);
				fChar.read((char *)&byLeft, 1);
				fChar.read((char *)&byTop, 1);
				fTxt << (int)byLeft << ";" << (int)byTop << "\n";
			}
		}
		fTxt.close();
		fChar.close();
	}
}
