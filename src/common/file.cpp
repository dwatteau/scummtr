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

#include "common/file.hpp"
#include "common/io.hpp"
#include "common/toolbox.hpp"

#include <cstring>

#include <algorithm>
#include <string>

/*
 * File
 */

const char *const File::TMP_SUFFIX = "~~tmp%.2X";

File::File() :
    _file(), _gpos(0), _ppos(0), _path(nullptr), _size(0),
    _part(), _mode((std::ios::openmode)0), _seekedg(false), _seekedp(false)
{
}

File::File(const char *filename, std::ios::openmode mode) :
    _file(filename, mode), _gpos(0), _ppos(0), _path(nullptr), _size(0),
    _part(), _mode((std::ios::openmode)0), _seekedg(false), _seekedp(false)
{
	// The destructor won't be called if the constructor has failed.
	try
	{
		_onOpen(filename, mode);
	}
	catch (...)
	{
		_zap();
		throw;
	}
}

File::~File()
{
	// A destructor shouldn't throw exceptions, so let's catch them all.
	try
	{
		close();
	}
	catch (std::exception &e)
	{
		std::cerr << "Unhandled exception in File::~File:" << std::endl << e.what() << std::endl;
	}
	catch (...)
	{
		std::cerr << "Unhandled exception in File::~File." << std::endl;
	}
}

void File::_zap()
{
	_size = 0;
	delete[] _path;
	_path = nullptr;
	_mode = (std::ios::openmode)0;
	_seekedg = false;
	_seekedp = false;
	_part._zap();
}

std::streamsize File::_getStreamSize()
{
	std::streampos start, end, pos;

	if (_mode & std::ios::in)
	{
		pos = _file.tellg();
		_file.seekg(0, std::ios::beg);

		start = _file.tellg();
		_file.seekg(0, std::ios::end);

		end = _file.tellg();
		_file.seekg(pos);

		return (std::streamsize)(end - start);
	}
	else
	{
		pos = _file.tellp();
		_file.seekp(0, std::ios::beg);

		start = _file.tellp();
		_file.seekp(0, std::ios::end);

		end = _file.tellp();
		_file.seekp(pos);

		return (std::streamsize)(end - start);
	}
}

void File::_onOpen(const char *filename, std::ios::openmode mode)
{
	if (!(mode & (std::ios::in | std::ios::out)))
		throw std::logic_error("File::_onOpen: !(mode & (std::ios::in | std::ios::out))");

	_gpos = 0;
	_ppos = 0;
	_mode = mode;
	if (_file.is_open())
	{
		_path = xstrdup(filename);
		_part._zap();
		_part._file = this;
		_setSize(_getStreamSize());
	}
}

std::string File::_tmpPath(const char *srcPath)
{
	std::string tmpPath;

	for (int i = 0; i < 256; ++i)
	{
		tmpPath = srcPath;
		tmpPath += xsprintf(File::TMP_SUFFIX, i);

		if (!File::exists(tmpPath.c_str()))
			return tmpPath;
	}

	throw File::AlreadyExists(xsprintf("Cannot find a free name for a temporary copy of %s", srcPath));
}

