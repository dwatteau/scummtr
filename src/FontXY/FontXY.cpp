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

#include <algorithm>
#include <fstream>
#include <string>
#include <vector>
#include <list>
#include <iostream>

// TODO paramètres

int main(int argc, char **argv)
{
	if (argc != 3 || argv[1][0] == 0 || argv[1][1] != 0 || ((argv[1][0] | 0x20) != 'i' && (argv[1][0] | 0x20) != 'o'))
	{
		std::cout << "Usage: FontXY {i|o} <CHAR file>\n\nExamples:\tFontXY o CHAR_0002\n\t\tFontXY i CHAR_0003" << std::endl;
		return 0;
	}
	bool	bImport = (argv[1][0] | 0x20) == 'i';
	char	*pszChar = argv[2];
	char	szTxt[] = "XY.txt";

	if (bImport)
	{
		char		szLine[1024];
		// Ouvre le fichier CHAR
		std::fstream	fChar(pszChar, std::fstream::in | std::fstream::out | std::fstream::binary);
		// Ouvre le fichier TXT
		std::ifstream	fTxt(szTxt, std::ifstream::in);
		// Saute le header
		fChar.seekg(8 + 0x17);
		// Nombre de chars (<= 0x100)
		short	snNumChars;
		fChar.read((char *)&snNumChars, sizeof snNumChars);
		int		nNumChars = snNumChars;
		if (nNumChars > 0x100 || nNumChars <= 0) { std::cerr << "Error" << std::endl; return 1; }
		// Pour chaque char
		for (int i = 0; i < nNumChars; ++i)
		{
			// Offset
			unsigned int	uOffset;
			fChar.seekg(8 + 0x19 + i * 4);
			fChar.read((char *)&uOffset, sizeof uOffset);
			// Si offset == 0, saute une ligne
			if (uOffset == 0)
			{
				fTxt.getline(szLine, sizeof szLine, '\n');
			}
			// Sinon, lecture de la ligne "x,y" et mise à jour dans fichier CHAR
			else
			{
				int			nLeft, nTop;
				signed char	byLeft, byTop;
				fTxt >> nLeft;
				fTxt.getline(szLine, sizeof szLine, ';');
				fTxt >> nTop;
				fTxt.getline(szLine, sizeof szLine, '\n');
				byLeft = (signed char)nLeft;
				byTop = (signed char)nTop;
				fChar.seekp(8 + 0x15 + uOffset + 2);
				fChar.write((char *)&byLeft, sizeof byLeft);
				fChar.write((char *)&byTop, sizeof byTop);
			}
		}
	}
	else
	{
		// Ouvre le fichier CHAR
		std::ifstream	fChar(pszChar, std::ifstream::in | std::ifstream::binary);
		// Ouvre le fichier TXT
		std::ofstream	fTxt(szTxt, std::ofstream::out | std::ofstream::trunc);
		// Saute le header
		fChar.seekg(8 + 0x17);
		// Nombre de chars (<= 0x100)
		short	snNumChars;
		fChar.read((char *)&snNumChars, sizeof snNumChars);
		int		nNumChars = snNumChars;
		if (nNumChars > 0x100 || nNumChars <= 0) { std::cerr << "Error" << std::endl; return 1; }
		// Pour chaque char
		for (int i = 0; i < nNumChars; ++i)
		{
			// Offset
			unsigned int	uOffset;
			fChar.seekg(8 + 0x19 + i * 4);
			fChar.read((char *)&uOffset, sizeof uOffset);
			// Si offset == 0, saute une ligne
			if (uOffset == 0)
			{
				fTxt << std::endl;
			}
			// Sinon, lit valeurs, écrit dans le TXT
			else
			{
				fChar.seekg(8 + 0x15 + uOffset + 2);
				signed char	byLeft;
				signed char	byTop;
				fChar.read((char *)&byLeft, sizeof byLeft);
				fChar.read((char *)&byTop, sizeof byTop);
				fTxt << (int)byLeft << ";" << (int)byTop << std::endl;
			}
		}
	}
}
