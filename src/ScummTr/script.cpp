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

#include "ScummRp/scummrp.hpp"

#include "script.hpp"
#include "scummtr.hpp"

#include <algorithm>

/*
 * Script
 */

Script::Script() :
    _file(nullptr), _text(), _jump(), _spot(),
    _log(true), _gettingRscNameLimits(false), _usingRscNameLimits(false)

{
}

Script::Script(FilePart &f, std::streamoff o, std::streamsize s) :
    _file(new FilePart(f, o, s)), _text(), _jump(), _spot(),
    _log(true), _gettingRscNameLimits(false), _usingRscNameLimits(false)
{
}

Script::~Script()
{
}

void Script::_updateJumps(int32 offset, int32 diff, int lineNumber)
{
	int32 val;

	for (std::list<JumpRef>::iterator i = _jump.begin(); i != _jump.end(); ++i)
	{
		if (i->target >= offset)
			i->target += diff;

		if (i->offset >= offset)
			i->offset += diff;

		if (i->target > i->offset + 2)
			val = i->target - i->offset - 2;
		else
			val = i->offset + 2 - i->target;

		if ((uint32)val >= 0x8000)
			throw Script::Error(xsprintf("Line too long (jump too far) (line %i)", lineNumber));
	}
}

void Script::_writeJumps(std::string &buffer)
{
	int32 val;

	for (std::list<JumpRef>::iterator i = _jump.begin(); i != _jump.end(); ++i)
	{
		val = i->target - i->offset - 2;
		buffer[i->offset] = (char)(byte)(val & 0xFF);
		buffer[i->offset + 1] = (char)(byte)(val >> 8);
	}
}

void Script::importText(Text &input)
{
	int32 lengthDiff, totalDiff, lastEnd, minOffset;
	std::string buffer, s, t;

	parse();

	if (_text.size() == 0)
		return;

	totalDiff = 0;
	lastEnd = 0;
	for (std::list<StringRef>::iterator i = _text.begin(); i != _text.end(); ++i)
	{
		if (!input.nextLine(s, i->type))
			throw Script::Error("Not enough lines in imported text");

		lengthDiff = (int32)s.size() + 1 - i->length;
		_file->seekg(lastEnd, std::ios::beg);
		_file->read(t, i->offset - lastEnd);

		buffer.append(t);
		buffer.append(s);
		buffer += '\0';

		lastEnd = i->offset + i->length;
		minOffset = lastEnd + totalDiff;
		i->offset += totalDiff;
		i->length += lengthDiff;
		totalDiff += lengthDiff;

		_updateJumps(minOffset, lengthDiff, input.lineNumber());

		for (std::list<int32>::iterator j = _spot.begin(); j != _spot.end(); ++j)
			if (*j >= minOffset)
				*j += lengthDiff;
	}

	_file->seekg(lastEnd, std::ios::beg);
	_file->read(t, _file->size() - lastEnd);
	buffer.append(t);

	_writeJumps(buffer);
	if ((int32)buffer.size() < _file->size())
		_file->resize((int32)buffer.size());
	_file->seekp(0, std::ios::beg);

	_file->write(buffer);
}

void Script::exportText(Text &output, bool pad)
{
	std::string s;

	_usingRscNameLimits = pad;
	try
	{
		parse();
	}
	catch (...)
	{
		_usingRscNameLimits = false;
		throw;
	}
	_usingRscNameLimits = false;

	for (std::list<StringRef>::iterator i = _text.begin(); i != _text.end(); ++i)
	{
		_file->seekg(i->offset, std::ios::beg);
		_file->read(s, i->length - 1);

		if (pad && i->padding > i->length - 1)
			s.resize(i->padding, '@');

		output.addLine(s, i->type, i->opcode);
	}
}

void Script::getRscNameLimits()
{
	_gettingRscNameLimits = true;

	try
	{
		parse();
	}
	catch (...)
	{
		_gettingRscNameLimits = false;
		throw;
	}

	_gettingRscNameLimits = false;
}

void Script::setTrackedSpots(const std::list<int32> &spots)
{
	_spot.resize(spots.size());
	std::copy(spots.begin(), spots.end(), _spot.begin());
}

void Script::getTrackedSpots(std::list<int32> &spots) const
{
	spots.resize(_spot.size());
	std::copy(_spot.begin(), _spot.end(), spots.begin());
}

#ifdef SCUMMTR_CHECK_SCRIPT_JUMPS
void Script::_checkJumps()
{
	std::list<JumpRef>::iterator i;
	int32 pos;
	int count, n;

	_file->seekg(0, std::ios::beg);
	count = 0;
	n = (int)_jump.size();

	_log = false;
	try
	{
		if (ScummRp::game.version <= 2)
		{
#ifdef SCUMMTR_CHANGED_JUST_AFTER_RELEASE
			while ((pos = _file->tellg(std::ios::end)) < 0 && count < n)
#else
			while ((pos = _file->tellg(std::ios::beg)) < _file->size() && count < n)
#endif
			{
				for (i = _jump.begin(); i != _jump.end(); ++i)
					if (pos == i->target)
						++count;
				_opv12();
			}
		}
		else if (ScummRp::game.version <= 5)
		{
#ifdef SCUMMTR_CHANGED_JUST_AFTER_RELEASE
			while ((pos = _file->tellg(std::ios::end)) < 0 && count < n)
#else
			while ((pos = _file->tellg(std::ios::beg)) < _file->size() && count < n)
#endif
			{
				for (i = _jump.begin(); i != _jump.end(); ++i)
					if (pos == i->target)
						++count;
				_opv345();
			}
		}
		else if (ScummRp::game.version <= 7)
		{
#ifdef SCUMMTR_CHANGED_JUST_AFTER_RELEASE
			while ((pos = _file->tellg(std::ios::end)) < 0 && count < n)
#else
			while ((pos = _file->tellg(std::ios::beg)) < _file->size() && count < n)
#endif
			{
				for (i = _jump.begin(); i != _jump.end(); ++i)
					if (pos == i->target)
						++count;
				_opv67();
			}
		}
	}
	catch (Script::ParseError &)
	{
		_log = true;
		throw;
	}
	_log = true;

	if (count < n)
		throw Script::ParseError("Bad jump(s)");
}
#endif // SCUMMTR_CHECK_SCRIPT_JUMPS

void Script::parse()
{
	_text.resize(0);
	_jump.resize(0);
	_file->seekg(0, std::ios::beg);

	try
	{
		if (ScummRp::game.version <= 2)
#ifdef SCUMMTR_CHANGED_JUST_AFTER_RELEASE
			while (_file->tellg(std::ios::end) < 0)
#else
			while (_file->tellg(std::ios::beg) < _file->size())
#endif
				_opv12();
		else if (ScummRp::game.version <= 5)
#ifdef SCUMMTR_CHANGED_JUST_AFTER_RELEASE
			while (_file->tellg(std::ios::end) < 0)

#else
			while (_file->tellg(std::ios::beg) < _file->size())
#endif
				_opv345();
		else if (ScummRp::game.version <= 7)
#ifdef SCUMMTR_CHANGED_JUST_AFTER_RELEASE
			while (_file->tellg(std::ios::end) < 0)
#else
			while (_file->tellg(std::ios::beg) < _file->size())
#endif
				_opv67();
#ifdef SCUMMTR_CHECK_SCRIPT_JUMPS
		_checkJumps();
#endif // SCUMMTR_CHECK_SCRIPT_JUMPS
	}
	catch (File::UnexpectedEOF &)
	{
		throw Script::ParseError(xsprintf("Unexpected end of script at 0x%X in %s", _file->fullOffset() + _file->tellg(std::ios::beg), _file->name().c_str()));
	}
	catch (Script::ParseError &e)
	{
		throw Script::ParseError(xsprintf("Script error at 0x%X in %s (%s)", _file->fullOffset() + _file->tellg(std::ios::beg), _file->name().c_str(), e.what()));
	}
}

byte Script::_getByte()
{
	byte b;

	_file->getByte(b);

	return b;
}

byte Script::_peekByte()
{
	byte b;

	_file->getByte(b);
	_file->seekg(-1, std::ios::cur);

	return b;
}

uint16 Script::_getWord()
{
	uint16 w;

	_file->getLE16(w);

	return w;
}

int32 Script::_eatByteOrVar(byte flag)
{
	if (flag == 0)
		return _getByte();

	_eatVar();

	return -1;
}

int32 Script::_eatWordOrVar(byte flag)
{
	if (flag == 0)
		return _getWord();

	_eatVar();

	return -1;
}

void Script::_eatVar()
{
	if (ScummRp::game.version <= 2)
		_getByte();
	else if ((_getWord() & 0x2000) && ScummRp::game.version <= 5)
		_getWord();
}

void Script::_eatArgList()
{
	byte b;
	int i;

	i = 0;
	while ((b = _getByte()) != 0xFF)
	{
		if (i++ >= 16)
			throw Script::ParseError(xsprintf("Arg list too long at 0x%X in %s", _file->fullOffset(), _file->name().c_str()));

		_eatWordOrVar(b & 0x80);
	}
}

void Script::_eatJump()
{
	int16 val;
	int32 offset;

	offset = _file->tellg(std::ios::beg);
	val = (int16)_getWord();
	if (_log)
		_jump.push_back(JumpRef(offset, val + offset + sizeof(int16)));
}

int32 Script::_eatString(Text::LineType stringType, byte opcode)
{
	int32 start, length;

	start = _file->tellg(std::ios::beg);
	length = Text::getLineLength(_file, stringType);
	if (length > 0 && _log) // Skip empty lines
		_text.push_back(StringRef(start, length + 1, stringType, opcode));

	return (_log) ? length : 0;
}

/*
 * regexp
 *
 * '		OPCODE(o[256]_\([^)]+\)),*'
 * 'case 0x##: // \1'
 *
 * '		[/]\* \(.\)0 \*[/]\([^#]+\)##: \([^#]+\)##: \([^#]+\)##: \([^#]+\)##: '
 * '		// \10 \2\10: \3\11: \4\12: \5\13: '
 *
 * '		[/]\* \(.\)4 \*[/]\([^#]+\)##: \([^#]+\)##: \([^#]+\)##: \([^#]+\)##: '
 * '		// \14 \2\14: \3\15: \4\16: \5\17: '
 *
 * '		[/]\* \(.\)8 \*[/]\([^#]+\)##: \([^#]+\)##: \([^#]+\)##: \([^#]+\)##: '
 * '		// \18 \2\18: \3\19: \4\1A: \5\1B: '
 *
 * '		[/]\* \(.\)C \*[/]\([^#]+\)##: \([^#]+\)##: \([^#]+\)##: \([^#]+\)##: '
 * '		// \1C \2\1C: \3\1D: \4\1E: \5\1F: '
 *
 * for sorting:
 * 'case 0x\(..\): // \(.+\)'
 * '\2'
 * 'case 0x\(..\): // \(.+\)
\(case 0x..: // \2
\)*'
 * '\1'
 */

