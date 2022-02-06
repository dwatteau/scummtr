/*
 * SPDX-License-Identifier: MIT
 *
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

#include "common/io.hpp"
#include "common/toolbox.hpp"

#include "block.hpp"
#include "scummrp.hpp"
#include "rptypes.hpp"

#include <cstdio>
#include <cstring>

#include <algorithm>
#include <iostream>

/*
 * Block
 */

Block::Block() :
    _blockFormat(BFMT_NULL), _headerSize(0),
    _tag(0), _id(-1), _file(nullptr)
{
}

Block::Block(const Block &block) :
    _blockFormat(block._blockFormat), _headerSize(block._headerSize),
    _tag(block._tag), _id(block._id), _file(block._file)
{
}

Block &Block::operator=(const Block &block)
{
	_blockFormat = block._blockFormat;
	_headerSize = block._headerSize;
	_tag = block._tag;
	_id = block._id;
	_file = block._file;

	return *this;
}

Block::~Block()
{
}

int32 Block::getHeaderSize() const
{
	return _headerSize;
}

int32 Block::getSize() const
{
	if (_file == nullptr)
		return 0;

	return _file->size();
}

uint32 Block::getTag() const
{
	return _tag;
}

int Block::getId() const
{
	return _id;
}

const char *Block::tagToStr(uint32 tag)
{
	static char strTag[2][5] = { { 0 }, { 0 } };
	static int currentStr = 0;

	currentStr ^= 1;
	if (tag & 0xFFFF0000)
	{
		strTag[currentStr][0] = (char)(tag >> 24);
		strTag[currentStr][1] = (char)((tag >> 16) & 0xFF);
		strTag[currentStr][2] = (char)((tag >> 8) & 0xFF);
		strTag[currentStr][3] = (char)(tag & 0xFF);
		strTag[currentStr][4] = '\0';
	}
	else
	{
		strTag[currentStr][0] = (char)(tag >> 8);
		strTag[currentStr][1] = (char)(tag & 0xFF);
		strTag[currentStr][2] = '\0';
		if (strTag[currentStr][0] == '\0') // FIXME hack for Loom Talkie
		{
			strTag[currentStr][0] = '_';
			strTag[currentStr][1] = '_';
		}
	}

	return strTag[currentStr];
}

const char *Block::_fileName() const
{
	static char fileName[32] = { 0 };

	if (_id >= 0)
		snprintf(fileName, sizeof(fileName), "%s_%.4i", Block::tagToStr(_tag), _id);
	else
		snprintf(fileName, sizeof(fileName), "%s", Block::tagToStr(_tag));

	return fileName;
}

void Block::_readHeader(BlockFormat format, FilePart &file, int32 &size, uint32 &tag)
{
	uint16 w;

	switch (format)
	{
	case BFMT_SHORTTAG:
		file.getLE32(size);
		file.getBE16(w);
		tag = (uint32)w;
		break;
	case BFMT_LONGTAG:
		file.getBE32(tag);
		file.getBE32(size);
		break;
	case BFMT_LONGTAG_ALTSIZE:
		file.getBE32(tag);
		file.getBE32(size);
		size += 8;
		break;
	case BFMT_SIZEONLY:
		file.getLE16(w);
		size = (int32)w;
		break;
	case BFMT_NOHEADER:
		size = 0;
		break;
	case BFMT_NULL:
		throw Block::InvalidBlock("Block::_readHeader: No block format specified");
	}
}

void Block::_writeHeader(BlockFormat format, FilePart &file, int32 size, uint32 tag)
{
	switch (format)
	{
	case BFMT_SHORTTAG:
		file.putLE32(size);
		file.putBE16((uint16)tag);
		break;
	case BFMT_LONGTAG:
		file.putBE32(tag);
		file.putBE32(size);
		break;
	case BFMT_LONGTAG_ALTSIZE:
		file.putBE32(tag);
		file.putBE32(size - 8);
		break;
	case BFMT_SIZEONLY:
		if (size >= 0x10000)
			throw Block::InvalidDataFromDump("Block size too big");
		file.putLE16((uint16)size);
		break;
	case BFMT_NOHEADER:
		break;
	case BFMT_NULL:
		throw Block::InvalidBlock("Block::_writeHeader: No block format specified");
	}
}

/*
 * TreeBlock
 */

TreeBlock::TreeBlock() :
    Block(), _version(0), _parent(nullptr), _parentVersion(0),
    _nextSubblockOffset(0), _childrenCount(0)
{
}

TreeBlock::~TreeBlock()
{
	if (_parent)
		--_parent->_childrenCount;
	if (_childrenCount)
		ScummIO::crash("TreeBlock destroyed before its children");
}

TreeBlock::TreeBlock(const TreeBlock &block) :
    Block(block), _version(block._version), _parent(block._parent), _parentVersion(block._parentVersion),
    _nextSubblockOffset(block._nextSubblockOffset), _childrenCount(0)
{
	if (_parent != nullptr)
		++_parent->_childrenCount;
}

TreeBlock &TreeBlock::operator=(const TreeBlock &block)
{
	Block::operator=(block);
	_leaveParent();
	_version = block._version;
	_parent = block._parent;
	_parentVersion = block._parentVersion;
	_nextSubblockOffset = block._nextSubblockOffset;
	_childrenCount = 0;
	if (_parent)
		++_parent->_childrenCount;

	return *this;
}

void TreeBlock::_init()
{
}

int TreeBlock::_findIdInBlock(TreeBlock &block)
{
	byte b;
	uint16 w;
	TreeBlock subblock;

	switch (block._tag)
	{
	case MKTAG4('O','B','I','M'):
		block.firstBlock();
		while (block.nextBlock(subblock))
		{
			if (subblock._tag == MKTAG4('I','M','H','D'))
			{
				if (ScummRp::game.version < 7)
					subblock._file->seekg(subblock._headerSize, std::ios::beg);
				else
					subblock._file->seekg(subblock._headerSize + 4, std::ios::beg);
				subblock._file->getLE16(w);
				return w;
			}
		}
		throw Block::InvalidDataFromGame("Cannot find IMHD block in OBIM block", block._file->name(), block._file->fullOffset());
	case MKTAG4('O','B','C','D'):
		block.firstBlock();
		while (block.nextBlock(subblock))
		{
			if (subblock._tag == MKTAG4('C','D','H','D'))
			{
				if (ScummRp::game.version < 7)
					subblock._file->seekg(subblock._headerSize, std::ios::beg);
				else
					subblock._file->seekg(subblock._headerSize + 4, std::ios::beg);
				subblock._file->getLE16(w);
				return w;
			}
		}
		throw Block::InvalidDataFromGame("Cannot find CDHD block in OBCD block", block._file->name(), block._file->fullOffset());
	case MKTAG2('O','I'):
	case MKTAG2('O','C'):
	case MKTAG4('O','C','v','1'):
	case MKTAG4('O','C','v','2'):
	case MKTAG4('O','C','v','3'):
		block._file->seekg(block._headerSize, std::ios::beg);
		block._file->getLE16(w);
		return w;
	case MKTAG4('L','S','C','R'):
	case MKTAG2('L','S'):
		block._file->seekg(block._headerSize, std::ios::beg);
		block._file->getByte(b);
		return b;
	default:
		return -1;
	}
}

int TreeBlock::_findSubblockId(TreeBlock &subblock) const
{
	return TreeBlock::_findIdInBlock(subblock);
}

void TreeBlock::_leaveParent()
{
	if (_childrenCount > 0)
		throw std::logic_error("TreeBlock::_leaveParent: Block has children");

	if (_parent)
	{
		--_parent->_childrenCount;
		_parent = nullptr;
	}
}

void TreeBlock::_adopt(TreeBlock &subblock)
{
	subblock._parent = this;
	++_childrenCount;
	subblock._parentVersion = _version;
	++subblock._version;
}

void TreeBlock::_makeSubblock(TreeBlock &subblock, BlockFormat blockFormat, int32 headerSize)
{
	int32 size;

	subblock._blockFormat = blockFormat;
	subblock._headerSize = headerSize;
	_file->seekg(_nextSubblockOffset, std::ios::beg);
	Block::_readHeader(subblock._blockFormat, *_file, size, subblock._tag);

	if (size < 0 || size + _nextSubblockOffset > _file->size())
		throw Block::InvalidDataFromGame(xsprintf("Block too big: 0x%X", size), _file->name(), _nextSubblockOffset + _file->fullOffset());
	if (size < (int32)headerSize)
		throw Block::InvalidDataFromGame(xsprintf("Block too short: 0x%X", size), _file->name(), _nextSubblockOffset + _file->fullOffset());

	subblock._file = new FilePart(*_file, _nextSubblockOffset, size);
}

bool TreeBlock::_readNextSubblock(TreeBlock &subblock)
{
	subblock._leaveParent();

	if (_nextSubblockOffset == _file->size())
		return false;
	if (_nextSubblockOffset > _file->size())
		throw Block::InvalidBlock("TreeBlock::_readNextSubblock: Out of bounds");

	_makeSubblock(subblock, ScummRp::game.blockFormat, ScummRp::game.blockHeaderSize);
	subblock._id = _findSubblockId(subblock);
	_nextSubblockOffset += subblock._file->size();

	return true;
}

bool TreeBlock::nextBlock(TreeBlock &subblock)
{
	bool r;

	try
	{
		r = _readNextSubblock(subblock);
	}
	catch (Block::InvalidDataFromGame &e)
	{
		ScummIO::warning(xsprintf("Bad data was found and ignored at 0x%X in %s", e.offset(), e.file().c_str()));
		return false;
	}

	if (!r)
		return false;

	_adopt(subblock);
	subblock._init();

	return true;
}

template <class T>
T *TreeBlock::_nextBlock()
{
	T *subblock;

	subblock = nullptr;
	try
	{
		subblock = new T();
		if (nextBlock(*subblock))
			return subblock;

		delete subblock;
		return nullptr;
	}
	catch (...)
	{
		delete subblock;
		throw;
	}
}

TreeBlock *TreeBlock::nextBlock()
{
	return TreeBlock::_nextBlock<TreeBlock>();
}

void TreeBlock::firstBlock()
{
	_nextSubblockOffset = _headerSize;
}

void TreeBlock::_subblockUpdated(TreeBlock &subblock, int32 sizeDiff)
{
	if (_parent != nullptr)
		_parent->_subblockUpdated(*this, sizeDiff);

	if (sizeDiff == 0)
		return;

	_file->seekp(0, std::ios::beg);
	Block::_writeHeader(_blockFormat, *_file, _file->size(), _tag);
	if (_nextSubblockOffset > subblock._file->offset())
		_nextSubblockOffset += sizeDiff;
	subblock._parentVersion = ++_version;
}

