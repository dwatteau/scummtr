/*
 * Copyright (c) 2003-2005 Thomas Combeleran
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

#ifndef __TEXT_HPP_
#define __TEXT_HPP_

#include "file.hpp"

#include <string>

// TODO separate OutputText/InputText

/*
 * Text
 */

class Text
{
public:
	enum Charset {
		CHS_NULL   = 0,
		CHS_V3ANSI = 1,
		CHS_V1EN   = 2,
		CHS_V1DE   = 3,
		CHS_V1IT   = 4,
		CHS_V1FR   = 5
	};
	enum {
		TXT_NULL       = 0,
		TXT_BINARY     = 1 << 0,
		TXT_HEXA       = 1 << 1,
		TXT_CRLF       = 1 << 2,
		TXT_HEADER     = 1 << 3,
		TXT_OUT        = 1 << 4,
		TXT_OPCODE     = 1 << 5,
		TXT_USECHARSET = 1 << 6
	};
	enum LineType { LT_PLAIN, LT_RSC, LT_MSG, LT_OLDMSG };
	class Error : public std::runtime_error
	{
	public:
		Error(const std::string &message) : std::runtime_error(message) { }
	};
private:
	static const char CT_NULL[256];
	static const char CT_V3ANSI[256];
	static const char CT_V1EN[256];
	static const char CT_V1DE[256];
	static const char CT_V1IT[256];
	static const char CT_V1FR[256];
	static const char *const CHARSETS[];
private:
	File _file;
	int32 _cur;
	int32 _lineCount;
	int _lflfId;
	uint32 _tag;
	int _id;
	bool _escaped;
	bool _crlf;
	bool _header;
	bool _hex;
	bool _opcode;
	const char *const _charset;
	char _tesrahc[256];
private:
	static void _checkMsg(const std::string &s, int l);
	static void _checkRsc(const std::string &s, int l);
	static void _checkOldMsg(const std::string &s, int l);
	static void _checkPlain(const std::string &s, int l);
public:
	static int funcLen(byte c);
	static int getLengthRsc(FileHandle &f);
	static int getLengthOldMsg(FileHandle &f);
	static int getLengthMsg(FileHandle &f);
	static int getLengthPlain(FileHandle &f);
	static int getLineLength(FileHandle &f, Text::LineType t);
private:
	void _writeEscPlain(const std::string &s);
	void _writeEscRsc(const std::string &s);
	void _writeEscOldMsg(const std::string &s);
	void _writeEscMsg(const std::string &s);
	void _writeEsc(const std::string &s, Text::LineType t);

	void _spaceCharToBit(std::string &s) const;
	void _spaceBitToChar(std::string &s) const;
	void _unEsc(std::string &s, Text::LineType lineType) const;
	void _writeChar(byte c);
	void _writeEscChar(byte c);
	void _getBinaryLine(std::string &s, Text::LineType lineType);
public:
	void setInfo(int lflfId, uint32 tag, int id);
	const char *info() const;
	int32 lineNumber() const;
	void firstLine();
	bool nextLine(std::string &s, Text::LineType lineType);
	void clear();
	void addLine(std::string s, Text::LineType lineType, int op = -1);
public:
	Text(const char *path, int flags, Text::Charset charset);
	~Text();
private:
	Text();
private:
	Text(const Text &);
	Text &operator=(const Text &);
};

#endif // !__TEXT_HPP_