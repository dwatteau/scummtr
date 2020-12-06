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

#ifndef SCUMMRP_SCUMMRP_HPP
#define SCUMMRP_SCUMMRP_HPP

#include "block.hpp"
#include "toc.hpp"
#include "backup.hpp"
#include "rptypes.hpp"

// TODO make it instanciable (though it's not needed yet)

class ScummRp
{
protected:
	struct Parameter
	{
		char c;
		char *value;
		int maxSize;
		int isPath;
	};
	enum
	{
		OPT_NULL        = 0,
		OPT_EXPORT      = 1 << 0,
		OPT_IMPORT      = 1 << 1,
		OPT_LIST        = 1 << 2,
		OPT_SINGLE_FILE = 1 << 3,
		OPT_GAME_FILES  = 1 << 4,
		OPT_RAMFILES    = 1 << 5,
		OPT_TAG         = 1 << 6,
		OPT_INVALID     = 1 << 7
	};
	struct TOCSet
	{
	public:
		GlobalRoomIndex roomToc;
		TableOfContent scrpToc;
		TableOfContent sounToc;
		TableOfContent costToc;
		TableOfContent charToc;
	public:
		TOCSet() : roomToc(),
				   scrpToc(TableOfContent::TOCT_SCRP),
				   sounToc(TableOfContent::TOCT_SOUN),
				   costToc(TableOfContent::TOCT_COST),
				   charToc(TableOfContent::TOCT_CHAR)
		{
		}
	};
	enum
	{
		ACT_NULL = 0,
		ACT_IMPORT,
		ACT_EXPORT,
		ACT_SAVE,
		ACT_LOAD
	};
protected:
	static const GameDefinition _gameDef[];
public:
	static const char *const NAME;
	static const char *const VERSION;
	static const char *const AUTHOR;
protected:
	static const int MAX_PARAMS = 16;
	static const ScummRp::Parameter _rpParameters[];
protected:
	static GameDefinition _game;
	static ScummRp::TOCSet _mainTocSet;
	static ScummRp::TOCSet _tmpTocSet;
	static ScummRp::TOCSet _updTocSet;
	static TableOfContent *const _mainTocs[];
	static TableOfContent *const _tmpTocs[];
	static TableOfContent *const _updTocs[];
	static int _fileOptions;
	static int _options;
	static BackUp _backupSystem;
	static int _infoSlots;
	static TableOfContent *const *_tocs;
	static char _paramGameId[16];
	static char _paramGameDir[512];
	static char _paramDumpingDir[512];
	static char _paramTag[5];
	static uint32 _filterTag;
public:
	static const GameDefinition &game;
	static TableOfContent *const *const &tocs;
protected:
	static void _queueParam(char *pendingParams, char c);
	static bool _readOption(const char *arg, char *pendingParams);
	static void _getOptions(int argc, const char **argv, const ScummRp::Parameter *params);
	static bool _invalidOptions();
	static void _usage();
	static void _listGames();
	static void _explore(TreeBlock &tree, int action);
	template <int A> static void _exploreIndex(TreeBlock &index);
	static TreeBlock *_newIndex(const char *path);
	static LFLFile *_newLFL(const char *path, int id);
	static void _processGameFilesV123();
	static void _processGameFilesV4567();
	static int _findGameDef(const char *shortName);
	static void _prepareTmpIndex();
	static void _mergeTmpIndex();
	static void _updateMainIndex();
public:
	static int main(int argc, const char **argv);
private:
	ScummRp();
};

#endif
