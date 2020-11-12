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
 * Note: the last "official" binary was built on: 2003-10-12 13:54:34.
 * It seems that no functional change was made after that date.
 */

#include "scummrp.hpp"
#include "io.hpp"
#include "toolbox.hpp"

#include <iostream>
#include <iomanip>
#include <set>
#include <string.h>

using namespace std;

/*
 * ScummRp
 */

const char *const ScummRp::NAME    = "ScummRp";
const char *const ScummRp::VERSION = "0.2";
const char *const ScummRp::AUTHOR  = "Thomas Combeleran";

const GameDefinition &ScummRp::game = ScummRp::_game;
TableOfContent *const *const &ScummRp::tocs = ScummRp::_tocs;

const ScummRp::Parameter ScummRp::_rpParameters[] = {
	{ 'g', ScummRp::_paramGameId, sizeof ScummRp::_paramGameId, false },
	{ 'd', ScummRp::_paramDumpingDir, sizeof ScummRp::_paramDumpingDir, true },
	{ 'p', ScummRp::_paramGameDir, sizeof ScummRp::_paramGameDir, true },
	{ 't', ScummRp::_paramTag, sizeof ScummRp::_paramTag, false },
	{ 0, 0, 0 } };

char ScummRp::_paramGameId[16] = "";
char ScummRp::_paramGameDir[512] = ".";
char ScummRp::_paramDumpingDir[512] = ".";
char ScummRp::_paramTag[5] = "";

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable : 4127) // conditional expression is constant
#endif // _MSC_VER

// Uncomment to discover why gcc currently sucks (among other things)

// template <int A>
// void ScummRp::_explore(TreeBlock &tree)
// {
// 	TreeBlockPtr blockPtr;
// 	set<string> processedBlocks;
// 	string path, filename;

// 	tree.firstBlock();
// 	while ((blockPtr = tree.nextBlock()) != 0)
// 		if (blockPtr.is<LFLFPack>() || blockPtr.is<RoomBlock>())
// 			ScummRp::_explore<A>(*blockPtr);
// 		else
// 			switch (blockPtr->getTag())
// 			{
// 			case 'LECF':
// 			case 'LE':
// 				{
// 					ScummRp::_explore<A>(LECFPack(*blockPtr));
// 				}
// 				break;
// 			default:
// 				path = ScummRp::_paramDumpingDir;
// 				blockPtr->makePath(path, filename);
// 				if (path.size() > 0)
// 					xmkdir(path.c_str());
// 				path += filename;
// 				if (processedBlocks.find(filename) == processedBlocks.end())
// 				{
// 					if (A == ScummRp::ACT_IMPORT && File::exists(path.c_str()))
// 					{
// 						blockPtr->update(path.c_str());
// 						processedBlocks.insert(filename);
// 					}
// 					else if (A == ScummRp::ACT_EXPORT)
// 					{
// 						blockPtr->dump(path.c_str());
// 						processedBlocks.insert(filename);
// 					}
// 				}
// 				else
// 					ScummRpIO::warning(xsprintf("%s not unique. Only the first occurence was %s",
// 												filename.c_str(),
// 												A == ScummRp::ACT_IMPORT ? "replaced" : "dumped"));
// 			}
// }

