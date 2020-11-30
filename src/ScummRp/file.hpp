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

#ifndef SCUMMRP_FILE_HPP
#define SCUMMRP_FILE_HPP

#include "backup.hpp"
#include "types.hpp"
#include "toolbox.hpp"

#include <fstream>
#include <list>
#include <string>
#include <stdexcept>
#include <iostream>

class File;
class RAMFile;
template <class T> class SeqFile;
class FilePart;
class FileHandle;
class FilePartHandle;

// TODO make it simple and *logical* :)
// TODO remove all the unused methods (in File mainly)

/*
 * FileHandle
 */

class FileHandle
{
public:
	virtual FilePart *operator->() = 0;
	virtual FilePart &operator*() = 0;
	virtual const FilePart *operator->() const = 0;
	virtual const FilePart &operator*() const = 0;
};

/*
 * FilePart
 */

class FilePart
{
	friend class File;
public:
	class Error : public std::logic_error
	{
	public:
		Error(const std::string &message) : std::logic_error(message) { }
	};
private:
	File *_file;
	std::streamoff _offset;
	std::streamsize _size;
	FilePart *_parent;
	byte _xorKey;
	std::list<FilePart *> _children;
private:
	static void _reverse(uint8 &) { }
	static void _reverse(uint16 &i) { i = (i >> 8) | (i << 8); }
	static void _reverse(uint32 &i) { i = (i >> 24) | ((i >> 8) & 0x0000FF00) | ((i << 8) & 0x00FF0000) | (i << 24); }
	static void _xorBuffer(char *buffer, byte xorKey, std::streamsize n);
private:
	void _zap();
	void _move(std::streamoff start, std::streamoff shift);
	void _shiftFrame(std::streamoff start, std::streamoff shift);
	void _leaveParent();
	void _adopt(FilePart &f);
public:
	bool eof();
	std::streamsize size() const;
	void resize(std::streamsize size);
	std::streamoff offset() const;
	std::streamoff fullOffset() const;
	std::string path() const;
	std::string name() const;
	void setXORKey(byte b);
	byte getXORKey() const;
	template <bool B, class T> T get(T &i);
	template <bool B, class T> void put(T i);
	// for GCC
	void getLE(byte &i);
	void getLE(uint16 &i);
	void putLE(byte &i);
	void putLE(uint16 &i);
	uint8 getByte(uint8 &i);
	uint16 getLE16(uint16 &i);
	uint32 getLE32(uint32 &i);
	uint16 getBE16(uint16 &i);
	uint32 getBE32(uint32 &i);
	void putByte(uint8 i);
	void putLE16(uint16 i);
	void putLE32(uint32 i);
	void putBE16(uint16 i);
	void putBE32(uint32 i);
	int8 getByte(int8 &i);
	int16 getLE16(int16 &i);
	int32 getLE32(int32 &i);
	int16 getBE16(int16 &i);
	int32 getBE32(int32 &i);
	void putByte(int8 i);
	void putLE16(int16 i);
	void putLE32(int32 i);
	void putBE16(int16 i);
	void putBE32(int32 i);
	FilePart &seekg(std::streamoff off, std::ios::seekdir dir);
	FilePart &seekp(std::streamoff off, std::ios::seekdir dir);
	std::streamoff tellg(std::ios::seekdir dir);
	std::streamoff tellp(std::ios::seekdir dir);
	FilePart &read(std::string &s, std::streamsize n);
	FilePart &read(char *s, std::streamsize n);
	FilePart &write(const std::string &s);
	FilePart &write(const char *s, std::streamsize n);
	FilePart &write(FilePart &f, std::streamsize n);
	FilePart &write(File &f, std::streamsize n);
public:
	FilePart(File &file);
	FilePart(FilePart &parent, std::streamoff o, std::streamsize s);
	virtual ~FilePart();
public:
	FilePart(const FilePart &f);
	FilePart &operator=(const FilePart &f);
private: // No public default constructor
	FilePart();
};

/*
 * File
 */

