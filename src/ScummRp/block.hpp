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

#ifndef SCUMMRP_BLOCK_HPP
#define SCUMMRP_BLOCK_HPP

#include "common/types.hpp"
#include "common/file.hpp"
#include "common/backup.hpp"

#include "toc.hpp"
#include "rptypes.hpp"

#include <stdexcept>
#include <string>
#include <vector>

// FIXME RoomPack should be a TreeBlock

// Rp
class Block;
class TreeBlock;
class LeafBlock;
class RoomBlock;
class LFLFPack;
class LECFPack;
class GlobalTocBlock;
class GlobalTocBlockV1;
class BlocksFile;
class IndexFile;
class LFLFile;
class OldLFLFile;
class OldLFLFileV1;
class OldLFLFileV2;
class OldLFLFileV3;
class OldIndexFile;
class OldIndexFileV1;
class OldIndexFileV2;
class OldIndexFileV3;
class OldRoom;
class OldRoomV1;
class OldRoomV2;
class OldRoomV3;
class OldHDBlock;

// Tr
class TextBlock;
class ScriptBlock;
class ObjectNameBlock;
class ObjectCodeBlock;
class OldObjectCodeBlock;
class OldObjectCodeBlockV1;

//
class Room;
class RoomPack;

template <class T> class BlockPtr;

/*
 * Block
 */

class Block
{
public:
	class InvalidBlock : public std::logic_error
	{
	public:
		InvalidBlock(const std::string &message) : std::logic_error(message) { }
	};
	class InvalidDataFromGame : public std::runtime_error
	{
	private:
		std::string _file;
		int32 _offset;

	public:
		InvalidDataFromGame(const std::string &message, const std::string &f, int32 o) :
		    std::runtime_error(message), _file(f), _offset(o)
		{
		}
		const std::string &file() { return _file; }
		int32 offset() { return _offset; }
		~InvalidDataFromGame() throw() override { }
	};
	class InvalidDataFromDump : public std::runtime_error
	{
	public:
		InvalidDataFromDump(const std::string &message) : std::runtime_error(message) { }
	};

protected:
	BlockFormat _blockFormat;
	int _headerSize;
	uint32 _tag;
	int _id;
	FilePartHandle _file;

protected:
	static void _readHeader(BlockFormat format, FilePart &file, int32 &size, uint32 &tag);
	static void _writeHeader(BlockFormat format, FilePart &file, int32 size, uint32 tag);

public:
	static const char *tagToStr(uint32 tag);

protected:
	const char *_fileName() const;

public:
	int32 getHeaderSize() const;
	int32 getSize() const;
	uint32 getTag() const;
	int getId() const;

public:
	Block();
	virtual ~Block() = 0;

public:
	Block(const Block &block);
	Block &operator=(const Block &block);
};

/*
 * TreeBlock
 */

