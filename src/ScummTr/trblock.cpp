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

#include "common/file.hpp"
#include "common/io.hpp"

#include "trblock.hpp"
#include "scummtr.hpp"

/*
 * TextBlock
 */

TextBlock::TextBlock() :
    LeafBlock()
{
}

TextBlock::~TextBlock()
{
}

TextBlock::TextBlock(const TreeBlock &block) :
    LeafBlock(block)
{
}

TextBlock &TextBlock::operator=(const TreeBlock &block)
{
	LeafBlock::operator=(block);
	return *this;
}

int TextBlock::_lflfId() const
{
	const TreeBlock *block;

	for (block = _parent; block != nullptr; block = block->_parent)
		if (dynamic_cast<const RoomPack *>(block) != nullptr)
			break;

	if (block != nullptr)
		return block->_id;

	return -1;
}

int TextBlock::_ownId() const
{
	const TreeBlock *block;

	for (block = this; block != nullptr; block = block->_parent)
		if (block->_id != -1)
			return block->_id;

	return -1;
}

/*
 * ScriptBlock
 */

ScriptBlock::ScriptBlock(int32 subHeaderSize) :
    TextBlock(), _script(nullptr)
{
	_headerSize += subHeaderSize;
}

ScriptBlock::~ScriptBlock()
{
	delete _script;
}

ScriptBlock::ScriptBlock(const TreeBlock &block, int32 subHeaderSize) :
    TextBlock(block), _script(nullptr)
{
	_headerSize += subHeaderSize;
}

ScriptBlock &ScriptBlock::operator=(const TreeBlock &block)
{
	delete _script;
	_script = nullptr;
	TextBlock::operator=(block);

	return *this;
}

void ScriptBlock::importText(Text &input)
{
	int32 oldSize;

	oldSize = _file->size();

	if (_script == nullptr)
		_script = new Script(*_file, _headerSize, _file->size() - _headerSize);

	input.setInfo(_lflfId(), _tag, _ownId());

	try
	{
		_script->importText(input);

		_file->seekp(0, std::ios::beg);
		Block::_writeHeader(_blockFormat, *_file, _file->size(), _tag);

		if (_parent != nullptr)
			_parent->_subblockUpdated(*this, _file->size() - oldSize);
	}
	catch (Script::ParseError &e)
	{
		ScummRpIO::error(xsprintf("%s %s", e.what(), input.info()));
	}
}

void ScriptBlock::exportText(Text &output, bool pad)
{
	if (_script == nullptr)
		_script = new Script(*_file, _headerSize, _file->size() - _headerSize);

	output.setInfo(_lflfId(), _tag, _ownId());
	try
	{
		_script->exportText(output, pad);
	}
	catch (Script::ParseError &e)
	{
		ScummRpIO::error(xsprintf("%s %s", e.what(), output.info()));
	}
}

void ScriptBlock::getRscNameLimits()
{
	if (_script == nullptr)
		_script = new Script(*_file, _headerSize, _file->size() - _headerSize);

	try
	{
		_script->getRscNameLimits();
	}
	catch (Script::ParseError &)
	{
	}
}

/*
 * ObjectNameBlock
 */

ObjectNameBlock::ObjectNameBlock() :
    TextBlock()
{
}

ObjectNameBlock::~ObjectNameBlock()
{
}

ObjectNameBlock::ObjectNameBlock(const TreeBlock &block) :
    TextBlock(block)
{
}

ObjectNameBlock &ObjectNameBlock::operator=(const TreeBlock &block)
{
	TextBlock::operator=(block);

	return *this;
}

void ObjectNameBlock::importText(Text &input)
{
	std::string s;
	int32 oldSize;

	oldSize = _file->size();
	if (oldSize - _headerSize - 1 <= 0)
		return; // Ignore empty lines

	if (!input.nextLine(s, Text::LT_RSC))
		throw File::UnexpectedEOF("Not enough lines in imported text");

	_file->seekp(_headerSize, std::ios::beg);

	s += '\0';
	_file->write(s);

	if (oldSize > (int32)s.size() + _headerSize)
		_file->resize((int32)s.size() + _headerSize);

	_file->seekp(0, std::ios::beg);
	Block::_writeHeader(_blockFormat, *_file, _file->size(), _tag);

	if (_parent != nullptr)
		_parent->_subblockUpdated(*this, _file->size() - oldSize);
}