void ScummRp::_explore(TreeBlock &tree, int action)
{
	TreeBlockPtr blockPtr;
	set<string> processedBlocks;
	string path, filename;

	tree.firstBlock();
	while ((blockPtr = tree.nextBlock()) != 0)
	{
		path = ScummRp::_paramDumpingDir;
		blockPtr->makePath(path, filename);
		if (blockPtr.is<LFLFPack>() || blockPtr.is<RoomBlock>())
		{
			if (processedBlocks.find(filename) == processedBlocks.end())
				ScummRp::_explore(*blockPtr, action);
			else
				ScummRpIO::warning(xsprintf("%s not unique. Only the first occurence was explored.",
											filename.c_str()));
		}
		else
			switch (blockPtr->getTag())
			{
			case MKTAG4('L','E','C','F'):
			case MKTAG2('L','E'):
				if (processedBlocks.find(filename) == processedBlocks.end())
				{
					TreeBlockPtr lecf;

					lecf = new LECFPack(*blockPtr);
					ScummRp::_explore(*lecf, action);
				}
				else
					ScummRpIO::warning(xsprintf("%s not unique. Only the first occurence was explored.",
												filename.c_str()));
				break;
			default:
				if (ScummRp::_filterTag && blockPtr->getTag() != ScummRp::_filterTag)
					break;
				if (processedBlocks.find(filename) == processedBlocks.end())
				{
					if (action == ScummRp::ACT_IMPORT)
					{
						path += filename;
						if (File::exists(path.c_str()))
						{
							blockPtr->update(path.c_str());
							processedBlocks.insert(filename);
						}
					}
					else if (action == ScummRp::ACT_EXPORT)
					{
						if (path.size() > 0)
							xmkdir(path.c_str());
						path += filename;
						blockPtr->dump(path.c_str());
						processedBlocks.insert(filename);
					}
				}
				else
					ScummRpIO::warning(xsprintf("%s not unique. Only the first occurence was %s.", filename.c_str(),
												action == ScummRp::ACT_IMPORT ? "replaced" : "dumped"));
			}
	}
}

template <int A> void ScummRp::_exploreIndex(TreeBlock &index)
{
	GlobalTocBlockPtr tocBlockPtr;

	index.firstBlock();
	while ((tocBlockPtr = index.nextBlock()) != 0)
		switch (tocBlockPtr->getTag())
		{
		case MKTAG4('D','R','O','O'):
		case MKTAG2('0','R'):
		case MKTAG4('0','R','v','1'):
		case MKTAG4('0','R','v','2'):
		case MKTAG4('0','R','v','3'):
			if (A == ScummRp::ACT_SAVE)
				tocBlockPtr->importToc(ScummRp::_mainTocSet.roomToc);
			else if (A == ScummRp::ACT_LOAD)
				tocBlockPtr->exportToc(ScummRp::_mainTocSet.roomToc);
			break;
		case MKTAG4('D','S','C','R'):
		case MKTAG2('0','S'):
		case MKTAG4('0','S','v','1'):
		case MKTAG4('0','S','v','2'):
		case MKTAG4('0','S','v','3'):
			if (A == ScummRp::ACT_SAVE)
				tocBlockPtr->importToc(ScummRp::_mainTocSet.scrpToc);
			else if (A == ScummRp::ACT_LOAD)
				tocBlockPtr->exportToc(ScummRp::_mainTocSet.scrpToc);
			break;
		case MKTAG4('D','S','O','U'):
		case MKTAG2('0','N'):
		case MKTAG4('0','N','v','1'):
		case MKTAG4('0','N','v','2'):
		case MKTAG4('0','N','v','3'):
			if (A == ScummRp::ACT_SAVE)
				tocBlockPtr->importToc(ScummRp::_mainTocSet.sounToc);
			else if (A == ScummRp::ACT_LOAD)
				tocBlockPtr->exportToc(ScummRp::_mainTocSet.sounToc);
			break;
		case MKTAG4('D','C','O','S'):
		case MKTAG2('0','C'):
		case MKTAG4('0','C','v','1'):
		case MKTAG4('0','C','v','2'):
		case MKTAG4('0','C','v','3'):
			if (A == ScummRp::ACT_SAVE)
				tocBlockPtr->importToc(ScummRp::_mainTocSet.costToc);
			else if (A == ScummRp::ACT_LOAD)
				tocBlockPtr->exportToc(ScummRp::_mainTocSet.costToc);
			break;
		case MKTAG4('D','C','H','R'):
			if (A == ScummRp::ACT_SAVE)
				tocBlockPtr->importToc(ScummRp::_mainTocSet.charToc);
			else if (A == ScummRp::ACT_LOAD)
				tocBlockPtr->exportToc(ScummRp::_mainTocSet.charToc);
			break;
		}
}