class TreeBlock : public Block
{
	// TreeBlock familly
	friend class RoomBlock;
	friend class LFLFPack;
	friend class LECFPack;
	friend class GlobalTocBlock;
	friend class GlobalTocBlockV1;
	friend class BlocksFile;
	friend class IndexFile;
	friend class LFLFile;
	friend class OldLFLFile;
	friend class OldLFLFileV1;
	friend class OldLFLFileV2;
	friend class OldLFLFileV3;
	friend class OldIndexFile;
	friend class OldIndexFileV1;
	friend class OldIndexFileV2;
	friend class OldIndexFileV3;
	friend class OldRoom;
	friend class OldRoomV1;
	friend class OldRoomV2;
	friend class OldRoomV3;
	friend class TextBlock;
	friend class ScriptBlock;
	friend class ObjectNameBlock;
	friend class ObjectCodeBlock;
	friend class OldObjectCodeBlock;
	friend class OldObjectCodeBlockV1;

protected:
	class ForbiddenMethod : public std::logic_error
	{
	public:
		ForbiddenMethod(const std::string &message) : std::logic_error(message) { }
	};
	class OutdatedBlockException : public std::runtime_error
	{
	public:
		OutdatedBlockException(const std::string &message) : std::runtime_error(message) { }
	};

protected:
	int _version;
	TreeBlock *_parent;
	int _parentVersion;
	int _nextSubblockOffset;
	int _childrenCount;

protected:
	static int _findIdInBlock(TreeBlock &block);

protected:
	virtual void _init();
	virtual int _findSubblockId(TreeBlock &subblock) const;
	virtual bool _readNextSubblock(TreeBlock &subblock);
	virtual void _subblockUpdated(TreeBlock &subblock, int32 sizeDiff);
	virtual void _adopt(TreeBlock &subblock);
	void _makeSubblock(TreeBlock &subblock, BlockFormat blockFormat, int32 headerSize);
	void _leaveParent();
	template <class T> T *_nextBlock();
public:
	virtual TreeBlock *nextBlock();
	virtual bool nextBlock(TreeBlock &subblock);
	virtual void firstBlock();
	void makePath(std::string &dir, std::string &name) const;
	void dump(const char *basePath);
	void update(const char *basePath);

public:
	TreeBlock();
	~TreeBlock() override;

public:
	TreeBlock(const TreeBlock &block);
	virtual TreeBlock &operator=(const TreeBlock &block);
};

/*
 * RoomPack
 */

class RoomPack
{
public:
	class BadOffset : public std::runtime_error
	{
	public:
		BadOffset(const std::string &message) : std::runtime_error(message) { }
	};

protected:
	static int _findNextLFLFRootBlock(byte roomId, int32 currentOffset, int32 roomSize, const TableOfContent *&toc, int &id);
	static void _checkDupOffset(byte roomId, int32 offset);
	static void _eraseOffsetsInRange(byte roomId, int32 start, int32 end);

protected:
	virtual void _moveLFLFRootBlockInToc(byte roomId, int32 minOffset, int32 n) const;

public:
	RoomPack();
	virtual ~RoomPack() = 0;
};

/*
 * Room
 */

class Room
{
public:
	class IdNotUnique : public std::runtime_error
	{
	public:
		IdNotUnique(const std::string &message) : std::runtime_error(message) { }
	};

private:
	struct IdAndTag
	{
		int32 id;
		uint32 tag;

		bool operator==(const IdAndTag &it) { return it.id == id && it.tag == tag; }
		IdAndTag() : id(0), tag(0) { }
		IdAndTag(int32 i, uint32 t) : id(i), tag(t) { }
	};

private:
	std::vector<IdAndTag> _uIdsSoFar;

protected:
	void _uniqueId(uint32 tag, int32 id);
	void _resetUIdList();

public:
	Room();
	virtual ~Room() = 0;
};

/*
 * LECFPack
 */

class LECFPack : public TreeBlock
{
protected:
	int _firstBlockOffset;
	RoomIndex _loff;
	int _LOFFOffset;

protected:
	void _init() override;
	int _findSubblockId(TreeBlock &subblock) const override;
	void _subblockUpdated(TreeBlock &subblock, int32 sizeDiff) override;

public:
	TreeBlock *nextBlock() override;
	bool nextBlock(TreeBlock &subblock) override;
	void firstBlock() override;

public:
	LECFPack();
	~LECFPack() override;

private: // Not copiable
	LECFPack(const LECFPack &);
	LECFPack &operator=(const LECFPack &);

public: // TreeBlock part still copiable
	LECFPack(const TreeBlock &block);
	LECFPack &operator=(const TreeBlock &block) override;
};

/*
 * GlobalTocBlock
 */

class GlobalTocBlock : public TreeBlock
{
public:
	virtual void exportToc(TableOfContent &toc);
	virtual void importToc(TableOfContent &toc);

public:
	GlobalTocBlock();
	~GlobalTocBlock() override;

private: // Not copiable
	GlobalTocBlock(const GlobalTocBlock &);
	GlobalTocBlock &operator=(const GlobalTocBlock &);
};

