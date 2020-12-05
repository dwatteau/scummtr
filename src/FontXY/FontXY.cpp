/*
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
 * Note: the last "official" binary was built on: 2006-02-19 16:25:03.
 */

#include "common/types.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <list>
#include <string>
#include <vector>

int main(int argc, char **argv)
{
	if (argc != 3 || argv[1][0] == 0 || argv[1][1] != 0 || ((argv[1][0] | 0x20) != 'i' && (argv[1][0] | 0x20) != 'o'))
	{
		std::cout << "FontXY 0.1 by Thomas Combeleran\n" << std::endl;
		std::cout << "Usage: FontXY {i|o} <CHAR file>\n\nExamples:\tFontXY o CHAR_0002\n\t\tFontXY i CHAR_0003" << std::endl;
		return 0;
	}
	bool bImport = (argv[1][0] | 0x20) == 'i';
	char *pszChar = argv[2];
	char szTxt[] = "XY.txt"; // TODO: param

	if (bImport)
	{
		char szLine[1024];
		std::fstream fChar(pszChar, std::fstream::in | std::fstream::out | std::fstream::binary);
		std::ifstream fTxt(szTxt, std::ifstream::in);

		// Seek after the header
		fChar.seekg(8 + 0x17);

		int16 snNumChars; // <= 0x100
		fChar.read((char *)&snNumChars, 2);
		int nNumChars = snNumChars;
		if (nNumChars > 0x100 || nNumChars <= 0) { std::cerr << "Error" << std::endl; return 1; }

		for (int i = 0; i < nNumChars; ++i)
		{
			uint32 uOffset;
			fChar.seekg(8 + 0x19 + i * 4);
			fChar.read((char *)&uOffset, 4);

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
				fChar.seekp(8 + 0x15 + uOffset + 2);
				fChar.write((char *)&byLeft, 1);
				fChar.write((char *)&byTop, 1);
			}
		}
	}
	else
	{
		std::ifstream fChar(pszChar, std::ifstream::in | std::ifstream::binary);
		std::ofstream fTxt(szTxt, std::ofstream::out | std::ofstream::trunc);

		// Seek after the header
		fChar.seekg(8 + 0x17);

		int16 snNumChars; // <= 0x100
		fChar.read((char *)&snNumChars, 2);
		int nNumChars = snNumChars;
		if (nNumChars > 0x100 || nNumChars <= 0) { std::cerr << "Error" << std::endl; return 1; }

		for (int i = 0; i < nNumChars; ++i)
		{
			uint32 uOffset;
			fChar.seekg(8 + 0x19 + i * 4);
			fChar.read((char *)&uOffset, 4);

			// Just add an empty line
			if (uOffset == 0)
			{
				fTxt << std::endl;
			}
			// Read the values, and write them in the TXT file
			else
			{
				fChar.seekg(8 + 0x15 + uOffset + 2);
				int8 byLeft;
				int8 byTop;
				fChar.read((char *)&byLeft, 1);
				fChar.read((char *)&byTop, 1);
				fTxt << (int)byLeft << ";" << (int)byTop << std::endl;
			}
		}
	}
}