#ifdef _MSC_VER
# pragma warning(pop)
#endif // _MSC_VER

int ScummRp::_findGameDef(const char *shortName)
{
	int i;

	for (i = 0; ScummRp::_gameDef[i].shortName != 0; ++i)
		if (strcmp(ScummRp::_gameDef[i].shortName, shortName) == 0)
			return i;
	return -1;
}

TreeBlock *ScummRp::_newIndex(const char *path)
{
	if (ScummRp::_game.version <= 1)
		return new OldIndexFileV1(path, ScummRp::_fileOptions,
								  ScummRp::_backupSystem, -1, ScummRp::_game.indexXorKey);
	else if (ScummRp::_game.version <= 2)
		return new OldIndexFileV2(path, ScummRp::_fileOptions,
								  ScummRp::_backupSystem, -1, ScummRp::_game.indexXorKey);
	else if (ScummRp::_game.version <= 3
			 && ScummRp::_game.blockFormat ==  BFMT_SIZEONLY)
		return new OldIndexFileV3(path, ScummRp::_fileOptions,
								  ScummRp::_backupSystem, -1, ScummRp::_game.indexXorKey);
	else
		return new IndexFile(path, ScummRp::_fileOptions,
							 ScummRp::_backupSystem, -1, ScummRp::_game.indexXorKey);
}

LFLFile *ScummRp::_newLFL(const char *path, int id)
{
	if (ScummRp::_game.version <= 1)
		return new OldLFLFileV1(path, ScummRp::_fileOptions, ScummRp::_backupSystem,
								id, ScummRp::_game.dataXorKey);
	else if (ScummRp::_game.version <= 2)
		return new OldLFLFileV2(path, ScummRp::_fileOptions, ScummRp::_backupSystem,
								id, ScummRp::_game.dataXorKey);
	else if (ScummRp::_game.version <= 3
			 && ScummRp::_game.blockFormat ==  BFMT_SIZEONLY)
		return new OldLFLFileV3(path, ScummRp::_fileOptions, ScummRp::_backupSystem,
								id, ScummRp::_game.dataXorKey);
	else
		return new LFLFile(path, ScummRp::_fileOptions, ScummRp::_backupSystem,
						   id, ScummRp::_game.dataXorKey);
}

void ScummRp::_prepareTmpIndex()
{
	int i;

	for (i = 0; ScummRp::_mainTocs[i] != 0; ++i)
	{
		*ScummRp::_updTocs[i] = *ScummRp::_mainTocs[i];
		*ScummRp::_tmpTocs[i] = *ScummRp::_mainTocs[i];
	}
	ScummRp::_tocs = ScummRp::_tmpTocs;
}

void ScummRp::_mergeTmpIndex()
{
	int i;

	for (i = 0; ScummRp::_mainTocs[i] != 0; ++i)
	{
		ScummRp::_updTocs[i]->merge(*ScummRp::_tmpTocs[i]);
		*ScummRp::_tmpTocs[i] = *ScummRp::_mainTocs[i];
	}
}

void ScummRp::_updateMainIndex()
{
	int i;

	for (i = 0; ScummRp::_mainTocs[i] != 0; ++i)
		ScummRp::_mainTocs[i]->merge(*ScummRp::_updTocs[i]);
	ScummRp::_tocs = ScummRp::_mainTocs;
}