void TreeBlock::makePath(std::string &dir, std::string &name) const
{
	if (_parent != nullptr)
	{
		_parent->makePath(dir, name);
		dir += name;
	}
	if (dir.size() > 0 && dir[dir.size() - 1] != '/')
		dir += '/';
	name = _fileName();
}

void TreeBlock::dump(const char *path)
{
	File output;

	output.open(path, std::ios::binary | std::ios::out | std::ios::trunc);
	if (!output.is_open())
	{
		ScummIO::warning(xsprintf("Cannot open %s", path));
		return;
	}

	ScummIO::info(INF_LISTING, xsprintf("Exporting %s", path));
	_file->seekg(0, std::ios::beg);
	output.write(*_file, _file->size());
	output.close();
}

void TreeBlock::update(const char *path)
{
	File input;
	int32 newSize, sizeDiff;
	int id;

	if (_childrenCount > 0)
		throw std::logic_error("TreeBlock::update: The block has children");

	input.open(path, std::ios::binary | std::ios::in);
	if (!input.is_open())
		return;

	newSize = input.size();
	sizeDiff = newSize - _file->size();
	if (newSize < _headerSize)
	{
		ScummIO::warning(xsprintf("%s not updated: File size < Header size", _fileName()));
		input.close();
		return;
	}

	ScummIO::info(INF_LISTING, xsprintf("Importing %s", path));

	if (newSize < _file->size())
		_file->resize(newSize);
	_file->seekp(0, std::ios::beg);
	input.seekg(0, std::ios::beg);

	_file->write(input, newSize);
	_file->seekp(0, std::ios::beg);
	Block::_writeHeader(_blockFormat, *_file, _file->size(), _tag); // Ignore the imported header

	if (_parent != nullptr)
		_parent->_subblockUpdated(*this, sizeDiff);

	id = TreeBlock::_findIdInBlock(*this);
	if (id != _id && id != -1)
		throw InvalidDataFromDump(xsprintf("%s has the id %i instead of %i", _fileName(), id, _id));

	input.close();
}

/*
 * LeafBlock
 */

LeafBlock::LeafBlock() :
    TreeBlock()
{
}

LeafBlock::~LeafBlock()
{
}

LeafBlock::LeafBlock(const TreeBlock &block) :
    TreeBlock(block)
{
}

LeafBlock &LeafBlock::operator=(const TreeBlock &block)
{
	TreeBlock::operator=(block);
	return *this;
}

/*
 * GlobalTocBlock
 */

GlobalTocBlock::GlobalTocBlock() :
    TreeBlock()
{
}

GlobalTocBlock::~GlobalTocBlock()
{
}

void GlobalTocBlock::exportToc(TableOfContent &toc)
{
	_file->seekg(_headerSize, std::ios::beg);
	toc.load(*_file, ScummRp::game.globalTocFormat);
}

void GlobalTocBlock::importToc(TableOfContent &toc)
{
	_file->seekp(_headerSize, std::ios::beg);
	toc.save(*_file, ScummRp::game.globalTocFormat);
}

/*
 * GlobalTocBlockV1
 */

GlobalTocBlockV1::GlobalTocBlockV1() :
    GlobalTocBlock()
{
}

GlobalTocBlockV1::~GlobalTocBlockV1()
{
}

void GlobalTocBlockV1::exportToc(TableOfContent &toc)
{
	_file->seekg(_headerSize, std::ios::beg);
	toc.load(*_file, GTCFMT_8SEP16, _file->size() / 3);
}

void GlobalTocBlockV1::importToc(TableOfContent &toc)
{
	_file->seekp(_headerSize, std::ios::beg);
	toc.save(*_file, GTCFMT_8SEP16, true);
}

/*
 * BlocksFile
 */

BlocksFile::BlocksFile() :
    TreeBlock(), _ownFile(), _ownRAMFile(), _ownSeqFile(), _ownSeqRAMFile()
{
	_blockFormat = BFMT_NOHEADER;
	_headerSize = 0;
}

BlocksFile::BlocksFile(const char *path, int opts, BackUp &bak, int id, uint32 tag, byte xorKey) :
    TreeBlock(), _ownFile(), _ownRAMFile(), _ownSeqFile(), _ownSeqRAMFile()
{
	_init(path, opts, &bak, id, tag, xorKey);
}

BlocksFile::BlocksFile(const char *path, int opts, int id, uint32 tag, byte xorKey) :
    TreeBlock(), _ownFile(), _ownRAMFile(), _ownSeqFile(), _ownSeqRAMFile()
{
	_init(path, opts, nullptr, id, tag, xorKey);
}

void BlocksFile::_init(const char *path, int opts, BackUp *bak, int id, uint32 tag, byte xorKey)
{
	File *f;
	int32 size;

	size = File::fileSize(path);
	if (opts & BlocksFile::BFOPT_AUTO)
	{
		opts &= ~(BlocksFile::BFOPT_SEQFILE | BlocksFile::BFOPT_RAM);
		if (size > 0x100000)
			opts |= BlocksFile::BFOPT_SEQFILE;
		else
			opts |= BlocksFile::BFOPT_RAM;
	}

	// force BFOPT_SEQFILE for 10+ MB files (exponential time otherwise!)
	// FIXME this is only for programs like ScummRp & ScummTr
	if (size > 0xA00000 && ((opts & BlocksFile::BFOPT_RAM) || !(opts & BlocksFile::BFOPT_SEQFILE)))
	{
		opts |= BlocksFile::BFOPT_SEQFILE;
		opts &= ~BlocksFile::BFOPT_RAM;
		ScummIO::info(INF_GLOBAL, "File too big, forced -O and disabled -m");
	}

	_blockFormat = BFMT_NOHEADER;
	_headerSize = 0;
	_id = id;
	_tag = tag;
	if (opts & BlocksFile::BFOPT_READONLY)
	{
		f = (opts & BlocksFile::BFOPT_RAM) ? &_ownRAMFile : &_ownFile;
		f->open(path, std::ios::binary | std::ios::in);
	}
	else
	{
		f = (opts & BlocksFile::BFOPT_RAM) ? &_ownRAMFile : &_ownFile;
		if (bak != nullptr && (opts & BlocksFile::BFOPT_BACKUP))
		{
			if (opts & BlocksFile::BFOPT_SEQFILE)
			{
				if (opts & BlocksFile::BFOPT_RAM)
				{
					_ownSeqRAMFile.open(path, *bak);
					f = &_ownSeqRAMFile;
				}
				else
				{
					_ownSeqFile.open(path, *bak);
					f = &_ownSeqFile;
				}
			}
			else
			{
				f->open(bak->backup(path).c_str(), std::ios::binary | std::ios::out | std::ios::in);
			}
		}
		else
		{
			f->open(path, std::ios::binary | std::ios::out | std::ios::in);
		}
	}

	if (!f->is_open())
		throw std::runtime_error(xsprintf("Cannot open %s", path));

	_file = new FilePart(*f);
	_file->setXORKey(xorKey);
}

BlocksFile::~BlocksFile()
{
	_file.del(); // FilePart must be closed before the actual File.
}

/*
 * Room
 */

Room::Room() :
    _uIdsSoFar()
{
}

Room::~Room()
{
}

void Room::_uniqueId(uint32 tag, int32 id)
{
	if (std::find(_uIdsSoFar.begin(), _uIdsSoFar.end(), Room::IdAndTag(id, tag)) != _uIdsSoFar.end())
		throw Room::IdNotUnique(xsprintf("%s #%i is not unique", Block::tagToStr(tag), id));

	_uIdsSoFar.push_back(Room::IdAndTag(id, tag));
}

void Room::_resetUIdList()
{
	_uIdsSoFar.resize(0);
	_uIdsSoFar.reserve(32);
}

/*
 * RoomPack
 */

RoomPack::RoomPack()
{
}

RoomPack::~RoomPack()
{
}

int RoomPack::_findNextLFLFRootBlock(byte roomId, int32 currentOffset, int32 roomSize, const TableOfContent *&toc, int &id)
{
	int blockId;
	int32 nextOffset, offset;

	toc = nullptr;
	id = -1;
	nextOffset = roomSize;
	for (int i = 0; ScummRp::tocs[i] != nullptr; ++i)
	{
		ScummRp::tocs[i]->firstId(roomId);
		while (ScummRp::tocs[i]->nextId(blockId, roomId))
		{
			offset = (int32)(*ScummRp::tocs[i])[blockId].offset;
			if (offset >= currentOffset && offset < nextOffset)
			{
				nextOffset = offset;
				toc = ScummRp::tocs[i];
				id = blockId;
			}
		}
	}

	return nextOffset;
}

void RoomPack::_moveLFLFRootBlockInToc(byte roomId, int32 minOffset, int32 n) const
{
	int blockId;

	for (int i = 0; ScummRp::tocs[i] != nullptr; ++i)
	{
		ScummRp::tocs[i]->firstId(roomId);
		while (ScummRp::tocs[i]->nextId(blockId, roomId))
			if ((int32)(*ScummRp::tocs[i])[blockId].offset >= minOffset)
				(*ScummRp::tocs[i])[blockId].offset += n;
	}
}

