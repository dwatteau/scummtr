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

#include "toolbox.hpp"
#include "file.hpp"
#include "backup.hpp"
#include "io.hpp"

using namespace std;

const char *const BackUp::SUFFIX = "~~scummrp-tmp";

BackUp::BackUp() :
	_files()
{
}

BackUp::~BackUp()
{
	cancelChanges();
}

string BackUp::_backupPath(const char *f)
{
	string bak(f);

	bak += BackUp::SUFFIX;
	return bak;
}

string BackUp::backup(const char *f, bool createCopy)
{
	string bak;

	if (File::isReadOnly(f))
		throw File::IOError(xsprintf("Cannot open %s", f));
	bak = _backupPath(f);
	if (createCopy)
		try
		{
			File::copy(f, bak.c_str(), true);
		}
		catch (File::IOError &)
		{
			throw File::IOError(xsprintf("Couldn't create backup file: %s", bak.c_str()));
		}
	else
		if (File::exists(bak.c_str()))
			throw File::AlreadyExists(xsprintf("%s already exists", bak.c_str()));
	_files.push_back(string(f));
	return bak;
}

void BackUp::cancelChanges()
{
	list<string>::iterator i;
	string bak;

	for (i = _files.begin(); i !=_files.end(); ++i)
		try
		{
			xremove(_backupPath(i->c_str()).c_str());
		}
		catch (runtime_error &e)
		{
			ScummRpIO::warning(e.what());
		}
	_files.clear();
}

void BackUp::applyChanges()
{
	list<string>::iterator i;
	string bak;

	for (i = _files.begin(); i != _files.end(); ++i)
		try
		{
			xremove(i->c_str());
			xrename(_backupPath(i->c_str()).c_str(), i->c_str());
		}
		catch (runtime_error &e)
		{
			ScummRpIO::warning(e.what());
		}
	_files.clear();
}
