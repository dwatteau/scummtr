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

#ifndef SCUMM_COMMON_IO_HPP
#define SCUMM_COMMON_IO_HPP

#include "common/types.hpp"
#include "common/toolbox.hpp"

enum
{
	INF_NULL = 0,
	INF_GLOBAL = 1 << 0,
	INF_DETAIL = 1 << 1,
	INF_LISTING = 1 << 2,
	INF_ALL = -1
};

class ScummIO
{
private:
	static bool _quiet;
	static uint32 _infoSlots;

public:
	static void info(const char *msg);
	static void info(int slots, const char *msg);
	static void warning(const char *msg);
	static void majorIssue(const char *msg);
	SCUMMTR_NORETURN_PREFIX static void fatal(const char *msg) __attribute__((noreturn));
	SCUMMTR_NORETURN_PREFIX static void crash(const char *msg) __attribute__((noreturn));
	static void setQuiet(bool q);
	static void setInfoSlots(uint32 infoSlots);
};

#endif