class File : public FileHandle
{
	friend class FilePart;
protected:
	static const char *const TMP_SUFFIX;
	static const int CHUNK_SIZE = 0x800;
public:
	class IOError : public std::runtime_error
	{
	public:
		IOError(const std::string &message) : std::runtime_error(message) { }
	};
	class UnexpectedEOF : public std::runtime_error
	{
	public:
		UnexpectedEOF(const std::string &message) : std::runtime_error(message) { }
	};
	class AlreadyExists : public std::runtime_error
	{
	public:
		AlreadyExists(const std::string &message) : std::runtime_error(message) { }
	};
protected:
	std::fstream _file;
	std::streamoff _gpos;
	std::streamoff _ppos;
	char *_path;
	std::streamsize _size;
	FilePart _part;
	std::ios::openmode _mode;
	bool _seekedg;
	bool _seekedp;
protected:
	template <class T1, class T2>
	static void _copyDataFromFileToFile(T1 &f1, T2 &f2, std::streamsize n);
	static std::string _tmpPath(const char *srcPath);
public:
	static bool exists(const char *path);
	static bool isReadOnly(const char *path);
	static void copy(const char *src, const char *dest, bool failIfExists);
	static std::streamsize fileSize(const char *path);
protected:
	virtual std::streamsize _getStreamSize();
	virtual void _onOpen(const char *filename, std::ios::openmode mode);
	virtual void _truncateAndClose();
	virtual void _zap();
	virtual void _moveFwd(std::streamoff offset, std::streamsize n);
	virtual void _moveBwd(std::streamoff offset, std::streamsize n);
	virtual void _setSize(std::streamsize newSize);
public:
	virtual FilePart *operator->() { return &_part; }
	virtual FilePart &operator*() { return _part; }
	virtual const FilePart *operator->() const { return &_part; }
	virtual const FilePart &operator*() const { return _part; }
	virtual void truncate(std::streamsize newSize);
	virtual std::streamsize size() const;
	virtual File &seekg(std::streamoff off, std::ios::seekdir dir);
	virtual File &seekp(std::streamoff off, std::ios::seekdir dir);
	virtual std::streamoff tellg(std::ios::seekdir dir);
	virtual std::streamoff tellp(std::ios::seekdir dir);
	virtual void open(const char *filename, std::ios::openmode mode = std::ios::in | std::ios::out | std::ios::binary);
	virtual void close();
	virtual bool is_open();
	virtual void move(std::streamoff offset, std::streamsize n);
	virtual void moveEnd(std::streamoff offset);
	virtual File &read(char *s, std::streamsize n);
	virtual File &write(const char *s, std::streamsize n);
	virtual File &write(const std::string &s);
	virtual File &write(File &f, std::streamsize n);
	virtual File &write(FilePart &f, std::streamsize n);
	virtual File &getline(std::string &s, char delim);
	virtual char get();
	virtual File &put(char c);
public:
	File();
	explicit File(const char *filename, std::ios::openmode mode = std::ios::in | std::ios::out | std::ios::binary);
	virtual ~File();
private: // Not copiable
	File(const File &);
	File &operator=(const File &);
};

/*
 * FilePartHandle
 */

class FilePartHandle : public FileHandle
{
private:
	FilePart *_ptr;
public:
	void del() { ::delete _ptr; _ptr = 0; }
	virtual FilePart *operator->() { return _ptr; }
	virtual const FilePart *operator->() const { return _ptr; }
	virtual FilePart &operator*() { return *_ptr; }
	virtual const FilePart &operator*() const { return *_ptr; }
	FilePartHandle &operator=(FilePart *p) { if (_ptr != 0) del(); _ptr = p; return *this; }
	bool operator!=(const FilePart *p) const { return _ptr != p; }
	bool operator==(const FilePart *p) const { return _ptr == p; }
public:
	explicit FilePartHandle(FilePart *p) : _ptr(p) { }
	FilePartHandle() : _ptr(0) { }
	explicit FilePartHandle(const FilePartHandle &f) : _ptr(f._ptr != 0 ? new FilePart(*f._ptr) : 0) { }
	virtual ~FilePartHandle() { ::delete _ptr; }
	FilePartHandle &operator=(const FilePartHandle &f) { _ptr = f._ptr != 0 ? new FilePart(*f._ptr) : 0; return *this; }
};

/*
 * RAMFile
 */

class RAMFile : public File
{
protected:
	byte *_mem;
    bool _out;
	std::streamsize _capacity;
protected:
	virtual void _zap();
	virtual void _zapRAM();
	virtual void _load();
	virtual void _save();
	virtual void _reallocAtLeast(std::streamsize sz);
public:
	virtual void open(const char *filename, std::ios::openmode mode = std::ios::in | std::ios::out | std::ios::binary);
	virtual void close();
	virtual File &read(char *s, std::streamsize n);
	virtual File &write(const char *s, std::streamsize n);
	virtual File &getline(std::string &s, char delim);
public:
	RAMFile();
	explicit RAMFile(const char *filename, std::ios::openmode mode = std::ios::in | std::ios::out | std::ios::binary);
	virtual ~RAMFile();
private: // Not copiable
	RAMFile(const RAMFile &);
	RAMFile &operator=(const RAMFile &);
};

/*
 * SeqFile
 */