void ObjectNameBlock::exportText(Text &output, bool pad)
{
	std::string s;
	byte b;

	s.reserve(32);

	_file->seekg(_headerSize, std::ios::beg);

	for (_file->getByte(b); b != 0; _file->getByte(b))
		s += (char)b;

	output.setInfo(_lflfId(), _tag, _ownId());

	if (pad && s.size() > 0 && ScummTr::getRscNameMaxLengh(ScummTr::RSCT_OBJECT, _id) > (int32)s.size())
		s.resize(ScummTr::getRscNameMaxLengh(ScummTr::RSCT_OBJECT, _id), '@');

	output.addLine(s, Text::LT_RSC);
}

void ObjectNameBlock::getRscNameLimits()
{
	int i;
	byte b;

	_file->seekg(_headerSize, std::ios::beg);

	i = 0;
	for (_file->getByte(b); b != 0; _file->getByte(b))
		++i;

	ScummTr::setRscNameMaxLengh(ScummTr::RSCT_OBJECT, _id, i);
}

/*
 * ObjectCodeBlock
 */

ObjectCodeBlock::ObjectCodeBlock() :
    TextBlock(), _script(nullptr)
{
}

ObjectCodeBlock::~ObjectCodeBlock()
{
	delete _script;
}

ObjectCodeBlock::ObjectCodeBlock(const TreeBlock &block) :
    TextBlock(block), _script(nullptr)
{
}

ObjectCodeBlock &ObjectCodeBlock::operator=(const TreeBlock &block)
{
	delete _script;
	_script = nullptr;
	TextBlock::operator=(block);

	return *this;
}

template <class T, int I>
void ObjectCodeBlock::_tListVerbs(std::list<int32> &l, int32 scriptOffset)
{
	T o;
	byte b;

	l.resize(0);
	_file->seekg(I + _headerSize, std::ios::beg);

	for (_file->getByte(b); b != 0; _file->getByte(b))
	{
		(*_file).getLE(o);
		l.push_back((int32)o - scriptOffset);
	}
}

template <class T, int I>
void ObjectCodeBlock::_tUpdateVerbs(const std::list<int32> &l, int32 scriptOffset, int n)
{
	T o;

	_file->seekp(I + _headerSize, std::ios::beg);

	for (std::list<int32>::const_iterator i = l.begin(); i != l.end(); ++i)
	{
		_file->seekp(1, std::ios::cur);
		if (((uint32)(*i + scriptOffset) >> (sizeof(T) * 8)) != 0)
			throw ObjectCodeBlock::Error(xsprintf("Line too long (line %i)", n));

		o = (T)(*i + scriptOffset);
		_file->putLE(o);
	}
}

template <class T, int I>
int32 ObjectCodeBlock::_tFindScriptOffset()
{
	int32 min;
	T o;
	byte b;

	_file->seekg(I + _headerSize, std::ios::beg);

	min = _file->size();
	for (_file->getByte(b); b != 0; _file->getByte(b))
	{
		_file->getLE(o);
		if ((int32)o < min)
			min = (int32)o;
	}

	return min;
}

void ObjectCodeBlock::_listVerbs(std::list<int32> &l, int32 scriptOffset)
{
	ObjectCodeBlock::_tListVerbs<uint16, 0x00>(l, scriptOffset);
}

void ObjectCodeBlock::_updateVerbs(const std::list<int32> &l, int32 scriptOffset, int n)
{
	ObjectCodeBlock::_tUpdateVerbs<uint16, 0x00>(l, scriptOffset, n);
}

int32 ObjectCodeBlock::_findScriptOffset()
{
	return ObjectCodeBlock::_tFindScriptOffset<uint16, 0x00>();
}

