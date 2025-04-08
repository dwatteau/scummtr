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

#include "common/toolbox.hpp"

#include <cerrno>
#include <cstdarg>
#include <cstdio>
#include <cstring>

#include <iostream>
#include <stdexcept>
#include <string>

#if defined(_WIN32) && !defined(__DJGPP__)
#  include <direct.h>
#  define mkdir(path, mode) _mkdir(path)
#else /* assume Unix */
#  include <sys/types.h>
#  include <sys/stat.h>
#endif

const char *xsprintf(_Printf_format_string_ const char *format, ...)
{
	static const int MAX_MSG_SIZE = 1024;
	static const int MAX_USES = 8;
	static char errorMessage[MAX_USES][MAX_MSG_SIZE];
	static int currentStr = 0;
	va_list va;

	currentStr = (currentStr + 1) % MAX_USES;
	va_start(va, format);
	vsnprintf(errorMessage[currentStr], MAX_MSG_SIZE, format, va);
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
	if (remove(path) != 0)
	{
		std::string what("Cannot remove ");
		what += path;
		throw std::runtime_error(what);
	}
}

// note: newname must not already exist, because non-POSIX systems don't accept
// this. Call xremove() beforehand if necessary.
void xrename(const char *oldname, const char *newname)
{
	if (rename(oldname, newname) != 0)
	{
		std::string what("Cannot rename ");
		what += oldname;
		what += " to ";
		what += newname;
		throw std::runtime_error(what);
	}
}

int xmkdir(const char *path)
{
	char *tmpPath;
	int ret;
	char prevChar;

	ret = -1;
	tmpPath = xstrdup(path);
	prevChar = '\0';
	for (int i = 0; tmpPath[i] != '\0'; ++i)
	{
		if (tmpPath[i] == '/' && i > 0 && prevChar != '/')
		{
			tmpPath[i] = '\0';
			ret = mkdir(tmpPath, 0755);
			if (ret != 0 && errno != EEXIST)
			{
				std::string what("Cannot create ");
				what += tmpPath;
				what += " directory";
				throw std::runtime_error(what);
			}
			tmpPath[i] = '/';
		}
		prevChar = tmpPath[i];
	}
	if (prevChar != '/')
	{
		ret = mkdir(tmpPath, 0755);
		if (ret != 0 && errno != EEXIST)
		{
			std::string what("Cannot create ");
			what += tmpPath;
			what += " directory";
			throw std::runtime_error(what);
		}
	}

	delete[] tmpPath;

	return ret;
}

void printCommonDisclaimer()
{
	std::cout << "ALWAYS MAKE BACKUPS before making changes to your games!\n\n";

	std::cout << "For documentation, answers to common problems, or reporting issues,\n";
	std::cout << "please visit: <https://github.com/dwatteau/scummtr>" << std::endl;
}