void RoomPack::_checkDupOffset(byte roomId, int32 offset)
{
	int n;

	n = 0;
	for (int i = 0; ScummRp::tocs[i] != nullptr; ++i)
	{
		int j = ScummRp::tocs[i]->count(roomId, offset);
		if (j == 2 && ScummRp::tocs[i]->getType() == TableOfContent::TOCT_SCRP)
		{
			// Hack for Sam & Max CD English
			if (roomId == 1 && ScummRp::tocs[i]->getSize() == 122
				&& (*ScummRp::tocs[i])[8].offset == (*ScummRp::tocs[i])[9].offset
				&& (*ScummRp::tocs[i])[8].roomId == (*ScummRp::tocs[i])[9].roomId)
			{
				(*ScummRp::tocs[i])[8].offset = -1;
				j = 1;
				ScummIO::info(INF_DETAIL, "Removed SCRP_0008 from index (duplicate of SCRP_0009)");
			}
			// Hack for Maniac Mansion V1 (1)
			else if (roomId == 8 && ScummRp::tocs[i]->getSize() == 200
				 && (*ScummRp::tocs[i])[7].offset == (*ScummRp::tocs[i])[12].offset
				 && (*ScummRp::tocs[i])[7].roomId == (*ScummRp::tocs[i])[12].roomId)
			{
				(*ScummRp::tocs[i])[7].offset = -1;
				j = 1;
				ScummIO::info(INF_DETAIL, "Removed SC_0007 from index (duplicate of SC_0012)");
			}
			// Hack for Maniac Mansion V1 (2)
			else if (roomId == 8 && ScummRp::tocs[i]->getSize() == 200
				 && (*ScummRp::tocs[i])[8].offset == (*ScummRp::tocs[i])[13].offset
				 && (*ScummRp::tocs[i])[8].roomId == (*ScummRp::tocs[i])[13].roomId)
			{
				(*ScummRp::tocs[i])[8].offset = -1;
				j = 1;
				ScummIO::info(INF_DETAIL, "Removed SC_0008 from index (duplicate of SC_0013)");
			}
			// Hack for Loom EGA English (1)
			else if (roomId == 11 && ScummRp::tocs[i]->getSize() == 200
				 && (*ScummRp::tocs[i])[51].offset == (*ScummRp::tocs[i])[52].offset
				 && (*ScummRp::tocs[i])[51].roomId == (*ScummRp::tocs[i])[52].roomId)
			{
				(*ScummRp::tocs[i])[51].offset = -1;
				j = 1;
				ScummIO::info(INF_DETAIL, "Removed SC_0051 from index (duplicate of SC_0052)");
			}
			// Hack for Loom EGA English (2).  This one only appears in Loom 1.0 (8 Mar 90)
			else if (roomId == 18 && ScummRp::tocs[i]->getSize() == 200
				 && (*ScummRp::tocs[i])[55].offset == (*ScummRp::tocs[i])[56].offset
				 && (*ScummRp::tocs[i])[55].roomId == (*ScummRp::tocs[i])[56].roomId)
			{
				(*ScummRp::tocs[i])[55].offset = -1;
				j = 1;
				ScummIO::info(INF_DETAIL, "Removed SC_0055 from index (duplicate of SC_0056)");
			}
		}
		else if (j == 2 && ScummRp::tocs[i]->getType() == TableOfContent::TOCT_COST)
		{
			// Hack for Monkey1 Floppy VGA
			if (roomId == 59 && ScummRp::tocs[i]->getSize() == 199
				&& (*ScummRp::tocs[i])[10].offset == (*ScummRp::tocs[i])[117].offset
				&& (*ScummRp::tocs[i])[10].roomId == (*ScummRp::tocs[i])[117].roomId)
			{
				(*ScummRp::tocs[i])[117].offset = -1;
				j = 1;
				ScummIO::info(INF_DETAIL, "Removed CO_0117 from index (duplicate of CO_0010)");
			}
		}
		n += j;
		if (n > 1)
			throw RoomPack::BadOffset(xsprintf("Duplicate offset in index: 0x%X in room %i. Please report this bug with your exact version of the game", offset, roomId));
	}
}

void RoomPack::_eraseOffsetsInRange(byte roomId, int32 start, int32 end)
{
	int blockId;

	for (int i = 0; ScummRp::tocs[i] != nullptr; ++i)
	{
		ScummRp::tocs[i]->firstId(roomId);
		while (ScummRp::tocs[i]->nextId(blockId, roomId))
		{
			TableOfContent::TocElement &el = (*ScummRp::tocs[i])[blockId];
			if (el.offset >= start && el.offset < end)
			{
				ScummIO::info(INF_DETAIL, xsprintf("Removed (%.2u, 0x%X) from the index", el.roomId, el.offset));
				el.offset = -1;
				el.roomId = (byte)-1;
			}
		}
	}
}

/*
 * IndexFile
 */

IndexFile::IndexFile(const char *path, int opts, BackUp &bak, int id, byte xorKey) :
    BlocksFile(path, opts, bak, id, 0, xorKey)
{
}

IndexFile::IndexFile(const char *path, int opts, int id, byte xorKey) :
    BlocksFile(path, opts, id, 0, xorKey)
{
}

IndexFile::~IndexFile()
{
}

bool IndexFile::nextBlock(TreeBlock &subblock)
{
	return TreeBlock::nextBlock(subblock);
}

TreeBlock *IndexFile::nextBlock()
{
	return TreeBlock::_nextBlock<GlobalTocBlock>();
}

/*
 * LFLFile
 */

LFLFile::LFLFile(const char *path, int opts, BackUp &bak, int id, byte xorKey) :
    BlocksFile(path, opts, bak, id, MKTAG2('L','F'), xorKey), RoomPack()
{
	RoomPack::_eraseOffsetsInRange((byte)_id, _file->size(), 0xFFFFFFFF);
}

LFLFile::LFLFile(const char *path, int opts, int id, byte xorKey) :
    BlocksFile(path, opts, id, MKTAG2('L','F'), xorKey), RoomPack()
{
	RoomPack::_eraseOffsetsInRange((byte)_id, _file->size(), 0xFFFFFFFF);
}

LFLFile::~LFLFile()
{
}

template <class T>
TreeBlock *LFLFile::_nextLFLSubblock()
{
	const TableOfContent *toc;
	int id;
	TreeBlock *subblock = nullptr;

	_nextSubblockOffset = RoomPack::_findNextLFLFRootBlock((byte)_id, _nextSubblockOffset - _headerSize, _file->size() - _headerSize, toc, id);
	_checkDupOffset((byte)_id, _nextSubblockOffset);
	_nextSubblockOffset += _headerSize;
	try
	{
		if (dynamic_cast<const GlobalRoomIndex *>(toc) == nullptr)
			subblock = new TreeBlock;
		else
			subblock = new T;

		if (!nextBlock(*subblock))
		{
			delete subblock;
			subblock = nullptr;
			return nullptr;
		}

		if (dynamic_cast<const GlobalRoomIndex *>(toc) == nullptr)
			subblock->_id = id;
	}
	catch (...)
	{
		delete subblock;
		throw;
	}

	return subblock;
}

bool LFLFile::nextBlock(TreeBlock &subblock)
{
	return TreeBlock::nextBlock(subblock);
}

TreeBlock *LFLFile::nextBlock()
{
	return LFLFile::_nextLFLSubblock<RoomBlock>();
}

void LFLFile::_subblockUpdated(TreeBlock &subblock, int32 sizeDiff)
{
	int32 minOffset;

	TreeBlock::_subblockUpdated(subblock, sizeDiff);
	if (sizeDiff == 0)
		return;

	minOffset = (int32)(subblock._file->offset() - _headerSize + subblock._file->size() - sizeDiff);
	// invalid offsets have to be erased, because the shift could make them "valid" and hide the real valid blocks
	RoomPack::_eraseOffsetsInRange((byte)_id, subblock._file->offset() + 1 - _headerSize, minOffset);
	_moveLFLFRootBlockInToc((byte)_id, minOffset, sizeDiff);
}

/*
 * LFLFPack
 */

LFLFPack::LFLFPack() :
    TreeBlock(), RoomPack()
{
}

LFLFPack::LFLFPack(const TreeBlock &block) :
    TreeBlock(block)
{
	_init();
}

LFLFPack::~LFLFPack()
{
}

LFLFPack &LFLFPack::operator=(const TreeBlock &block)
{
	TreeBlock::operator=(block);

	_init();

	return *this;
}

void LFLFPack::_init()
{
	// LF blocks have 1 word for the room number (6 + 2 = 8 bytes)
	// LFLF blocks have a normal 8 bytes header
	_headerSize = 8;

	for (int i = 0; ScummRp::tocs[i] != nullptr; ++i)
		ScummRp::tocs[i]->accessing((byte)_id);

	RoomPack::_eraseOffsetsInRange((byte)_id, _file->size(), 0xFFFFFFFF);
}

bool LFLFPack::nextBlock(TreeBlock &subblock)
{
	return TreeBlock::nextBlock(subblock);
}

TreeBlock *LFLFPack::nextBlock()
{
	const TableOfContent *toc;
	int id;
	TreeBlock *subblock = nullptr;

	_nextSubblockOffset = RoomPack::_findNextLFLFRootBlock((byte)_id, _nextSubblockOffset - _headerSize, _file->size() - _headerSize, toc, id);
	_checkDupOffset((byte)_id, _nextSubblockOffset);
	_nextSubblockOffset += _headerSize;
	try
	{
		if (dynamic_cast<const GlobalRoomIndex *>(toc) == nullptr)
			subblock = new TreeBlock;
		else
			subblock = new RoomBlock;

		if (!TreeBlock::nextBlock(*subblock))
		{
			delete subblock;
			subblock = nullptr;
			return nullptr;
		}

		if (dynamic_cast<const GlobalRoomIndex *>(toc) == nullptr)
			subblock->_id = id;
	}
	catch (...)
	{
		delete subblock;
		throw;
	}

	return subblock;
}

void LFLFPack::_subblockUpdated(TreeBlock &subblock, int32 sizeDiff)
{
	int32 minOffset;

	TreeBlock::_subblockUpdated(subblock, sizeDiff);
	if (sizeDiff == 0)
		return;

	minOffset = (int32)(subblock._file->offset() + subblock._file->size() - sizeDiff - _headerSize);
	RoomPack::_eraseOffsetsInRange((byte)_id, subblock._file->offset() + 1 - _headerSize, minOffset);
	_moveLFLFRootBlockInToc((byte)_id, minOffset, sizeDiff);
}

/*
 * LECFPack
 */

LECFPack::LECFPack() :
    TreeBlock(), _firstBlockOffset(0), _loff()
{
}

LECFPack::~LECFPack()
{
}

LECFPack::LECFPack(const TreeBlock &block) :
    TreeBlock(block), _firstBlockOffset(0), _loff()
{
	_init();
}

LECFPack &LECFPack::operator=(const TreeBlock &block)
{
	TreeBlock::operator=(block);

	_firstBlockOffset = 0;
	_init();

	return *this;
}

void LECFPack::firstBlock()
{
	TreeBlock::firstBlock();
	_nextSubblockOffset = _firstBlockOffset;
}

bool LECFPack::nextBlock(TreeBlock &subblock)
{
	byte roomId;
	int32 nextOffset;

	if (_nextSubblockOffset == 0)
		throw TreeBlock::InvalidBlock("LECFPack::nextBlock: Index not loaded yet");

	nextOffset = _file->size();
	if (_tag == MKTAG4('L','E','C','F'))
	{
		nextOffset += ScummRp::game.blockHeaderSize;
		_nextSubblockOffset += ScummRp::game.blockHeaderSize;
	}

	_loff.firstId();
	while (_loff.nextId(roomId))
		if ((int32)_loff[roomId] >= _nextSubblockOffset && (int32)_loff[roomId] < nextOffset)
			nextOffset = _loff[roomId];

	if (_tag == MKTAG4('L','E','C','F'))
		nextOffset -= ScummRp::game.blockHeaderSize;
	_nextSubblockOffset = nextOffset;

	return TreeBlock::nextBlock(subblock);
}

TreeBlock *LECFPack::nextBlock()
{
	return TreeBlock::_nextBlock<LFLFPack>();
}