/*
 * GlobalTocBlockV1
 */

class GlobalTocBlockV1 : public GlobalTocBlock
{
public:
	void exportToc(TableOfContent &toc) override;
	void importToc(TableOfContent &toc) override;

public:
	GlobalTocBlockV1();
	~GlobalTocBlockV1() override;

private: // Not copiable
	GlobalTocBlockV1(const GlobalTocBlockV1 &);
	GlobalTocBlockV1 &operator=(const GlobalTocBlockV1 &);
};

/*
 * BlocksFile
 */

class BlocksFile : public TreeBlock
{
public:
	enum
	{
		BFOPT_NULL = 0,
		BFOPT_AUTO = 1 << 0,
		BFOPT_SEQFILE = 1 << 1,
		BFOPT_RAM = 1 << 2,
		BFOPT_READONLY = 1 << 3,
		BFOPT_BACKUP = 1 << 4
	};

protected:
	File _ownFile;
	RAMFile _ownRAMFile;
	SeqFile<File> _ownSeqFile;
	SeqFile<RAMFile> _ownSeqRAMFile;

protected:
	void _init(const char *path, int opts, BackUp *bak, int id, uint32 tag, byte xorKey);

public:
	BlocksFile(const char *path, int opts, BackUp &bak, int id, uint32 tag, byte xorKey);
	BlocksFile(const char *path, int opts, int id, uint32 tag, byte xorKey);
	~BlocksFile() override;

private: // No default constructor
	BlocksFile();

private: // Not copiable
	BlocksFile(const BlocksFile &);
	BlocksFile &operator=(const BlocksFile &);
};

/*
 * LFLFPack
 */

class LFLFPack : public TreeBlock, public RoomPack
{
protected:
	void _init() override;
	void _subblockUpdated(TreeBlock &subblock, int32 sizeDiff) override;

public:
	TreeBlock *nextBlock() override;
	bool nextBlock(TreeBlock &subblock) override;

public:
	LFLFPack();
	~LFLFPack() override;

private: // Not copiable
	LFLFPack(const LFLFPack &);
	LFLFPack &operator=(const LFLFPack &);

public: // TreeBlock part still copiable
	LFLFPack(const TreeBlock &block);
	LFLFPack &operator=(const TreeBlock &block) override;
};

/*
 * LFLFile
 */

class LFLFile : public BlocksFile, public RoomPack
{
protected:
	void _subblockUpdated(TreeBlock &subblock, int32 sizeDiff) override;

public:
	TreeBlock *nextBlock() override;
	bool nextBlock(TreeBlock &subblock) override;
	template <class T> TreeBlock *_nextLFLSubblock();
public:
	LFLFile(const char *path, int opts, BackUp &bak, int id, byte xorKey);
	LFLFile(const char *path, int opts, int id, byte xorKey);
	~LFLFile() override;

private: // No default constructor
	LFLFile();

private: // Not copiable
	LFLFile(const LFLFile &);
	LFLFile &operator=(const LFLFile &);
};

/*
 * OldLFLFile
 */

class OldLFLFile : public LFLFile
{
protected:
	std::vector<int32> _blocks;

protected:
	void _moveLFLFRootBlockInToc(byte roomId, int32 minOffset, int32 n) const override;
	virtual uint32 _tags(int i) const = 0;
	void _subblockUpdated(TreeBlock &subblock, int32 sizeDiff) override;

public:
	bool nextBlock(TreeBlock &subblock) override;

public:
	OldLFLFile(const char *path, int opts, BackUp &bak, int id, byte xorKey);
	OldLFLFile(const char *path, int opts, int id, byte xorKey);
	~OldLFLFile() override = 0;
};

