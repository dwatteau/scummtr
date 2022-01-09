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

#ifndef SCUMMRP_TOC_HPP
#define SCUMMRP_TOC_HPP

#include "common/types.hpp"
#include "common/file.hpp"

#include "rptypes.hpp"

#include <stdexcept>
#include <string>

/*
 * TableOfContent
 */

class TableOfContent
{
public:
	static const int INVALID_ID = -1;

public:
	struct TocElement
	{
		byte roomId;
		int32 offset;
	};
	enum Type
	{
		TOCT_NULL = 0,
		TOCT_LFLF,
		TOCT_ROOM,
		TOCT_SCRP,
		TOCT_SOUN,
		TOCT_COST,
		TOCT_CHAR
	};

public:
	class Error : public std::runtime_error
	{
	public:
		Error(const std::string &message) : std::runtime_error(message) { }
	};
	class InvalidElement : public std::logic_error
	{
	public:
		InvalidElement(const std::string &message) : std::logic_error(message) { }
	};
	class InvalidId : public std::logic_error
	{
	public:
		InvalidId(const std::string &message) : std::logic_error(message) { }
	};

protected:
	TocElement *_toc;
	int _size;
	TableOfContent::Type _type;
	int _iterator[256];
	bool _accessed[256];

protected:
	bool _idInRange(int id) const;
	bool _validItem(int id) const;
	bool _validId(int id) const;
	void _load16Sep32(FilePart &file);
	void _load8Sep16(FilePart &file, int size);
	void _load8Mix32(FilePart &file);
	void _load16Mix32(FilePart &file);
	void _save16Sep32(FilePart &file) const;
	void _save8Sep16(FilePart &file, bool fixedSize) const;
	void _save8Mix32(FilePart &file) const;
	void _save16Mix32(FilePart &file) const;
	virtual void _zap();

public:
	TableOfContent::Type getType() const;
	bool accessed(byte roomId) const;
	void accessing(byte roomId);
	int getSize() const;
	void load(FilePart &file, GlobalTocFormat format);
	void save(FilePart &file, GlobalTocFormat format);
	virtual void merge(const TableOfContent &t);
	virtual int count(byte roomId, int32 offset) const;
	virtual int findId(byte roomId, int32 offset) const;
	virtual void firstId(byte roomId);
	virtual bool nextId(int &id, byte roomId);
	virtual TableOfContent::TocElement &operator[](int id);
	virtual TableOfContent::TocElement operator[](int id) const;
	virtual void load(FilePart &file, GlobalTocFormat format, int size);
	virtual void save(FilePart &file, GlobalTocFormat format, bool fixedSize);

public:
	TableOfContent(TableOfContent::Type t);
	virtual ~TableOfContent();

private:
	TableOfContent();

public:
	TableOfContent(const TableOfContent &t);
	TableOfContent &operator=(const TableOfContent &t);
};

/*
 * GlobalRoomIndex
 */

class GlobalRoomIndex : public TableOfContent
{
public:
	class Error : public std::runtime_error
	{
	public:
		Error(const std::string &message) : std::runtime_error(message) { }
	};
	class IndexTooShort : public std::runtime_error
	{
	public:
		IndexTooShort(const std::string &message) : std::runtime_error(message) { }
	};

protected:
	bool _first;

protected:
	void _zap() override;

public:
	TableOfContent::TocElement &operator[](int id) override;
	TableOfContent::TocElement operator[](int id) const override;
	void merge(const TableOfContent &t) override;
	int count(byte roomId, int32 offset) const override;
	int findId(byte roomId, int32 offset) const override;
	void firstId(byte roomId) override;
	bool nextId(int &id, byte roomId) override;
	void load(FilePart &file, GlobalTocFormat format, int size) override;
	void save(FilePart &file, GlobalTocFormat format, bool fixedSize) override;
	virtual int numberOfDisks() const;

public:
	GlobalRoomIndex();
	~GlobalRoomIndex() override;

public:
	GlobalRoomIndex &operator=(const GlobalRoomIndex &);

private:
	GlobalRoomIndex(const GlobalRoomIndex &);
};

/*
 * RoomIndex
 */

class RoomIndex
{
private:
	TableOfContent _toc;
	int _iterator;
	int _map[256];

public:
	void load(FilePart &file);
	void save(FilePart &file);
	void update(byte roomId, int32 offset);
	byte findId(int32 offset) const;
	int32 &operator[](byte roomId);
	void firstId();
	bool nextId(byte &roomId);

public:
	RoomIndex();
	~RoomIndex();

private: // Not copiable
	RoomIndex(const RoomIndex &);
	RoomIndex &operator=(const RoomIndex &);
};

#endif