void LECFPack::_init()
{
	TreeBlock block;

	TreeBlock::firstBlock();
	if (TreeBlock::nextBlock(block) && (block._tag == MKTAG4('L','O','F','F') || block._tag == MKTAG2('F','O')))
	{
		_LOFFOffset = _headerSize + block._headerSize;
		_firstBlockOffset = block._file->size();
		block._file->seekg(block._headerSize, std::ios::beg);
		_loff.load(*block._file);
		return;
	}

	throw TreeBlock::InvalidDataFromGame("Cannot find LOFF or FO block in LECF pack", _file->name(), _file->fullOffset());
}

int LECFPack::_findSubblockId(TreeBlock &subblock) const
{
	switch (subblock._tag)
	{
	case MKTAG4('L','F','L','F'):
		return _loff.findId((uint32)(subblock._file->offset() + subblock._headerSize));
	case MKTAG2('L','F'):
		return _loff.findId((uint32)(subblock._file->offset()));
	default:
		return TreeBlock::_findSubblockId(subblock);
	}
}

void LECFPack::_subblockUpdated(TreeBlock &subblock, int32 sizeDiff)
{
	int32 minOffset;
	byte roomId;

	TreeBlock::_subblockUpdated(subblock, sizeDiff);
	if (sizeDiff == 0)
		return;

	minOffset = (int32)(subblock._file->offset() + subblock._file->size() - sizeDiff);
	_loff.firstId();

	while (_loff.nextId(roomId))
		if (_loff[roomId] >= minOffset)
			_loff[roomId] += sizeDiff;

	_file->seekp(_LOFFOffset, std::ios::beg);
	_loff.save(*_file);
}

/*
 * RoomBlock
 */

RoomBlock::RoomBlock() :
    TreeBlock(), Room()
{
}

RoomBlock::~RoomBlock()
{
}

RoomBlock::RoomBlock(const TreeBlock &block) :
    TreeBlock(block), Room()
{
}

RoomBlock &RoomBlock::operator=(const TreeBlock &block)
{
	TreeBlock::operator=(block);
	return *this;
}

void RoomBlock::firstBlock()
{
	TreeBlock::firstBlock();
	_resetUIdList();
}

bool RoomBlock::nextBlock(TreeBlock &subblock)
{
	if (TreeBlock::nextBlock(subblock))
	{
		if (TreeBlock::_findIdInBlock(subblock) != -1)
		{
			try // try catch for ScummTr
			{
				_uniqueId(subblock._tag, subblock._id);
			}
			catch (Room::IdNotUnique &e)
			{
				ScummIO::warning(e.what());
			}
		}
		return true;
	}

	return false;
}

TreeBlock *RoomBlock::nextBlock()
{
	return TreeBlock::_nextBlock<TreeBlock>();
}

/*
 * OldIndexFile
 */

OldIndexFile::OldIndexFile(const char *path, int opts, BackUp &bak, int id, byte xorKey) :
    BlocksFile(path, opts, bak, id, 0, xorKey), _pos(0)
{
	_headerSize = 2; // To skip the magic number
}

OldIndexFile::OldIndexFile(const char *path, int opts, int id, byte xorKey) :
    BlocksFile(path, opts, id, 0, xorKey), _pos(0)
{
	_headerSize = 2; // To skip the magic number
}

OldIndexFile::~OldIndexFile()
{
}

void OldIndexFile::firstBlock()
{
	BlocksFile::firstBlock();
	_pos = 0;
}

TreeBlock *OldIndexFile::nextBlock()
{
	return TreeBlock::_nextBlock<GlobalTocBlock>();
}

bool OldIndexFile::nextBlock(TreeBlock &subblock)
{
	byte b;
	uint16 w;
	int32 size;

	subblock._leaveParent();
	if (_tags(_pos) == 0)
		return false;

	if (_nextSubblockOffset >= _file->size())
		throw Block::InvalidDataFromGame("Incomplete index file", _file->name(), _file->fullOffset());

	subblock._blockFormat = BFMT_NOHEADER;
	subblock._headerSize = 0;
	subblock._tag = _tags(_pos);
	subblock._id = -1;
	subblock._nextSubblockOffset = 0;

	if (_pos == 0) // Object flags (different format)
	{
		_file->seekg(_nextSubblockOffset, std::ios::beg);
		_file->getLE16(w);
		size = sizeof(uint16) + w * (int32)_sizeOfObjFlag();
	}
	else
	{
		_file->seekg(_nextSubblockOffset, std::ios::beg);
		_file->getByte(b);
		size = sizeof(byte) + b * (sizeof(uint16) + sizeof(byte));
	}

	if (size + _nextSubblockOffset > _file->size())
		throw Block::InvalidDataFromGame(xsprintf("Block too big: 0x%X", size), _file->name(), _nextSubblockOffset + _file->fullOffset());

	subblock._file = new FilePart(*_file, _nextSubblockOffset, size);
	_nextSubblockOffset += subblock._file->size();
	_adopt(subblock);
	subblock._init();
	++_pos;

	return true;
}

/*
 * OldIndexFileV1
 */

const uint32 OldIndexFileV1::TAGS[] = { MKTAG4('0','O','v','1'), MKTAG4('0','R','v','1'), MKTAG4('0','C','v','1'), MKTAG4('0','S','v','1'), MKTAG4('0','N','v','1'), 0 };
const int OldIndexFileV1::MM_SIZES[] = { 0x0320, 0xA5, 0x69, 0x258, 0x12C, 0 };
const int OldIndexFileV1::ZAK_SIZES[] = { 0x0307, 0xB7, 0x6F, 0x1D1, 0x168, 0 };

OldIndexFileV1::OldIndexFileV1(const char *path, int opts, BackUp &bak, int id, byte xorKey) :
    OldIndexFile(path, opts, bak, id, xorKey)
{
}

OldIndexFileV1::OldIndexFileV1(const char *path, int opts, int id, byte xorKey) :
    OldIndexFile(path, opts, id, xorKey)
{
}

OldIndexFileV1::~OldIndexFileV1()
{
}

TreeBlock *OldIndexFileV1::nextBlock()
{
	return TreeBlock::_nextBlock<GlobalTocBlockV1>();
}

bool OldIndexFileV1::nextBlock(TreeBlock &subblock)
{
	int32 size;

	subblock._leaveParent();
	if (_tags(_pos) == 0)
		return false;

	if (_nextSubblockOffset >= _file->size())
		throw Block::InvalidDataFromGame("Incomplete index file", _file->name(), _file->fullOffset());

	subblock._blockFormat = BFMT_NOHEADER;
	subblock._headerSize = 0;
	subblock._tag = _tags(_pos);
	subblock._id = -1;
	subblock._nextSubblockOffset = 0;

	switch (ScummRp::game.id)
	{
	case GID_MANIAC:
		size = MM_SIZES[_pos];
		break;
	case GID_ZAK:
		size = ZAK_SIZES[_pos];
		break;
	default:
		throw std::logic_error("OldIndexFileV1::nextBlock: V1 game is neither Zak nor MM");
	}

	if (size + _nextSubblockOffset > _file->size())
		throw Block::InvalidDataFromGame(xsprintf("Block too big: 0x%X", size), _file->name(), _file->fullOffset() + _nextSubblockOffset);

	subblock._file = new FilePart(*_file, _nextSubblockOffset, size);
	_nextSubblockOffset += subblock._file->size();
	_adopt(subblock);
	subblock._init();
	++_pos;

	return true;
}

/*
 * OldIndexFileV2
 */

const uint32 OldIndexFileV2::TAGS[] = { MKTAG4('0','O','v','2'), MKTAG4('0','R','v','2'), MKTAG4('0','C','v','2'), MKTAG4('0','S','v','2'), MKTAG4('0','N','v','2'), 0 };

OldIndexFileV2::OldIndexFileV2(const char *path, int opts, BackUp &bak, int id, byte xorKey) :
    OldIndexFile(path, opts, bak, id, xorKey)
{
}

OldIndexFileV2::OldIndexFileV2(const char *path, int opts, int id, byte xorKey) :
    OldIndexFile(path, opts, id, xorKey)
{
}

OldIndexFileV2::~OldIndexFileV2()
{
}

/*
 * OldIndexFileV3
 */

const uint32 OldIndexFileV3::TAGS[] = { MKTAG4('0','O','v','3'), MKTAG4('0','R','v','3'), MKTAG4('0','C','v','3'), MKTAG4('0','S','v','3'), MKTAG4('0','N','v','3'), 0 };

OldIndexFileV3::OldIndexFileV3(const char *path, int opts, BackUp &bak, int id, byte xorKey) :
    OldIndexFile(path, opts, bak, id, xorKey)
{
}

OldIndexFileV3::OldIndexFileV3(const char *path, int opts, int id, byte xorKey) :
    OldIndexFile(path, opts, id, xorKey)
{
}

OldIndexFileV3::~OldIndexFileV3()
{
}

/*
 * OldLFLFile
 */

OldLFLFile::OldLFLFile(const char *path, int opts, BackUp &bak, int id, byte xorKey) :
    LFLFile(path, opts, bak, id, xorKey), _blocks()
{
	RoomPack::_eraseOffsetsInRange((byte)_id, _file->size(), 0x0000FFFF);
}

OldLFLFile::OldLFLFile(const char *path, int opts, int id, byte xorKey) :
    LFLFile(path, opts, id, xorKey), _blocks()
{
	RoomPack::_eraseOffsetsInRange((byte)_id, _file->size(), 0x0000FFFF);
}

OldLFLFile::~OldLFLFile()
{
}

void OldLFLFile::_moveLFLFRootBlockInToc(byte roomId, int32 minOffset, int32 n) const
{
	int blockId;

	for (int i = 0; ScummRp::tocs[i] != nullptr; ++i)
	{
		ScummRp::tocs[i]->firstId(roomId);
		while (ScummRp::tocs[i]->nextId(blockId, roomId))
		{
			if ((int32)(*ScummRp::tocs[i])[blockId].offset >= minOffset)
			{
				(*ScummRp::tocs[i])[blockId].offset += n;
				if ((*ScummRp::tocs[i])[blockId].offset >= 0xFFFF)
					throw RoomPack::BadOffset(xsprintf("Offset too far: 0x%X in %.2i.LFL", (*ScummRp::tocs[i])[blockId].offset, roomId));
			}
		}
	}
}

void OldLFLFile::_subblockUpdated(TreeBlock &subblock, int32 sizeDiff)
{
	LFLFile::_subblockUpdated(subblock, sizeDiff);

	if (sizeDiff == 0)
		return;

	for (int i = 0; i < (int)_blocks.size(); ++i)
		_blocks[i] += sizeDiff;
}