/*
 * OldLFLFileV1
 */

class OldLFLFileV1 : public OldLFLFile
{
protected:
	static const uint32 TAGS[];

protected:
	uint32 _tags(int i) const override { return OldLFLFileV1::TAGS[i]; }

public:
	TreeBlock *nextBlock() override;

public:
	OldLFLFileV1(const char *path, int opts, BackUp &bak, int id, byte xorKey);
	OldLFLFileV1(const char *path, int opts, int id, byte xorKey);
	~OldLFLFileV1() override;

private: // No default constructor
	OldLFLFileV1();

private: // Not copiable
	OldLFLFileV1(const OldLFLFileV1 &);
	OldLFLFileV1 &operator=(const OldLFLFileV1 &);
};

/*
 * OldLFLFileV2
 */

class OldLFLFileV2 : public OldLFLFile
{
protected:
	static const uint32 TAGS[];

protected:
	uint32 _tags(int i) const override { return OldLFLFileV2::TAGS[i]; }

public:
	TreeBlock *nextBlock() override;

public:
	OldLFLFileV2(const char *path, int opts, BackUp &bak, int id, byte xorKey);
	OldLFLFileV2(const char *path, int opts, int id, byte xorKey);
	~OldLFLFileV2() override;

private: // No default constructor
	OldLFLFileV2();

private: // Not copiable
	OldLFLFileV2(const OldLFLFileV2 &);
	OldLFLFileV2 &operator=(const OldLFLFileV2 &);
};

/*
 * OldLFLFileV3
 */

class OldLFLFileV3 : public OldLFLFile
{
protected:
	static const uint32 TAGS[];

protected:
	uint32 _tags(int i) const override { return OldLFLFileV3::TAGS[i]; }

public:
	TreeBlock *nextBlock() override;

public:
	OldLFLFileV3(const char *path, int opts, BackUp &bak, int id, byte xorKey);
	OldLFLFileV3(const char *path, int opts, int id, byte xorKey);
	~OldLFLFileV3() override;

private: // No default constructor
	OldLFLFileV3();

private: // Not copiable
	OldLFLFileV3(const OldLFLFileV3 &);
	OldLFLFileV3 &operator=(const OldLFLFileV3 &);
};

/*
 * IndexFile
 */

class IndexFile : public BlocksFile
{
public:
	TreeBlock *nextBlock() override;
	bool nextBlock(TreeBlock &subblock) override;

public:
	IndexFile(const char *path, int opts, int id, byte xorKey);
	IndexFile(const char *path, int opts, BackUp &bak, int id, byte xorKey);
	~IndexFile() override;

private: // No default constructor
	IndexFile();

private: // Not copiable
	IndexFile(const IndexFile &);
	IndexFile &operator=(const IndexFile &);
};

/*
 * OldIndexFile
 */

class OldIndexFile : public BlocksFile
{
protected:
	int _pos;

protected:
	virtual uint32 _tags(int i) const = 0;
	virtual size_t _sizeOfObjFlag() const = 0;

public:
	void firstBlock() override;
	bool nextBlock(TreeBlock &subblock) override;
	TreeBlock *nextBlock() override;

public:
	OldIndexFile(const char *path, int opts, int id, byte xorKey);
	OldIndexFile(const char *path, int opts, BackUp &bak, int id, byte xorKey);
	~OldIndexFile() override = 0;
};

/*
 * OldIndexFileV1
 */

class OldIndexFileV1 : public OldIndexFile
{
protected:
	static const uint32 TAGS[];
	static const int ZAK_SIZES[];
	static const int MM_SIZES[];

protected:
	uint32 _tags(int i) const override { return OldIndexFileV1::TAGS[i]; }
	size_t _sizeOfObjFlag() const override { return sizeof(byte); }

public:
	TreeBlock *nextBlock() override;
	bool nextBlock(TreeBlock &subblock) override;

public:
	OldIndexFileV1(const char *path, int opts, int id, byte xorKey);
	OldIndexFileV1(const char *path, int opts, BackUp &bak, int id, byte xorKey);
	~OldIndexFileV1() override;

private: // No default constructor
	OldIndexFileV1();

private: // Not copiable
	OldIndexFileV1(const OldIndexFileV1 &);
	OldIndexFileV1 &operator=(const OldIndexFileV1 &);
};

