/*
 * Copyright (c) 2003 Thomas Combeleran
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

#include "toc.hpp"
#include "toolbox.hpp"

#include <cstring>

/*
 * TableOfContent
 */

TableOfContent::TableOfContent(TableOfContent::Type t) :
	_toc(nullptr), _size(0), _type(t)
{
	_zap();
}

TableOfContent::~TableOfContent()
{
	_zap();
}

TableOfContent &TableOfContent::operator=(const TableOfContent &t)
{
	_zap();
	_size = t._size;
	_toc = new TocElement[_size];
	for (int i = 0; i < _size; ++i)
	{
		_toc[i].roomId = t._toc[i].roomId;
		_toc[i].offset = t._toc[i].offset;
	}
	return *this;
}

TableOfContent::Type TableOfContent::getType() const
{
	return _type;
}

void TableOfContent::accessing(byte roomId)
{
	_accessed[roomId] = true;
}

bool TableOfContent::accessed(byte roomId) const
{
	return _accessed[roomId];
}

void TableOfContent::merge(const TableOfContent &t)
{
	bool a[256];

	memcpy(a, _accessed, sizeof a);
	for (int i = 0; i < _size; ++i)
	{
		if (_toc[i].roomId != t._toc[i].roomId)
			throw std::logic_error("TableOfContent::merge: different roomIds");

		if (t._accessed[_toc[i].roomId])
		{
			if (_accessed[_toc[i].roomId] && _toc[i].offset != t._toc[i].offset)
				throw TableOfContent::Error(xsprintf("LFLF %i differently indexed from one file to another", _toc[i].roomId));

			_toc[i].offset = t._toc[i].offset;
			a[_toc[i].roomId] = true;
		}
	}
	memcpy(_accessed, a, sizeof a);
}

void TableOfContent::_zap()
{
	memset(_iterator, 0, sizeof _iterator);
	memset(_accessed, 0, sizeof _accessed);
	delete[] _toc;
	_toc = nullptr;
	_size = 0;
}

bool TableOfContent::_validItem(int id) const
{
	return _toc[id].offset > 0 && _toc[id].roomId != 0xFF;
}

bool TableOfContent::_idInRange(int id) const
{
	return id >= 0 && id < _size;
}

bool TableOfContent::_validId(int id) const
{
	if (!_idInRange(id))
		return false;

	return _validItem(id);
}

int TableOfContent::getSize() const
{
	return _size;
}

TableOfContent::TocElement &TableOfContent::operator[](int id)
{
	if (!_idInRange(id))
		throw TableOfContent::InvalidId(xsprintf("TableOfContent::operator[]: Invalid Id: %i", id));

	return _toc[id];
}

TableOfContent::TocElement TableOfContent::operator[](int id) const
{
	if (!_idInRange(id))
		throw TableOfContent::InvalidId(xsprintf("TableOfContent::operator[]: Invalid Id: %i", id));

	return _toc[id];
}

int TableOfContent::count(byte roomId, int32 offset) const
{
	int total;

	total = 0;
	for (int i = 1; i < _size; ++i)
		if (_toc[i].offset == offset && _toc[i].roomId == roomId)
			++total;

	return total;
}

int TableOfContent::findId(byte roomId, int32 offset) const
{
	for (int i = 1; i < _size; ++i)
		if (_toc[i].offset == offset && _toc[i].roomId == roomId)
			return i;

	throw TableOfContent::InvalidElement(xsprintf("TableOfContent::findId: Cannot find element (%u)", offset));
}

void TableOfContent::firstId(byte roomId)
{
	_iterator[roomId] = 0;
}

bool TableOfContent::nextId(int &id, byte roomId)
{
	while (_idInRange(_iterator[roomId]) && (_toc[_iterator[roomId]].roomId != roomId || !_validItem(_iterator[roomId])))
		++_iterator[roomId];

	id = _validId(_iterator[roomId]) ? _iterator[roomId]++ : TableOfContent::INVALID_ID;

	return id != TableOfContent::INVALID_ID;
}

void TableOfContent::_load16Sep32(FilePart &file)
{
	uint16 w;

	_zap();
	try
	{
		file.getLE16(w);
		_size = (int)w;
		_toc = new TocElement[_size];

		for (int i = 0; i < _size; ++i)
			file.getByte(_toc[i].roomId);

		for (int i = 0; i < _size; ++i)
			file.getLE32(_toc[i].offset);
	}
	catch (...) { _zap(); throw; }
}