bool OldLFLFile::nextBlock(TreeBlock &subblock)
{
	const TableOfContent *toc;
	uint16 w;
	byte b;
	int32 o;
	int id;
	bool gap;

	toc = nullptr;
	gap = false;
	o = _nextSubblockOffset;

	while (true)
	{
		_nextSubblockOffset = RoomPack::_findNextLFLFRootBlock((byte)_id, _nextSubblockOffset, _file->size(), toc, id);
		if (_nextSubblockOffset >= _file->size() - 4)
		{
			if (o < _nextSubblockOffset)
				ScummIO::warning(xsprintf("%.2i.LFL should actually end at 0x%X", _id, o));
			return false;
		}

		if (o < _nextSubblockOffset)
			gap = true;

		_file->seekg(_nextSubblockOffset, std::ios::beg);
		_file->getLE16(w);
		_file->getByte(b);

		// TODO Hack for Indy3 Mac. In 76.LFL SO_27 (0x4CA8) has a wrong size and hides 4 sounds.
		// XXX: this isn't just triggered in Indy3 Mac, all EGA games appear to hit this...
		if (w >= 4 && b == 0 && w + _nextSubblockOffset <= _file->size())
		{
			ScummIO::info(INF_DETAIL, xsprintf("Fixing unexpected size at 0x%X in %.2i.LFL", o, _id));
			break;
		}

		++_nextSubblockOffset;
	}
	if (gap)
		ScummIO::warning(xsprintf("Gap at 0x%X in %.2i.LFL", o, _id));

	_checkDupOffset((byte)_id, _nextSubblockOffset);

	if (toc == nullptr)
		throw std::logic_error("OldLFLFile::nextBlock: TOC shouldn't be null");

	switch (toc->getType())
	{
	case TableOfContent::TOCT_ROOM:
		subblock._tag = _tags(0);
		break;
	case TableOfContent::TOCT_COST:
		subblock._tag = _tags(1);
		break;
	case TableOfContent::TOCT_SCRP:
		subblock._tag = _tags(2);
		break;
	case TableOfContent::TOCT_SOUN:
		subblock._tag = _tags(3);
		break;
	default:
		throw std::logic_error("OldLFLFile::nextBlock: Unknown TOC type");
	}

	if (!TreeBlock::nextBlock(subblock))
		return false;

	if (toc->getType() != TableOfContent::TOCT_ROOM)
		subblock._id = id;

	return true;
}

/*
 * OldLFLFileV1
 */

const uint32 OldLFLFileV1::TAGS[] = { MKTAG4('R','O','v','1'), MKTAG4('C','O','v','1'), MKTAG4('S','C','v','1'), MKTAG4('S','O','v','1') };

OldLFLFileV1::OldLFLFileV1(const char *path, int opts, BackUp &bak, int id, byte xorKey) :
    OldLFLFile(path, opts, bak, id, xorKey)
{
	_tag = MKTAG4('L','F','v','1');
}

OldLFLFileV1::OldLFLFileV1(const char *path, int opts, int id, byte xorKey) :
    OldLFLFile(path, opts, id, xorKey)
{
	_tag = MKTAG4('L','F','v','1');
}

OldLFLFileV1::~OldLFLFileV1()
{
}

TreeBlock *OldLFLFileV1::nextBlock()
{
	return LFLFile::_nextLFLSubblock<OldRoomV1>();
}

/*
 * OldLFLFileV2
 */

const uint32 OldLFLFileV2::TAGS[] = { MKTAG4('R','O','v','2'), MKTAG4('C','O','v','2'), MKTAG4('S','C','v','2'), MKTAG4('S','O','v','2') };

OldLFLFileV2::OldLFLFileV2(const char *path, int opts, BackUp &bak, int id, byte xorKey) :
    OldLFLFile(path, opts, bak, id, xorKey)
{
	_tag = MKTAG4('L','F','v','2');
}

OldLFLFileV2::OldLFLFileV2(const char *path, int opts, int id, byte xorKey) :
    OldLFLFile(path, opts, id, xorKey)
{
	_tag = MKTAG4('L','F','v','2');
}

OldLFLFileV2::~OldLFLFileV2()
{
}

TreeBlock *OldLFLFileV2::nextBlock()
{
	return LFLFile::_nextLFLSubblock<OldRoomV2>();
}

/*
 * OldLFLFileV3
 */

const uint32 OldLFLFileV3::TAGS[] = { MKTAG4('R','O','v','3'), MKTAG4('C','O','v','3'), MKTAG4('S','C','v','3'), MKTAG4('S','O','v','3') };

OldLFLFileV3::OldLFLFileV3(const char *path, int opts, BackUp &bak, int id, byte xorKey) :
    OldLFLFile(path, opts, bak, id, xorKey)
{
	_tag = MKTAG4('L','F','v','3');
}

OldLFLFileV3::OldLFLFileV3(const char *path, int opts, int id, byte xorKey) :
    OldLFLFile(path, opts, id, xorKey)
{
	_tag = MKTAG4('L','F','v','3');
}

OldLFLFileV3::~OldLFLFileV3()
{
}

TreeBlock *OldLFLFileV3::nextBlock()
{
	return LFLFile::_nextLFLSubblock<OldRoomV3>();
}

/*
 * OldRoom
 */

OldRoom::OldRoom() :
    RoomBlock(), _type(BT_NULL), _pos(0), _n(0),
    _bmSize(), _bxSize(0), _exSize(0), _enSize(0), _oiId(),
    _oiSize(), _oLSTOC(0), _lsSize(), _updated(false)
{
}

OldRoom::OldRoom(const TreeBlock &block) :
    RoomBlock(block), _type(BT_NULL), _pos(0), _n(0),
    _bmSize(), _bxSize(0), _exSize(0), _enSize(0), _oiId(),
    _oiSize(), _oLSTOC(0), _lsSize(), _updated(false)
{
}

OldRoom::~OldRoom()
{
}

OldRoom &OldRoom::operator=(const TreeBlock &block)
{
	TreeBlock::operator=(block);

	_type = BT_NULL;
	_pos = 0;
	_n = 0;
	_bmSize.resize(0);
	_bxSize = 0;
	_exSize = 0;
	_enSize = 0;
	_oiId.resize(0);
	_oiSize.resize(0);
	_oLSTOC = 0;
	_lsSize.resize(0);
	_updated = false;

	return *this;
}

bool OldRoom::_nextSubblockType()
{
	do
	{
		++_type;
		_pos = 0;
		_nextSubblockOffset = 0;

		switch (_type)
		{
		case BT_HD:
		case BT_BX:
		case BT_NL:
		case BT_SL:
		case BT_EX:
		case BT_EN:
			_n = 1;
			break;
		case BT_BM:
			_n = _bmNbr();
			break;
		case BT_OI:
			_n = (int)_oiSize.size();
			while (_pos < _n && _oiId[_pos] == -1)
				++_pos;
			break;
		case BT_OC:
			_n = (int)_oiSize.size();
			break;
		case BT_LS:
			_n = (int)_lsSize.size();
			break;
		default:
			_type = BT_END;
			return false;
		}
	} while (_pos >= _n);

	return true;
}

void OldRoom::firstBlock()
{
	RoomBlock::firstBlock();
	_type = BT_NULL;
	_nextSubblockType();
}

bool OldRoom::nextBlock(TreeBlock &)
{
	throw TreeBlock::ForbiddenMethod("OldRoom::nextBlock: Shouldn't be here");
}

void OldRoom::_prepareBlockO(TreeBlock &subblock, int32 offset, int32 size, int type)
{
	subblock._tag = _tags(type);
	subblock._blockFormat = BFMT_NOHEADER;
	subblock._headerSize = 0;
	subblock._id = -1;
	subblock._file = new FilePart(*_file, offset, size);
}

void OldRoom::_prepareBlockOO(TreeBlock &subblock, int32 offsetToOffset, int32 size, int type)
{
	uint16 w;

	_file->seekg(offsetToOffset, std::ios::beg);
	_file->getLE16(w);

	_prepareBlockO(subblock, w, size, type);
}

TreeBlock *OldRoom::nextBlock()
{
	TreeBlock *subblock;
	OldOIBlock *oiBlock;
	OldLSBlock *lsBlock;
	uint16 w;
	byte b;

	subblock = nullptr;
	while (_pos >= _n)
		if (!_nextSubblockType())
			break;

	try
	{
		switch (_type)
		{
		case BT_END:
			// If the user imports bad data (OI & OC blocks), it will be impossible
			// to read this room anymore.
			// So for now let's check the room again when we've reached the end of it.
			// It should actually be done somewhere else (the exploring function should
			// call validate() or whatever when it has finished updating blocks).
			// It cannot be done from _subblockUpdated() because OI blocks depend on
			// OC blocks and vice versa.
			if (_updated)
			{
				try
				{
					ScummIO::setQuiet(true);
					_init();
					ScummIO::setQuiet(false);
				}
				catch (std::exception &)
				{
					ScummIO::setQuiet(false);
					throw;
				}
			}
			return nullptr;
		case BT_HD:
			subblock = new LeafBlock();
			_prepareBlockO(*subblock, _headerSize, 4, _type);
			break;
		case BT_BX:
			subblock = new LeafBlock();
			_prepareBlockO(*subblock, _getBXOffset(), _bxSize, _type);
			break;
		case BT_BM:
			subblock = new LeafBlock();
			_prepareBlockOO(*subblock, _ooBM() + _pos * 2, _bmSize[_pos], _type);
			subblock->_tag = _bmTags(_pos);
			break;
		case BT_OI:
			oiBlock = new OldOIBlock();
			oiBlock->_num = _pos;
			subblock = oiBlock;
			_prepareBlockOO(*subblock, _oObjTOC() + _pos * 2, _oiSize[_pos], _type);
			subblock->_id = _oiId[_pos];
			// skip invalid OI offsets
			++_pos;
			while (_pos < _n && _oiId[_pos] == -1)
				++_pos;
			--_pos;
			break;
		case BT_NL:
			subblock = new LeafBlock();
			_file->seekg(_oNLSize(), std::ios::beg);
			_file->getByte(b);
			_prepareBlockO(*subblock, _oObjTOC() + (int)_oiSize.size() * 4, b, _type);
			break;
		case BT_SL:
			subblock = new LeafBlock();
			_file->seekg(_oNLSize(), std::ios::beg);
			_file->getByte(b);
			w = (uint16)(_oObjTOC() + _oiSize.size() * 4 + b);
			_file->seekg(_oSLSize(), std::ios::beg);
			_file->getByte(b);
			_prepareBlockO(*subblock, w, b, _type);
			break;
		case BT_OC:
			subblock = new LeafBlock();
			_file->seekg(_oObjTOC() + _pos * 2 + _n * 2, std::ios::beg);
			_file->getLE16(w);
			_nextSubblockOffset = w;
			_makeSubblock(*subblock, BFMT_SIZEONLY, 4);
			subblock->_tag = _tags(_type);
			subblock->_id = _findIdInBlock(*subblock);
			break;
		case BT_EX:
			if (_exSize == 0)
			{
				++_pos;
				return nextBlock();
			}

			subblock = new LeafBlock();
			_prepareBlockOO(*subblock, _ooEX(), _exSize, _type);
			break;
		case BT_EN:
			if (_enSize == 0)
			{
				++_pos;
				return nextBlock();
			}

			subblock = new LeafBlock();
			_prepareBlockOO(*subblock, _ooEN(), _enSize, _type);
			break;
		case BT_LS:
			lsBlock = new OldLSBlock();
			lsBlock->_num = _pos;
			subblock = lsBlock;
			_prepareBlockOO(*subblock, _oLSTOC + _pos * 3 + 1, _lsSize[_pos], _type);
			_file->seekg(_oLSTOC + _pos * 3, std::ios::beg);
			_file->getByte(b);
			subblock->_id = b;
			break;
		case BT_NULL:
		default:
			throw Block::InvalidBlock("OldRoom::nextBlock: Invalid subblock type");
		}

		++_pos;
		_nextSubblockOffset = subblock->_file->offset() + subblock->_file->size();
		_adopt(*subblock);
		subblock->_init();

		if (TreeBlock::_findIdInBlock(*subblock) != -1)
		{
			try
			{
				_uniqueId(subblock->_tag, subblock->_id);
			}
			catch (Room::IdNotUnique &e)
			{
				ScummIO::warning(e.what());
			}
		}
		return subblock;
	}
	catch (...)
	{
		delete subblock;
		throw;
	}
}