void Script::_opv67()
{
	byte opcode, mainOpcode;

	opcode = mainOpcode = _getByte();
	switch (opcode)
	{
	case 0x00: // pushByte
		_getByte();
		break;
	case 0x01: // pushWord
		_getWord();
		break;
	case 0x02: // pushByteVar
		_getByte();
		break;
	case 0x03: // pushWordVar
		_getWord();
		break;
	case 0x06: // byteArrayRead
		_getByte();
		break;
	case 0x07: // wordArrayRead
		_getWord();
		break;
	case 0x0A: // byteArrayIndexedRead
		_getByte();
		break;
	case 0x0B: // wordArrayIndexedRead
		_getWord();
		break;
	case 0x0C: // dup
	case 0x0D: // not
	case 0x0E: // eq
	case 0x0F: // neq
	case 0x10: // gt
	case 0x11: // lt
	case 0x12: // le
	case 0x13: // ge
	case 0x14: // add
	case 0x15: // sub
	case 0x16: // mul
	case 0x17: // div
	case 0x18: // land
	case 0x19: // lor
	case 0x1A: // pop
	case 0xA7: // pop
		break;
	case 0x42: // writeByteVar
		_getByte();
		break;
	case 0x43: // writeWordVar
		_getWord();
		break;
	case 0x46: // byteArrayWrite
		_getByte();
		break;
	case 0x47: // wordArrayWrite
		_getWord();
		break;
	case 0x4A: // byteArrayIndexedWrite
		_getByte();
		break;
	case 0x4B: // wordArrayIndexedWrite
		_getWord();
		break;
	case 0x4E: // byteVarInc
		_getByte();
		break;
	case 0x4F: // wordVarInc
		_getWord();
		break;
	case 0x52: // byteArrayInc
		_getByte();
		break;
	case 0x53: // wordArrayInc
		_getWord();
		break;
	case 0x56: // byteVarDec
		_getByte();
		break;
	case 0x57: // wordVarDec
		_getWord();
		break;
	case 0x5A: // byteArrayDec
		_getByte();
		break;
	case 0x5B: // wordArrayDec
		_getWord();
		break;
	case 0x5C: // jumpTrue
		_eatJump();
		break;
	case 0x5D: // jumpFalse
		_eatJump();
		break;
	case 0x5E: // startScriptEx
	case 0x5F: // startScript
	case 0x60: // startObjectEx
	case 0x61: // setObjectState
	case 0x62: // setObjectXY
	case 0x63: // drawBlastObject
	case 0x64: // setBlastObjectWindow
	case 0x65: // stopObjectCode
	case 0x66: // stopObjectCode
	case 0x67: // endCutscene
	case 0x68: // cutscene
	case 0x69: // stopMusic
	case 0x6A: // freezeUnfreeze
		break;
	case 0x6B: // cursorCommand
		switch (_getByte())
		{
		case 0x90:
		case 0x91:
		case 0x92:
		case 0x93:
		case 0x94:
		case 0x95:
		case 0x96:
		case 0x97:
		case 0x99:
		case 0x9A:
		case 0x9C:
		case 0x9D:
		case 0xD6:
			break;
		default:
			throw Script::ParseError("cursorCommand");
		}
		break;
	case 0x6C: // breakHere
	case 0x6D: // ifClassOfIs
	case 0x6E: // setClass
	case 0x6F: // getState
	case 0x70: // setState
	case 0x71: // setOwner
	case 0x72: // getOwner
		break;
	case 0x73: // jump
		_eatJump();
		break;
	case 0x74: // startSound
	case 0x75: // stopSound
	case 0x76: // startMusic
	case 0x77: // stopObjectScript
	case 0x78: // panCameraTo
	case 0x79: // actorFollowCamera
	case 0x7A: // setCameraAt
	case 0x7B: // loadRoom
	case 0x7C: // stopScript
	case 0x7D: // walkActorToObj
	case 0x7E: // walkActorTo
	case 0x7F: // putActorInRoom
	case 0x80: // putActorAtObject
	case 0x81: // faceActor
	case 0x82: // animateActor
	case 0x83: // doSentence
	case 0x84: // pickupObject
	case 0x85: // loadRoomWithEgo
	case 0x87: // getRandomNumber
	case 0x88: // getRandomNumberRange
	case 0x8A: // getActorMoving
	case 0x8B: // isScriptRunning
	case 0x8C: // getActorRoom
	case 0x8D: // getObjectX
	case 0x8E: // getObjectY
	case 0x8F: // getObjectOldDir
	case 0x90: // getActorWalkBox
	case 0x91: // getActorCostume
	case 0x92: // findInventory
	case 0x93: // getInventoryCount
	case 0x94: // getVerbFromXY
	case 0x95: // beginOverride
	case 0x96: // endOverride
		break;
	case 0x97: // setObjectName
		_eatString(Text::LT_RSC, mainOpcode);
		break;
	case 0x98: // isSoundRunning
	case 0x99: // setBoxFlags
	case 0x9A: // createBoxMatrix
		break;
	case 0x9B: // resourceRoutines
		switch (_getByte())
		{
		case 100:
		case 101:
		case 102:
		case 103:
		case 104:
		case 105:
		case 106:
		case 107:
		case 108:
		case 109:
		case 110:
		case 111:
		case 112:
		case 113:
		case 114:
		case 115:
		case 116:
		case 117:
		case 118:
		case 119:
			break;
		default:
			throw Script::ParseError("resourceRoutines");
		}
		break;
	case 0x9C: // roomOps
		switch (_getByte())
		{
		case 172:
		case 174:
		case 175:
		case 176:
		case 177:
		case 179:
		case 180:
		case 181:
		case 182:
		case 183:
			break;
		case 184:
			throw Script::ParseError("roomOps case 184");
		case 185:
			throw Script::ParseError("roomOps case 185");
		case 186:
		case 187:
		case 213:
			break;
		default:
			throw Script::ParseError("roomOps");
		}
		break;
	case 0x9D: // actorOps
		switch (_getByte())
		{
		case 76:
		case 77:
		case 78:
		case 79:
		case 80:
		case 81:
		case 82:
		case 83:
		case 84:
		case 85:
		case 86:
		case 87:
			break;
		case 88:
			_eatString(Text::LT_RSC, mainOpcode);
			break;
		case 89:
		case 91:
		case 92:
		case 93:
		case 225:
		case 94:
		case 95:
		case 96:
		case 97:
		case 98:
		case 99:
		case 197:
			break;
		case 198:
		case 215:
		case 216:
		case 217:
		case 218:
		case 227:
		case 228:
		case 229:
		case 230:
		case 231:
		case 233:
		case 234:
		case 235:
			if (ScummRp::game.version <= 6)
				throw Script::ParseError("actorOps case > 197");
			break;
		default:
			throw Script::ParseError("actorOps");
		}
		break;
	case 0x9E: // verbOps
		switch (_getByte())
		{
		case 124:
			break;
		case 125:
			_eatString(Text::LT_RSC, mainOpcode);
			break;
		case 126:
		case 127:
		case 128:
		case 129:
		case 130:
		case 131:
		case 132:
		case 133:
		case 134:
		case 135:
		case 136:
		case 137:
		case 139:
		case 140:
		case 196:
			break;
		case 255:
			if (ScummRp::game.id != GID_TENTACLE)
				throw Script::ParseError("verbOps case 0xFF");
			break;
		default:
			throw Script::ParseError("verbOps");
		}
		break;
	case 0x9F: // getActorFromXY
	case 0xA0: // findObject
	case 0xA1: // pseudoRoom
	case 0xA2: // getActorElevation
	case 0xA3: // getVerbEntrypoint
		break;
	case 0xA4: // arrayOps
		opcode = _getByte();
		_getWord();
		switch (opcode)
		{
		case 205:
			_eatString(Text::LT_RSC, mainOpcode);
			break;
		case 208:
			break;
		case 212:
			break;
		default:
			throw Script::ParseError("arrayOps");
		}
		break;
	case 0xA5: // saveRestoreVerbs
		switch (_getByte())
		{
		case 141:
		case 142:
		case 143:
			break;
		default:
			throw Script::ParseError("saveRestoreVerbs");
		}
		break;
	case 0xA6: // drawBox
	case 0xA8: // getActorWidth
		break;
	case 0xA9: // wait
		switch (_getByte())
		{
		case 168:
			_getWord();
			break;
		case 169:
		case 170:
		case 171:
			break;
		case 226:
			_getWord();
			break;
		case 232:
			_getWord();
			break;
		default:
			throw Script::ParseError("wait");
		}
		break;
	case 0xAA: // getActorScaleX
	case 0xAB: // getActorAnimCounter1
	case 0xAC: // soundKludge
	case 0xAD: // isAnyOf
		break;
	case 0xAE: // quitPauseRestart
		switch (_getByte())
		{
		case 158:
		case 159:
		case 160:
			break;
		default:
			throw Script::ParseError("quitPauseRestart");
		}
		break;
	case 0xAF: // isActorInBox
	case 0xB0: // delay
	case 0xB1: // delaySeconds
	case 0xB2: // delayMinutes
	case 0xB3: // stopSentence
		break;
	case 0xB4: // printLine
	case 0xB5: // printCursor
	case 0xB6: // printDebug
	case 0xB7: // printSystem
	case 0xB8: // printActor
	case 0xB9: // printEgo
		switch (_getByte())
		{
		case 0x41:
		case 0x42:
		case 0x43:
		case 0x45:
		case 0x47:
		case 0x48:
			break;
		case 0x4A:
			break;
		case 0x4B:
			_eatString(Text::LT_MSG, mainOpcode);
			break;
		case 0xFE:
		case 0xFF:
			break;
		case 0x49:
		case 0xF9:
		default:
			throw Script::ParseError("print");
		}
		break;
	case 0xBA: // talkActor
	case 0xBB: // talkEgo
		_eatString(Text::LT_MSG, mainOpcode);
		break;
	case 0xBC: // dim
		switch (_getByte())
		{
		case 199:
		case 200:
		case 201:
		case 202:
		case 203:
		case 204:
			break;
		default:
			throw Script::ParseError("dim");
		}
		_getWord();
		break;
	case 0xBD: // dummy
	case 0xBE: // startObjectQuick
	case 0xBF: // startScriptQuick
		break;
	case 0xC0: // dim2
		switch (_getByte())
		{
		case 199:
		case 200:
		case 201:
		case 202:
		case 203:
			break;
		default:
			throw Script::ParseError("dim2");
		}
		_getWord();
		break;
	case 0xC4: // abs
	case 0xC5: // distObjectObject
	case 0xC6: // distObjectPt
	case 0xC7: // distPtPt
	case 0xC8: // kernelGetFunctions
	case 0xC9: // kernelSetFunctions
	case 0xCA: // delayFrames
	case 0xCB: // pickOneOf
	case 0xCC: // pickOneOfDefault
	case 0xCD: // stampObject
	case 0xD0: // getDateTime
	case 0xD1: // stopTalking
	case 0xD2: // getAnimateVariable
		break;
	case 0xD4: // shuffle
		_getWord();
		break;
	case 0xD5: // jumpToScript
	case 0xD6: // band
	case 0xD7: // bor
	case 0xD8: // isRoomScriptRunning
	case 0xD9: // closeFile
		break;
	case 0xDA: // openFile
		_eatString(Text::LT_PLAIN, mainOpcode);
		break;
	case 0xDB: // readFile
	case 0xDD: // findAllObjects
		break;
	case 0xDE: // deleteFile
		_eatString(Text::LT_PLAIN, mainOpcode);
		break;
	case 0xE0: // unknownE0
		_getByte();
		throw Script::ParseError(xsprintf("unknown%.2X", opcode));
	case 0xE1: // unknownE1
		if (ScummRp::game.id != GID_DIG)
			throw Script::ParseError(xsprintf("unknown%.2X", opcode));
		break;
	case 0xE2: // localizeArray
		break;
	case 0xE3: // pickVarRandom
		_getWord();
		break;
	case 0xE4: // unknownE4
		throw Script::ParseError(xsprintf("unknown%.2X", opcode));
	case 0xEA: // unknownEA
		switch (_getByte())
		{
		case 197:
			_getWord();
			break;
		case 202:
			_getWord();
			break;
		default:
			throw Script::ParseError(xsprintf("unknown%.2X default case", opcode));
		}
		throw Script::ParseError(xsprintf("unknown%.2X", opcode));
	case 0xEC: // getActorLayer
		break;
	case 0xED: // getObjectNewDir
		break;
	case 0xFA: // unknownFA
		_getByte();
		_eatString(Text::LT_PLAIN, mainOpcode); // what type of string exactly?
		throw Script::ParseError(xsprintf("Unknown%.2X", opcode));
	default:
		throw Script::ParseError(xsprintf("Unknown opcode 0x%.2X", opcode));
	}
}