template <class T> class SeqFile : public T
{
public:
	class Error : public std::logic_error
	{
	public:
		Error(const std::string &message) : std::logic_error(message) { }
	};
protected:
	T _srcFile;
	std::streamoff _tmpSize;
	std::streamoff _shift;
public:
	virtual void open(const char *, std::ios::openmode)
	{
		throw std::logic_error("SeqFile::open: Shouldn't be here");
	}
	virtual void open(const char *filename, BackUp &backupSystem)
	{
		T::open(backupSystem.backup(filename, false).c_str(), std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc);
		if (!File::isReadOnly(filename))
			_srcFile.open(filename, std::ios::binary | std::ios::in);
		if (this->is_open() != _srcFile.is_open())
		{
			T::close();
			_srcFile.close();
		}
		_shift = 0;
		_tmpSize = 0;
		this->_setSize(_srcFile.size());
	}
	virtual void close()
	{
		if (!this->is_open())
			return;
		if (_tmpSize < this->_size) {
			if (_tmpSize - _shift < _srcFile.size())
			{
				this->seekp(_tmpSize, std::ios::beg);
				_srcFile.seekg(_tmpSize - _shift, std::ios::beg);
				File::write(_srcFile, _srcFile.size() + _shift - _tmpSize);
			}
			else
			{
				T::close();
				_srcFile.close();
				throw typename SeqFile::Error("SeqFile::close: Wrong size");
			}
		}
		T::close();
		_srcFile.close();
	}
	virtual File &getline(std::string &, char)
	{
		throw std::logic_error("SeqFile::getline: Shouldn't be here");
	}
	virtual File &read(char *s, std::streamsize n)
	{
		std::streamoff gpos, ppos;

		gpos = this->tellg(std::ios::beg);
		if (gpos + n > this->_size)
			throw File::UnexpectedEOF(xsprintf("Unexpected EOF in: %s", this->_path));
		if (gpos + n > _tmpSize)
		{
			ppos = this->tellp(std::ios::beg);
			this->seekp(_tmpSize, std::ios::beg);
			_srcFile.seekg(_tmpSize - _shift, std::ios::beg);
			File::write(_srcFile, gpos + n - _tmpSize);
			this->seekp(ppos, std::ios::beg);
		}
		this->seekg(gpos, std::ios::beg);
		T::read(s, n);
		return *this;
	}
	virtual File &write(const char *s, std::streamsize n)
	{
		std::streamoff ppos;

		ppos = this->tellp(std::ios::beg);
		if (ppos + n > this->_size)
			this->_setSize(ppos + n);
		if (ppos > _tmpSize)
		{
			this->seekp(_tmpSize, std::ios::beg);
			_srcFile.seekg(_tmpSize - _shift, std::ios::beg);
			if (ppos - _shift > _srcFile.size())
			{
				if (_srcFile.size() > _tmpSize - _shift)
					File::write(_srcFile, _srcFile.size() - _tmpSize + _shift);
				_tmpSize = ppos;
			}
			else
				File::write(_srcFile, ppos - _tmpSize);
		}
		if (ppos + n > _tmpSize)
			_tmpSize = ppos + n;
		this->seekp(ppos, std::ios::beg);
		T::write(s, n);
		return *this;
	}
	virtual void move(std::streamoff, std::streamsize)
	{
		throw std::logic_error("SeqFile::move: Shouldn't be here");
	}
	virtual void moveEnd(std::streamoff offset)
	{
		std::streamoff ppos;
		std::streamsize oldSize;
		char c;

		oldSize = this->_size;
		if (offset == 0)
			return;
		ppos = this->tellp(std::ios::beg);
		if (offset > 0)
			if (ppos == _tmpSize && _tmpSize >= _srcFile.size() + _shift)
				return;
			else if (_tmpSize > ppos)
				T::move(offset, _tmpSize - ppos);
			else
			{
				if (_tmpSize < ppos)
				{
					this->seekg(ppos, std::ios::beg);
					read(&c, 0);
				}
				_tmpSize += offset;
			}
		else
			if (-offset > ppos)
				throw std::out_of_range("SeqFile::_moveEnd: Tried moving data past beginning of file");
			else if (_tmpSize < ppos + offset)
			{
				this->seekg(ppos + offset, std::ios::beg);
				read(&c, 0);
			}
			else if (_tmpSize > ppos)
			{
				T::move(offset, _tmpSize - ppos);
				_tmpSize += offset;
			}
			else
				_tmpSize = ppos + offset;
		this->_setSize(oldSize + offset);
		_shift += offset;
		this->seekp(ppos, std::ios::beg);
	}
public:
	SeqFile() :
		T(), _srcFile(), _tmpSize(0), _shift(0)
	{
	}
	SeqFile(const char *filename, BackUp &backupSystem) :
		T(backupSystem.backup(filename, false).c_str(), std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc),
		_srcFile(filename, std::ios::binary | std::ios::in | std::ios::out), _tmpSize(0), _shift(0)
	{
		if (this->is_open() != _srcFile.is_open())
		{
			T::close();
			_srcFile.close();
		}
		this->_setSize(_srcFile.size());
	}
	virtual ~SeqFile()
	{
		// A destructor shouldn't throw exceptions, so let's catch them all.
		try
		{
			this->close();
		}
		catch (std::exception &e)
		{
			try
			{
				T::close();
				_srcFile.close();
			}
			catch (std::exception &) { }
			std::cerr << "Unhandled exception in SeqFile::~SeqFile:" << std::endl << e.what() << std::endl;
		}
		catch (...)
		{
			try
			{
				T::close();
				_srcFile.close();
			}
			catch (std::exception &) { }
			std::cerr << "Unhandled exception in SeqFile::~SeqFile." << std::endl;
		}
	}
private: // Not copiable
	SeqFile(const SeqFile &);
	SeqFile &operator=(const SeqFile &);
};

#endif