// Here is a function which "guesses" the block sizes in OldRoom
// Bad OI offsets actually are valid offsets to other parts of the room
// The order of blocks is always the same: BX, BM, OI[], OC[], EX, EN, LS[]
void OldRoom::_getSizes()
{
	byte objNbr, nlSize, slSize;
	uint16 w, bmLastOffset;
	int32 bxEnd, bmEnd, ocEnd, exEnd, enEnd;
	std::vector<uint16> bmOffsets, oiOffsets, ocOffsets, lsOffsets;

	bmEnd = ocEnd = exEnd = enEnd = (uint16)_file->size();
	_file->seekg(_oObjNbr(), std::ios::beg);
	_file->getByte(objNbr);
	_file->seekg(_oNLSize(), std::ios::beg);
	_file->getByte(nlSize);
	_file->seekg(_oSLSize(), std::ios::beg);
	_file->getByte(slSize);

	// -- LS --
	_getLSOffsets(lsOffsets, objNbr, nlSize, slSize);
	if (lsOffsets.size() > 0)
		enEnd = *min_element(lsOffsets.begin(), lsOffsets.end());
	_calcSizes(_lsSize, lsOffsets, _file->size());

	// -- EN --
	_file->seekg(_ooEN(), std::ios::beg);
	_file->getLE16(w);
	if (w != 0)
	{
		_enSize = enEnd - w;
		bmEnd = ocEnd = exEnd = w;
	}
	else
	{
		_enSize = 0;
	}

	// -- EX --
	_file->seekg(_ooEX(), std::ios::beg);
	_file->getLE16(w);
	if (w != 0)
	{
		_exSize = exEnd - w;
		bmEnd = ocEnd = w;
	}
	else
	{
		_exSize = 0;
	}

	// -- BM (1) --
	_getBMOffsets(bmOffsets);
	if (bmOffsets.size() == 0)
		throw std::logic_error("OldRoom::_getSizes: no BM offsets");

	bmLastOffset = *max_element(bmOffsets.begin(), bmOffsets.end());
	bxEnd = *min_element(bmOffsets.begin(), bmOffsets.end());

	// -- OI, OC --
	if (objNbr > 0)
	{
		int i;

		oiOffsets.resize(objNbr);
		ocOffsets.resize(objNbr);
		_file->seekg(_oObjTOC(), std::ios::beg);

		for (i = 0; i < objNbr; ++i)
			_file->getLE16(oiOffsets[i]);

		for (i = 0; i < objNbr; ++i)
			_file->getLE16(ocOffsets[i]);

		_getOIInfo(bmLastOffset, oiOffsets, ocOffsets);
		std::sort(oiOffsets.begin(), oiOffsets.end());
		std::sort(ocOffsets.begin(), ocOffsets.end());

		for (i = 0; i < objNbr && oiOffsets[i] == 0; ++i)
			;
		bmEnd = (i == objNbr) ? ocOffsets[0] : oiOffsets[i];

		_checkOCSizes(ocOffsets, ocEnd);
	}

	// -- BM (2) --
	_calcSizes(_bmSize, bmOffsets, bmEnd);

	// -- BX --
	_bxSize = bxEnd - _getBXOffset();

	// check sizes
	if (_bxSize < 0 || _exSize < 0 || _enSize < 0)
		throw Block::InvalidDataFromGame("Bad offset in room", _file->name(), _file->fullOffset());

	_updated = false;
}

void OldRoom::_findMostLikelyOIId(std::vector<int> &candidates) const
{
	struct
	{
		int oiNbr;
		int candidatesNbr;
		int candidates[8];
		int candidatesIds[8];
		int best;
	} static const pref[] =
	{
		// Maniac Mansion, PC (enhanced), English
		{ 15, 2, { 6, 8 }, { 395, 547 }, 0 }, // room 45
		// Zak McKracken, PC (enhanced), English
		{ 30, 2, { 23, 27 }, { 122, 123 }, 1 },          // room 02
		{ 10, 2, { 4, 9 }, { 153, 155 }, 1 },            // room 04
		{ 5, 2, { 0, 1 }, { 386, 389 }, 1 },             // room 20
		{ 8, 6, { 0, 1, 2, 3, 4, 5 },
		  { 480, 481, 482, 483, 484, 485 }, 5 },         // room 29
		{ 10, 2, { 1, 9 }, { 506, 505 }, 0 },            // room 31
		{ 8, 2, { 1, 5 }, { 514, 513 }, 0 },             // room 32
		{ 24, 2, { 5, 14 }, { 525, 526 }, 1 },           // room 33
		{ 15, 3, { 10, 11, 12 }, { 609, 610, 611 }, 2 }, // room 41
		{ 15, 2, { 2, 3 }, { 614, 615 }, 1 },            // room 41
		// Zak McKracken, PC (classic), English
		{ 13, 2, { 1, 2 }, { 169, 167 }, 0 },   // room 05
		{ 22, 2, { 4, 5 }, { 466, 467 }, 1 },   // room 28
		{ 22, 2, { 6, 9 }, { 469, 470 }, 1 },   // room 28
		{ 22, 2, { 15, 17 }, { 473, 474 }, 1 }, // room 28
		// Indiana Jones and the Last Crusade, PC (EGA), English
		{ 25, 2, { 12, 14 }, { 131, 130 }, 0 }, // room 07
		{ 6, 2, { 3, 4 }, { 518, 517 }, 0 },    // room 34
		{ 15, 2, { 2, 3 }, { 538, 539 }, 1 },   // room 36
		{ 22, 2, { 2, 3 }, { 642, 643 }, 1 },   // room 48
		// Indiana Jones and the Last Crusade, Macintosh, English
		{ 29, 2, { 2, 3 }, { 180, 181 }, 1 },          // room 15
		{ 23, 3, { 1, 2, 10 }, { 224, 225, 226 }, 2 }, // room 17
		{ 32, 2, { 4, 8 }, { 250, 253 }, 1 },          // room 18
		{ 6, 2, { 1, 4 }, { 314, 313 }, 0 },           // room 21
		{ 14, 2, { 0, 2 }, { 522, 524 }, 1 },          // room 35
		{ 4, 2, { 0, 1 }, { 665, 666 }, 1 },           // room 50
		{ 14, 2, { 2, 3 }, { 836, 835 }, 0 },          // room 73
		// Loom, PC (EGA), English
		{ 7, 2, { 3, 6 }, { 163, 166 }, 1 },           // room 03
		{ 32, 3, { 0, 4, 11 }, { 167, 171, 178 }, 2 }, // room 04
		{ 47, 2, { 3, 4 }, { 207, 208 }, 1 },          // room 06
		{ 17, 2, { 2, 9 }, { 269, 276 }, 1 },          // room 09
		{ 17, 2, { 0, 1 }, { 429, 430 }, 1 },          // room 24
		{ 3, 2, { 1, 2 }, { 447, 448 }, 1 },           // room 25
		{ 35, 3, { 0, 1, 11 }, { 465, 466, 476 }, 2 }, // room 28
		{ 18, 2, { 0, 1 }, { 503, 504 }, 1 },          // room 30
		{ 18, 2, { 4, 16 }, { 507, 519 }, 1 },         // room 30
		{ 14, 2, { 4, 5 }, { 685, 686 }, 1 },          // room 46
		{ 9, 3, { 1, 2, 3 }, { 696, 697, 698 }, 2 },   // room 47
		// Loom, Macintosh, English
		{ 32, 2, { 4, 11 }, { 171, 178 }, 1 }, // room 04
		{ 9, 2, { 2, 3 }, { 697, 698 }, 1 },   // room 47
		//
		{ 0, 0, { 0 }, { 0 }, 0 }
	};
	std::string msg;
	int i, j;

	for (i = 0; pref[i].oiNbr != 0; ++i)
	{
		if (pref[i].oiNbr != (int)_oiId.size() || pref[i].candidatesNbr != (int)candidates.size())
			continue;

		for (j = 0; j < pref[i].candidatesNbr; ++j)
			if (pref[i].candidates[j] != candidates[j] || _oiId[pref[i].candidates[j]] != pref[i].candidatesIds[j])
				break;

		if (j == pref[i].candidatesNbr)
		{
			candidates[pref[i].best] = pref[i].candidates[0];
			candidates[0] = pref[i].candidates[pref[i].best];
			break;
		}
	}

	msg = xsprintf("%s_%.4i might actually be %s_%.4i", Block::tagToStr(_tags(BT_OI)), _oiId[candidates[0]], Block::tagToStr(_tags(BT_OI)), _oiId[candidates[1]]);
	for (i = 2; i < (int)candidates.size(); ++i)
		msg += xsprintf(" or %s_%.4i", Block::tagToStr(_tags(BT_OI)), _oiId[candidates[i]]);

	if (pref[i].oiNbr != 0)
	{
		msg += " (unlikely)";
		ScummIO::info(INF_DETAIL, msg.c_str());
	}
	else
	{
		ScummIO::warning(msg.c_str());
	}
}