void ScummRp::_processGameFilesV123()
{
	int i;
	char dataFileName[32];
	string indexPath(ScummRp::_paramGameDir);
	TreeBlockPtr index;

	indexPath += '/';
	indexPath += ScummRp::_game.indexFileName;
	index = ScummRp::_newIndex(indexPath.c_str());
	ScummRp::_exploreIndex<ScummRp::ACT_LOAD>(*index);
	for (i = 1; i < 98; ++i)
	{
		string dataPath(ScummRp::_paramGameDir);
		TreeBlockPtr room;

		sprintf(dataFileName, ScummRp::_game.dataFileName, i);
		dataPath += '/';
		dataPath += dataFileName;
		if (File::exists(dataPath.c_str()))
		{
			room = ScummRp::_newLFL(dataPath.c_str(), i);
			if (ScummRp::_options & ScummRp::OPT_IMPORT)
				ScummRp::_explore(*room, ScummRp::ACT_IMPORT);
			else
				ScummRp::_explore(*room, ScummRp::ACT_EXPORT);
		}
	}
	if (ScummRp::_options & ScummRp::OPT_IMPORT)
	{
		ScummRpIO::setQuiet(true);
		ScummRp::_exploreIndex<ScummRp::ACT_SAVE>(*index);
		ScummRpIO::setQuiet(false);
	}
}

void ScummRp::_processGameFilesV4567()
{
	int i;
	char dataFileName[32];
	string indexPath(ScummRp::_paramGameDir);
	TreeBlockPtr index;
	int numberOfDisks;

	indexPath += '/';
	indexPath += ScummRp::_game.indexFileName;
	index = ScummRp::_newIndex(indexPath.c_str());
	ScummRp::_exploreIndex<ScummRp::ACT_LOAD>(*index);
	numberOfDisks = ScummRp::_mainTocSet.roomToc.numberOfDisks();
	ScummRp::_prepareTmpIndex();
	for (i = 1; i < numberOfDisks; ++i)
	{
		string dataPath(ScummRp::_paramGameDir);
		TreeBlockPtr disk;

		sprintf(dataFileName, ScummRp::_game.dataFileName, i);
		dataPath += '/';
		dataPath += dataFileName;
		disk = new BlocksFile(dataPath.c_str(), ScummRp::_fileOptions,
							  ScummRp::_backupSystem, i, MKTAG4('D','I','S','K'), ScummRp::_game.dataXorKey);
		if (ScummRp::_options & ScummRp::OPT_IMPORT)
			ScummRp::_explore(*disk, ScummRp::ACT_IMPORT);
		else
			ScummRp::_explore(*disk, ScummRp::ACT_EXPORT);
		ScummRp::_mergeTmpIndex();
	}
	ScummRp::_updateMainIndex();
	if (ScummRp::_options & ScummRp::OPT_IMPORT)
	{
		ScummRpIO::setQuiet(true);
		ScummRp::_exploreIndex<ScummRp::ACT_SAVE>(*index);
		ScummRpIO::setQuiet(false);
	}
}

int ScummRp::main(int argc, const char **argv)
{
	int r, g, i;

	r = 0;
	ScummRp::_getOptions(argc, argv, ScummRp::_rpParameters);
	ScummRpIO::setInfoSlots(ScummRp::_infoSlots);
	ScummRpIO::info(INF_GLOBAL, xsprintf("%s %s by %s", ScummRp::NAME, ScummRp::VERSION, ScummRp::AUTHOR));
	ScummRpIO::info(INF_GLOBAL, "");
	if (ScummRp::_options & ScummRp::OPT_LIST)
	{
		ScummRp::_listGames();
		return 0;
	}
	if (ScummRp::_invalidOptions())
	{
		ScummRp::_usage();
		return 0;
	}
	g = ScummRp::_findGameDef(ScummRp::_paramGameId);
	if (g == -1)
	{
		ScummRp::_listGames();
		return 0;
	}
	ScummRp::_filterTag = 0;
	for (i = 0; ScummRp::_paramTag[i] != 0; ++i)
		ScummRp::_filterTag = (ScummRp::_filterTag << 8) | ScummRp::_paramTag[i];
	ScummRp::_game = ScummRp::_gameDef[g];
	if (ScummRp::_options & ScummRp::OPT_IMPORT)
		ScummRp::_fileOptions |= BlocksFile::BFOPT_BACKUP;
	else
		ScummRp::_fileOptions |= BlocksFile::BFOPT_READONLY;
	if (ScummRp::_game.version <= 4)
		ScummRp::_fileOptions &= ~BlocksFile::BFOPT_SEQFILE;
	if (ScummRp::_game.version >= 7)
		ScummRp::_fileOptions |= BlocksFile::BFOPT_SEQFILE;
	if (ScummRp::_game.version < 4)
		ScummRp::_processGameFilesV123();
	else
		ScummRp::_processGameFilesV4567();
	ScummRp::_backupSystem.applyChanges();
	return 0;
}