void ObjectCodeBlock::_importText(Text &input, int32 oldSize, int32 scriptOffset)
{
	std::list<int32> verbs;

	_listVerbs(verbs, scriptOffset);
	if (_script == nullptr)
		_script = new Script(*_file, scriptOffset, _file->size() - scriptOffset);

	input.setInfo(_lflfId(), _tag, _ownId());
	try
	{
		_script->setTrackedSpots(verbs);
		_script->importText(input);
		_script->getTrackedSpots(verbs);
	}
	catch (Script::ParseError &e)
	{
		ScummRpIO::error(xsprintf("%s %s", e.what(), input.info()));
	}

	_updateVerbs(verbs, scriptOffset, input.lineNumber());

	_file->seekp(0, std::ios::beg);
	Block::_writeHeader(_blockFormat, *_file, _file->size(), _tag);

	if (_parent != nullptr)
		_parent->_subblockUpdated(*this, _file->size() - oldSize);
}

void ObjectCodeBlock::importText(Text &input)
{
	_importText(input, _file->size(), _findScriptOffset());
}

void ObjectCodeBlock::exportText(Text &output, bool pad)
{
	int32 o;

	o = _findScriptOffset();
	if (_script == nullptr)
		_script = new Script(*_file, o, _file->size() - o);

	output.setInfo(_lflfId(), _tag, _ownId());
	try
	{
		_script->exportText(output, pad);
	}
	catch (Script::ParseError &e)
	{
		ScummRpIO::error(xsprintf("%s %s", e.what(), output.info()));
	}
}

void ObjectCodeBlock::getRscNameLimits()
{
	int32 o;

	o = _findScriptOffset();
	if (_script == nullptr)
		_script = new Script(*_file, o, _file->size() - o);

	try
	{
		_script->getRscNameLimits();
	}
	catch (Script::ParseError &)
	{
	}
}

/*
 * OldObjectCodeBlock
 */

OldObjectCodeBlock::OldObjectCodeBlock() :
    ObjectCodeBlock()
{
}

OldObjectCodeBlock::~OldObjectCodeBlock()
{
}

OldObjectCodeBlock::OldObjectCodeBlock(const TreeBlock &block) :
    ObjectCodeBlock(block)
{
}

OldObjectCodeBlock &OldObjectCodeBlock::operator=(const TreeBlock &block)
{
	ObjectCodeBlock::operator=(block);
	return *this;
}

void OldObjectCodeBlock::_listVerbs(std::list<int32> &l, int32 scriptOffset)
{
	ObjectCodeBlock::_tListVerbs<uint16, 0x0D>(l, scriptOffset);
}

void OldObjectCodeBlock::_updateVerbs(const std::list<int32> &l, int32 scriptOffset, int n)
{
	ObjectCodeBlock::_tUpdateVerbs<uint16, 0x0D>(l, scriptOffset, n);
}

int32 OldObjectCodeBlock::_findScriptOffset()
{
	return ObjectCodeBlock::_tFindScriptOffset<uint16, 0x0D>();
}

template <int I>
void OldObjectCodeBlock::_exportName(Text &output, bool pad)
{
	std::string s;
	byte b;
	uint32 tag;

	tag = ((_tag & 0xFFFF0000) != 0) ? (_tag & 0xFF00FFFF) | ('N' << 16) : (_tag & 0x0000FF00) | 'N';
	output.setInfo(_lflfId(), tag, _ownId());

	s.reserve(32);

	_file->seekg(I + _headerSize, std::ios::beg);
	_file->getByte(b);

	_file->seekg(b, std::ios::beg);
	for (_file->getByte(b); b != 0; _file->getByte(b))
		s += (char)b;

	if (pad && s.size() > 0 && ScummTr::getRscNameMaxLengh(ScummTr::RSCT_OBJECT, _id) > (int32)s.size())
		s.resize(ScummTr::getRscNameMaxLengh(ScummTr::RSCT_OBJECT, _id), '@');

	output.addLine(s, Text::LT_RSC);
}