/*
 * OldIndexFileV2
 */

class OldIndexFileV2 : public OldIndexFile
{
protected:
	static const uint32 TAGS[];

protected:
	uint32 _tags(int i) const override { return OldIndexFileV2::TAGS[i]; }
	size_t _sizeOfObjFlag() const override { return sizeof(byte); }

public:
	OldIndexFileV2(const char *path, int opts, int id, byte xorKey);
	OldIndexFileV2(const char *path, int opts, BackUp &bak, int id, byte xorKey);
	~OldIndexFileV2() override;

private: // No default constructor
	OldIndexFileV2();

private: // Not copiable
	OldIndexFileV2(const OldIndexFileV2 &);
	OldIndexFileV2 &operator=(const OldIndexFileV2 &);
};

/*
 * OldIndexFileV3
 */

class OldIndexFileV3 : public OldIndexFile
{
protected:
	static const uint32 TAGS[];

protected:
	uint32 _tags(int i) const override { return OldIndexFileV3::TAGS[i]; }
	size_t _sizeOfObjFlag() const override { return sizeof(uint32); }

public:
	OldIndexFileV3(const char *path, int opts, int id, byte xorKey);
	OldIndexFileV3(const char *path, int opts, BackUp &bak, int id, byte xorKey);
	~OldIndexFileV3() override;

private: // No default constructor
	OldIndexFileV3();

private: // Not copiable
	OldIndexFileV3(const OldIndexFileV3 &);
	OldIndexFileV3 &operator=(const OldIndexFileV3 &);
};

/*
 * RoomBlock
 */

class RoomBlock : public TreeBlock, public Room
{
public:
	void firstBlock() override;
	TreeBlock *nextBlock() override;
	bool nextBlock(TreeBlock &subblock) override;

public:
	RoomBlock();
	~RoomBlock() override;

private: // Not copiable
	RoomBlock(const RoomBlock &);
	RoomBlock &operator=(const RoomBlock &);

public: // TreeBlock part still copiable
	RoomBlock(const TreeBlock &block);
	RoomBlock &operator=(const TreeBlock &block) override;
};

/*
 * OldRoom
 */

class OldRoom : public RoomBlock
{
protected:
	enum BlockType
	{
		BT_NULL = 0,
		BT_HD = 1,
		BT_BX = 2,
		BT_BM = 3,
		BT_OI = 4,
		BT_NL = 5,
		BT_SL = 6,
		BT_OC = 7,
		BT_EX = 8,
		BT_EN = 9,
		BT_LS = 10,
		BT_END = 11
	};
	struct OIInfo // needed for _getOIInfo (cannot be local because it's used in a template)
	{
		int num;
		uint16 offset;
		uint16 size;