void ScummRp::_queueParam(char *pendingParams, char c)
{
	int j;

	for (j = 0; pendingParams[j] != 0; ++j)
		if (pendingParams[j] == c)
            return;
	if (j >= ScummRp::MAX_PARAMS)
		throw logic_error("ScummRp::_queueParam: Too many parameters");
	pendingParams[j] = c;
	pendingParams[j + 1] = 0;
}

bool ScummRp::_readOption(const char *arg, char *pendingParams)
{
	int	i;
	char c;

	i = 0;
	if (arg[i++] == '-')
		while ((c = arg[i++]) != 0)
			switch (c)
			{
			case '-':
				return false;
			case 'o':
				ScummRp::_options |= ScummRp::OPT_EXPORT;
				break;
			case 'i':
				ScummRp::_options |= ScummRp::OPT_IMPORT;
				break;
			case 'L':
				ScummRp::_options |= ScummRp::OPT_LIST;
				return false;
			case 's':
				ScummRp::_fileOptions = BlocksFile::BFOPT_NULL;
				break;
			case 'O':
				ScummRp::_fileOptions &= ~BlocksFile::BFOPT_AUTO;
				ScummRp::_fileOptions |= BlocksFile::BFOPT_SEQFILE;
				break;
			case 'm':
				ScummRp::_fileOptions &= ~BlocksFile::BFOPT_AUTO;
				ScummRp::_fileOptions |= BlocksFile::BFOPT_RAM;
				break;
			case 'q':
				ScummRp::_infoSlots = INF_NULL;
				break;
			case 'v':
				ScummRp::_infoSlots |= INF_DETAIL;
				break;
			case 'V':
				ScummRp::_infoSlots |= INF_LISTING;
				break;
			case 'g':
				ScummRp::_options |= ScummRp::OPT_GAME_FILES;
				ScummRp::_queueParam(pendingParams, c);
				break;
			case 't':
			case 'd':
			case 'p':
				ScummRp::_queueParam(pendingParams, c);
				break;
			}
	return true;
}

void ScummRp::_usage()
{
	cout << "options:" << endl;
	cout << endl;
	cout << " -g gameid  " << "Select a game" << endl;
	cout << " -i         " << "Import blocks into the game files (Input)" << endl;
	cout << " -o         " << "Export blocks from the game files (Output)" << endl;
	cout << " -d path    " << "Path to dumping directory" << endl;
	cout << " -p path    " << "Path to the game directory" << endl;
	cout << " -t tag     " << "Only export/import blocks with this tag" << endl;
// 	cout << " -m         " << "Work in memory (whole game files are loaded in RAM)" << endl;
// 	cout << " -O         " << "Optimize for sequential access (with -i)" << endl;
// 	cout << " -s         " << "Slow mode (disable automatic -m or -O)" << endl;
	cout << " -L         " << "List supported games" << endl;
	cout << " -v         " << "Verbose" << endl;
	cout << " -V         " << "More verbose (lists blocks)" << endl;
	cout << " -q         " << "Quiet" << endl;
	cout << endl;
	cout << "Example:" << endl;
	cout << "ScummRp -gp monkey2 ./mi2 -id ./dumps/mi2" << endl;
}