void TableOfContent::_save16Sep32(FilePart &file) const
{
	file.putLE16((uint16)_size);

	for (int i = 0; i < _size; ++i)
		file.putByte(_toc[i].roomId);

	for (int i = 0; i < _size; ++i)
		file.putLE32(_toc[i].offset);
}

void TableOfContent::_load8Sep16(FilePart &file, int size)
{
	byte b;
	uint16 w;

	_zap();
	try
	{
		if (size == 0)
		{
			file.getByte(b);
			_size = (int)b;
		}
		else
			_size = size;
		_toc = new TocElement[_size];

		for (int i = 0; i < _size; ++i)
			file.getByte(_toc[i].roomId);

		for (int i = 0; i < _size; ++i)
		{
			file.getLE16(w);
			_toc[i].offset = (w == 0xFFFF) ? -1 : (int32)w;
		}
	}
	catch (...) { _zap(); throw; }
}

void TableOfContent::_save8Sep16(FilePart &file, bool fixedSize) const
{
	if (!fixedSize)
		file.putByte((byte)_size);

	for (int i = 0; i < _size; ++i)
		file.putByte(_toc[i].roomId);

	for (int i = 0; i < _size; ++i)
		file.putLE16((uint16)_toc[i].offset);
}

void TableOfContent::_load8Mix32(FilePart &file)
{
	byte b;

	_zap();
	try
	{
		file.getByte(b);
		_size = (int)b;
		_toc = new TocElement[_size];
		for (int i = 0; i < _size; ++i)
		{
			file.getByte(_toc[i].roomId);
			file.getLE32(_toc[i].offset);
		}
	}
	catch (...) { _zap(); throw; }
}

void TableOfContent::_save8Mix32(FilePart &file) const
{
	file.putByte((byte)_size);
	for (int i = 0; i < _size; ++i)
	{
		file.putByte(_toc[i].roomId);
		file.putLE32(_toc[i].offset);
	}
}

void TableOfContent::_load16Mix32(FilePart &file)
{
	uint16 w;

	_zap();
	try
	{
		file.getLE16(w);
		_size = (int)w;
		_toc = new TocElement[_size];
		for (int i = 0; i < _size; ++i)
		{
			file.getByte(_toc[i].roomId);
			file.getLE32(_toc[i].offset);
		}
	}
	catch (...) { _zap(); throw; }
}

void TableOfContent::_save16Mix32(FilePart &file) const
{
	file.putLE16((uint16)_size);
	for (int i = 0; i < _size; ++i)
	{
		file.putByte(_toc[i].roomId);
		file.putLE32(_toc[i].offset);
	}
}

void TableOfContent::load(FilePart &file, GlobalTocFormat format, int size)
{
	switch (format)
	{
	case GTCFMT_8SEP16:
		_load8Sep16(file, size);
		break;
	default:
		throw std::logic_error("TableOfContent::load: Invalid format (not V1)");
	}
}

void TableOfContent::load(FilePart &file, GlobalTocFormat format)
{
	switch (format)
	{
	case GTCFMT_16SEP32:
		_load16Sep32(file);
		break;
	case GTCFMT_8SEP16:
		_load8Sep16(file, 0);
		break;
	case GTCFMT_8MIX32:
		_load8Mix32(file);
		break;
	case GTCFMT_16MIX32:
		_load16Mix32(file);
		break;
	case GTCFMT_NULL:
		throw std::logic_error("TableOfContent::load: Invalid format");
	}
}

void TableOfContent::save(FilePart &file, GlobalTocFormat format, bool fixedSize)
{
	switch (format)
	{
	case GTCFMT_8SEP16:
		_save8Sep16(file, fixedSize);
		break;
	default:
		throw std::logic_error("TableOfContent::save: Invalid format (not V1)");
	}
}

void TableOfContent::save(FilePart &file, GlobalTocFormat format)
{
	switch (format)
	{
	case GTCFMT_16SEP32:
		_save16Sep32(file);
		break;
	case GTCFMT_8SEP16:
		_save8Sep16(file, false);
		break;
	case GTCFMT_8MIX32:
		_save8Mix32(file);
		break;
	case GTCFMT_16MIX32:
		_save16Mix32(file);
		break;
	case GTCFMT_NULL:
		throw std::logic_error("TableOfContent::save: Invalid format");
	}
}