void Script::_opv12()
{
	byte opcode, mainOpcode;

	opcode = mainOpcode = _getByte();
	switch (opcode)
	{
	case 0x00: // stopObjectCode
	case 0xA0: // stopObjectCode
		break;
	case 0x01: // putActor
	case 0x21: // putActor
	case 0x41: // putActor
	case 0x61: // putActor
	case 0x81: // putActor
	case 0xA1: // putActor
	case 0xC1: // putActor
	case 0xE1: // putActor
		_eatByteOrVar(opcode & 0x80);
		_eatByteOrVar(opcode & 0x40);
		_eatByteOrVar(opcode & 0x20);
		break;
	case 0x02: // startMusic
	case 0x82: // startMusic
		_eatByteOrVar(opcode & 0x80);
		break;
	case 0x03: // getActorRoom
	case 0x83: // getActorRoom
		_eatVar();
		_eatByteOrVar(opcode & 0x80);
		break;
	case 0x04: // isGreaterEqual
	case 0x84: // isGreaterEqual
		_eatVar();
		_eatWordOrVar(opcode & 0x80);
		_eatJump();
		break;
	case 0x05: // drawObject
	case 0x25: // drawObject
	case 0x45: // drawObject
	case 0x65: // drawObject
	case 0x85: // drawObject
	case 0xA5: // drawObject
	case 0xC5: // drawObject
	case 0xE5: // drawObject
		_eatWordOrVar(opcode & 0x80);
		_eatByteOrVar(opcode & 0x40);
		_eatByteOrVar(opcode & 0x20);
		break;
	case 0x06: // getActorElevation
	case 0x86: // getActorElevation
		_eatVar();
		_eatByteOrVar(opcode & 0x80);
		break;
	case 0x07: // setState08
	case 0x87: // setState08
		_eatWordOrVar(opcode & 0x80);
		break;
	case 0x08: // isNotEqual
	case 0x88: // isNotEqual
		_eatVar();
		_eatWordOrVar(opcode & 0x80);
		_eatJump();
		break;
	case 0x09: // faceActor
	case 0x49: // faceActor
	case 0x89: // faceActor
	case 0xC9: // faceActor
		_eatByteOrVar(opcode & 0x80);
		_eatWordOrVar(opcode & 0x40);
		break;
	case 0x0A: // assignVarWordIndirect
	case 0x8A: // assignVarWordIndirect
		_eatVar();
		_eatWordOrVar(opcode & 0x80);
		break;
	case 0x0B: // setObjPreposition
	case 0x4B: // setObjPreposition
	case 0x8B: // setObjPreposition
	case 0xCB: // setObjPreposition
		if (ScummRp::game.version == 1 && ScummRp::game.id == GID_MANIAC)
			throw Script::ParseError(xsprintf("Unknown opcode 0x%.2X", opcode));
		_eatWordOrVar(opcode & 0x80);
		_getByte();
		break;
	case 0x0C: // resourceRoutines
	case 0x8C: // resourceRoutines
		_eatByteOrVar(opcode & 0x80);
		_getByte();
		break;
	case 0x0D: // walkActorToActor
	case 0x4D: // walkActorToActor
	case 0x8D: // walkActorToActor
	case 0xCD: // walkActorToActor
		_eatByteOrVar(opcode & 0x80);
		_eatByteOrVar(opcode & 0x40);
		_getByte();
		break;
	case 0x0E: // putActorAtObject
	case 0x4E: // putActorAtObject
	case 0x8E: // putActorAtObject
	case 0xCE: // putActorAtObject
		_eatByteOrVar(opcode & 0x80);
		_eatWordOrVar(opcode & 0x40);
		break;
	case 0x0F: // ifNotState08
	case 0x8F: // ifNotState08
		_eatWordOrVar(opcode & 0x80);
		_eatJump();
		break;
	case 0x10: // getObjectOwner
	case 0x90: // getObjectOwner
		_eatVar();
		_eatWordOrVar(opcode & 0x80);
		break;
	case 0x11: // animateActor
	case 0x51: // animateActor
	case 0x91: // animateActor
	case 0xD1: // animateActor
		_eatByteOrVar(opcode & 0x80);
		_eatByteOrVar(opcode & 0x40);
		break;
	case 0x12: // panCameraTo
	case 0x92: // panCameraTo
		_eatByteOrVar(opcode & 0x80);
		break;
	case 0x13: // actorSet
	case 0x53: // actorSet
	case 0x93: // actorSet
	case 0xD3: // actorSet
		{
			int32 actor, l;

			actor = _eatByteOrVar(opcode & 0x80);
			_eatByteOrVar(opcode & 0x40);
			switch (_getByte())
			{
			case 0x01:
				break;
			case 0x02:
				if (ScummRp::game.version == 2)
					_getByte();
				break;
			case 0x03:
				l = _eatString(Text::LT_PLAIN, mainOpcode);
				if (l > 0)
				{
					if (_gettingRscNameLimits)
						ScummTr::setRscNameMaxLengh(ScummTr::RSCT_ACTOR, actor, l);
					else if (_usingRscNameLimits)
						_text.back().padding = ScummTr::getRscNameMaxLengh(ScummTr::RSCT_ACTOR, actor);
				}
				break;
			case 0x04:
			case 0x05:
				break;
			default:
				throw Script::ParseError("actorSet");
			}
		}
		break;
	case 0x14: // print
	case 0x94: // print
		_eatByteOrVar(opcode & 0x80);
		// fallthrough
	case 0xD8: // printEgo
		_eatString(Text::LT_OLDMSG, mainOpcode);
		break;
	case 0x15: // actorFromPos
	case 0x55: // actorFromPos
	case 0x95: // actorFromPos
	case 0xD5: // actorFromPos
		_eatVar();
		_eatByteOrVar(opcode & 0x80);
		_eatByteOrVar(opcode & 0x40);
		break;
	case 0x16: // getRandomNr
	case 0x96: // getRandomNr
		_eatVar();
		_eatByteOrVar(opcode & 0x80);
		break;
	case 0x17: // clearState02
	case 0x97: // clearState02
		_eatWordOrVar(opcode & 0x80);
		break;
	case 0x18: // jumpRelative
		_eatJump();
		break;
	case 0x19: // doSentence
	case 0x39: // doSentence
	case 0x59: // doSentence
	case 0x79: // doSentence
		if (_peekByte() == 0xFB)
		{
			_getByte();
			break;
		}
		if (_peekByte() == 0xFC)
		{
			_getByte();
			break;
		}
		// else:
		// fallthrough
	case 0x99: // doSentence
	case 0xB9: // doSentence
	case 0xD9: // doSentence
	case 0xF9: // doSentence
		_eatByteOrVar(opcode & 0x80);
		_eatWordOrVar(opcode & 0x40);
		_eatWordOrVar(opcode & 0x20);
		switch (_getByte())
		{
		case 0x00:
		case 0x01:
		case 0x02:
			break;
		default:
			throw Script::ParseError("doSentence");
		}
		break;
	case 0x1A: // move
	case 0x9A: // move
		_eatVar();
		_eatWordOrVar(opcode & 0x80);
		break;
	case 0x1B: // setBitVar
	case 0x5B: // setBitVar
	case 0x9B: // setBitVar
	case 0xDB: // setBitVar
		_getWord();
		_eatByteOrVar(opcode & 0x80);
		_eatByteOrVar(opcode & 0x40);
		break;
	case 0x1C: // startSound
	case 0x9C: // startSound
		_eatByteOrVar(opcode & 0x80);
		break;
	case 0x1D: // ifClassOfIs
	case 0x5D: // ifClassOfIs
	case 0x9D: // ifClassOfIs
	case 0xDD: // ifClassOfIs
		_eatWordOrVar(opcode & 0x80);
		_eatByteOrVar(opcode & 0x40);
		_eatJump();
		break;
	case 0x1E: // walkActorTo
	case 0x3E: // walkActorTo
	case 0x5E: // walkActorTo
	case 0x7E: // walkActorTo
	case 0x9E: // walkActorTo
	case 0xBE: // walkActorTo
	case 0xDE: // walkActorTo
	case 0xFE: // walkActorTo
		_eatByteOrVar(opcode & 0x80);
		_eatByteOrVar(opcode & 0x40);
		_eatByteOrVar(opcode & 0x20);
		break;
	case 0x1F: // ifState02
	case 0x9F: // ifState02
		_eatWordOrVar(opcode & 0x80);
		_eatJump();
		break;
	case 0x20: // stopMusic
		break;
	case 0x22: // saveLoadGame
	case 0xA2: // saveLoadGame
		_eatVar();
		_eatByteOrVar(opcode & 0x80);
		break;
	case 0x23: // getActorY
	case 0xA3: // getActorY
		_eatVar();
		_eatByteOrVar(opcode & 0x80);
		break;
	case 0x24: // loadRoomWithEgo
	case 0x64: // loadRoomWithEgo
	case 0xA4: // loadRoomWithEgo
	case 0xE4: // loadRoomWithEgo
		_eatWordOrVar(opcode & 0x80);
		_eatByteOrVar(opcode & 0x40);
		_getByte();
		_getByte();
		break;
	case 0x26: // setVarRange
	case 0xA6: // setVarRange
		{
			byte b;

			_eatVar();
			b = _getByte();
			do
			{
				if (opcode & 0x80)
					_getWord();
				else
					_getByte();
			} while (--b != 0);
		}
		break;
	case 0x27: // setState04
	case 0xA7: // setState04
		_eatWordOrVar(opcode & 0x80);
		break;
	case 0x28: // equalZero
		_eatVar();
		_eatJump();
		break;
	case 0x29: // setOwnerOf
	case 0x69: // setOwnerOf
	case 0xA9: // setOwnerOf
	case 0xE9: // setOwnerOf
		_eatWordOrVar(opcode & 0x80);
		_eatByteOrVar(opcode & 0x40);
		break;
	case 0x2A: // addIndirect
	case 0xAA: // addIndirect
		_eatVar();
		_eatWordOrVar(opcode & 0x80);
		break;
	case 0x2B: // delayVariable
		if (ScummRp::game.version == 1 && ScummRp::game.id == GID_MANIAC)
			throw Script::ParseError(xsprintf("Unknown opcode 0x%.2X", opcode));
		_eatVar();
		break;
	case 0x2C: // assignVarByte
		if (ScummRp::game.version == 1 && ScummRp::game.id == GID_MANIAC)
			throw Script::ParseError(xsprintf("Unknown opcode 0x%.2X", opcode));
		_eatVar();
		_getByte();
		break;
	case 0x2D: // putActorInRoom
	case 0x6D: // putActorInRoom
	case 0xAD: // putActorInRoom
	case 0xED: // putActorInRoom
		_eatByteOrVar(opcode & 0x80);
		_eatByteOrVar(opcode & 0x40);
		break;
	case 0x2E: // delay
		_getByte();
		_getByte();
		_getByte();
		break;
	case 0x2F: // ifNotState04
	case 0xAF: // ifNotState04
		_eatWordOrVar(opcode & 0x80);
		_eatJump();
		break;
	case 0x30: // setBoxFlags
	case 0xB0: // setBoxFlags
		_eatByteOrVar(opcode & 0x80);
		_getByte();
		break;
	case 0x31: // getBitVar
	case 0xB1: // getBitVar
		_eatVar();
		_getWord();
		_eatByteOrVar(opcode & 0x80);
		break;
	case 0x32: // setCameraAt
	case 0xB2: // setCameraAt
		_eatByteOrVar(opcode & 0x80);
		break;
	case 0x33: // roomOps
	case 0x73: // roomOps
	case 0xB3: // roomOps
	case 0xF3: // roomOps
		_eatByteOrVar(opcode & 0x80);
		_eatByteOrVar(opcode & 0x40);
		switch (_getByte() & 0x1F)
		{
		case 0x01:
		case 0x02:
			break;
		default:
			throw Script::ParseError("roomOps");
		}
		break;
	case 0x34: // getDist
	case 0x74: // getDist
	case 0xB4: // getDist
	case 0xF4: // getDist
		_eatVar();
		_eatWordOrVar(opcode & 0x80);
		_eatWordOrVar(opcode & 0x40);
		break;
	case 0x35: // findObject
	case 0x75: // findObject
	case 0xB5: // findObject
	case 0xF5: // findObject
		_eatVar();
		_eatByteOrVar(opcode & 0x80);
		_eatByteOrVar(opcode & 0x40);
		break;
	case 0x36: // walkActorToObject
	case 0x76: // walkActorToObject
	case 0xB6: // walkActorToObject
	case 0xF6: // walkActorToObject
		_eatByteOrVar(opcode & 0x80);
		_eatWordOrVar(opcode & 0x40);
		break;
	case 0x37: // setState01
	case 0xB7: // setState01
		_eatWordOrVar(opcode & 0x80);
		break;
	case 0x38: // isLessEqual
	case 0xB8: // isLessEqual
		_eatVar();
		_eatWordOrVar(opcode & 0x80);
		_eatJump();
		break;
	case 0x3A: // subtract
	case 0xBA: // subtract
		_eatVar();
		_eatWordOrVar(opcode & 0x80);
		break;
	case 0x3B: // waitForActor
	case 0xBB: // waitForActor
		_eatByteOrVar(opcode & 0x80);
		break;
	case 0x3C: // stopSound
	case 0xBC: // stopSound
		_eatByteOrVar(opcode & 0x80);
		break;
	case 0x3D: // setActorElevation
	case 0x7D: // setActorElevation
	case 0xBD: // setActorElevation
	case 0xFD: // setActorElevation
		_eatByteOrVar(opcode & 0x80);
		_eatByteOrVar(opcode & 0x40);
		break;
	case 0x3F: // ifNotState01
	case 0xBF: // ifNotState01
		_eatWordOrVar(opcode & 0x80);
		_eatJump();
		break;
	case 0x40: // cutscene
		break;
	case 0x42: // startScript
	case 0xC2: // startScript
		_eatByteOrVar(opcode & 0x80);
		break;
	case 0x43: // getActorX
	case 0xC3: // getActorX
		_eatVar();
		_eatByteOrVar(opcode & 0x80);
		break;
	case 0x44: // isLess
	case 0xC4: // isLess
		_eatVar();
		_eatWordOrVar(opcode & 0x80);
		_eatJump();
		break;
	case 0x46: // increment
		_eatVar();
		break;
	case 0x47: // clearState08
	case 0xC7: // clearState08
		_eatWordOrVar(opcode & 0x80);
		break;
	case 0x48: // isEqual
	case 0xC8: // isEqual
		_eatVar();
		_eatWordOrVar(opcode & 0x80);
		_eatJump();
		break;
	case 0x4A: // chainScript
	case 0xCA: // chainScript
		_eatByteOrVar(opcode & 0x80);
		break;
	case 0x4C: // waitForSentence
		break;
	case 0x4F: // ifState08
	case 0xCF: // ifState08
		_eatWordOrVar(opcode & 0x80);
		_eatJump();
		break;
	case 0x50: // pickupObject
	case 0xD0: // pickupObject
		_eatWordOrVar(opcode & 0x80);
		break;
	case 0x52: // actorFollowCamera
	case 0xD2: // actorFollowCamera
		_eatByteOrVar(opcode & 0x80);
		break;
	case 0x54: // setObjectName
	case 0xD4: // setObjectName
		{
			int32 obj, l;

			obj = _eatWordOrVar(opcode & 0x80);
			l = _eatString(Text::LT_PLAIN, mainOpcode);
			if (l > 0)
			{
				if (_gettingRscNameLimits)
					ScummTr::setRscNameMaxLengh(ScummTr::RSCT_OBJECT, obj, l);
				else if (_usingRscNameLimits)
					_text.back().padding = ScummTr::getRscNameMaxLengh(ScummTr::RSCT_OBJECT, obj);
			}
		}
		break;
	case 0x56: // getActorMoving
	case 0xD6: // getActorMoving
		_eatVar();
		_eatByteOrVar(opcode & 0x80);
		break;
	case 0x57: // setState02
	case 0xD7: // setState02
		_eatWordOrVar(opcode & 0x80);
		break;
	case 0x58: // beginOverride
		break;
	case 0x5A: // add
	case 0xDA: // add
		_eatVar();
		_eatWordOrVar(opcode & 0x80);
		break;
	case 0x5F: // ifNotState02
	case 0xDF: // ifNotState02
		_eatWordOrVar(opcode & 0x80);
		_eatJump();
		break;
	case 0x60: // cursorCommand
	case 0xE0: // cursorCommand
		_eatWordOrVar(opcode & 0x80);
		break;
	case 0x62: // stopScript
	case 0xE2: // stopScript
		_eatByteOrVar(opcode & 0x80);
		break;
	case 0x63: // getActorFacing
	case 0xE3: // getActorFacing
		_eatVar();
		_eatByteOrVar(opcode & 0x80);
		break;
	case 0x66: // getClosestObjActor
	case 0xE6: // getClosestObjActor
		_eatVar();
		_eatWordOrVar(opcode & 0x80);
		break;
	case 0x67: // clearState04
	case 0xE7: // clearState04
		_eatWordOrVar(opcode & 0x80);
		break;
	case 0x68: // isScriptRunning
	case 0xE8: // isScriptRunning
		_eatVar();
		_eatByteOrVar(opcode & 0x80);
		break;
	case 0x6A: // subIndirect
	case 0xEA: // subIndirect
		_eatVar();
		_eatWordOrVar(opcode & 0x80);
		break;
	case 0x6C: // getObjPreposition
	case 0xEC: // getObjPreposition
		if (ScummRp::game.version == 1 && ScummRp::game.id == GID_MANIAC)
			throw Script::ParseError(xsprintf("Unknown opcode 0x%.2X", opcode));
		_eatVar();
		_eatWordOrVar(opcode & 0x80);
		break;
	case 0x6F: // ifState04
	case 0xEF: // ifState04
		_eatWordOrVar(opcode & 0x80);
		_eatJump();
		break;
	case 0x70: // lights
	case 0xF0: // lights
		_eatByteOrVar(opcode & 0x80);
		_getByte();
		_getByte();
		break;
	case 0x71: // getActorCostume
	case 0xF1: // getActorCostume
		_eatVar();
		_eatByteOrVar(opcode & 0x80);
		break;
	case 0x72: // loadRoom
	case 0xF2: // loadRoom
		_eatByteOrVar(opcode & 0x80);
		break;
	case 0x77: // clearState01
	case 0xF7: // clearState01
		_eatWordOrVar(opcode & 0x80);
		break;
	case 0x78: // isGreater
	case 0xF8: // isGreater
		_eatVar();
		_eatWordOrVar(opcode & 0x80);
		_eatJump();
		break;
	case 0x7A: // verbOps
	case 0xFA: // verbOps
		{
			byte verb;
			int32 l;

			verb = _getByte();
			switch (verb)
			{
			case 0x00:
				_eatByteOrVar(opcode & 0x80);
				break;
			case 0xFF:
				_getByte();
				_getByte();
				break;
			default:
				_getByte();
				_getByte();
				_eatByteOrVar(opcode & 0x80);
				_getByte();
				l = _eatString(Text::LT_PLAIN, mainOpcode);
				if (l > 0)
				{
					if (_gettingRscNameLimits)
						ScummTr::setRscNameMaxLengh(ScummTr::RSCT_VERB, verb, l);
					else if (_usingRscNameLimits)
						_text.back().padding = ScummTr::getRscNameMaxLengh(ScummTr::RSCT_VERB, verb);
				}
			}
		}
		break;
	case 0x7B: // getActorWalkBox
	case 0xFB: // getActorWalkBox
		_eatVar();
		_eatByteOrVar(opcode & 0x80);
		break;
	case 0x7C: // isSoundRunning
	case 0xFC: // isSoundRunning
		_eatVar();
		_eatByteOrVar(opcode & 0x80);
		break;
	case 0x7F: // ifState01
	case 0xFF: // ifState01
		_eatWordOrVar(opcode & 0x80);
		_eatJump();
		break;
	case 0x80: // breakHere
		break;
	case 0x98: // restart
		break;
	case 0xA8: // notEqualZero
		_eatVar();
		_eatJump();
		break;
	case 0xAB: // switchCostumeSet // TODO check this
		if (ScummRp::game.features & GF_NES)
			_getByte();
		else
			throw Script::ParseError("switchCostumeSet");
		break;
	case 0xAC: // drawSentence
		if (ScummRp::game.version == 1 && ScummRp::game.id == GID_MANIAC)
			throw Script::ParseError(xsprintf("Unknown opcode 0x%.2X", opcode));
		break;
	case 0xAE: // waitForMessage
	case 0xC0: // endCutscene
		break;
	case 0xC6: // decrement
		_eatVar();
		break;
	case 0xCC: // pseudoRoom
		_getByte();
		while (_getByte() != 0)
			;
		break;
	case 0xEE: // dummy
		break;
	default:
		throw Script::ParseError(xsprintf("Unknown opcode 0x%.2X", opcode));
	}
}

