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

#ifndef __SCRIPT_HPP_
#define __SCRIPT_HPP_

#include "text.hpp"
#include "file.hpp"
#include "toolbox.hpp"

#include <string>
#include <stdexcept>
#include <list>

#ifndef SCUMMTR_LAST_2005_CHANGE
// note: this was enabled in the last official release from 2003, but after
// some debugging in this area in 2004, it was disabled altogether on 2005-11-22.
// No official release was ever shipped without CHECK_SCRIPT_JUMPS.
#define CHECK_SCRIPT_JUMPS
#endif

/*
 * Script
 */

class Script
{
private:
	struct StringRef
	{
		int32 offset;
		int32 length;
		Text::LineType type;
		byte opcode;
		int32 padding;

		StringRef(int32 o, int32 l, Text::LineType t, byte op) :
			offset(o), length(l), type(t), opcode(op), padding(0) { }
		StringRef() : offset(0), length(0), type(Text::LT_PLAIN), opcode(0), padding(0) { }
	};
	struct JumpRef
	{
		int32 offset;
		int32 target;
		bool valid;

		JumpRef(int32 o, int32 t) : offset(o), target(t) { }
		JumpRef() : offset(0), target(0) { }
	};
public:
	class Error : public std::runtime_error
	{
	public:
		Error(const std::string &message) : std::runtime_error(message) { }
	};
	class ParseError : public Script::Error
	{
	public:
		ParseError(const std::string &message) : Script::Error(message) { }
	};
private:
	static const int MAX_RECURSION = 32;
private:
	FilePartHandle _file;
	std::list<Script::StringRef> _text;
	std::list<Script::JumpRef> _jump;
	std::list<int32> _spot;
	bool _log;
	bool _gettingRscNameLimits;
	bool _usingRscNameLimits;
public:
	byte _peekByte();
	byte _getByte();
	uint16 _getWord();
	int32 _eatByteOrVar(byte flag);
	int32 _eatWordOrVar(byte flag);
	void _eatVar();
	void _eatArgList();
	void _eatJump();
	int32 _eatString(Text::LineType stringType, byte opcode);
	void _opv12();
	void _opv345(int r = 0);
	void _opv67();
	void _updateJumps(int32 offset, int32 diff, int lineNumber);
	void _writeJumps(std::string &buffer);
#ifdef CHECK_SCRIPT_JUMPS
	void _checkJumps();
#endif // CHECK_SCRIPT_JUMPS
public:
	void setTrackedSpots(const std::list<int32> &spots);
	void getTrackedSpots(std::list<int32> &spots) const;
	void importText(Text &input);
	void exportText(Text &output, bool pad);
	void getRscNameLimits();
	void parse();
public:
	Script(FilePart &f, std::streamoff o, std::streamsize s);
	~Script();
private:
	Script();
private:
	Script(const Script &);
	Script &operator=(const Script &);
};

#endif // !__SCRIPT_HPP_