/*
 * GlobalRoomIndex
 */

GlobalRoomIndex::GlobalRoomIndex() :
	TableOfContent(TableOfContent::TOCT_ROOM), _first(false)
{
}

GlobalRoomIndex::~GlobalRoomIndex()
{
	_zap();
}

void GlobalRoomIndex::_zap()
{
	TableOfContent::_zap();
	_first = false;
}

void GlobalRoomIndex::merge(const TableOfContent &t)
{
	for (int i = 0; i < _size; ++i)
		if (t.accessed((byte)i))
		{
			if (_accessed[i] && _toc[i].offset != t[i].offset)
				throw TableOfContent::Error(xsprintf("LFLF %i differently indexed from one file to another", i));

			_toc[i].offset = t[i].offset;
			_accessed[i] = true;
		}
}

int GlobalRoomIndex::count(byte, int32) const
{
	return 0;
}

int GlobalRoomIndex::findId(byte, int32) const
{
	throw std::logic_error("GlobalRoomIndex::findId: Shouldn't be here");
}

void GlobalRoomIndex::firstId(byte)
{
	_first = true;
}

bool GlobalRoomIndex::nextId(int &id, byte roomId)
{
	if (!_first)
		return false;

	_first = false;
	id = roomId;

	return true;
}

TableOfContent::TocElement &GlobalRoomIndex::operator[](int id)
{
	if (id >= _size)
		throw GlobalRoomIndex::IndexTooShort("Room index too short");

	return TableOfContent::operator[](id);
}

TableOfContent::TocElement GlobalRoomIndex::operator[](int id) const
{
	if (id >= _size)
		throw GlobalRoomIndex::IndexTooShort("Room index too short");

	return TableOfContent::operator[](id);
}

void GlobalRoomIndex::load(FilePart &file, GlobalTocFormat format, int size)
{
	TableOfContent::load(file, format, size);

	if (_size > 256)
		throw GlobalRoomIndex::Error("RoomIndex too big (> 256)");

	// hack for V1
	if (size > 0) // means V1
		for (int i = 0; i < _size; ++i)
			_toc[i].offset = 0;
}

void GlobalRoomIndex::save(FilePart &file, GlobalTocFormat format, bool fixedSize)
{
	// hack for V1
	if (fixedSize) // means V1
		TableOfContent::save(file, format, fixedSize);
}

int GlobalRoomIndex::numberOfDisks() const
{
	int max;

	max = 0;
	for (int i = 0; i < _size; ++i)
		if (_toc[i].roomId > max)
			max = _toc[i].roomId;

	return ++max;
}

/*
 * RoomIndex
 */

RoomIndex::RoomIndex() :
	_toc(TableOfContent::TOCT_LFLF), _iterator(0)
{
	memset(_map, -1, sizeof _map);
}

RoomIndex::~RoomIndex()
{
}

byte RoomIndex::findId(int32 offset) const
{
	for (int i = 0; i < _toc.getSize(); ++i)
		if (_toc[i].offset == offset)
			return _toc[i].roomId;

	throw TableOfContent::InvalidElement(xsprintf("RoomIndex::findId: Cannot find element (%u)", offset));
}

int32 &RoomIndex::operator[](byte roomId)
{
	if (_map[roomId] != -1)
		return _toc[_map[roomId]].offset;

	throw TableOfContent::InvalidId(xsprintf("RoomIndex::operator[]: Invalid Id: %i", roomId));
}

void RoomIndex::load(FilePart &file)
{
	memset(_map, -1, sizeof _map);
	_toc.load(file, GTCFMT_8MIX32);

	for (int i = 0; i < _toc.getSize(); ++i)
		_map[_toc[i].roomId] = i;
}

void RoomIndex::save(FilePart &file)
{
	_toc.save(file, GTCFMT_8MIX32);
}

void RoomIndex::firstId()
{
	_iterator = 0;
}

bool RoomIndex::nextId(byte &roomId)
{
	if (_iterator >= _toc.getSize())
		return false;

	roomId = _toc[_iterator++].roomId;
	return true;
}
