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

#include "common/io.hpp"

#include <cstdlib>
#include <iostream>

/*
 * ScummIO
 */

uint32 ScummIO::_infoSlots = 0;
bool ScummIO::_quiet = false;

void ScummIO::info(const char *msg)
{
	std::cout << msg << std::endl;
}

void ScummIO::info(int slots, const char *msg)
{
	if (ScummIO::_infoSlots & slots && !ScummIO::_quiet)
		std::cout << msg << std::endl;
}

void ScummIO::crash(const char *msg)
{
	std::cerr << "CRASH: " << msg << std::endl;
	std::abort();
}

void ScummIO::fatal(const char *msg)
{
	std::cerr << "ERROR: " << msg << std::endl;
	std::exit(1);
}

void ScummIO::error(const char *msg)
{
	if (!ScummIO::_quiet)
		std::cerr << "ERROR: " << msg << std::endl;
}

void ScummIO::warning(const char *msg)
{
	if (!ScummIO::_quiet)
		std::cerr << "WARNING: " << msg << std::endl;
}

void ScummIO::setQuiet(bool q)
{
	ScummIO::_quiet = q;
}

void ScummIO::setInfoSlots(uint32 infoSlots)
{
	ScummIO::_infoSlots = infoSlots;
}