// oiOffset.size() == ocOffset.size() && oiOffset.size() > 0
void OldRoom::_getOIInfo(uint16 bmLastOffset, std::vector<uint16> &oiOffset, const std::vector<uint16> &ocOffset)
{
	byte b;
	uint16 firstOCOffset, w;
	std::vector<OldRoom::OIInfo> oiInfo;
	std::vector<int> v;
	int n;
	bool ambiguousSize;

	n = (int)oiOffset.size();
	_oiId.resize(n);
	_oiSize.resize(n);
	oiInfo.reserve(n);
	firstOCOffset = *min_element(ocOffset.begin(), ocOffset.end());

	for (int i = 0; i < n; ++i)
	{
		uint16 width, height;

		_file->seekg(ocOffset[i] + _oOCId(), std::ios::beg);
		_file->getLE16(w);
		_oiId[i] = w;

		_file->seekg(ocOffset[i] + _oOCWidth(), std::ios::beg);
		_file->getByte(b);
		width = (uint16)(b << 3);

		_file->seekg(ocOffset[i] + _oOCHeight(), std::ios::beg);
		_file->getByte(b);
		height = (uint16)(b & 0xF8);

		if (oiOffset[i] >= firstOCOffset || oiOffset[i] <= bmLastOffset) // (1)
		{
			oiOffset[i] = 0;
		}
		else
		{
			w = _getOISize(width, height, oiOffset[i], ambiguousSize);
			oiInfo.push_back(OldRoom::OIInfo(i, oiOffset[i], w));
			if (ambiguousSize) // FIXME Some OI blocks ending with 0x82 are one byte too short. Why?
				oiInfo.push_back(OldRoom::OIInfo(i, oiOffset[i], w - 1));
		}
	}

	oiInfo.push_back(OldRoom::OIInfo(-1, firstOCOffset, 0)); // (2)
	std::stable_sort(oiInfo.begin(), oiInfo.end());

	for (int i = 0, j = 0; i < (int)oiInfo.size() - 1; i = j)
	{
		for (j = i + 1; oiInfo[j].offset == oiInfo[i].offset; ++j) // always ends thanks to (1) & (2)
			;

		v.resize(0);
		v.reserve(j - i);
		for (int k = i; k < j; ++k)
		{
			if (oiInfo[k].offset + oiInfo[k].size == oiInfo[j].offset)
			{
				v.push_back(oiInfo[k].num);
			}
			else
			{
				// FIXME Hack for zakv2 which has two bad OI blocks in room 35.
				if (ScummRp::game.id == GID_ZAK && ScummRp::game.version == 2
					&& oiInfo[k].size == 0x308
					&& oiInfo[k].num == 11 && _oiId[oiInfo[k].num] == 553
					&& oiInfo[j].offset - oiInfo[k].offset == 0x384)
				{
					ScummIO::warning("Erroneous OI #553?");
					oiInfo[k].size = 0x384;
					v.push_back(oiInfo[k].num);
				}
				else if (ScummRp::game.id == GID_ZAK && ScummRp::game.version == 2
					 && oiInfo[k].size == 0x3DE
					 && oiInfo[k].num == 12 && _oiId[oiInfo[k].num] == 554
					 && oiInfo[j].offset - oiInfo[k].offset == 0x3C2)
				{
					ScummIO::warning("Erroneous OI #554?");
					oiInfo[k].size = 0x3C2;
					v.push_back(oiInfo[k].num);
				}
			}
		}

		if (v.size() == 0)
			throw Block::InvalidDataFromGame(xsprintf("Bad %s offset", Block::tagToStr(_tags(BT_OI))), _file->name(), _file->fullOffset());

		if (v.size() > 1)
			_findMostLikelyOIId(v);

		for (int k = i; k < j; ++k)
			if (oiInfo[k].num != v[0])
				oiOffset[oiInfo[k].num] = 0;
	}

	for (int i = 0; i < n; ++i)
	{
		if (oiOffset[i] == 0)
		{
			_oiId[i] = -1;
			_oiSize[i] = 0;
		}
	}

	for (int i = 0; i < (int)oiInfo.size() - 1; ++i)
		if (oiOffset[oiInfo[i].num] != 0)
			_oiSize[oiInfo[i].num] = oiInfo[i].size;
}

void OldRoom::_defCheckOCSizes(const std::vector<uint16> &ocOffset, int32 ocEnd)
{
	uint16 w;
	int lastObj, objNbr;

	objNbr = (int)ocOffset.size();
	lastObj = objNbr - 1;
	for (int i = 0; i < objNbr; ++i)
	{
		int32 end;

		end = (i < lastObj) ? ocOffset[i + 1] : ocEnd;
		_file->seekg(ocOffset[i], std::ios::beg);
		_file->getLE16(w);

		if ((int32)(w + ocOffset[i]) != end)
			throw Block::InvalidDataFromGame(xsprintf("Bad %s offset or size", Block::tagToStr(_tags(BT_OC))), _file->name(), _file->fullOffset());
	}
}

void OldRoom::_getBMOffsets(std::vector<uint16> &bmOffset)
{
	uint16 w;

	bmOffset.resize(_bmNbr());
	_file->seekg(_ooBM(), std::ios::beg);
	for (int i = 0; i < _bmNbr(); ++i)
	{
		_file->getLE16(w);
		bmOffset[i] = w;
	}
}

void OldRoom::_calcSizes(std::vector<int32> &sizes, const std::vector<uint16> &offsets, int32 end)
{
	std::vector<uint16> orderedOffsets;
	std::vector<uint16>::iterator pos;
	size_t n;

	n = offsets.size();
	sizes.resize(n);
	if (n == 0)
		return;

	orderedOffsets.resize(n);
	std::copy(offsets.begin(), offsets.end(), orderedOffsets.begin());
	std::sort(orderedOffsets.begin(), orderedOffsets.end());

	if (orderedOffsets.back() > end)
		throw Block::InvalidDataFromGame(xsprintf("Bad offset in room %i", _parent->_id), _file->name(), _file->fullOffset());

	for (int i = 0; i < (int)n; ++i)
	{
		pos = std::find(orderedOffsets.begin(), orderedOffsets.end(), offsets[i]);
		if (i < (int)n - 1)
			sizes[i] = *(pos + 1) - *pos;
		else
			sizes[i] = end - *pos;
	}
}

void OldRoom::_updateOffset(int32 offsetToOffset, int32 minOffset, int32 shift, uint32 subblockTag)
{
	uint16 offset;

	_file->seekg(offsetToOffset, std::ios::beg);
	_file->getLE16(offset);
	if (offset >= minOffset)
	{
		if ((offset + shift) & 0xFFFF0000)
			throw InvalidDataFromDump(xsprintf("%s block too big", tagToStr(subblockTag)));

		_file->seekp(offsetToOffset, std::ios::beg);
		_file->putLE16((uint16)(offset + shift));
	}
}

void OldRoom::_cleanup()
{
	// Erase bad OI offsets
	// _findMostLikelyOIId has to be perfect before using this
	for (int i = 0; i < (int)_oiSize.size(); ++i)
	{
		if (_oiId[i] == -1)
		{
			_file->seekp(_oObjTOC() + 2 * i, std::ios::beg);
			_file->putLE16((uint16)0);
		}
	}
}

void OldRoom::_subblockUpdated(TreeBlock &subblock, int32 sizeDiff)
{
	uint16 w;
	int32 minOffset;
	OldOIBlock *oiBlockPtr;
	OldLSBlock *lsBlockPtr;

	_updated = true;
	_cleanup();
	TreeBlock::_subblockUpdated(subblock, sizeDiff);
	if (sizeDiff == 0)
		return;

	minOffset = subblock._file->offset() + subblock._file->size() - sizeDiff;
	if ((subblock._tag & 0xFFFFFF00) == (MKTAG4('H','D','v','#') & 0xFFFFFF00)) // TODO _tagToType(): 'HDv1' -> BT_HD
		throw InvalidDataFromDump(xsprintf("%s blocks must always be 4 bytes long", tagToStr(subblock._tag)));

	if ((oiBlockPtr = dynamic_cast<OldOIBlock *>(&subblock)) != nullptr)
	{
		_oiSize[oiBlockPtr->_num] = subblock._file->size();
	}
	else if ((lsBlockPtr = dynamic_cast<OldLSBlock *>(&subblock)) != nullptr)
	{
		_lsSize[lsBlockPtr->_num] = subblock._file->size();
	}
	else if ((subblock._tag & 0xFFFFFF00) == (MKTAG4('N','L','v','#') & 0xFFFFFF00) || (subblock._tag & 0xFFFFFF00) == (MKTAG4('S','L','v','#') & 0xFFFFFF00))
	{
		if (subblock._file->size() & 0xFFFFFF00)
			throw InvalidDataFromDump(xsprintf("%s blocks must be smaller than 256 bytes", tagToStr(subblock._tag)));

		if ((subblock._tag & 0xFFFFFF00) == (MKTAG4('S','L','v','#') & 0xFFFFFF00))
			_file->seekp(_oSLSize(), std::ios::beg);
		else // ((subblock._tag & 0xFFFFFF00) == (MKTAG4('N','L','v','#') & 0xFFFFFF00))
			_file->seekp(_oNLSize(), std::ios::beg);

		_file->putByte((byte)subblock._file->size());
		_oLSTOC = (uint16)(_oLSTOC + sizeDiff);
	}

	w = _getBXOffset();
	if (w >= minOffset)
	{
		_setBXOffset((uint16)(w + sizeDiff));
		if (_getBXOffset() < w + sizeDiff)
			throw InvalidDataFromDump(xsprintf("%s block too big", tagToStr(subblock._tag)));
	}

	_updateOffset(_ooEX(), minOffset, sizeDiff, subblock._tag);
	_updateOffset(_ooEN(), minOffset, sizeDiff, subblock._tag);

	for (int i = 0; i < _bmNbr(); ++i)
		_updateOffset(_ooBM() + i * 2, minOffset, sizeDiff, subblock._tag);

	for (int i = 0; i < (int)_oiSize.size() * 2; ++i)
		_updateOffset(_oObjTOC() + 2 * i, minOffset, sizeDiff, subblock._tag);

	for (int i = 0; i < (int)_lsSize.size(); ++i)
		_updateOffset(_oLSTOC + 3 * i + 1, minOffset, sizeDiff, subblock._tag);
}

/*
 * OldRoomV1
 */

const uint32 OldRoomV1::TAGS[] = { 0, MKTAG4('H','D','v','1'), MKTAG4('B','X','v','1'), MKTAG4('B','M','v','1'), MKTAG4('O','I','v','1'), MKTAG4('N','L','v','1'), MKTAG4('S','L','v','1'), MKTAG4('O','C','v','1'), MKTAG4('E','X','v','1'), MKTAG4('E','N','v','1'), 0 };

const uint32 OldRoomV1::BMTAGS[] = { MKTAG4('B','1','v','1'), MKTAG4('B','2','v','1'), MKTAG4('B','3','v','1'), MKTAG4('B','4','v','1'), MKTAG4('B','5','v','1'), 0 };

OldRoomV1::OldRoomV1() :
    OldRoom()
{
}

OldRoomV1::OldRoomV1(const TreeBlock &block) :
    OldRoom(block)
{
	_init();
}