		bool operator<(const OIInfo &right) const { return offset < right.offset; }
		OIInfo(int n, uint16 o, uint16 s) : num(n), offset(o), size(s) { }
	};

protected:
	int _type;
	int _pos;
	int _n;
	std::vector<int32> _bmSize;
	int _bxSize;
	int _exSize;
	int _enSize;
	std::vector<int> _oiId;
	std::vector<int32> _oiSize;
	int32 _oLSTOC;
	std::vector<int32> _lsSize;
	bool _updated;

protected:
	virtual uint32 _tags(int i) const = 0;
	virtual uint32 _bmTags(int i) const = 0;
	virtual int _oBMWidth() const = 0;
	virtual int _oBMHeight() const = 0;
	virtual int _ooBM() const = 0;
	virtual int _oObjNbr() const = 0;
	virtual int _ooBX() const = 0;
	virtual int _oNLSize() const = 0;
	virtual int _oSLSize() const = 0;
	virtual int _ooEX() const = 0;
	virtual int _ooEN() const = 0;
	virtual int _oObjTOC() const = 0;
	virtual int _oOCId() const = 0;
	virtual int _oOCWidth() const = 0;
	virtual int _oOCHeight() const = 0;
	virtual int _bmNbr() const = 0;
	virtual uint16 _getOISize(uint16 width, uint16 height, uint16 offset, bool &ambiguous) = 0;
	virtual void _checkOCSizes(const std::vector<uint16> &ocOffset, int32 ocEnd) = 0;
	virtual void _setBXOffset(uint16 o) = 0;
	virtual uint16 _getBXOffset() = 0;
	virtual void _getLSOffsets(std::vector<uint16> &lsOffset, byte objNbr, byte nlSize, byte slSize) = 0;
	void _defCheckOCSizes(const std::vector<uint16> &ocOffset, int32 ocEnd);
	void _getBMOffsets(std::vector<uint16> &bmOffset);
	void _calcSizes(std::vector<int32> &sizes, const std::vector<uint16> &offsets, int32 end);
	bool _nextSubblockType();
	void _findMostLikelyOIId(std::vector<int> &candidates) const;
	void _getOIInfo(uint16 bmMOffset, std::vector<uint16> &oiOffset, const std::vector<uint16> &ocOffset);
	void _getSizes();
	void _prepareBlockO(TreeBlock &subblock, int32 offset, int32 size, int type);
	void _prepareBlockOO(TreeBlock &subblock, int32 offsetToOffset, int32 size, int type);
	void _updateOffset(int32 offsetToOffset, int32 minOffset, int32 shift, uint32 subblockTag);
	void _cleanup();
	void _subblockUpdated(TreeBlock &subblock, int32 sizeDiff) override;

public:
	TreeBlock *nextBlock() override;
	bool nextBlock(TreeBlock &subblock) override;
	void firstBlock() override;

public:
	OldRoom();
	~OldRoom() override = 0;

public: // TreeBlock copy
	OldRoom(const TreeBlock &block);
	OldRoom &operator=(const TreeBlock &block) override;
};

/*
 * OldRoomV1
 */

class OldRoomV1 : public OldRoom
{
protected:
	static const uint32 TAGS[];
	static const uint32 BMTAGS[];

protected:
	uint32 _tags(int i) const override { return OldRoomV1::TAGS[i]; }
	uint32 _bmTags(int i) const override { return OldRoomV1::BMTAGS[i]; }
	int _oBMWidth() const override { return 0x04; }
	int _oBMHeight() const override { return 0x06; }
	int _ooBM() const override { return 0x0A; }
	int _oObjNbr() const override { return 0x14; }
	int _ooBX() const override { return 0x15; }
	int _oNLSize() const override { return 0x16; }
	int _oSLSize() const override { return 0x17; }
	int _ooEX() const override { return 0x18; }
	int _ooEN() const override { return 0x1A; }
	int _oObjTOC() const override { return 0x1C; }
	int _oOCId() const override { return 0x04; }
	int _oOCWidth() const override { return 0x09; }
	int _oOCHeight() const override { return 0x0D; }
	int _bmNbr() const override { return 5; }
	void _init() override;
	uint16 _getOISize(uint16 width, uint16 height, uint16 offset, bool &ambiguous) override;
	void _checkOCSizes(const std::vector<uint16> &ocOffset, int32 ocEnd) override;
	void _setBXOffset(uint16 o) override;
	uint16 _getBXOffset() override;
	void _getLSOffsets(std::vector<uint16> &lsOffset, byte objNbr, byte nlSize, byte slSize) override;

public:
	OldRoomV1();
	~OldRoomV1() override;

private: // Not copiable
	OldRoomV1(const OldRoomV1 &);
	OldRoomV1 &operator=(const OldRoomV1 &);

public: // TreeBlock part still copiable
	OldRoomV1(const TreeBlock &block);
	OldRoomV1 &operator=(const TreeBlock &block) override;
};

