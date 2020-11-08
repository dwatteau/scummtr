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

#include "io.hpp"

#include <stdlib.h>
#include <iostream>

using namespace std;

/*
 * ScummRpIO
 */

uint32 ScummRpIO::_infoSlots = 0;
bool ScummRpIO::_quiet = false;

void ScummRpIO::info(const char *msg)
{
	cout << msg << endl;
}

void ScummRpIO::info(int slots, const char *msg)
{
	if (ScummRpIO::_infoSlots & slots && !ScummRpIO::_quiet)
		cout << msg << endl;
}

void ScummRpIO::crash(const char *msg)
{
	cerr << "CRASH: " << msg << endl;
	abort();
}

void ScummRpIO::fatal(const char *msg)
{
	cerr << "ERROR: " << msg << endl;
	exit(1);
}

void ScummRpIO::error(const char *msg)
{
	if (!ScummRpIO::_quiet)
		cerr << "ERROR: " << msg << endl;
}

void ScummRpIO::warning(const char *msg)
{
	if (!ScummRpIO::_quiet)
		cerr << "WARNING: " << msg << endl;
}

void ScummRpIO::setQuiet(bool q)
{
	ScummRpIO::_quiet = q;
}

void ScummRpIO::setInfoSlots(uint32 infoSlots)
{
	ScummRpIO::_infoSlots = infoSlots;
}
