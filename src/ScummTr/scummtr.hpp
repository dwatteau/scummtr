/*
 * Copyright (c) 2003-2005 Thomas Combeleran
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

#ifndef SCUMMTR_SCUMMTR_HPP
#define SCUMMTR_SCUMMTR_HPP

#include "scummrp.hpp"
#include "block.hpp"
#include "text.hpp"
#include "rptypes.hpp"
#include "types.hpp"

#include <vector>

// TODO make it instanciable (though it's not needed yet)

class ScummTr : public ScummRp
{
public:
	enum { ACT_RSCNAMELIMITS = 0x100 };
	enum RscType { RSCT_ACTOR = 0, RSCT_OBJECT = 1, RSCT_VERB = 2 };
protected:
	static const GameDefinition _gameDef[];
public:
	static const char *const NAME;
	static const char *const VERSION;
	static const char *const AUTHOR;
protected:
	static const ScummRp::Parameter _trParameters[];
protected:
	static char _paramLanguage[3];
	static char _paramTextFile[512];
	static char _paramPaddedRsc[16];
	static int _textOptions;
	static int _paddedRsc;
	static struct RscNameLimits
	{
		int32 anyRsc[3];
		std::vector<int32> rsc[3];
	} _rscNameLimits;
	static bool _exportWithPadding;
	static bool _maxPadding;
protected:
	static bool _readOption(const char *arg, char *pendingParams);
	static void _getOptions(int argc, const char **argv, const ScummRp::Parameter *params);
	static bool _invalidOptions();
	static void _usage();
	static void _explore(TreeBlock &tree, int action, Text &text);
	static void _processGameFilesV123();
	static void _processGameFilesV4567();
	static Text::Charset _selectCharset();
public:
	static void setRscNameMaxLengh(ScummTr::RscType t, int32 id, int32 l);
	static int32 getRscNameMaxLengh(ScummTr::RscType t, int32 id);
	static int main(int argc, const char **argv);
private:
	ScummTr();
};

#endif