/*
 * OldRoomV2
 */

class OldRoomV2 : public OldRoom
{
protected:
	static const uint32 TAGS[];
	static const uint32 BMTAGS[];

protected:
	uint32 _tags(int i) const override { return OldRoomV2::TAGS[i]; }
	uint32 _bmTags(int i) const override { return OldRoomV2::BMTAGS[i]; }
	int _oBMWidth() const override { return 0x04; }
	int _oBMHeight() const override { return 0x06; }
	int _ooBM() const override { return 0x0A; }
	int _oObjNbr() const override { return 0x14; }
	int _ooBX() const override { return 0x15; }
	int _oNLSize() const override { return 0x16; }
	int _oSLSize() const override { return 0x17; }
	int _ooEX() const override { return 0x18; }
	int _ooEN() const override { return 0x1A; }
	int _oObjTOC() const override { return 0x1C; }
	int _oOCId() const override { return 0x04; }
	int _oOCWidth() const override { return 0x09; }
	int _oOCHeight() const override { return 0x0D; }
	int _bmNbr() const override { return 2; }
	void _init() override;
	uint16 _getOISize(uint16 width, uint16 height, uint16 offset, bool &ambiguous) override;
	void _checkOCSizes(const std::vector<uint16> &ocOffset, int32 ocEnd) override;
	void _setBXOffset(uint16 o) override;
	uint16 _getBXOffset() override;
	void _getLSOffsets(std::vector<uint16> &lsOffset, byte objNbr, byte nlSize, byte slSize) override;

public:
	OldRoomV2();
	~OldRoomV2() override;

private: // Not copiable
	OldRoomV2(const OldRoomV2 &);
	OldRoomV2 &operator=(const OldRoomV2 &);

public: // TreeBlock part still copiable
	OldRoomV2(const TreeBlock &block);
	OldRoomV2 &operator=(const TreeBlock &block) override;
};

/*
 * OldRoomV3
 */

class OldRoomV3 : public OldRoom
{
protected:
	static const uint32 TAGS[];
	static const uint32 BMTAGS[];

protected:
	uint32 _tags(int i) const override { return OldRoomV3::TAGS[i]; }
	uint32 _bmTags(int i) const override { return OldRoomV3::BMTAGS[i]; }
	int _oBMWidth() const override { return 0x04; }
	int _oBMHeight() const override { return 0x06; }
	int _ooBM() const override { return 0x0A; }
	int _oObjNbr() const override { return 0x14; }
	int _ooBX() const override { return 0x15; }
	int _oNLSize() const override { return 0x17; }
	int _oSLSize() const override { return 0x18; }
	int _ooEX() const override { return 0x19; }
	int _ooEN() const override { return 0x1B; }
	int _oObjTOC() const override { return 0x1D; }
	int _oOCId() const override { return 0x04; }
	int _oOCWidth() const override { return 0x09; }
	int _oOCHeight() const override { return 0x0F; }
	int _bmNbr() const override { return 2; }
	void _init() override;
	uint16 _getOISize(uint16 width, uint16 height, uint16 offset, bool &ambiguous) override;
	void _checkOCSizes(const std::vector<uint16> &ocOffset, int32 ocEnd) override;
	void _setBXOffset(uint16 o) override;
	uint16 _getBXOffset() override;
	void _getLSOffsets(std::vector<uint16> &lsOffset, byte objNbr, byte nlSize, byte slSize) override;

public:
	OldRoomV3();
	~OldRoomV3() override;

private: // Not copiable
	OldRoomV3(const OldRoomV3 &);
	OldRoomV3 &operator=(const OldRoomV3 &);

public: // TreeBlock part still copiable
	OldRoomV3(const TreeBlock &block);
	OldRoomV3 &operator=(const TreeBlock &block) override;
};