void ScummRp::_getOptions(int argc, const char **argv, const ScummRp::Parameter *params)
{
	char pendingParams[MAX_PARAMS + 1];
	int i, j, k;

	pendingParams[0] = 0;
	for (i = 1; i < argc && argv[i] && ScummRp::_readOption(argv[i], pendingParams); ++i)
		for (j = 0; pendingParams[j] != 0; pendingParams[j++] = 0)
			for (k = 0; params[k].c != 0; ++k)
				if (params[k].c == pendingParams[j])
				{
					++i;
					if (i < argc && argv[i])
					{
						strncpy(params[k].value, argv[i], params[k].maxSize - 1);
						params[k].value[params[k].maxSize - 1] = 0;
#ifdef _WIN32
						if (params[k].isPath)
							for (char *p = strchr(params[k].value, '\\'); p != 0;
								 p = strchr(params[k].value, '\\'))
								*p++ = '/';
#endif // _WIN32
					}
					break;
				}
}

bool ScummRp::_invalidOptions()
{
	static const uint32 exclusive[] = { ScummRp::OPT_IMPORT | ScummRp::OPT_EXPORT,
										0 };
	static const uint32 mandatory[] = { ScummRp::OPT_IMPORT | ScummRp::OPT_EXPORT,
										ScummRp::OPT_GAME_FILES,
										0 };
	int i;

	for (i = 0; mandatory[i] != 0; ++i)
		if (!(ScummRp::_options & mandatory[i]))
			return true;
	for (i = 0; exclusive[i] != 0; ++i)
		if ((ScummRp::_options & exclusive[i]) == exclusive[i])
			return true;
	return false;
}

void ScummRp::_listGames()
{
	int i;
	size_t l1, l2;

	cout << "supported games:" << endl;
	cout << endl;
	cout << "id" << setw(12) << "| " << setw(0) << "description"
		 << setw(42) << "| " << setw(0) << "file" << endl;
	cout << "------------|-------------------------------------"
		"---------------|-------------" << endl;
	for (i = 0; ScummRp::_gameDef[i].shortName != 0; ++i)
	{
		l1 = strlen(ScummRp::_gameDef[i].shortName);
		l2 = strlen(ScummRp::_gameDef[i].name);
		cout << ScummRp::_gameDef[i].shortName
			 << setw(14 - (streamsize)l1) << "| "
			 << setw(0) << ScummRp::_gameDef[i].name
			 << setw(53 - (streamsize)l2) << "| "
			 << setw(0) << ScummRp::_gameDef[i].indexFileName
			 << endl;
	}
}

/*
 * Game definitions
 */