template <int I>
void OldObjectCodeBlock::_importName(Text &input, int32 &scriptOffset)
{
	std::string s;
	FilePartHandle f;
	byte b, o;
	std::list<int32> verbs;
	int32 size, sizeDiff;

	_file->seekg(I + _headerSize, std::ios::beg);
	_file->getByte(o);

	_file->seekg(o, std::ios::beg);
	for (_file->getByte(b); b != 0; _file->getByte(b))
		;

	size = _file->tellg(std::ios::beg) - o;
	if (size <= 1)
		return; // Ignore empty lines

	if (!input.nextLine(s, Text::LT_RSC))
		throw File::UnexpectedEOF("Not enough lines in imported text");

	f = new FilePart(*_file, o, size);

	f->resize((int32)s.size() + 1);
	f->seekp(0, std::ios::beg);

	f->write(s);
	f->putByte((byte)0);

	sizeDiff = f->size() - size;
	scriptOffset += sizeDiff;
	if (sizeDiff != 0)
	{
		_listVerbs(verbs, scriptOffset);

		for (std::list<int32>::iterator i = verbs.begin(); i != verbs.end(); ++i)
			*i += sizeDiff;

		_updateVerbs(verbs, scriptOffset, input.lineNumber());
	}
}

void OldObjectCodeBlock::importText(Text &input)
{
	int32 oldSize, scriptOffset;

	oldSize = _file->size();
	scriptOffset = _findScriptOffset();

	OldObjectCodeBlock::_importName<0x0C>(input, scriptOffset);

	_importText(input, oldSize, scriptOffset);
}

void OldObjectCodeBlock::exportText(Text &output, bool pad)
{
	OldObjectCodeBlock::_exportName<0x0C>(output, pad);

	ObjectCodeBlock::exportText(output, pad);
}

void OldObjectCodeBlock::getRscNameLimits()
{
	int i;
	byte b;

	_file->seekg(0x0C + _headerSize, std::ios::beg);
	_file->getByte(b);

	_file->seekg(b, std::ios::beg);
	i = 0;
	for (_file->getByte(b); b != 0; _file->getByte(b))
		++i;

	ScummTr::setRscNameMaxLengh(ScummTr::RSCT_OBJECT, _id, i);

	ObjectCodeBlock::getRscNameLimits();
}

/*
 * OldObjectCodeBlockV1
 */

OldObjectCodeBlockV1::OldObjectCodeBlockV1() :
    OldObjectCodeBlock()
{
}

OldObjectCodeBlockV1::~OldObjectCodeBlockV1()
{
}

OldObjectCodeBlockV1::OldObjectCodeBlockV1(const TreeBlock &block) :
    OldObjectCodeBlock(block)
{
}

OldObjectCodeBlockV1 &OldObjectCodeBlockV1::operator=(const TreeBlock &block)
{
	OldObjectCodeBlock::operator=(block);
	return *this;
}

void OldObjectCodeBlockV1::_listVerbs(std::list<int32> &l, int32 scriptOffset)
{
	ObjectCodeBlock::_tListVerbs<byte, 0x0B>(l, scriptOffset);
}

void OldObjectCodeBlockV1::_updateVerbs(const std::list<int32> &l, int32 scriptOffset, int n)
{
	ObjectCodeBlock::_tUpdateVerbs<byte, 0x0B>(l, scriptOffset, n);
}

int32 OldObjectCodeBlockV1::_findScriptOffset()
{
	return ObjectCodeBlock::_tFindScriptOffset<byte, 0x0B>();
}

void OldObjectCodeBlockV1::importText(Text &input)
{
	int32 oldSize, scriptOffset;

	oldSize = _file->size();
	scriptOffset = _findScriptOffset();

	OldObjectCodeBlock::_importName<0x0A>(input, scriptOffset);

	_importText(input, oldSize, scriptOffset);
}

void OldObjectCodeBlockV1::exportText(Text &output, bool pad)
{
	OldObjectCodeBlock::_exportName<0x0A>(output, pad);

	ObjectCodeBlock::exportText(output, pad);
}

void OldObjectCodeBlockV1::getRscNameLimits()
{
	int i;
	byte b;

	_file->seekg(0x0A + _headerSize, std::ios::beg);
	_file->getByte(b);

	_file->seekg(b, std::ios::beg);
	i = 0;
	for (_file->getByte(b); b != 0; _file->getByte(b))
		++i;

	ScummTr::setRscNameMaxLengh(ScummTr::RSCT_OBJECT, _id, i);

	ObjectCodeBlock::getRscNameLimits();
}
