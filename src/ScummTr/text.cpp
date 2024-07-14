/*
 * SPDX-License-Identifier: MIT
 *
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

#include "common/toolbox.hpp"
#include "common/io.hpp"
#include "ScummRp/block.hpp" // for tagToStr
#include "ScummRp/scummrp.hpp"

#include "text.hpp"

const char Text::CT_NULL[256] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	' ', '!', '"', '#', '$', '%', '&', '\'', '(', ')', '*', '+', ',', '-', '.', '/',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?',
	'@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
	'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '[', '\\', ']', '^', '_',
	'`', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
	'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '{', '|', '}', '~', 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

const char Text::CT_V1EN[256] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	' ', '!', '"', '#', '$', '\x85', '&', '\'', '(', ')', '*', '+', ',', '-', '.', '/', // XXX: \x85
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '\xa9', '>', '?',
	'@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
	'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '[', '\xa3', ']', '^', 0,
	'"', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
	'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

const char Text::CT_V1DE[256] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	' ', '!', '"', '#', '$', '%', '&', '\'', '(', ')', '*', '+', ',', '-', '.', '/',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '\xa9', '>', '?',
	'@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
	'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '\xfc', '\xe4', '\xdc', '^', 0,
	'"', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
	'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '\xf6', '\xc4', '\xd6', '\xdf', 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

const char Text::CT_V1FR[256] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	' ', '!', '"', '#', '$', '%', '&', '\'', '(', ')', '*', '+', ',', '-', '.', '/',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '\xe0', '\xe2', '\xe7', '?',
	'@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
	'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '\xe9', '\xe8', '\xea', '^', '\xef',
	'"', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
	'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '\xee', '\xf4', '\xf9', '\xfb', 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

const char Text::CT_V1IT[256] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	' ', '!', '"', '#', '$', '%', '&', '\'', '(', ')', '*', '+', ',', '-', '.', '/',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '\xe0', '\xe1', '\xe8', '?',
	'@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
	'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '\xe9', 0, '\xea', '^', '\xef',
	'"', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
	'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '\xec', '\xf2', '\xf9', '\xfb', 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

const char Text::CT_V3ANSI[256] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, '\xa9', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	' ', '!', '"', '#', '$', '%', '&', '\'', '(', ')', '*', '+', ',', '-', '.', '/',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?',
	'@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
	'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '[', '\\', ']', '^', '_',
	'"', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
	'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '{', '|', '}', '~', 0,
	'\xc7', '\xfc', '\xe9', '\xe2', '\xe4', '\xe0', 0, '\xe7', '\xea', '\xeb', '\xe8', '\xef', '\xee', '\xec', '\xc4', 0,
	'\xc9', '\xe6', '\xc6', '\xf4', '\xf6', '\xf2', '\xfb', '\xf9', 0, '\xd6', '\xdc', 0, 0, 0, 0, 0,
	'\xe1', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, '\xdf', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

const char *const Text::CHARSETS[] =
{
	Text::CT_NULL,
	Text::CT_V3ANSI,
	Text::CT_V1EN,
	Text::CT_V1DE,
	Text::CT_V1IT,
	Text::CT_V1FR
};

Text::Text(const char *path, int flags, Text::Charset charset) :
    _file(path, (flags & Text::TXT_OUT) != 0 ? (std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc) : (std::ios::in | std::ios::binary)),
    _cur(0), _lineCount(0), _lflfId(-1), _tag(0), _id(-1),
    _binary((flags & Text::TXT_BINARY) != 0), _comments((flags & Text::TXT_NO_COMMENT) == 0),
    _crlf((flags & Text::TXT_CRLF) != 0),
    _header((flags & Text::TXT_HEADER) != 0), _hex((flags & Text::TXT_HEXA) != 0),
    _opcode((flags & Text::TXT_OPCODE) != 0),
    _rawText((flags & Text::TXT_RAW) != 0),
    _charset(Text::CHARSETS[(flags & Text::TXT_CHARSET1252) != 0 ? (int)charset : (int)Text::CHS_NULL])
{
	if (!_file.is_open())
		throw File::IOError(xsprintf("Cannot open %s", path));

	for (int i = 0; i < 256; ++i)
		_finalCharset[i] = (char)(byte)i;

	for (int i = 0; i < 256; ++i)
		_finalCharset[(byte)_charset[i]] = (char)(byte)i;
}

Text::~Text()
{
}

void Text::setInfo(int lflfId, uint32 tag, int id)
{
	_lflfId = lflfId;
	_tag = tag;
	_id = id;
}

const char *Text::info() const
{
	return xsprintf("[%.3i:%s#%.4i]", _lflfId, Block::tagToStr(_tag), _id);
}

const char *Text::internalCommentHeader() const
{
	const char *eol = (_crlf) ? "\r\n" : "\n";
	const char *encoding = (_rawText) ? "RAW" : "Windows-1252/ISO-8859-1 (\xa1""caf\xe9-pi\xf1""ata Stra\xdf""e!)";

	return xsprintf(
	    ";; ScummTR note: every line starting with this prefix is ignored%s"
	    ";; ScummTR note: file encoding is %s%s",
	    eol, encoding, eol);
}

int32 Text::lineNumber() const
{
	return _lineCount;
}

void Text::firstLine()
{
	_cur = 0;
	_lineCount = 0;
}

int Text::funcLen(byte c)
{
	switch (c)
	{
	case 0x01:
	case 0x02:
	case 0x03:
	case 0x08:
		return 0;
	case 0x04:
	case 0x05:
	case 0x06:
	case 0x07:
	case 0x09:
	case 0x0A:
	case 0x0C:
	case 0x0D:
	case 0x0E:
		return 2;
	default:
		throw Text::Error(xsprintf("Unknown function id 0x%X", c));
	}
}

void Text::_writeChar(byte b)
{
	char c;

	c = (_rawText) ? (char)b : _charset[b];

	if ((!_rawText && c == '\0') || (_rawText && b < 0x20))
		_writeEscChar(b);
	else if (c == '\\')
		_file.write("\\\\", 2);
	else
		_file.write(&c, 1);
}

void Text::_writeEscChar(byte b)
{
	char c;

	if (_hex)
	{
		_file.write("\\x", 2);
		c = (char)(b / 16) + '0';
		if (c > '9')
			c += 'A' - '9' - 1;
		_file.write(&c, 1);

		b %= 16;
		c = (char)b + '0';
		if (c > '9')
			c += 'A' - '9' - 1;
		_file.write(&c, 1);
	}
	else
	{
		_file.write("\\", 1);
		c = (char)(b / 100) + '0';
		_file.write(&c, 1);

		b %= 100;
		c = (char)(b / 10) + '0';
		_file.write(&c, 1);

		b %= 10;
		c = (char)b + '0';
		_file.write(&c, 1);
	}
}

void Text::_writeEscPlain(const std::string &s)
{
	size_t size;

	size = s.size();
	for (size_t i = 0; i < size; ++i)
		_writeChar((byte)s[i]);
}

void Text::_writeEscRsc(const std::string &s)
{
	size_t size, countdown;

	countdown = 0;
	size = s.size();
	for (size_t i = 0; i < size; ++i)
	{
		byte b = (byte)s[i];
		if (countdown > 0)
		{
			_writeEscChar(b);
			--countdown;
		}
		else if (b == 0xFF)
		{
			if (ScummRp::game.id == GID_INDY3 && i < size - 1)
			{
				byte nextByte = (byte)s[i + 1];
				if (nextByte == 0x20)
				{
					ScummIO::warning("Fixing likely misencoded German Eszett character for Indy3");
					b = 0xE1;
					_writeChar(b);
					continue;
				}
			}

			_writeEscChar(b);
			countdown = 3;
		}
		else if (b == 0xFE)
		{
			_writeEscChar(b);
			countdown = 1;
		}
		else
		{
			_writeChar(b);
		}
	}
}

void Text::_writeEscOldMsg(const std::string &s)
{
	size_t size, countdown;

	countdown = 0;
	size = s.size();
	for (size_t i = 0; i < size; ++i)
	{
		if (countdown > 0)
		{
			_writeEscChar((byte)s[i]);
			--countdown;
		}
		else if (s[i] < 8 && s[i] > 3)
		{
			_writeEscChar((byte)s[i]);
			countdown = 1;
		}
		else
		{
			_writeChar((byte)s[i]);
		}
	}
}

void Text::_writeEscMsg(const std::string &s)
{
	bool func;
	size_t size, countdown;

	func = false;
	countdown = 0;
	size = s.size();
	for (size_t i = 0; i < size; ++i)
	{
		byte b = (byte)s[i];
		if (countdown > 0)
		{
			_writeEscChar(b);
			--countdown;
		}
		else if (func)
		{
			_writeEscChar(b);
			countdown = Text::funcLen(b);
			func = false;
		}
		else if (b == 0xFF || b == 0xFE)
		{
			// Ignore badly-encoded German Eszett (0xFF instead of 0xE1) in Indy3 German.
			// This is a bug in the original and official German releases of the game.
			// See issue #58, https://bugs.scummvm.org/ticket/1675, and
			// https://bugs.scummvm.org/ticket/2715.
			if (ScummRp::game.id == GID_INDY3 && b == 0xFF && i < size - 1)
			{
				byte nextByte = (byte)s[i + 1];
				if (nextByte == 0x2E || nextByte == 0x20)
				{
					ScummIO::warning("Fixing likely misencoded German Eszett character for Indy3");
					b = 0xE1;
					_writeChar(b);
					continue;
				}
			}
			_writeEscChar(b);
			func = true;
		}
		else
		{
			_writeChar(b);
		}
	}
}

void Text::_writeEsc(const std::string &s, Text::LineType t)
{
	switch (t)
	{
	case LT_PLAIN:
		_writeEscPlain(s);
		break;
	case LT_RSC:
		_writeEscRsc(s);
		break;
	case LT_MSG:
		_writeEscMsg(s);
		break;
	case LT_OLDMSG:
		_writeEscOldMsg(s);
		break;
	}

	if (_crlf)
		_file.write("\r\n", 2);
	else
		_file.write("\n", 1);
}

void Text::_checkMsg(const std::string &s, int l)
{
	size_t size, countdown;
	bool func;

	func = false;
	countdown = 0;
	size = s.size();
	for (size_t i = 0; i < size; ++i)
	{
		if (countdown > 0)
		{
			--countdown;
		}
		else if (func)
		{
			countdown = Text::funcLen((byte)s[i]);
			func = false;
		}
		else if ((byte)s[i] == 0xFF || (byte)s[i] == 0xFE)
		{
			func = true;
		}
		else if (s[i] == '\0')
		{
			throw Text::Error(xsprintf("NULL char in line %i", l));
		}
	}

	if (countdown > 0 || func)
		throw Text::Error(xsprintf("Truncated function in line %i (msg)", l));
}

void Text::_checkRsc(const std::string &s, int l)
{
	size_t size, countdown;

	countdown = 0;
	size = s.size();
	for (size_t i = 0; i < size; ++i)
	{
		if (countdown > 0)
			--countdown;
		else if ((byte)s[i] == 0xFF)
			countdown = 3;
		else if ((byte)s[i] == 0xFE)
			countdown = 1;
		else if (s[i] == '\0')
			throw Text::Error(xsprintf("NULL char in line %i", l));
	}

	if (countdown > 0)
		throw Text::Error(xsprintf("Truncated function in line %i (rsc)", l));
}

void Text::_checkOldMsg(const std::string &s, int l)
{
	size_t size, countdown;

	countdown = 0;
	size = s.size();
	for (size_t i = 0; i < size; ++i)
	{
		if (countdown > 0)
			--countdown;
		else if (s[i] < 8 && s[i] > 3)
			countdown = 1;
		else if (s[i] == '\0')
			throw Text::Error(xsprintf("NULL char in line %i", l));
	}

	if (countdown > 0)
		throw Text::Error(xsprintf("Truncated function in line %i (oldmsg)", l));
}

void Text::_checkPlain(const std::string &s, int l)
{
	size_t size;

	size = s.size();
	for (size_t i = 0; i < size; ++i)
		if (s[i] == '\0')
			throw Text::Error(xsprintf("NULL char in line %i", l));
}

void Text::_unEsc(std::string &s, Text::LineType t) const
{
	size_t size;
	size_t i, j;

	size = s.size();
	for (j = i = 0; i < size; ++i)
	{
		if (s[i] != '\\')
		{
			if (_rawText)
				s[j++] = (char)(byte)s[i];
			else
				s[j++] = _finalCharset[(byte)s[i]];
		}
		else
		{
			if (i + 1 >= size || (i + 3 >= size && s[i + 1] != '\\'))
				throw Text::Error(xsprintf("Truncated escaping in line %i", _lineCount));

			if (s[++i] == '\\')
			{
				s[j++] = '\\';
			}
			else
			{
				byte a, b, c;
				unsigned int n, base;

				if (s[i] == 'x')
				{
					base = 16;

					a = 0;

					b = (byte)((s[++i] | 0x20) - '0');
					if (b > 9)
						b -= 'a' - '9' - 1;

					c = (byte)((s[++i] | 0x20) - '0');
					if (c > 9)
						c -= 'a' - '9' - 1;

					n = b * 16 + c;
				}
				else
				{
					base = 10;

					a = (byte)(s[i] - '0');
					b = (byte)(s[++i] - '0');
					c = (byte)(s[++i] - '0');

					n = a * 100 + b * 10 + c;
				}

				if ((unsigned int)n >= 0x100 || a >= base || b >= base || c >= base)
					throw Text::Error(xsprintf("Bad escaping in line %i", _lineCount));

				s[j++] = (char)n;
			}
		}
	}

	s.resize(j);

	switch (t)
	{
	case LT_PLAIN:
		_checkPlain(s, _lineCount);
		break;
	case LT_RSC:
		_checkRsc(s, _lineCount);
		break;
	case LT_MSG:
		_checkMsg(s, _lineCount);
		break;
	case LT_OLDMSG:
		_checkOldMsg(s, _lineCount);
		break;
	}
}

int Text::getLineLength(FileHandle &f, Text::LineType t)
{
	switch (t)
	{
	case LT_PLAIN:
		return Text::getLengthPlain(f);
	case LT_RSC:
		return Text::getLengthRsc(f);
	case LT_MSG:
		return Text::getLengthMsg(f);
	case LT_OLDMSG:
		return Text::getLengthOldMsg(f);
	}
	throw std::logic_error("Text::getLineLength: Wrong type");
}

int Text::getLengthRsc(FileHandle &f)
{
	byte b;
	int32 start;

	start = f->tellg(std::ios::beg);
	while (!f->eof() && f->getByte(b) != 0)
	{
		if (b == 0xFF)
			f->seekg(3, std::ios::cur);
		else if (b == 0xFE)
			f->seekg(1, std::ios::cur);
	}

	return f->tellg(std::ios::beg) - start - 1;
}

int Text::getLengthOldMsg(FileHandle &f)
{
	byte b;
	int32 start;

	start = f->tellg(std::ios::beg);
	while (!f->eof() && f->getByte(b) != 0)
		if (b < 8 && b > 3)
			f->getByte(b);

	return f->tellg(std::ios::beg) - start - 1;
}

int Text::getLengthMsg(FileHandle &f)
{
	byte b, nextByte;
	int32 start;
	int i;

	start = f->tellg(std::ios::beg);
	while (!f->eof() && f->getByte(b) != 0)
	{
		if (b == 0xFF || b == 0xFE)
		{
			f->getByte(nextByte);

			// Ignore badly encoded German Eszett characters that would be interpreted
			// as a \255 escape sequence. See also Text::_writeEscMsg() comment above.
			if (ScummRp::game.id == GID_INDY3 && b == 0xFF && (nextByte == 0x2E || nextByte == 0x20))
			{
				ScummIO::info(INF_DETAIL, xsprintf("Ignoring 0x%.2X%.2X sequence in Indy3 (likely a bogus German Eszett)", b, nextByte));
				continue;
			}

			i = Text::funcLen(nextByte);
			f->seekg(i, std::ios::cur);
		}
	}

	return f->tellg(std::ios::beg) - start - 1;
}

int Text::getLengthPlain(FileHandle &f)
{
	byte b;
	int32 start;

	start = f->tellg(std::ios::beg);
	while (!f->eof() && f->getByte(b) != 0)
		;

	return f->tellg(std::ios::beg) - start - 1;
}

void Text::_getBinaryLine(std::string &s, Text::LineType lineType)
{
	int l;

	switch (lineType)
	{
	case LT_OLDMSG:
		l = Text::getLengthOldMsg(_file);
		break;
	case LT_MSG:
		l = Text::getLengthMsg(_file);
		break;
	case LT_RSC:
		l = Text::getLengthRsc(_file);
		break;
	case LT_PLAIN:
		l = Text::getLengthPlain(_file);
		break;
	default:
		throw std::logic_error("Text::_getBinaryLine: Wrong type");
	}

	_file->read(s, l);
}

void Text::_spaceCharToBit(std::string &s) const
{
	size_t last;
	size_t i, j;

	last = s.size() - 1;
	for (j = i = 0; i < last; ++i)
	{
		if ((byte)s[i] & 0x80)
			throw Text::Error(xsprintf("char > 0x80 in line %i", _lineCount));

		if (s[i + 1] == ' ' && !(s[i] > 3 && s[i] < 8))
		{
			s[j++] = (char)((byte)s[i++] | 0x80);
		}
		else
		{
			s[j++] = s[i];
			if (s[i] < 8 && s[i] > 3 && i + 1 < last)
				s[j++] = s[++i];
		}
	}

	if (i < s.size())
		s[j++] = s[i];

	s.resize(j);
}

void Text::_spaceBitToChar(std::string &s) const
{
	std::string s2;
	size_t size;

	size = s.size();
	s2.reserve(2 * size);
	for (size_t i = 0; i < size; ++i)
	{
		if ((byte)s[i] & 0x80)
		{
			s2 += s[i] & 0x7F;
			s2 += ' ';
		}
		else
		{
			s2 += s[i];
			if (s[i] < 8 && s[i] > 3 && i + 1 < size)
				s2 += s[++i];
		}
	}

	s = s2;
}

bool Text::nextLine(std::string &s, Text::LineType lineType)
{
	if (_cur >= _file.size())
	{
		s.resize(0);
		return false;
	}

	_file.seekg(_cur, std::ios::beg);
	if (_binary)
	{
		_getBinaryLine(s, lineType);
	}
	else
	{
		_file.getline(s, '\n');

		// Ignore lines starting with an internal comment
		if (s.find(";; ScummTR note: ", 0) == 0)
		{
			++_lineCount;
			_cur = _file.tellg(std::ios::beg);
			return Text::nextLine(s, lineType);
		}

		if (_header && s[0] == '[')
		{
			size_t endHeader = s.find(']', 0);
			if (endHeader != std::string::npos)
				s.erase(0, endHeader + 1);
		}

		if (_opcode && s[0] == '(')
		{
			size_t endPos = s.find(')', 0);
			if (endPos != std::string::npos)
				s.erase(0, endPos + 1);
		}

		if (_crlf)
			s.resize(s.size() - 1);

		_unEsc(s, lineType);
	}

	if (s.size() == 0)
		throw Text::Error(xsprintf("Empty lines are forbidden (line %i)", _lineCount));

	if (lineType == Text::LT_OLDMSG)
		Text::_spaceCharToBit(s);

	++_lineCount;
	_cur = _file.tellg(std::ios::beg);

	return true;
}

void Text::addExportHeaders()
{
	if (_comments && !_binary)
		_file.write(internalCommentHeader());
}

void Text::addLine(std::string s, Text::LineType lineType, int op)
{
	if (s.size() == 0) // empty lines are ignored
		return;

	if (lineType == Text::LT_OLDMSG)
		Text::_spaceBitToChar(s);

	_file.seekp(0, std::ios::end);

	if (_binary)
	{
		_file.write(s);
		_file->putByte((byte)0);
	}
	else
	{
		if (_header)
			_file.write(info());

		if (_opcode)
		{
			if (op >= 0)
				_file.write(xsprintf("(%.2X)", op));
			else
				_file.write(xsprintf("(__)"));
		}

		_writeEsc(s, lineType);
	}
}

void Text::clear()
{
	_file.truncate(0);
	firstLine();
}