const GameDefinition ScummRp::_gameDef[] = {
	// Old LFL format
	{ "maniacv1", "Maniac Mansion", "00.LFL", "%.2u.LFL", GID_MANIAC,
	  1, 0xFF, 0xFF, GTCFMT_8SEP16, BFMT_SIZEONLY, 4, GF_OLD_BUNDLE },
// 	{ "maniacnes", "Maniac Mansion (NES)", "00.LFL", "%.2u.LFL", GID_MANIAC,
// 	  1, 0xFF, 0xFF, GTCFMT_8SEP16, BFMT_SIZEONLY, 4, GF_OLD_BUNDLE | GF_NES },
	{ "zakv1", "Zak McKracken and the Alien Mindbenders", "00.LFL", "%.2u.LFL", GID_ZAK,
	  1, 0xFF, 0xFF, GTCFMT_8SEP16, BFMT_SIZEONLY, 4, GF_OLD_BUNDLE },
	{ "maniacv2", "Maniac Mansion (enhanced)", "00.LFL", "%.2u.LFL", GID_MANIAC,
	  2, 0xFF, 0xFF, GTCFMT_8SEP16, BFMT_SIZEONLY, 4, GF_OLD_BUNDLE },
	{ "zakv2", "Zak McKracken and the Alien Mindbenders (enhanced)", "00.LFL", "%.2u.LFL", GID_ZAK,
	  2, 0xFF, 0xFF, GTCFMT_8SEP16, BFMT_SIZEONLY, 4, GF_OLD_BUNDLE },
	{ "indy3", "Indiana Jones and the Last Crusade", "00.LFL", "%.2u.LFL", GID_INDY3,
	  3, 0xFF, 0xFF, GTCFMT_8SEP16, BFMT_SIZEONLY, 4, GF_OLD_BUNDLE },
	{ "indy3mac", "Indiana Jones and the Last Crusade (Macintosh)", "00.LFL", "%.2u.LFL", GID_INDY3,
	  3, 0xFF, 0xFF, GTCFMT_8SEP16, BFMT_SIZEONLY, 4, GF_OLD_BUNDLE | GF_MACINTOSH },
	{ "loom", "Loom", "00.LFL", "%.2u.LFL", GID_LOOM,
	  3, 0xFF, 0xFF, GTCFMT_8SEP16, BFMT_SIZEONLY, 4, GF_OLD_BUNDLE },
	// FM Towns LFL format
	{ "indy3towns", "Indiana Jones and the Last Crusade (FM Towns)", "00.LFL", "%.2u.LFL", GID_INDY3,
	  3, 0x00, 0x00, GTCFMT_16MIX32, BFMT_SHORTTAG, 6, GF_FMTOWNS },
	{ "indy3vga", "Indiana Jones and the Last Crusade (256 colours)", "00.LFL", "%.2u.LFL", GID_INDY3,
	  3, 0x00, 0x00, GTCFMT_16MIX32, BFMT_SHORTTAG, 6, GF_NULL },
	{ "zaktowns", "Zak McKracken and the Alien Mindbenders (FM Towns)", "00.LFL", "%.2u.LFL", GID_ZAK,
	  3, 0x00, 0x00, GTCFMT_16MIX32, BFMT_SHORTTAG, 6, GF_FMTOWNS },
	{ "loomtowns", "Loom (FM Towns)", "00.LFL", "%.2u.LFL", GID_LOOM,
	  3, 0x00, 0x00, GTCFMT_16MIX32, BFMT_SHORTTAG, 6, GF_FMTOWNS },
	// Old LEC format
	{ "monkey", "Monkey Island 1", "000.LFL", "DISK%.2u.LEC", GID_MONKEY,
	  4, 0x00, 0x69, GTCFMT_16MIX32, BFMT_SHORTTAG, 6, GF_NULL },
	{ "loomcd", "Loom (Talkie)", "000.LFL", "DISK%.2u.LEC", GID_LOOM,
	  4, 0x00, 0x69, GTCFMT_16MIX32, BFMT_SHORTTAG, 6, GF_NULL },
	// New LEC format
	{ "monkeycd", "Monkey Island 1 (CD)", "MONKEY.000", "MONKEY.%.3u", GID_MONKEY,
	  5, 0x69, 0x69, GTCFMT_16SEP32, BFMT_LONGTAG, 8, GF_NULL },
	{ "monkeycdalt", "Monkey Island 1 (CD, MONKEY1.000)", "MONKEY1.000", "MONKEY1.%.3u", GID_MONKEY,
	  5, 0x69, 0x69, GTCFMT_16SEP32, BFMT_LONGTAG, 8, GF_NULL },
	{ "monkeysega", "Monkey Island 1 (SegaCD)", "GAME.000", "GAME.%.3u", GID_MONKEY,
	  5, 0x69, 0x69, GTCFMT_16SEP32, BFMT_LONGTAG, 8, GF_NULL },
	{ "monkey2", "Monkey Island 2: LeChuck's revenge", "MONKEY2.000", "MONKEY2.%.3u", GID_MONKEY2,
	  5, 0x69, 0x69, GTCFMT_16SEP32, BFMT_LONGTAG, 8, GF_NULL },
	{ "atlantis", "Indiana Jones and the Fate of Atlantis", "ATLANTIS.000", "ATLANTIS.%.3u", GID_INDY4,
	  5, 0x69, 0x69, GTCFMT_16SEP32, BFMT_LONGTAG, 8, GF_NULL },
	{ "tentacle", "Day Of The Tentacle", "TENTACLE.000", "TENTACLE.%.3u", GID_TENTACLE,
	  6, 0x69, 0x69, GTCFMT_16SEP32, BFMT_LONGTAG, 8, GF_NULL },
	{ "samnmax", "Sam & Max", "SAMNMAX.000", "SAMNMAX.%.3u", GID_SAMNMAX,
	  6, 0x69, 0x69, GTCFMT_16SEP32, BFMT_LONGTAG, 8, GF_NULL },
	{ "samnmaxalt", "Sam & Max (SAMNMAX.SM0)", "SAMNMAX.SM0", "SAMNMAX.SM%u", GID_SAMNMAX,
	  6, 0x69, 0x69, GTCFMT_16SEP32, BFMT_LONGTAG, 8, GF_NULL },
	{ "ft", "Full Throttle", "FT.LA0", "FT.LA%u", GID_FT,
	  7, 0x00, 0x00, GTCFMT_16SEP32, BFMT_LONGTAG, 8, GF_NULL },
	{ "dig", "The Dig", "DIG.LA0", "DIG.LA%u", GID_DIG,
	  7, 0x00, 0x00, GTCFMT_16SEP32, BFMT_LONGTAG, 8, GF_NULL },
// 	{ "comi", "The Curse of Monkey Island", "COMI.LA0", "COMI.LA%u", GID_CMI,
// 	  8, 0x00, 0x00, FNFMT_GAME_LA0, GTCFMT_16SEP32, BFMT_LONGTAG, 8, GF_NULL },
	//
	{ 0, 0, 0, 0, GID_NULL, 0, 0, 0, GTCFMT_NULL, BFMT_NULL, 0, GF_NULL } };

