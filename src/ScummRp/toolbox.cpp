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

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <iostream>
#include <string>
#include <stdexcept>

#ifdef SCUMMRP_ORIGINAL_BUT_WRONG_INCLUDES
#if defined (_MSC_VER)
# include <direct.h>
# define mkdir _mkdir
#elif defined (UNIX)
# include <sys/stat.h>
# include <sys/types.h>
#else
# include <dir.h>
#endif
#else
#if defined (_WIN32)
# include <direct.h>
# define mkdir _mkdir
#else /* assume Unix */
# include <sys/stat.h>
# include <sys/types.h>
#endif
#endif

using namespace std;

/*
 *
 */

const char *xsprintf(const char *format, ...)
{
	static const int MAX_MSG_SIZE = 1024;
	static const int MAX_USES = 8;
	static char errorMessage[MAX_USES][MAX_MSG_SIZE];
	static int currentStr = 0;
	va_list va;

	currentStr = (currentStr + 1) % MAX_USES;
	va_start(va, format);
#ifdef _MSC_VER
	_vsnprintf(errorMessage[currentStr], MAX_MSG_SIZE, format, va);
#else
	vsprintf(errorMessage[currentStr], format, va); // FIXME unsafe sprintf
#endif
	va_end(va);
	return errorMessage[currentStr];
}

char *xstrdup(const char *src)
{
	char *dest;
	size_t size;

	size = strlen(src) + 1;
	dest = new char[size];
	memcpy(dest, src, size);
	return dest;
}

void xremove(const char *path)
{
	if (remove(path))
	{
		string what("Cannot remove "); what += path;
		throw runtime_error(what);
	}
}

void xrename(const char *oldname, const char *newname)
{
	if (rename(oldname, newname))
	{
		string what("Cannot rename "); what += oldname; what += " to "; what += newname;
		throw runtime_error(what);
	}
}

int xmkdir(const char *path)
{
	int i;
	char *tmpPath;
	int ret;
	char prevChar;

	ret = -1;
	tmpPath = xstrdup(path);
	prevChar = 0;
	for (i = 0; tmpPath[i]; ++i)
	{
		if (tmpPath[i] == '/' && i > 0 && prevChar != '/')
		{
			tmpPath[i] = 0;
#ifdef _WIN32
			ret = mkdir(tmpPath);
#else
			ret = mkdir(tmpPath, 0755);
#endif
			tmpPath[i] = '/';
		}
		prevChar = tmpPath[i];
	}
	if (prevChar != '/')
	{
#ifdef _WIN32
		ret = mkdir(tmpPath);
#else
		ret = mkdir(tmpPath, 0755);
#endif
	}
	delete []tmpPath;
	return ret;
}