void File::_truncateAndClose()
{
	std::string tmpPath;
	File tmpFile;

	seekg(0, std::ios::beg);
	tmpPath = File::_tmpPath(_path);
	tmpFile.open(tmpPath.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
	try
	{
		tmpFile.write(*this, _size);
		tmpFile.close();
	}
	catch (std::runtime_error &)
	{
		try
		{
			tmpFile.close();
			xremove(tmpPath.c_str());
		}
		catch (std::runtime_error &)
		{
		}
		throw;
	}
	_file.close();
	xremove(_path);
	xrename(tmpPath.c_str(), _path);
}

void File::_setSize(std::streamsize newSize)
{
	if (newSize < _size)
		for (std::list<FilePart *>::iterator i = _part._children.begin(); i != _part._children.end(); ++i)
			if ((*i)->_offset + (*i)->_size > _part._offset + newSize)
				throw FilePart::Error("File::_setSize: Children too big");

	_size = newSize;
	_part._size = _size;
}

bool File::exists(const char *path)
{
	File f;

	f.open(path, std::ios::in | std::ios::binary);
	if (!f.is_open())
		return false;

	f.close();

	return true;
}

template <class T1, class T2>
void File::_copyDataFromFileToFile(T1 &f1, T2 &f2, std::streamsize n)
{
	char chunk[File::CHUNK_SIZE];
	std::streamsize written, chunkSize;

	chunkSize = File::CHUNK_SIZE;
	for (written = 0; written < n; written += chunkSize)
	{
		if (chunkSize > n - written)
			chunkSize = n - written;

		f2.read(chunk, chunkSize);
		f1.write(chunk, chunkSize);
	}
}

std::streamsize File::fileSize(const char *path)
{
	File f(path, std::ios::in | std::ios::binary);

	if (!f.is_open())
		return 0;

	return f.size();
}

bool File::isReadOnly(const char *path)
{
	File f;

	f.open(path, std::ios::in | std::ios::out | std::ios::binary);
	if (!f.is_open())
		return true;

	f.close();

	return false;
}

void File::copy(const char *src, const char *dest, bool failIfExists)
{
	File fSrc, fDest;

	if (failIfExists && File::exists(dest))
		throw File::AlreadyExists(xsprintf("%s already exists", dest));

	fSrc.open(src, std::ios::in | std::ios::binary);
	fDest.open(dest, std::ios::out | std::ios::binary | std::ios::trunc);

	fDest.write(fSrc, fSrc.size());

	fDest.close();
	fSrc.close();
}

void File::open(const char *filename, std::ios::openmode mode)
{
	_part._zap();
	mode |= std::ios::binary;
	_file.open(filename, mode);
	_onOpen(filename, mode);
}

void File::close()
{
	try
	{
		_part._zap();
		if (_file.is_open())
		{
			if (_size < _getStreamSize())
				_truncateAndClose();
			else
				_file.close();
		}
	}
	catch (...)
	{
		_zap();
		throw;
	}
	_zap();
}

File &File::seekg(std::streamoff off, std::ios::seekdir dir)
{
	switch (dir)
	{
	case std::ios::beg:
		_gpos = off;
		break;
	case std::ios::cur:
		_gpos += off;
		break;
	case std::ios::end:
		_gpos = off + _size;
		break;
	default:
		throw std::logic_error("File::seekg: invalid seekdir");
	}

	_seekedg = true;

	return *this;
}

File &File::seekp(std::streamoff off, std::ios::seekdir dir)
{
	switch (dir)
	{
	case std::ios::beg:
		_ppos = off;
		break;
	case std::ios::cur:
		_ppos += off;
		break;
	case std::ios::end:
		_ppos = off + _size;
		break;
	default:
		throw std::logic_error("File::seekp: invalid seekdir");
	}

	_seekedp = true;

	return *this;
}

std::streamoff File::tellg(std::ios::seekdir dir)
{
	switch (dir)
	{
	case std::ios::beg:
		return _gpos;
	case std::ios::end:
		return _gpos - (std::streamoff)_size;
	case std::ios::cur:
		return 0;
	default:
		throw std::logic_error("File::tellg: invalid seekdir");
	}
}

std::streamoff File::tellp(std::ios::seekdir dir)
{
	switch (dir)
	{
	case std::ios::beg:
		return _ppos;
	case std::ios::end:
		return _ppos - (std::streamoff)_size;
	case std::ios::cur:
		return 0;
	default:
		throw std::logic_error("File::tellp: invalid seekdir");
	}
}

char File::get()
{
	char c;

	read(&c, sizeof c);

	return c;
}

File &File::put(char c)
{
	return write(&c, sizeof c);
}

// 2 only methods which directly read from _file
File &File::getline(std::string &s, char delim)
{
	if (_seekedg)
	{
		_seekedg = false;
		_file.seekg(_gpos, std::ios::beg);
	}

	std::getline(_file, s, delim);
	if (_file.fail())
		throw File::IOError(xsprintf("File::getline: %s", _path));

	_gpos += (std::streamoff)s.size();
	if (_gpos != _size)
		++_gpos;

	return *this;
}

File &File::read(char *s, std::streamsize n)
{
	if (n == 0)
		return *this;

	if (_seekedg)
	{
		_seekedg = false;
		_file.seekg(_gpos, std::ios::beg);
	}

	if (_gpos + n > _size)
		throw File::UnexpectedEOF(xsprintf("Unexpected EOF in: %s", _path));

	_file.read(s, n);
	if (_file.fail())
		throw File::IOError(xsprintf("File::read: %s", _path));

	_gpos += n;

	return *this;
}

// only method which directly writes to _file
File &File::write(const char *s, std::streamsize n)
{
	if (n <= 0)
		throw File::IOError(xsprintf("File::write: n <= 0 in: %s", _path));

	if (_seekedp)
	{
		_seekedp = false;
		_file.seekp(_ppos, std::ios::beg);
	}

	if (_ppos + n > _size)
		_setSize(_ppos + n);

	_file.write(s, n);
	if (_file.fail())
		throw File::IOError(xsprintf("File::write: %s", _path));

	_ppos += n;

	return *this;
}

File &File::write(const std::string &s)
{
	write(s.c_str(), (std::streamsize)s.size());

	return *this;
}

File &File::write(File &f, std::streamsize n)
{
	File::_copyDataFromFileToFile<File, File>(*this, f, n);

	return *this;
}

File &File::write(FilePart &f, std::streamsize n)
{
	File::_copyDataFromFileToFile<File, FilePart>(*this, f, n);

	return *this;
}

bool File::is_open()
{
	return _file.is_open();
}

std::streamsize File::size() const
{
	return _size;
}

void File::_moveFwd(std::streamoff offset, std::streamsize n)
{
	std::streamoff putPos, getPos, cpyPos, endPos;
	std::streamsize chunkSize;
	char chunk[File::CHUNK_SIZE];

	chunkSize = File::CHUNK_SIZE;
	putPos = tellp(std::ios::beg);
	getPos = tellg(std::ios::beg);
	endPos = putPos + n;
	cpyPos = endPos;
	do
	{
		cpyPos -= chunkSize;
		if (cpyPos < putPos)
		{
			chunkSize += cpyPos - putPos;
			cpyPos = putPos;
		}
		seekg(cpyPos, std::ios::beg);
		read(chunk, chunkSize);
		seekp(cpyPos + offset, std::ios::beg);
		write(chunk, chunkSize);
	} while (cpyPos != putPos);

	if (getPos >= putPos && getPos < endPos)
		getPos += offset;

	seekp(putPos, std::ios::beg);
	seekg(getPos, std::ios::beg);
}

void File::_moveBwd(std::streamoff offset, std::streamsize n)
{
	std::streamoff putPos, getPos, cpyPos, endPos;
	std::streamsize chunkSize;
	char chunk[File::CHUNK_SIZE];

	putPos = tellp(std::ios::beg);
	if (offset > putPos)
		throw std::out_of_range("File::_moveBwd: Tried moving data past beginning of file");

	chunkSize = File::CHUNK_SIZE;
	getPos = tellg(std::ios::beg);
	endPos = putPos + n;
	cpyPos = putPos;
	while (cpyPos < endPos)
	{
		if (cpyPos + chunkSize > endPos)
			chunkSize = endPos - cpyPos;
		seekg(cpyPos, std::ios::beg);
		read(chunk, chunkSize);
		seekp(cpyPos - offset, std::ios::beg);
		write(chunk, chunkSize);
		cpyPos += chunkSize;
	}

	if (getPos >= putPos && getPos < endPos)
		getPos -= offset;

	seekp(putPos, std::ios::beg);
	seekg(getPos, std::ios::beg);
}

void File::move(std::streamoff offset, std::streamsize n)
{
	if (offset == 0 || n == 0)
		return;

	if (offset > 0)
		_moveFwd(offset, n);
	else
		_moveBwd(-offset, n);
}

void File::moveEnd(std::streamoff offset)
{
	std::streamoff putPos;

	putPos = tellp(std::ios::beg);
	if (putPos > _size)
		throw File::UnexpectedEOF(xsprintf("Unexpected EOF in: %s", _path));

	move(offset, _size - putPos);
	if (offset < 0)
		_setSize(_size + offset);
}

void File::truncate(std::streamsize newSize)
{
	if (newSize > _size || newSize < 0)
		throw std::out_of_range("File::truncate: New size is larger");

	_setSize(newSize);
}

/*
 * FilePart
 */

FilePart::FilePart() :
    _file(nullptr), _offset(0), _size(0),
    _parent(nullptr), _xorKey(0), _children()
{
}

FilePart::FilePart(File &file) :
    _file(&file), _offset(0), _size(file.size()),
    _parent(&file._part), _xorKey(0), _children()
{
	_parent->_children.push_back(this);
}

FilePart::FilePart(FilePart &parent, std::streamoff o, std::streamsize s) :
    _file(parent._file), _offset(o + parent._offset), _size(s),
    _parent(&parent), _xorKey(parent._xorKey), _children()
{
	if (_offset + _size > parent._offset + parent._size)
		throw FilePart::Error("FilePart::FilePart: Size too big");

	_parent->_children.push_back(this);
}

FilePart::~FilePart()
{
	try
	{
		_zap();
	}
	catch (FilePart::Error &e)
	{
		ScummIO::fatal(e.what());
	}
}

FilePart::FilePart(const FilePart &f) :
    _file(f._file), _offset(f._offset), _size(f._size),
    _parent(nullptr), _xorKey(f._xorKey), _children()
{
	if (f._parent == nullptr)
		throw FilePart::Error("FilePart::FilePart: only 1 root per File is allowed");

	f._parent->_adopt(*this);
}

FilePart &FilePart::operator=(const FilePart &f)
{
	if (_children.size() > 0)
	{
		for (std::list<FilePart *>::iterator i = _children.begin(); i != _children.end(); ++i)
			(*i)->_parent = nullptr;

		throw FilePart::Error("FilePart destroyed before its children");
	}

	_file = f._file;
	_offset = f._offset;
	_size = f._size;
	_parent = nullptr;
	_xorKey = f._xorKey;

	if (f._parent == nullptr)
		throw FilePart::Error("FilePart::FilePart: only 1 root per File is allowed");

	f._parent->_adopt(*this);

	return *this;
}

void FilePart::_leaveParent()
{
	if (_parent != nullptr)
	{
		_parent->_children.remove(this);
		_parent = nullptr;
	}
}

void FilePart::_adopt(FilePart &f)
{
	f._leaveParent();
	_children.push_back(&f);
	f._parent = this;
}

void FilePart::_zap()
{
	if (_children.size() > 0)
	{
		for (std::list<FilePart *>::iterator i = _children.begin(); i != _children.end(); ++i)
			(*i)->_parent = nullptr;

		throw FilePart::Error("FilePart destroyed before its children");
	}

	if (_parent != nullptr)
		_leaveParent();
	else
		_file = nullptr;

	_offset = 0;
	_size = 0;
	_xorKey = 0;
	_children.resize(0);
}

void FilePart::setXORKey(byte b)
{
	_xorKey = b;
}

byte FilePart::getXORKey() const
{
	return _xorKey;
}

FilePart &FilePart::seekg(std::streamoff off, std::ios::seekdir dir)
{
	if (dir == std::ios::end)
	{
		dir = std::ios::beg;
		off += _size;
	}
	else if (dir == std::ios::beg)
	{
		off += _offset;
	}

	_file->seekg(off, dir);

	return *this;
}

FilePart &FilePart::seekp(std::streamoff off, std::ios::seekdir dir)
{
	if (dir == std::ios::end)
	{
		dir = std::ios::beg;
		off += _size;
	}
	else if (dir == std::ios::beg)
	{
		off += _offset;
	}

	_file->seekp(off, dir);

	return *this;
}

std::streamoff FilePart::tellg(std::ios::seekdir dir)
{
	switch (dir)
	{
	case std::ios::beg:
		return _file->tellg(std::ios::beg) - _offset;
	case std::ios::end:
		return _file->tellg(std::ios::beg) - _offset - _size;
	case std::ios::cur:
		return 0;
	default:
		throw std::logic_error("FilePart::tellg: invalid seekdir");
	}
}

std::streamoff FilePart::tellp(std::ios::seekdir dir)
{
	switch (dir)
	{
	case std::ios::beg:
		return _file->tellp(std::ios::beg) - _offset;
	case std::ios::end:
		return _file->tellp(std::ios::beg) - _offset - _size;
	case std::ios::cur:
		return 0;
	default:
		throw std::logic_error("FilePart::tellp: invalid seekdir");
	}
}

bool FilePart::eof()
{
	return tellg(std::ios::end) == 0;
}

void FilePart::_xorBuffer(char *buffer, byte xorKey, std::streamsize n)
{
	if (xorKey == 0)
		return;

	for (std::streamsize i = 0; i < n; i++)
		buffer[i] ^= xorKey;
}

FilePart &FilePart::read(std::string &s, std::streamsize n)
{
	char *buffer;

	buffer = nullptr;
	if (n < 0)
		throw std::logic_error(xsprintf("FilePart::read: %s", _file->_path));

	try
	{
		buffer = new char[n];
		read(buffer, n);
		s.resize(0);
		s.append(buffer, n);
	}
	catch (...)
	{
		delete[] buffer;
		throw;
	}

	delete[] buffer;

	return *this;
}

FilePart &FilePart::write(const std::string &s)
{
	if (s.size() > 0)
		write(s.data(), (std::streamsize)s.size());

	return *this;
}

// only method which directly reads from _file
FilePart &FilePart::read(char *s, std::streamsize n)
{
	std::streamoff pos;

	pos = tellg(std::ios::beg);
	if (pos < 0 || n < 0)
		throw std::logic_error(xsprintf("FilePart::read: %s", _file->_path));

	if (pos + n > _size)
		throw File::UnexpectedEOF(xsprintf("Unexpected EOF in: %s <0x%X, 0x%X>", _file->_path, _offset, _size));

	_file->read(s, n);
	if (_xorKey != 0)
		FilePart::_xorBuffer(s, _xorKey, n);

	return *this;
}

// only method which directly writes to _file
FilePart &FilePart::write(const char *s, std::streamsize n)
{
	std::streamoff pos;

	pos = tellp(std::ios::beg);
	if (pos < 0 || n < 0)
		throw std::logic_error(xsprintf("FilePart::write: %s", _file->_path));

	if (pos + n > _size)
	{
		resize(pos + n);
		seekp(pos, std::ios::beg);
	}

	if (_xorKey == 0)
	{
		_file->write(s, n);
	}
	else
	{
		char *xored = nullptr;

		try
		{
			xored = new char[n];
			memcpy(xored, s, n);
			FilePart::_xorBuffer(xored, _xorKey, n);
			_file->write(xored, n);
		}
		catch (...)
		{
			delete[] xored;
			throw;
		}
		delete[] xored;
	}

	return *this;
}

FilePart &FilePart::write(File &f, std::streamsize n)
{
	File::_copyDataFromFileToFile<FilePart, File>(*this, f, n);

	return *this;
}

FilePart &FilePart::write(FilePart &f, std::streamsize n)
{
	File::_copyDataFromFileToFile<FilePart, FilePart>(*this, f, n);

	return *this;
}

std::string FilePart::path() const
{
	return std::string(_file->_path);
}

std::string FilePart::name() const
{
	int i;

	for (i = (int)strlen(_file->_path) - 1; i >= 0 && _file->_path[i] != '/'; --i)
		;

	return std::string(_file->_path + ++i);
}

std::streamsize FilePart::size() const
{
	return _size;
}

std::streamoff FilePart::offset() const
{
	if (_parent != nullptr)
		return _offset - _parent->_offset;

	return _offset;
}

std::streamoff FilePart::fullOffset() const
{
	return _offset;
}

void FilePart::resize(std::streamsize newSize)
{
	std::streamsize oldSize;
	std::streamoff oldOffset;

	oldSize = _size;
	oldOffset = _offset;
	if (newSize < oldSize)
		for (std::list<FilePart *>::iterator i = _children.begin(); i != _children.end(); ++i)
			if ((*i)->_offset + (*i)->_size > _offset + newSize)
				throw FilePart::Error("FilePart::resize: Children too big");

	if (newSize == oldSize)
		return;

	_file->_part._move(_offset + _size, newSize - _size);
	if (oldSize == 0)
	{
		_offset = oldOffset;
		_size = newSize;
	}
}

void FilePart::_shiftFrame(std::streamoff start, std::streamoff shift)
{
	if (_offset + _size < start)
		return;

	if (_offset >= start)
	{
		_offset += shift;
	}
	else
	{
		_size += shift;
		if (_size < 0)
			throw FilePart::Error("FilePart::_shiftFrame: Implosion");
	}

	for (std::list<FilePart *>::iterator i = _children.begin(); i != _children.end(); ++i)
		(*i)->_shiftFrame(start, shift);
}

void FilePart::_move(std::streamoff start, std::streamoff shift)
{
	if (shift > 0)
	{
		_file->seekp(start, std::ios::beg);
		_file->moveEnd(shift);
	}

	for (std::list<FilePart *>::iterator i = _children.begin(); i != _children.end(); ++i)
		(*i)->_shiftFrame(start, shift);

	if (shift < 0)
	{
		_file->seekp(start, std::ios::beg);
		_file->moveEnd(shift);
	}
}

template <bool B, class T>
T FilePart::get(T &i)
{
	read((char *)&i, sizeof i);

	if (sizeof(T) > 1)
	{
		if (cpu_is_little_endian())
		{
			if (B)
				FilePart::_reverse(i);
		}
		else
		{
			if (!B)
				FilePart::_reverse(i);
		}
	}

	return i;
}

template <bool B, class T>
void FilePart::put(T i)
{
	if (sizeof(T) > 1)
	{
		if (cpu_is_little_endian())
		{
			if (B)
				FilePart::_reverse(i);
		}
		else
		{
			if (!B)
				FilePart::_reverse(i);
		}
	}

	write((const char *)&i, sizeof i);
}

// for GCC
void FilePart::getLE(byte &i) { get<false, byte>(i); }
void FilePart::getLE(uint16 &i) { get<false, uint16>(i); }
void FilePart::putLE(byte &i) { put<false, byte>(i); }
void FilePart::putLE(uint16 &i) { put<false, uint16>(i); }

uint8 FilePart::getByte(uint8 &i)
{
	return get<false, uint8>(i);
}

uint16 FilePart::getLE16(uint16 &i)
{
	return get<false, uint16>(i);
}

uint32 FilePart::getLE32(uint32 &i)
{
	return get<false, uint32>(i);
}

uint16 FilePart::getBE16(uint16 &i)
{
	return get<true, uint16>(i);
}

uint32 FilePart::getBE32(uint32 &i)
{
	return get<true, uint32>(i);
}

void FilePart::putByte(uint8 i)
{
	put<false, byte>(i);
}

void FilePart::putLE16(uint16 i)
{
	put<false, uint16>(i);
}

void FilePart::putLE32(uint32 i)
{
	put<false, uint32>(i);
}

void FilePart::putBE16(uint16 i)
{
	put<true, uint16>(i);
}

void FilePart::putBE32(uint32 i)
{
	put<true, uint32>(i);
}

int8 FilePart::getByte(int8 &i)
{
	return (int8)get<false, uint8>((uint8 &)i);
}

int16 FilePart::getLE16(int16 &i)
{
	return (int16)get<false, uint16>((uint16 &)i);
}

int32 FilePart::getLE32(int32 &i)
{
	return (int32)get<false, uint32>((uint32 &)i);
}

int16 FilePart::getBE16(int16 &i)
{
	return (int16)get<true, uint16>((uint16 &)i);
}

int32 FilePart::getBE32(int32 &i)
{
	return (int32)get<true, uint32>((uint32 &)i);
}

void FilePart::putByte(int8 i)
{
	put<false, byte>((byte)i);
}

void FilePart::putLE16(int16 i)
{
	put<false, uint16>((uint16 &)i);
}

void FilePart::putLE32(int32 i)
{
	put<false, uint32>((uint32 &)i);
}

void FilePart::putBE16(int16 i)
{
	put<true, uint16>((uint16 &)i);
}

void FilePart::putBE32(int32 i)
{
	put<true, uint32>((uint32 &)i);
}

/*
 * RAMFile
 */

RAMFile::RAMFile() :
    File(), _mem(nullptr), _out(false), _capacity(0)
{
}

RAMFile::RAMFile(const char *filename, std::ios::openmode mode) :
    File(filename, mode), _mem(nullptr), _out(false), _capacity(0)
{
	// The destructor won't be called if the constructor has failed.
	try
	{
		_load();
	}
	catch (...)
	{
		_zap();
		throw;
	}
	_out = (mode & std::ios::out) != 0;
}

RAMFile::~RAMFile()
{
	if (is_open())
		_save();

	_zapRAM();
}

void RAMFile::_load()
{
	_zapRAM();

	_reallocAtLeast(_size);
	seekg(0, std::ios::beg);

	File::seekg(0, std::ios::beg);
	File::read((char *)_mem, _size);
}

void RAMFile::_save()
{
	if (_mem != nullptr && _out)
	{
		seekp(0, std::ios::beg);
		File::seekp(0, std::ios::beg);
		File::write((char *)_mem, _size);
	}
}

void RAMFile::_reallocAtLeast(std::streamsize sz)
{
	byte *buffer;
	std::streamsize oldCapacity;

	oldCapacity = _capacity;
	_capacity = (sz / File::CHUNK_SIZE + 1) * File::CHUNK_SIZE;
	buffer = new byte[_capacity];

	if (_mem != nullptr)
		memcpy(buffer, _mem, oldCapacity);

	delete[] _mem;
	_mem = buffer;
}

void RAMFile::_zapRAM()
{
	delete[] _mem;
	_mem = nullptr;
	_gpos = 0;
	_ppos = 0;
	_out = false;
	_capacity = 0;
}

void RAMFile::_zap()
{
	File::_zap();
	_zapRAM();
}

void RAMFile::open(const char *filename, std::ios::openmode mode)
{
	File::open(filename, mode);

	if (is_open())
		_load();

	_out = (mode & std::ios::out) != 0;
}

void RAMFile::close()
{
	if (is_open())
		_save();

	File::close();
	_zap();
}

// 2 only methods which directly write to _mem
File &RAMFile::getline(std::string &s, char delim)
{
	std::streamoff i;
	const char *mem;

	for (i = _gpos; i < _size; ++i)
		if (_mem[i] == delim)
			break;

	i -= _gpos;
	s.resize(i);

	mem = (char *)_mem + _gpos;

	if (i + _gpos == _size)
		_gpos = _size;
	else
		_gpos += i + 1;

	while (--i >= 0)
		s[i] = mem[i];

	return *this;
}

File &RAMFile::read(char *s, std::streamsize n)
{
	if (n == 0)
		return *this;

	if ((std::streamsize)_gpos + n > _size)
		throw File::UnexpectedEOF(xsprintf("Unexpected EOF in: %s", _path));

	if (_gpos < 0 || n < 0)
		throw File::IOError(xsprintf("RAMFile::read: %s", _path));

	memcpy(s, _mem + _gpos, n);
	_gpos += n;

	return *this;
}

// only method which directly writes to _mem
File &RAMFile::write(const char *s, std::streamsize n)
{
	if (_ppos < 0 || n <= 0 || !_out)
		throw File::IOError(xsprintf("RAMFile::write: %s", _path));

	if (_capacity < (std::streamsize)_ppos + n)
		_reallocAtLeast((std::streamsize)_ppos + n);

	memcpy(_mem + _ppos, s, n);
	_ppos += n;
	if (_size < _ppos)
		_setSize(_ppos);

	return *this;
}