/*
 *
 */

GameDefinition ScummRp::_game = { 0, 0, 0, 0, GID_NULL, 0, 0, 0, GTCFMT_NULL, BFMT_NULL, 0, GF_NULL };

int ScummRp::_fileOptions = BlocksFile::BFOPT_AUTO;
int ScummRp::_options = ScummRp::OPT_NULL;
BackUp ScummRp::_backupSystem;
int ScummRp::_infoSlots = INF_GLOBAL;
TableOfContent *const *ScummRp::_tocs = ScummRp::_mainTocs;
uint32 ScummRp::_filterTag = 0;

ScummRp::TOCSet ScummRp::_mainTocSet;
ScummRp::TOCSet ScummRp::_tmpTocSet;
ScummRp::TOCSet ScummRp::_updTocSet;

TableOfContent *const ScummRp::_mainTocs[] = {
	&ScummRp::_mainTocSet.roomToc, &ScummRp::_mainTocSet.scrpToc,
	&ScummRp::_mainTocSet.sounToc, &ScummRp::_mainTocSet.costToc,
	&ScummRp::_mainTocSet.charToc,
	0 };
TableOfContent *const ScummRp::_tmpTocs[] = {
	&ScummRp::_tmpTocSet.roomToc, &ScummRp::_tmpTocSet.scrpToc,
	&ScummRp::_tmpTocSet.sounToc, &ScummRp::_tmpTocSet.costToc,
	&ScummRp::_tmpTocSet.charToc,
	0 };
TableOfContent *const ScummRp::_updTocs[] = {
	&ScummRp::_updTocSet.roomToc, &ScummRp::_updTocSet.scrpToc,
	&ScummRp::_updTocSet.sounToc, &ScummRp::_updTocSet.costToc,
	&ScummRp::_updTocSet.charToc,
	0 };