/*
 * LeafBlock
 */

class LeafBlock : public TreeBlock
{
	int _findSubblockId(TreeBlock &) const override
	{
		throw TreeBlock::ForbiddenMethod("LeafBlock::_findSubblockId: shouldn't be here");
	}
	void _readSubblockHeader(TreeBlock &, BlockFormat, unsigned int)
	{
		throw TreeBlock::ForbiddenMethod("LeafBlock::_readSubblockHeader: shouldn't be here");
	}
	bool _readNextSubblock(TreeBlock &) override
	{
		throw TreeBlock::ForbiddenMethod("LeafBlock::_readNextSubblock: shouldn't be here");
	}
	void _subblockUpdated(TreeBlock &, int32) override
	{
		throw TreeBlock::ForbiddenMethod("LeafBlock::_subblockUpdated: shouldn't be here");
	}
	void _adopt(TreeBlock &) override
	{
		throw TreeBlock::ForbiddenMethod("LeafBlock::_adopt: shouldn't be here");
	}

public:
	TreeBlock *nextBlock() override
	{
		throw TreeBlock::ForbiddenMethod("LeafBlock::nextBlock: shouldn't be here");
	}
	bool nextBlock(TreeBlock &) override
	{
		throw TreeBlock::ForbiddenMethod("LeafBlock::nextBlock: shouldn't be here");
	}
	void firstBlock() override
	{
		throw TreeBlock::ForbiddenMethod("LeafBlock::firstBlock: shouldn't be here");
	}

public:
	LeafBlock();
	~LeafBlock() override;

private:
	LeafBlock(const LeafBlock &);
	LeafBlock &operator=(const LeafBlock &);

public:
	LeafBlock(const TreeBlock &block);
	LeafBlock &operator=(const TreeBlock &block) override;
};

/*
 * OldOIBlock
 */

class OldOIBlock : public LeafBlock
{
	friend class OldRoom;

private:
	int _num;
};

/*
 * OldLSBlock
 */

class OldLSBlock : public LeafBlock
{
	friend class OldRoom;

private:
	int _num;
};

/*
 * BlockPtr
 */

// Used to get rid of "try { } catch (...) { delete block; }"
template <class T>
class BlockPtr
{
private:
	T *_ptr;

public:
	void del()
	{
		::delete _ptr;
		_ptr = nullptr;
	}
	T *operator->() const { return _ptr; }
	T &operator*() { return *_ptr; }
	const T &operator*() const { return *_ptr; }
	BlockPtr<T> &operator=(T *p)
	{
		if (_ptr != nullptr) del();
		_ptr = p;
		return *this;
	}
	BlockPtr<T> &operator=(Block *p)
	{
		if (_ptr != nullptr)
			del();
		if (p == nullptr)
			_ptr = nullptr;
		else if ((_ptr = dynamic_cast<T *>(p)) == nullptr)
			throw std::logic_error("BlockPtr::operator=: Impossible cast");
		return *this;
	}
	template <class T2> bool is() const { return dynamic_cast<T2 *> (_ptr) != nullptr; }
	bool operator!=(Block *p) const { return _ptr != p; }
	bool operator==(Block *p) const { return _ptr == p; }
public:
	BlockPtr<T>() : _ptr(nullptr) { }
	~BlockPtr<T>()
	{
		::delete _ptr;
	}

private: // Not copiable
	BlockPtr<T>(const BlockPtr<T> &);
	BlockPtr<T> &operator=(const BlockPtr<T> &);
};

typedef BlockPtr<TreeBlock> TreeBlockPtr;
typedef BlockPtr<GlobalTocBlock> GlobalTocBlockPtr;

#endif