void Script::_opv345(int r)
{
	byte opcode, mainOpcode;

	if (r > Script::MAX_RECURSION)
		throw Script::ParseError("Recursion too deep");

	opcode = mainOpcode = _getByte();
	switch (opcode)
	{
	case 0x00: // stopObjectCode
	case 0xA0: // stopObjectCode
		break;
	case 0x01: // putActor
	case 0x21: // putActor
	case 0x41: // putActor
	case 0x61: // putActor
	case 0x81: // putActor
	case 0xA1: // putActor
	case 0xC1: // putActor
	case 0xE1: // putActor
		_eatByteOrVar(opcode & 0x80);
		_eatWordOrVar(opcode & 0x40);
		_eatWordOrVar(opcode & 0x20);
		break;
	case 0x02: // startMusic
	case 0x82: // startMusic
		if ((ScummRp::game.features & GF_FMTOWNS) && ScummRp::game.version == 3)
		{
			_eatVar();
			_eatByteOrVar(opcode & 0x80);
		}
		else
			_eatByteOrVar(opcode & 0x80);
		break;
	case 0x03: // getActorRoom
	case 0x83: // getActorRoom
		_eatVar();
		_eatByteOrVar(opcode & 0x80);
		break;
	case 0x04: // isGreaterEqual
	case 0x84: // isGreaterEqual
		_eatVar();
		_eatWordOrVar(opcode & 0x80);
		_eatJump();
		break;
	case 0x25: // pickupObject
	case 0x65: // pickupObject
	case 0xA5: // pickupObject
	case 0xE5: // pickupObject
		if (ScummRp::game.version == 5)
		{
			_eatWordOrVar(opcode & 0x80);
			_eatByteOrVar(opcode & 0x40);
			break;
		}
		// else goto drawObject
		// fallthrough
	case 0x05: // drawObject
	case 0x45: // drawObject
	case 0x85: // drawObject
	case 0xC5: // drawObject
		_eatWordOrVar(opcode & 0x80);
		if (ScummRp::game.version <= 4)
		{
			_eatWordOrVar(opcode & 0x40);
			_eatWordOrVar(opcode & 0x20);
		}
		else
		{
			opcode = _getByte();
			switch (opcode & 0x1F)
			{
			case 0x01:
				_eatWordOrVar(opcode & 0x80);
				_eatWordOrVar(opcode & 0x40);
				break;
			case 0x02:
				_eatWordOrVar(opcode & 0x80);
				break;
			case 0x1F:
				break;
			default:
				throw Script::ParseError("drawObject");
			}
		}
		break;
	case 0x06: // getActorElevation
	case 0x86: // getActorElevation
		_eatVar();
		_eatByteOrVar(opcode & 0x80);
		break;
	case 0x07: // setState
	case 0x47: // setState
	case 0x87: // setState
	case 0xC7: // setState
		_eatWordOrVar(opcode & 0x80);
		_eatByteOrVar(opcode & 0x40);
		break;
	case 0x08: // isNotEqual
	case 0x88: // isNotEqual
		_eatVar();
		_eatWordOrVar(opcode & 0x80);
		_eatJump();
		break;
	case 0x09: // faceActor
	case 0x49: // faceActor
	case 0x89: // faceActor
	case 0xC9: // faceActor
		_eatByteOrVar(opcode & 0x80);
		_eatWordOrVar(opcode & 0x40);
		break;
	case 0x0A: // startScript
	case 0x2A: // startScript
	case 0x4A: // startScript
	case 0x6A: // startScript
	case 0x8A: // startScript
	case 0xAA: // startScript
	case 0xCA: // startScript
	case 0xEA: // startScript
		{
#ifdef SCUMMTR_CHANGED_JUST_AFTER_RELEASE_DEBUG
			int i = _eatByteOrVar(opcode & 0x80);
			_eatArgList();
			if (i == 66)
				std::cout << "ahoy" << std::endl;
#else
			_eatByteOrVar(opcode & 0x80);
			_eatArgList();
#endif
		}
		break;
	case 0x0B: // getVerbEntrypoint
	case 0x4B: // getVerbEntrypoint
	case 0x8B: // getVerbEntrypoint
	case 0xCB: // getVerbEntrypoint
		_eatVar();
		_eatWordOrVar(opcode & 0x80);
		_eatWordOrVar(opcode & 0x40);
		break;
	case 0x0C: // resourceRoutines
	case 0x8C: // resourceRoutines
		opcode = _getByte();
		if (opcode != 0x11)
			_eatByteOrVar(opcode & 0x80);
		if (!(ScummRp::game.features & GF_FMTOWNS))
			if ((opcode & 0x3F) != (opcode & 0x1F))
				throw Script::ParseError("resourceRoutines");
		switch (opcode & 0x3F)
		{
		case 0x01:
		case 0x02:
		case 0x03:
		case 0x04:
		case 0x05:
		case 0x06:
		case 0x07:
			break;
		case 0x08:
			if (ScummRp::game.features & GF_FMTOWNS)
				throw Script::ParseError("resourceRoutines case 0x08");
			break;
		case 0x09:
		case 0x0A:
		case 0x0B:
		case 0x0C:
		case 0x0D:
		case 0x0E:
		case 0x0F:
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
			break;
		case 0x14:
			_eatWordOrVar(opcode & 0x40);
			break;
		case 0x20:
			throw Script::ParseError("resourceRoutines case 0x20");
		case 0x21:
			throw Script::ParseError("resourceRoutines case 0x21");
		case 0x23:
			if (ScummRp::game.id == GID_INDY3)
				throw Script::ParseError("resourceRoutines case 0x23");
			_eatByteOrVar(opcode & 0x40);
			break;
		case 0x24:
			_eatByteOrVar(opcode & 0x40);
			_getByte();
			break;
		case 0x25:
			_eatByteOrVar(opcode & 0x40);
			break;
		default:
			throw Script::ParseError(xsprintf("resourceRoutines: %d", opcode & 0x3F));
		}
		break;
	case 0x0D: // walkActorToActor
	case 0x4D: // walkActorToActor
	case 0x8D: // walkActorToActor
	case 0xCD: // walkActorToActor
		_eatByteOrVar(opcode & 0x80);
		_eatByteOrVar(opcode & 0x40);
		_getByte();
		break;
	case 0x0E: // putActorAtObject
	case 0x4E: // putActorAtObject
	case 0x8E: // putActorAtObject
	case 0xCE: // putActorAtObject
		_eatByteOrVar(opcode & 0x80);
		_eatWordOrVar(opcode & 0x40);
		break;
	case 0x0F: // getObjectState
	case 0x8F: // getObjectState
		if (ScummRp::game.version <= 4)
		{ // ifState
			_eatWordOrVar(opcode & 0x80);
			_eatByteOrVar(opcode & 0x40);
			_eatJump();
		}
		else
		{
			_eatVar();
			_eatWordOrVar(opcode & 0x80);
		}
		break;
	case 0x10: // getObjectOwner
	case 0x90: // getObjectOwner
		_eatVar();
		_eatWordOrVar(opcode & 0x80);
		break;
	case 0x11: // animateActor
	case 0x51: // animateActor
	case 0x91: // animateActor
	case 0xD1: // animateActor
		_eatByteOrVar(opcode & 0x80);
		_eatByteOrVar(opcode & 0x40);
		break;
	case 0x12: // panCameraTo
	case 0x92: // panCameraTo
		_eatWordOrVar(opcode & 0x80);
		break;
	case 0x13: // actorSet
	case 0x53: // actorSet
	case 0x93: // actorSet
	case 0xD3: // actorSet
		{
			static const byte convertTable[0x20] =
			{
				0x1E, 0x01, 0x00, 0x00,
				0x02, 0x03, 0x04, 0x05,
				0x06, 0x07, 0x08, 0x09,
				0x0A, 0x0B, 0x0C, 0x0D,
				0x0E, 0x0F, 0x10, 0x11,
				0x1E, 0x1E, 0x1E, 0x1E, // 0x1E means error
				0x1E, 0x1E, 0x1E, 0x1E,
				0x1E, 0x1E, 0x1E, 0x1E
			};
			int32 actor, l;

			actor = _eatByteOrVar(opcode & 0x80);
			while ((opcode = _getByte()) != 0xFF)
			{
				if (ScummRp::game.version <= 4)
					opcode = (opcode & 0xE0) | convertTable[(opcode & 0x1F)];

				switch (opcode & 0x1F)
				{
				case 0x00:
					_eatByteOrVar(opcode & 0x80);
					break;
				case 0x01:
					_eatByteOrVar(opcode & 0x80);
					break;
				case 0x02:
					_eatByteOrVar(opcode & 0x80);
					_eatByteOrVar(opcode & 0x40);
					break;
				case 0x03:
					_eatByteOrVar(opcode & 0x80);
					break;
				case 0x04:
					_eatByteOrVar(opcode & 0x80);
					break;
				case 0x05:
					_eatByteOrVar(opcode & 0x80);
					_eatByteOrVar(opcode & 0x40);
					break;
				case 0x06:
					_eatByteOrVar(opcode & 0x80);
					break;
				case 0x07:
					_eatByteOrVar(opcode & 0x80);
					_eatByteOrVar(opcode & 0x40);
					_eatByteOrVar(opcode & 0x20);
					break;
				case 0x08:
					break;
				case 0x09:
					_eatWordOrVar(opcode & 0x80);
					break;
				case 0x0A:
					break;
				case 0x0B:
					_eatByteOrVar(opcode & 0x80);
					_eatByteOrVar(opcode & 0x40);
					break;
				case 0x0C:
					_eatByteOrVar(opcode & 0x80);
					break;
				case 0x0D:
					l = _eatString(Text::LT_RSC, mainOpcode);
					if (l > 0)
					{
						if (_gettingRscNameLimits)
							ScummTr::setRscNameMaxLengh(ScummTr::RSCT_ACTOR, actor, l);
						else if (_usingRscNameLimits)
							_text.back().padding = ScummTr::getRscNameMaxLengh(ScummTr::RSCT_ACTOR, actor);
					}
					break;
				case 0x0E:
					_eatByteOrVar(opcode & 0x80);
					break;
				case 0x0F:
					_eatArgList();
					throw Script::ParseError("actorSet case 0x0F");
				case 0x10:
					_eatByteOrVar(opcode & 0x80);
					break;
				case 0x11:
					if (ScummRp::game.version == 4)
					{
						_eatByteOrVar(opcode & 0x80);
					}
					else if (ScummRp::game.version == 5)
					{
						_eatByteOrVar(opcode & 0x80);
						_eatByteOrVar(opcode & 0x40);
					}
					else // (ScummRp::game.version == 3)
					{
						throw Script::ParseError("actorSet case 0x11");
					}
					break;
				case 0x12:
					break;
				case 0x13:
					_eatByteOrVar(opcode & 0x80);
					break;
				case 0x14:
				case 0x15:
					break;
				case 0x16:
					_eatByteOrVar(opcode & 0x80);
					break;
				case 0x17:
					_eatByteOrVar(opcode & 0x80);
					break;
				case 0x1E:
				default:
					throw Script::ParseError("actorSet");
				}
			}
		}
		break;
	case 0x14: // print
	case 0x94: // print
		_eatByteOrVar(opcode & 0x80);
		// fallthrough
	case 0xD8: // printEgo
		while ((opcode = _getByte()) != 0xFF)
		{
			switch (opcode & 0x0F)
			{
			case 0x0:
				_eatWordOrVar(opcode & 0x80);
				_eatWordOrVar(opcode & 0x40);
				break;
			case 0x1:
				_eatByteOrVar(opcode & 0x80);
				break;
			case 0x2:
				_eatWordOrVar(opcode & 0x80);
				break;
			case 0x3:
				_eatWordOrVar(opcode & 0x80);
				_eatWordOrVar(opcode & 0x40);
				break;
			case 0x4:
				break;
			case 0x6:
				if (ScummRp::game.version == 3) // Used in Loom only
					_eatWordOrVar(opcode & 0x80);
				break;
			case 0x7:
				break;
			case 0x8:
				_eatWordOrVar(opcode & 0x80);
				_eatWordOrVar(opcode & 0x40);
				break;
			case 0xF:
				_eatString(Text::LT_MSG, mainOpcode);
				break;
			default:
				throw Script::ParseError("print");
			}
			if ((opcode & 0x0F) == 0x0F)
				break;
		}
		break;
	case 0x15: // actorFromPos
	case 0x55: // actorFromPos
	case 0x95: // actorFromPos
	case 0xD5: // actorFromPos
		_eatVar();
		_eatWordOrVar(opcode & 0x80);
		_eatWordOrVar(opcode & 0x40);
		break;
	case 0x16: // getRandomNr
	case 0x96: // getRandomNr
		_eatVar();
		_eatByteOrVar(opcode & 0x80);
		break;
	case 0x17: // and
	case 0x97: // and
		_eatVar();
		_eatWordOrVar(opcode & 0x80);
		break;
	case 0x18: // jumpRelative
		_eatJump();
		break;
	case 0x19: // doSentence
	case 0x39: // doSentence
	case 0x59: // doSentence
	case 0x79: // doSentence
	case 0x99: // doSentence
	case 0xB9: // doSentence
	case 0xD9: // doSentence
	case 0xF9: // doSentence
		if (!(opcode & 0x80) && _peekByte() == 0xFE)
			_getByte();
		else
		{
			_eatByteOrVar(opcode & 0x80);
			_eatWordOrVar(opcode & 0x40);
			_eatWordOrVar(opcode & 0x20);
		}
		break;
	case 0x1A: // move
	case 0x9A: // move
		_eatVar();
		_eatWordOrVar(opcode & 0x80);
		break;
	case 0x1B: // multiply
	case 0x9B: // multiply
		_eatVar();
		_eatWordOrVar(opcode & 0x80);
		break;
	case 0x1C: // startSound
	case 0x9C: // startSound
		{
#ifdef SCUMMTR_CHANGED_JUST_AFTER_RELEASE_DEBUG
			int i = _eatByteOrVar(opcode & 0x80);
			switch (i)
			{
			case 29:
			case 35:
			case 38:
			case 39:
			case 40:
			case 41:
			case 42:
			case 54:
			case 66:
			case 67:
			case 69:
			case 70:
			case 75:
			case 76:
			case 77:
			case 80:
				std::cout << "startSound(" << i << ")" << std::endl;
				break;
			case -1:
				std::cout << "startSound(?)" << std::endl;
				break;
			}
#else
			_eatByteOrVar(opcode & 0x80);
#endif
		}
		break;
	case 0x1D: // ifClassOfIs
	case 0x9D: // ifClassOfIs
		_eatWordOrVar(opcode & 0x80);
		_eatArgList();
		_eatJump();
		break;
	case 0x1E: // walkActorTo
	case 0x3E: // walkActorTo
	case 0x5E: // walkActorTo
	case 0x7E: // walkActorTo
	case 0x9E: // walkActorTo
	case 0xBE: // walkActorTo
	case 0xDE: // walkActorTo
	case 0xFE: // walkActorTo
		_eatByteOrVar(opcode & 0x80);
		_eatWordOrVar(opcode & 0x40);
		_eatWordOrVar(opcode & 0x20);
		break;
	case 0x1F: // isActorInBox
	case 0x5F: // isActorInBox
	case 0x9F: // isActorInBox
	case 0xDF: // isActorInBox
		_eatByteOrVar(opcode & 0x80);
		_eatByteOrVar(opcode & 0x40);
		_eatJump();
		break;
	case 0x20: // stopMusic
		break;
	case 0x22: // getAnimCounter
	case 0xA2: // getAnimCounter
		_eatVar();
		_eatByteOrVar(opcode & 0x80);
		break;
	case 0x23: // getActorY
	case 0xA3: // getActorY
		_eatVar();
		if (ScummRp::game.id == GID_INDY3 && !(ScummRp::game.features & GF_MACINTOSH))
			_eatByteOrVar(opcode & 0x80);
		else
			_eatWordOrVar(opcode & 0x80);
		break;
	case 0x24: // loadRoomWithEgo
	case 0x64: // loadRoomWithEgo
	case 0xA4: // loadRoomWithEgo
	case 0xE4: // loadRoomWithEgo
		_eatWordOrVar(opcode & 0x80);
		_eatByteOrVar(opcode & 0x40);
		_getWord();
		_getWord();
		break;
	case 0x26: // setVarRange
	case 0xA6: // setVarRange
		{
			byte b;

			_eatVar();
			b = _getByte();
			do
			{
				if (opcode & 0x80)
					_getWord();
				else
					_getByte();
			} while (--b != 0);
		}
		break;
	case 0x27: // stringOps
		opcode = _getByte();
		switch (opcode & 0x1F)
		{
		case 0x01:
			_eatByteOrVar(opcode & 0x80);
			_eatString(Text::LT_RSC, mainOpcode);
			break;
		case 0x02:
			_eatByteOrVar(opcode & 0x80);
			_eatByteOrVar(opcode & 0x40);
			break;
		case 0x03:
			_eatByteOrVar(opcode & 0x80);
			_eatByteOrVar(opcode & 0x40);
			_eatByteOrVar(opcode & 0x20);
			break;
		case 0x04:
			_eatVar();
			_eatByteOrVar(opcode & 0x80);
			_eatByteOrVar(opcode & 0x40);
			break;
		case 0x05:
			_eatByteOrVar(opcode & 0x80);
			_eatByteOrVar(opcode & 0x40);
			break;
		default:
			throw Script::ParseError("stringOps");
		}
		break;
	case 0x28: // equalZero
		_eatVar();
		_eatJump();
		break;
	case 0x29: // setOwnerOf
	case 0x69: // setOwnerOf
	case 0xA9: // setOwnerOf
	case 0xE9: // setOwnerOf
		_eatWordOrVar(opcode & 0x80);
		_eatByteOrVar(opcode & 0x40);
		break;
	case 0x2B: // delayVariable
		_eatVar();
		break;
	case 0x2C: // cursorCommand
		opcode = _getByte();
		switch (opcode & 0x1F)
		{
		case 0x01:
		case 0x02:
		case 0x03:
		case 0x04:
		case 0x05:
		case 0x06:
		case 0x07:
		case 0x08:
			break;
		case 0x0A:
			_eatByteOrVar(opcode & 0x80);
			_eatByteOrVar(opcode & 0x40);
			if (ScummRp::game.id != GID_LOOM)
				throw Script::ParseError("cursorCommand case 0x0A");
			break;
		case 0x0B:
			_eatByteOrVar(opcode & 0x80);
			_eatByteOrVar(opcode & 0x40);
			_eatByteOrVar(opcode & 0x20);
			break;
		case 0x0C:
			_eatByteOrVar(opcode & 0x80);
			break;
		case 0x0D:
			_eatByteOrVar(opcode & 0x80);
			break;
		case 0x0E:
			if (ScummRp::game.version == 3)
			{
				_eatByteOrVar(opcode & 0x80);
				_eatByteOrVar(opcode & 0x40);
			}
			else
				_eatArgList();
			break;
		default:
			throw Script::ParseError("cursorCommand");
		}
		break;
	case 0x2D: // putActorInRoom
	case 0x6D: // putActorInRoom
	case 0xAD: // putActorInRoom
	case 0xED: // putActorInRoom
		_eatByteOrVar(opcode & 0x80);
		_eatByteOrVar(opcode & 0x40);
		break;
	case 0x2E: // delay
		_getByte();
		_getByte();
		_getByte();
		break;
	case 0x2F: // ifNotState
	case 0x6F: // ifNotState
	case 0xAF: // ifNotState
	case 0xEF: // ifNotState
		_eatWordOrVar(opcode & 0x80);
		_eatByteOrVar(opcode & 0x40);
		_eatJump();
		break;
	case 0x30: // matrixOps
	case 0xB0: // matrixOps
		if (ScummRp::game.version == 3)
		{
			_eatByteOrVar(opcode & 0x80);
			_getByte();
			break;
		}
		opcode = _getByte();
		switch (opcode & 0x1F)
		{
		case 0x01:
			_eatByteOrVar(opcode & 0x80);
			_eatByteOrVar(opcode & 0x40);
			break;
		case 0x02:
			_eatByteOrVar(opcode & 0x80);
			_eatByteOrVar(opcode & 0x40);
			break;
		case 0x03:
			_eatByteOrVar(opcode & 0x80);
			_eatByteOrVar(opcode & 0x40);
			break;
		case 0x04:
			break;
		default:
			throw Script::ParseError("matrixOps");
		}
		break;
	case 0x31: // getInventoryCount
	case 0xB1: // getInventoryCount
		_eatVar();
		_eatByteOrVar(0x80);
		break;
	case 0x32: // setCameraAt
	case 0xB2: // setCameraAt
		_eatWordOrVar(0x80);
		break;
	case 0x33: // roomOps
	case 0x73: // roomOps
	case 0xB3: // roomOps
	case 0xF3: // roomOps
		if (ScummRp::game.version == 3)
		{
			_eatWordOrVar(opcode & 0x80);
			_eatWordOrVar(opcode & 0x40);
		}

		opcode = _getByte();
		if (ScummRp::game.version == 3 && (opcode & 0x1F) > 6)
			throw Script::ParseError("roomOps case > 6");

		switch (opcode & 0x1F)
		{
		case 0x01:
			if (ScummRp::game.version > 3)
			{
				_eatWordOrVar(opcode & 0x80);
				_eatWordOrVar(opcode & 0x40);
			}
			break;
		case 0x02:
			if (ScummRp::game.version == 4)
			{
				_eatWordOrVar(opcode & 0x80);
				_eatWordOrVar(opcode & 0x40);
			}
			else if (ScummRp::game.version == 5)
			{
				throw Script::ParseError("roomOps case 0x02");
			}
			break;
		case 0x03:
			if (ScummRp::game.version > 3)
			{
				_eatWordOrVar(opcode & 0x80);
				_eatWordOrVar(opcode & 0x40);
			}
			break;
		case 0x04:
			if (ScummRp::game.version == 4)
			{
				_eatWordOrVar(opcode & 0x80);
				_eatWordOrVar(opcode & 0x40);
			}
			else if (ScummRp::game.version == 5)
			{
				_eatWordOrVar(opcode & 0x80);
				_eatWordOrVar(opcode & 0x40);
				_eatWordOrVar(opcode & 0x20);
				opcode = _getByte();
				_eatByteOrVar(opcode & 0x80);
			}
			break;
		case 0x05:
		case 0x06:
			break;
		case 0x07:
			_eatByteOrVar(opcode & 0x80);
			_eatByteOrVar(opcode & 0x40);
			opcode = _getByte();
			_eatByteOrVar(opcode & 0x80);
			_eatByteOrVar(opcode & 0x40);
			opcode = _getByte();
			_eatByteOrVar(opcode & 0x40);
			break;
		case 0x08:
			if (ScummRp::game.version <= 4)
			{
				if (ScummRp::game.version != 3)
				{
					_eatWordOrVar(opcode & 0x80);
					_eatWordOrVar(opcode & 0x40);
				}
				_eatWordOrVar(opcode & 0x20);
			}
			else
			{
				_eatByteOrVar(opcode & 0x80);
				_eatByteOrVar(opcode & 0x40);
				_eatByteOrVar(opcode & 0x20);
			}
			break;
		case 0x09:
			_eatByteOrVar(opcode & 0x80);
			_eatByteOrVar(opcode & 0x40);
			break;
		case 0x0A:
			_eatWordOrVar(opcode & 0x80);
			break;
		case 0x0B:
			_eatWordOrVar(opcode & 0x80);
			_eatWordOrVar(opcode & 0x40);
			_eatWordOrVar(opcode & 0x20);
			opcode = _getByte();
			_eatByteOrVar(opcode & 0x80);
			_eatByteOrVar(opcode & 0x40);
			break;
		case 0x0C:
			_eatWordOrVar(opcode & 0x80);
			_eatWordOrVar(opcode & 0x40);
			_eatWordOrVar(opcode & 0x20);
			opcode = _getByte();
			_eatByteOrVar(opcode & 0x80);
			_eatByteOrVar(opcode & 0x40);
			break;
		case 0x0D:
			_eatByteOrVar(opcode & 0x80);
			_eatString(Text::LT_PLAIN, mainOpcode);
			break;
		case 0x0E:
			_eatByteOrVar(opcode & 0x80);
			_eatString(Text::LT_PLAIN, mainOpcode);
			break;
		case 0x0F:
			_eatByteOrVar(opcode & 0x80);
			opcode = _getByte();
			_eatByteOrVar(opcode & 0x80);
			_eatByteOrVar(opcode & 0x40);
			opcode = _getByte();
			_eatByteOrVar(opcode & 0x80);
			break;
		case 0x10:
			_eatByteOrVar(opcode & 0x80);
			_eatByteOrVar(opcode & 0x40);
			break;
		default:
			throw Script::ParseError("roomOps");
		}
		break;
	case 0x34: // getDist
	case 0x74: // getDist
	case 0xB4: // getDist
	case 0xF4: // getDist
		_eatVar();
		_eatWordOrVar(opcode & 0x80);
		_eatWordOrVar(opcode & 0x40);
		break;
	case 0x35: // findObject
	case 0x75: // findObject
	case 0xB5: // findObject
	case 0xF5: // findObject
		_eatVar();
		_eatByteOrVar(opcode & 0x80);
		_eatByteOrVar(opcode & 0x40);
		break;
	case 0x36: // walkActorToObject
	case 0x76: // walkActorToObject
	case 0xB6: // walkActorToObject
	case 0xF6: // walkActorToObject
		_eatByteOrVar(opcode & 0x80);
		_eatWordOrVar(opcode & 0x40);
		break;
	case 0x37: // startObject
	case 0x77: // startObject
	case 0xB7: // startObject
	case 0xF7: // startObject
		_eatWordOrVar(opcode & 0x80);
		_eatByteOrVar(opcode & 0x40);
		_eatArgList();
		break;
	case 0x38: // lessOrEqual
	case 0xB8: // lessOrEqual
		_eatVar();
		_eatWordOrVar(opcode & 0x80);
		_eatJump();
		break;
	case 0x3A: // subtract
	case 0xBA: // subtract
		_eatVar();
		_eatWordOrVar(opcode & 0x80);
		break;
	case 0x3B: // getActorScale
	case 0xBB: // getActorScale
		if (ScummRp::game.id == GID_LOOM)
			break;

		if (ScummRp::game.id == GID_INDY3)
		{
			if (ScummRp::game.features & GF_MACINTOSH)
				throw Script::ParseError("getActorScale");

			_eatByteOrVar(opcode & 0x80);
		}
		else
		{
			_eatVar();
			_eatByteOrVar(opcode & 0x80);
		}
		break;
	case 0x3C: // stopSound
	case 0xBC: // stopSound
		{
#ifdef SCUMMTR_CHANGED_JUST_AFTER_RELEASE_DEBUG
			int i = _eatByteOrVar(opcode & 0x80);
			switch (i)
			{
			case 29:
			case 35:
			case 38:
			case 39:
			case 40:
			case 41:
			case 42:
			case 54:
			case 66:
			case 67:
			case 69:
			case 70:
			case 75:
			case 76:
			case 77:
			case 80:
				std::cout << "stopSound(" << i << ")" << std::endl;
				break;
			case -1:
				std::cout << "stopSound(?)" << std::endl;
				break;
			}
#else
			_eatByteOrVar(opcode & 0x80);
#endif
		}
		break;
	case 0x3D: // findInventory
	case 0x7D: // findInventory
	case 0xBD: // findInventory
	case 0xFD: // findInventory
		_eatVar();
		_eatByteOrVar(opcode & 0x80);
		_eatByteOrVar(opcode & 0x40);
		break;
	case 0x3F: // drawBox
	case 0x7F: // drawBox
	case 0xBF: // drawBox
	case 0xFF: // drawBox
		_eatWordOrVar(opcode & 0x80);
		_eatWordOrVar(opcode & 0x40);
		opcode = _getByte();
		_eatWordOrVar(opcode & 0x80);
		_eatWordOrVar(opcode & 0x40);
		_eatByteOrVar(opcode & 0x20);
		break;
	case 0x40: // cutscene
		_eatArgList();
		break;
	case 0x42: // chainScript
	case 0xC2: // chainScript
		{
#ifdef SCUMMTR_CHANGED_JUST_AFTER_RELEASE_DEBUG
			int i = _eatByteOrVar(opcode & 0x80);
			_eatArgList();
			if (i == 66)
				std::cout << "ahoy" << std::endl;
#else
			_eatByteOrVar(opcode & 0x80);
			_eatArgList();
#endif
		}
		break;
	case 0x43: // getActorX
	case 0xC3: // getActorX
		_eatVar();
		if (ScummRp::game.id == GID_INDY3 && !(ScummRp::game.features & GF_MACINTOSH))
			_eatByteOrVar(opcode & 0x80);
		else
			_eatWordOrVar(opcode & 0x80);
		break;
	case 0x44: // isLess
	case 0xC4: // isLess
		_eatVar();
		_eatWordOrVar(opcode & 0x80);
		_eatJump();
		break;
	case 0x46: // increment
		_eatVar();
		break;
	case 0x48: // isEqual
	case 0xC8: // isEqual
		_eatVar();
		_eatWordOrVar(opcode & 0x80);
		_eatJump();
		break;
	case 0x4C: // soundKludge
		if (ScummRp::game.version >= 5)
			_eatArgList();
		break;
	case 0x4F: // ifState
	case 0xCF: // ifState
		_eatWordOrVar(opcode & 0x80);
		_eatByteOrVar(opcode & 0x40);
		_eatJump();
		break;
	case 0x50: // pickupObjectOld
	case 0xD0: // pickupObjectOld
		_eatWordOrVar(opcode & 0x80);
		break;
	case 0x52: // actorFollowCamera
	case 0xD2: // actorFollowCamera
		_eatByteOrVar(opcode & 0x80);
		break;
	case 0x54: // setObjectName
	case 0xD4: // setObjectName
		{
			int32 obj, l;

			obj = _eatWordOrVar(opcode & 0x80);
			l = _eatString(Text::LT_RSC, mainOpcode);
			if (l > 0)
			{
				if (_gettingRscNameLimits)
					ScummTr::setRscNameMaxLengh(ScummTr::RSCT_OBJECT, obj, l);
				else if (_usingRscNameLimits)
					_text.back().padding = ScummTr::getRscNameMaxLengh(ScummTr::RSCT_OBJECT, obj);
			}
		}
		break;
	case 0x56: // getActorMoving
	case 0xD6: // getActorMoving
		_eatVar();
		_eatByteOrVar(opcode & 0x80);
		break;
	case 0x57: // or
	case 0xD7: // or
		_eatVar();
		_eatWordOrVar(opcode & 0x80);
		break;
	case 0x58: // beginOverride
		_getByte();
		break;
	case 0x5A: // add
	case 0xDA: // add
		_eatVar();
		_eatWordOrVar(opcode & 0x80);
		break;
	case 0x5B: // divide
	case 0xDB: // divide
		_eatVar();
		_eatWordOrVar(opcode & 0x80);
		break;
	case 0x5C: // oldRoomEffect
	case 0xDC: // oldRoomEffect
		opcode = _getByte();
		if ((opcode & 0x1F) == 3)
			_eatWordOrVar(opcode & 0x80);
		break;
	case 0x5D: // setClass
	case 0xDD: // setClass
		_eatWordOrVar(opcode & 0x80);
		_eatArgList();
		break;
	case 0x60: // freezeScripts
	case 0xE0: // freezeScripts
		_eatByteOrVar(opcode & 0x80);
		break;
	case 0x62: // stopScript
	case 0xE2: // stopScript
		_eatByteOrVar(opcode & 0x80);
		break;
	case 0x63: // getActorFacing
	case 0xE3: // getActorFacing
		_eatVar();
		_eatByteOrVar(opcode & 0x80);
		break;
	case 0x66: // getClosestObjActor
	case 0xE6: // getClosestObjActor
		_eatVar();
		_eatWordOrVar(opcode & 0x80);
		break;
	case 0x67: // getStringWidth
	case 0xE7: // getStringWidth
		_eatVar();
		_eatByteOrVar(opcode & 0x80);
		break;
	case 0x68: // isScriptRunning
	case 0xE8: // isScriptRunning
		_eatVar();
		_eatByteOrVar(opcode & 0x80);
		break;
	case 0x6B: // debug
	case 0xEB: // debug
		_eatWordOrVar(opcode & 0x80);
		break;
	case 0x6C: // getActorWidth
	case 0xEC: // getActorWidth
		_eatVar();
		_eatByteOrVar(opcode & 0x80);
		break;
	case 0x6E: // stopObjectScript
	case 0xEE: // stopObjectScript
		_eatWordOrVar(opcode & 0x80);
		break;
	case 0x70: // lights
	case 0xF0: // lights
		_eatByteOrVar(opcode & 0x80);
		_getByte();
		_getByte();
		break;
	case 0x71: // getActorCostume
	case 0xF1: // getActorCostume
		_eatVar();
		_eatByteOrVar(opcode & 0x80);
		break;
	case 0x72: // loadRoom
	case 0xF2: // loadRoom
		_eatByteOrVar(opcode & 0x80);
		break;
	case 0x78: // isGreater
	case 0xF8: // isGreater
		_eatVar();
		_eatWordOrVar(opcode & 0x80);
		_eatJump();
		break;
	case 0x7A: // verbOps
	case 0xFA: // verbOps
		{
			int32 verb, l;

			verb = _eatByteOrVar(opcode & 0x80);
			while ((opcode = _getByte()) != 0xFF)
			{
				switch (opcode & 0x1F)
				{
				case 0x01:
					_eatWordOrVar(opcode & 0x80);
					break;
				case 0x02:
					l = _eatString(Text::LT_RSC, mainOpcode);
					if (l > 0)
					{
						if (_gettingRscNameLimits)
							ScummTr::setRscNameMaxLengh(ScummTr::RSCT_VERB, verb, l);
						else if (_usingRscNameLimits)
							_text.back().padding = ScummTr::getRscNameMaxLengh(ScummTr::RSCT_VERB, verb);
					}
					break;
				case 0x03:
					_eatByteOrVar(opcode & 0x80);
					break;
				case 0x04:
					_eatByteOrVar(opcode & 0x80);
					break;
				case 0x05:
					_eatWordOrVar(opcode & 0x80);
					_eatWordOrVar(opcode & 0x40);
					break;
				case 0x06:
				case 0x07:
				case 0x08:
				case 0x09:
					break;
				case 0x10:
					_eatByteOrVar(opcode & 0x80);
					break;
				case 0x11:
					break;
				case 0x12:
					_eatByteOrVar(opcode & 0x80);
					break;
				case 0x13:
					break;
				case 0x14:
					_eatWordOrVar(opcode & 0x80);
					break;
				case 0x15:
					if (ScummRp::game.id == GID_LOOM && ScummRp::game.version == 4)
					{
						_getByte();
						_getByte();
						_getByte();
						_getByte();
					}
					throw Script::ParseError("verbOps case 0x15");
				case 0x16:
					_eatWordOrVar(opcode & 0x80);
					_eatByteOrVar(opcode & 0x40);
					break;
				case 0x17:
					_eatByteOrVar(opcode & 0x80);
					break;
				default:
					throw Script::ParseError("verbOps");
				}
			}
		}
		break;
	case 0x7B: // getActorWalkBox
	case 0xFB: // getActorWalkBox
		_eatVar();
		_eatByteOrVar(opcode & 0x80);
		break;
	case 0x7C: // isSoundRunning
	case 0xFC: // isSoundRunning
		_eatVar();
		{
#ifdef SCUMMTR_CHANGED_JUST_AFTER_RELEASE_DEBUG
			int i = _eatByteOrVar(opcode & 0x80);
			switch (i)
			{
			case 29:
			case 35:
			case 38:
			case 39:
			case 40:
			case 41:
			case 42:
			case 54:
			case 66:
			case 67:
			case 69:
			case 70:
			case 75:
			case 76:
			case 77:
			case 80:
				std::cout << "isSoundRunning(" << i << ")" << std::endl;
				break;
			case -1:
				std::cout << "isSoundRunning(?)" << std::endl;
				break;
			}
#else
			_eatByteOrVar(opcode & 0x80);
#endif
		}
		break;
	case 0x80: // breakHere
		break;
	case 0x98: // quitPauseRestart
		switch (_getByte())
		{
		case 0x01:
		case 0x02:
		case 0x03:
			break;
		default:
			throw Script::ParseError("quitPauseRestart");
		}
		break;
	case 0xA7: // saveLoadVars
		if (ScummRp::game.version != 3)
			throw Script::ParseError("saveLoadVars");

		_getByte();
		while ((opcode = _getByte()) != 0)
		{
			byte op;

			op = opcode & 0x1F;
			switch (op)
			{
			case 0x01:
				_eatVar();
				_eatVar();
				break;
			case 0x02:
				_eatByteOrVar(opcode & 0x80);
				_eatByteOrVar(opcode & 0x40);
				break;
			case 0x03:
				_eatString(Text::LT_MSG, mainOpcode); // according to Zak256.
				break;
			case 0x04:
				break;
			case 0x1F:
				break;
			default:
				throw Script::ParseError("saveLoadVars");
			}
			if (op == 0x1F || op == 0x04)
				break;
		}
		break;
	case 0xA8: // notEqualZero
		_eatVar();
		_eatJump();
		break;
	case 0xAB: // saveRestoreVerbs
		opcode = _getByte();
		_eatByteOrVar(opcode & 0x80);
		_eatByteOrVar(opcode & 0x40);
		_eatByteOrVar(opcode & 0x20);

		switch (opcode)
		{
		case 0x01:
		case 0x02:
		case 0x03:
			break;
		default:
			throw Script::ParseError("saveRestoreVerbs");
		}
		break;
	case 0xAC: // expression
		_eatVar();
		while ((opcode = _getByte()) != 0xFF)
		{
			switch (opcode & 0x1F)
			{
			case 0x01:
				_eatWordOrVar(opcode & 0x80);
				break;
			case 0x02:
			case 0x03:
			case 0x04:
			case 0x05:
				break;
			case 0x06:
				_opv345(r);
				break;
			default:
				throw Script::ParseError(xsprintf("expression case %i", opcode & 0x1F));
			}
		}
		break;
	case 0xAE: // wait
		if (ScummRp::game.id == GID_INDY3 && !(ScummRp::game.features & GF_MACINTOSH))
			opcode = 2;
		else
			opcode = _getByte();
		switch (opcode & 0x1F)
		{
		case 0x01:
			_eatByteOrVar(opcode & 0x80);
			break;
		case 0x02:
		case 0x03:
		case 0x04:
			break;
		default:
			throw Script::ParseError("wait");
		}
		break;
	case 0xC0: // endCutscene
		break;
	case 0xC6: // decrement
		_eatVar();
		break;
	case 0xCC: // pseudoRoom
		_getByte();
		while (_getByte() != 0)
			;
		break;
	default:
		throw Script::ParseError(xsprintf("Unknown opcode 0x%.2X", opcode));
	}
}