OldRoomV1::~OldRoomV1()
{
}

OldRoomV1 &OldRoomV1::operator=(const TreeBlock &block)
{
	OldRoom::operator=(block);

	_init();

	return *this;
}

void OldRoomV1::_init()
{
	try
	{
		_getSizes();
	}
	catch (File::UnexpectedEOF &) // FIXME should be checked in _getSizes()
	{
		throw Block::InvalidDataFromGame("Bad offset in room", _file->name(), _file->fullOffset());
	}
}

void OldRoomV1::_checkOCSizes(const std::vector<uint16> &ocOffset, int32 ocEnd)
{
	// V1 files aren't clean enough to pass the same test as V2 and V3 files
	int i;
	uint16 w;
	int objNbr;

	// every block but the last one
	objNbr = (int)ocOffset.size() - 1;
	for (i = 0; i < objNbr; ++i)
	{
		_file->seekg(ocOffset[i], std::ios::beg);
		_file->getLE16(w);
		if (w + ocOffset[i] != ocOffset[i + 1])
			throw Block::InvalidDataFromGame(xsprintf("Bad %s offset or size", Block::tagToStr(_tags(BT_OC))), _file->name(), _file->fullOffset());
	}

	// last block
	_file->seekg(ocOffset.back(), std::ios::beg);
	_file->getLE16(w);
	if (w + ocOffset[i] > ocEnd) // <- difference with V2/V3. junk is allowed at the end.
		throw Block::InvalidDataFromGame(xsprintf("Bad %s offset or size", Block::tagToStr(_tags(BT_OC))), _file->name(), _file->fullOffset());
}

void OldRoomV1::_setBXOffset(uint16 o)
{
	_file->seekp(_ooBX(), std::ios::beg);
	_file->putByte((byte)o);
}

uint16 OldRoomV1::_getBXOffset()
{
	byte b;

	_file->seekg(_ooBX(), std::ios::beg);
	_file->getByte(b);

	return (uint16)b;
}

void OldRoomV1::_getLSOffsets(std::vector<uint16> &, byte, byte, byte)
{
	// no LS blocks in V1
}

uint16 OldRoomV1::_getOISize(uint16 width, uint16 height, uint16 offset, bool &ambiguous)
{
	int x, z;
	byte color, run, b;
	byte common[4];
	int size;

	try
	{
		ambiguous = false;

		_file->seekg(offset, std::ios::beg);
		size = (width >> 3) * (height >> 3) * 3;

		for (z = 0; z < 4; ++z)
			_file->getByte(common[z]);

		x = 0;
		while (x < size)
		{
			_file->getByte(color);
			if (color & 0x80)
			{
				run = common[(color >> 5) & 3];
				color &= 0x1F;
				x += color + 1;
			}
			else if (color & 0x40)
			{
				color &= 0x3F;
				_file->getByte(run);
				x += color + 1;
			}
			else
			{
				for (z = 0; z <= color; ++z)
				{
					_file->getByte(b);
					++x;
				}
			}
		}

		return (uint16)((int32)_file->tellg(std::ios::beg) - offset);
	}
	catch (File::UnexpectedEOF &)
	{
		return 0;
	}
}

/*
 * OldRoomV2
 */

const uint32 OldRoomV2::TAGS[] = { 0, MKTAG4('H','D','v','2'), MKTAG4('B','X','v','2'), MKTAG4('B','M','v','2'), MKTAG4('O','I','v','2'), MKTAG4('N','L','v','2'), MKTAG4('S','L','v','2'), MKTAG4('O','C','v','2'), MKTAG4('E','X','v','2'), MKTAG4('E','N','v','2'), 0 };

const uint32 OldRoomV2::BMTAGS[] = { MKTAG4('I','M','v','2'), MKTAG4('M','A','v','2'), 0 };

OldRoomV2::OldRoomV2() :
    OldRoom()
{
}

OldRoomV2::OldRoomV2(const TreeBlock &block) :
    OldRoom(block)
{
	_init();
}

OldRoomV2::~OldRoomV2()
{
}

OldRoomV2 &OldRoomV2::operator=(const TreeBlock &block)
{
	OldRoom::operator=(block);

	_init();

	return *this;
}

void OldRoomV2::_init()
{
	try
	{
		_getSizes();
	}
	catch (File::UnexpectedEOF &) // FIXME should be checked in _getSizes()
	{
		throw Block::InvalidDataFromGame("Bad offset in room", _file->name(), _file->fullOffset());
	}
}

void OldRoomV2::_checkOCSizes(const std::vector<uint16> &ocOffset, int32 ocEnd)
{
	_defCheckOCSizes(ocOffset, ocEnd);
}

void OldRoomV2::_setBXOffset(uint16 o)
{
	_file->seekp(_ooBX(), std::ios::beg);
	_file->putByte((byte)o);
}

uint16 OldRoomV2::_getBXOffset()
{
	byte b;

	_file->seekg(_ooBX(), std::ios::beg);
	_file->getByte(b);

	return (uint16)b;
}

void OldRoomV2::_getLSOffsets(std::vector<uint16> &, byte, byte, byte)
{
	// no LS blocks in V2
}

uint16 OldRoomV2::_getOISize(uint16 width, uint16 height, uint16 offset, bool &ambiguous)
{
	int x, y, z;
	byte color, run;

	try
	{
		ambiguous = false;

		_file->seekg(offset, std::ios::beg);

		x = y = 0;
		while (x < width)
		{
			_file->getByte(color);
			if (color & 0x80)
			{
				run = color & 0x7F;
				if (run == 0)
					_file->getByte(run);

				for (z = 0; z < run; ++z)
				{
					if (++y >= height)
					{
						y = 0;
						++x;
					}
				}
			}
			else
			{
				run = color >> 4;
				color = color & 0x0F;
				if (run == 0)
					_file->getByte(run);

				for (z = 0; z < run; ++z)
				{
					if (++y >= height)
					{
						y = 0;
						++x;
					}
				}
			}
		}

		x = y = 0;
		while (x < width)
		{
			_file->getByte(color);
			if (color & 0x80)
			{
				run = color & 0x7F;
				if (run == 0)
					_file->getByte(run);

				_file->getByte(color);

				for (z = 0; z < run; ++z)
				{
					if (++y == height)
					{
						y = 0;
						x += 8;
					}
				}

				// FIXME OI masks finished by 0x82 are 1 byte too short. Why?
				if (x == width && run == 0x02)
					ambiguous = true;
			}
			else
			{
				run = color;
				if (run == 0)
					_file->getByte(run);

				for (z = 0; z < run; ++z)
				{
					_file->getByte(color);
					if (++y == height)
					{
						y = 0;
						x += 8;
					}
				}
			}
		}

		return (uint16)((int32)_file->tellg(std::ios::beg) - offset);
	}
	catch (File::UnexpectedEOF &)
	{
		return 0;
	}
}

/*
 * OldRoomV3
 */

const uint32 OldRoomV3::TAGS[] = { 0, MKTAG4('H','D','v','3'), MKTAG4('B','X','v','3'), MKTAG4('B','M','v','3'), MKTAG4('O','I','v','3'), MKTAG4('N','L','v','3'), MKTAG4('S','L','v','3'), MKTAG4('O','C','v','3'), MKTAG4('E','X','v','3'), MKTAG4('E','N','v','3'), MKTAG4('L','S','v','3'), 0 };

const uint32 OldRoomV3::BMTAGS[] = { MKTAG4('I','M','v','3'), MKTAG4('M','A','v','3'), 0 };

OldRoomV3::OldRoomV3() :
    OldRoom()
{
}

OldRoomV3::OldRoomV3(const TreeBlock &block) :
    OldRoom(block)
{
	_init();
}

OldRoomV3::~OldRoomV3()
{
}

OldRoomV3 &OldRoomV3::operator=(const TreeBlock &block)
{
	OldRoom::operator=(block);

	_init();

	return *this;
}

void OldRoomV3::_init()
{
	try
	{
		_getSizes();
	}
	catch (File::UnexpectedEOF &) // FIXME should be checked in _getSizes()
	{
		throw Block::InvalidDataFromGame("Bad offset in room", _file->name(), _file->fullOffset());
	}
}

void OldRoomV3::_checkOCSizes(const std::vector<uint16> &ocOffset, int32 ocEnd)
{
	_defCheckOCSizes(ocOffset, ocEnd);
}

void OldRoomV3::_setBXOffset(uint16 o)
{
	_file->seekp(_ooBX(), std::ios::beg);
	_file->putLE16(o);
}

uint16 OldRoomV3::_getBXOffset()
{
	uint16 w;

	_file->seekg(_ooBX(), std::ios::beg);
	_file->getLE16(w);

	return w;
}

void OldRoomV3::_getLSOffsets(std::vector<uint16> &lsOffset, byte objNbr, byte nlSize, byte slSize)
{
	uint16 w;
	byte lsId;

	_oLSTOC = _oObjTOC() + objNbr * 4 + nlSize + slSize;
	_file->seekg(_oLSTOC, std::ios::beg);

	for (_file->getByte(lsId); lsId != 0; _file->getByte(lsId))
	{
		_file->getLE16(w);
		lsOffset.push_back(w);
	}
}

uint16 OldRoomV3::_getOISize(uint16 width, uint16 height, uint16 offset, bool &ambiguous)
{
	int x, y, z;
	byte color, run;
	uint16 w;
	int32 end;
	int maskOffset;

	try
	{
		ambiguous = false;

		end = 0;
		_file->seekg(offset, std::ios::beg);
		_file->getLE16(w);

		_file->seekg(w - sizeof w, std::ios::cur);
		maskOffset = _file->tellg(std::ios::beg);

		x = y = 0;
		while (x < width)
		{
			_file->seekg(maskOffset + (x / 8) * 2, std::ios::beg);
			_file->getLE16(w);
			if (w != 0)
			{
				ambiguous = false;
				_file->seekg(maskOffset + w, std::ios::beg);
				while (y < height)
				{
					_file->getByte(color);
					if (color & 0x80)
					{
						run = color & 0x7F;
						if (run == 0)
							_file->getByte(run);
						_file->getByte(color);

						y += run;
						if (y > height)
							return 0;

						// FIXME OI masks finished by 0x82 can be 1 byte too short. Why?
						if (y == height && run == 0x02)
							ambiguous = true;
					}
					else
					{
						run = color;
						if (run == 0)
							_file->getByte(run);

						for (z = 0; z < run; ++z)
						{
							_file->getByte(color);
							++y;
						}
					}
				}
				end = _file->tellg(std::ios::beg);
			}
			y = 0;
			x += 8;
		}

		return (end > 0) ? (uint16)(end - offset) : (uint16)((int32)_file->tellg(std::ios::beg) - offset);
	}
	catch (File::UnexpectedEOF &)
	{
		return 0;
	}
}
