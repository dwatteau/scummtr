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

#include <cstring>

#include <iomanip>
#include <iostream>
#include <set>

/*
 * ScummRp
 */

const char *const ScummRp::NAME = "ScummRp";
const char *const ScummRp::VERSION = "0.2";
const char *const ScummRp::AUTHOR = "Thomas Combeleran";

const GameDefinition &ScummRp::game = ScummRp::_game;
TableOfContent *const *const &ScummRp::tocs = ScummRp::_tocs;

const ScummRp::Parameter ScummRp::_rpParameters[] =
{
	{ 'g', ScummRp::_paramGameId, sizeof ScummRp::_paramGameId, false },
	{ 'd', ScummRp::_paramDumpingDir, sizeof ScummRp::_paramDumpingDir, true },
	{ 'p', ScummRp::_paramGameDir, sizeof ScummRp::_paramGameDir, true },
	{ 't', ScummRp::_paramTag, sizeof ScummRp::_paramTag, false },
	{ '\0', nullptr, 0, false }
};

char ScummRp::_paramGameId[16] = "";
char ScummRp::_paramGameDir[512] = ".";
char ScummRp::_paramDumpingDir[512] = "DUMP";
char ScummRp::_paramTag[5] = "";

// template <int A>
// void ScummRp::_explore(TreeBlock &tree)
// {
// 	TreeBlockPtr blockPtr;
// 	std::set<string> processedBlocks;
// 	std::string path, filename;

// 	tree.firstBlock();
// 	while ((blockPtr = tree.nextBlock()) != nullptr)
// 		if (blockPtr.is<LFLFPack>() || blockPtr.is<RoomBlock>())
// 			ScummRp::_explore<A>(*blockPtr);
// 		else
// 			switch (blockPtr->getTag())
// 			{
// 			case MKTAG4('L','E','C','F'):
// 			case MKTAG2('L','E'):
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
	std::set<std::string> processedBlocks;
	std::string path, filename;

	tree.firstBlock();
	while ((blockPtr = tree.nextBlock()) != nullptr)
	{
		path = ScummRp::_paramDumpingDir;
		blockPtr->makePath(path, filename);

		if (blockPtr.is<LFLFPack>() || blockPtr.is<RoomBlock>())
		{
			if (processedBlocks.find(filename) == processedBlocks.end())
				ScummRp::_explore(*blockPtr, action);
			else
				ScummRpIO::warning(xsprintf("%s not unique. Only the first occurence was explored.", filename.c_str()));
		}
		else
		{
			switch (blockPtr->getTag())
			{
			case MKTAG4('L','E','C','F'):
			case MKTAG2('L','E'):
				if (processedBlocks.find(filename) != processedBlocks.end())
					ScummRpIO::warning(xsprintf("%s not unique. Only the first occurence was explored.", filename.c_str()));

				{
					TreeBlockPtr lecf;

					lecf = new LECFPack(*blockPtr);
					ScummRp::_explore(*lecf, action);
				}
				break;
			default:
				if (ScummRp::_filterTag && blockPtr->getTag() != ScummRp::_filterTag)
					break;

				if (processedBlocks.find(filename) != processedBlocks.end())
					ScummRpIO::warning(xsprintf("%s not unique. Only the first occurence was %s.", filename.c_str(), action == ScummRp::ACT_IMPORT ? "replaced" : "dumped"));

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
		}
	}
}

template <int A>
void ScummRp::_exploreIndex(TreeBlock &index)
{
	GlobalTocBlockPtr tocBlockPtr;

	index.firstBlock();
	while ((tocBlockPtr = index.nextBlock()) != nullptr)
	{
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
}

int ScummRp::_findGameDef(const char *shortName)
{
	for (int i = 0; ScummRp::_gameDef[i].shortName != nullptr; ++i)
		if (strcmp(ScummRp::_gameDef[i].shortName, shortName) == 0)
			return i;

	return -1;
}

TreeBlock *ScummRp::_newIndex(const char *path)
{
	if (ScummRp::_game.version <= 1)
		return new OldIndexFileV1(path, ScummRp::_fileOptions, ScummRp::_backupSystem, -1, ScummRp::_game.indexXorKey);
	else if (ScummRp::_game.version <= 2)
		return new OldIndexFileV2(path, ScummRp::_fileOptions, ScummRp::_backupSystem, -1, ScummRp::_game.indexXorKey);
	else if (ScummRp::_game.version <= 3 && ScummRp::_game.blockFormat == BFMT_SIZEONLY)
		return new OldIndexFileV3(path, ScummRp::_fileOptions, ScummRp::_backupSystem, -1, ScummRp::_game.indexXorKey);
	else
		return new IndexFile(path, ScummRp::_fileOptions, ScummRp::_backupSystem, -1, ScummRp::_game.indexXorKey);
}

LFLFile *ScummRp::_newLFL(const char *path, int id)
{
	if (ScummRp::_game.version <= 1)
		return new OldLFLFileV1(path, ScummRp::_fileOptions, ScummRp::_backupSystem, id, ScummRp::_game.dataXorKey);
	else if (ScummRp::_game.version <= 2)
		return new OldLFLFileV2(path, ScummRp::_fileOptions, ScummRp::_backupSystem, id, ScummRp::_game.dataXorKey);
	else if (ScummRp::_game.version <= 3 && ScummRp::_game.blockFormat == BFMT_SIZEONLY)
		return new OldLFLFileV3(path, ScummRp::_fileOptions, ScummRp::_backupSystem, id, ScummRp::_game.dataXorKey);
	else
		return new LFLFile(path, ScummRp::_fileOptions, ScummRp::_backupSystem, id, ScummRp::_game.dataXorKey);
}

void ScummRp::_prepareTmpIndex()
{
	for (int i = 0; ScummRp::_mainTocs[i] != nullptr; ++i)
	{
		*ScummRp::_updTocs[i] = *ScummRp::_mainTocs[i];
		*ScummRp::_tmpTocs[i] = *ScummRp::_mainTocs[i];
	}
	ScummRp::_tocs = ScummRp::_tmpTocs;
}

void ScummRp::_mergeTmpIndex()
{
	for (int i = 0; ScummRp::_mainTocs[i] != nullptr; ++i)
	{
		ScummRp::_updTocs[i]->merge(*ScummRp::_tmpTocs[i]);
		*ScummRp::_tmpTocs[i] = *ScummRp::_mainTocs[i];
	}
}

void ScummRp::_updateMainIndex()
{
	for (int i = 0; ScummRp::_mainTocs[i] != nullptr; ++i)
		ScummRp::_mainTocs[i]->merge(*ScummRp::_updTocs[i]);

	ScummRp::_tocs = ScummRp::_mainTocs;
}

void ScummRp::_processGameFilesV123()
{
	char dataFileName[32];
	std::string indexPath(ScummRp::_paramGameDir);
	TreeBlockPtr index;

	indexPath += '/';
	indexPath += ScummRp::_game.indexFileName;

	index = ScummRp::_newIndex(indexPath.c_str());
	ScummRp::_exploreIndex<ScummRp::ACT_LOAD>(*index);

	for (int i = 1; i < 98; ++i)
	{
		std::string dataPath(ScummRp::_paramGameDir);
		TreeBlockPtr room;

		snprintf(dataFileName, sizeof(dataFileName), ScummRp::_game.dataFileName, i); // ignore -Wformat-security here, ScummRp::_game.dataFileName is internal and safe
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
	char dataFileName[32];
	std::string indexPath(ScummRp::_paramGameDir);
	TreeBlockPtr index;
	int numberOfDisks;

	indexPath += '/';
	indexPath += ScummRp::_game.indexFileName;

	index = ScummRp::_newIndex(indexPath.c_str());
	ScummRp::_exploreIndex<ScummRp::ACT_LOAD>(*index);

	numberOfDisks = ScummRp::_mainTocSet.roomToc.numberOfDisks();
	ScummRp::_prepareTmpIndex();
	for (int i = 1; i < numberOfDisks; ++i)
	{
		std::string dataPath(ScummRp::_paramGameDir);
		TreeBlockPtr disk;

		snprintf(dataFileName, sizeof(dataFileName), ScummRp::_game.dataFileName, i); // ignore -Wformat-security here, ScummRp::_game.dataFileName is internal and safe
		dataPath += '/';
		dataPath += dataFileName;
		disk = new BlocksFile(dataPath.c_str(), ScummRp::_fileOptions, ScummRp::_backupSystem, i, MKTAG4('D','I','S','K'), ScummRp::_game.dataXorKey);
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
	int g;

	ScummRp::_getOptions(argc, argv, ScummRp::_rpParameters);
	ScummRpIO::setInfoSlots(ScummRp::_infoSlots);
	ScummRpIO::info(INF_GLOBAL, xsprintf("%s %s (build %s) by %s", ScummRp::NAME, ScummRp::VERSION, SCUMMTR_BUILD_DATE, ScummRp::AUTHOR));
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
	for (int i = 0; ScummRp::_paramTag[i] != '\0'; ++i)
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

	for (j = 0; pendingParams[j] != '\0'; ++j)
		if (pendingParams[j] == c)
			return;

	if (j >= ScummRp::MAX_PARAMS)
		throw std::logic_error("ScummRp::_queueParam: Too many parameters");

	pendingParams[j] = c;
	pendingParams[j + 1] = '\0';
}

bool ScummRp::_readOption(const char *arg, char *pendingParams)
{
	int i;
	char c;

	i = 0;
	if (arg[i++] == '-')
	{
		while ((c = arg[i++]) != '\0')
		{
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
		}
	}

	return true;
}

void ScummRp::_usage()
{
	std::cout << "options:" << std::endl;
	std::cout << std::endl;
	std::cout << " -g gameid  " << "Select a game" << std::endl;
	std::cout << " -i         " << "Import blocks into the game files (Input)" << std::endl;
	std::cout << " -o         " << "Export blocks from the game files (Output)" << std::endl;
	std::cout << " -d path    " << "Path to dumping directory" << std::endl;
	std::cout << " -p path    " << "Path to the game directory" << std::endl;
	std::cout << " -t tag     " << "Only export/import blocks with this tag" << std::endl;
// 	std::cout << " -m         " << "Work in memory (whole game files are loaded in RAM)" << std::endl;
// 	std::cout << " -O         " << "Optimize for sequential access (with -i)" << std::endl;
// 	std::cout << " -s         " << "Slow mode (disable automatic -m or -O)" << std::endl;
	std::cout << " -L         " << "List supported games" << std::endl;
	std::cout << " -v         " << "Verbose" << std::endl;
	std::cout << " -V         " << "More verbose (lists blocks)" << std::endl;
	std::cout << " -q         " << "Quiet" << std::endl;
	std::cout << std::endl;
	std::cout << "Example:" << std::endl;
	std::cout << "scummrp -gp monkey2 ./mi2 -id ./dumps/mi2" << std::endl;
}

void ScummRp::_getOptions(int argc, const char **argv, const ScummRp::Parameter *params)
{
	char pendingParams[MAX_PARAMS + 1];

	pendingParams[0] = '\0';
	for (int i = 1; i < argc && argv[i] && ScummRp::_readOption(argv[i], pendingParams); ++i)
	{
		for (int j = 0; pendingParams[j] != '\0'; pendingParams[j++] = '\0')
		{
			for (int k = 0; params[k].c != '\0'; ++k)
			{
				if (params[k].c != pendingParams[j])
					continue;

				++i;
				if (i < argc && argv[i])
				{
					strncpy(params[k].value, argv[i], params[k].maxSize - 1);
					params[k].value[params[k].maxSize - 1] = '\0';
#ifdef _WIN32
					if (params[k].isPath)
						for (char *p = strchr(params[k].value, '\\'); p; p = strchr(params[k].value, '\\'))
							*p++ = '/';
#endif
				}
				break;
			}
		}
	}
}

bool ScummRp::_invalidOptions()
{
	static const uint32 exclusive[] = { ScummRp::OPT_IMPORT | ScummRp::OPT_EXPORT, 0 };
	static const uint32 mandatory[] = { ScummRp::OPT_IMPORT | ScummRp::OPT_EXPORT, ScummRp::OPT_GAME_FILES, 0 };

	for (int i = 0; mandatory[i] != 0; ++i)
		if (!(ScummRp::_options & mandatory[i]))
			return true;

	for (int i = 0; exclusive[i] != 0; ++i)
		if ((ScummRp::_options & exclusive[i]) == exclusive[i])
			return true;

	return false;
}

void ScummRp::_listGames()
{
	size_t l1, l2;

	std::cout << "supported games:" << std::endl;
	std::cout << std::endl;
	std::cout << "id" << std::setw(12) << "| " << std::setw(0) << "description"
		 << std::setw(42) << "| " << std::setw(0) << "file" << std::endl;
	std::cout << "------------|-------------------------------------"
		"---------------|-------------" << std::endl;

	for (int i = 0; ScummRp::_gameDef[i].shortName != nullptr; ++i)
	{
		l1 = strlen(ScummRp::_gameDef[i].shortName);
		l2 = strlen(ScummRp::_gameDef[i].name);
		std::cout << ScummRp::_gameDef[i].shortName
			 << std::setw(14 - (std::streamsize)l1) << "| "
			 << std::setw(0) << ScummRp::_gameDef[i].name
			 << std::setw(53 - (std::streamsize)l2) << "| "
			 << std::setw(0) << ScummRp::_gameDef[i].indexFileName
			 << std::endl;
	}
}

/*
 * Game definitions
 */

const GameDefinition ScummRp::_gameDef[] =
{
	{ "maniacv1", "Maniac Mansion (V1)", "00.LFL", "%.2u.LFL", GID_MANIAC,
	  1, 0xFF, 0xFF, GTCFMT_8SEP16, BFMT_SIZEONLY, 4, GF_OLD_BUNDLE },
	{ "maniacv2", "Maniac Mansion (V2)", "00.LFL", "%.2u.LFL", GID_MANIAC,
	  2, 0xFF, 0xFF, GTCFMT_8SEP16, BFMT_SIZEONLY, 4, GF_OLD_BUNDLE },
// 	{ "maniacnes", "Maniac Mansion (NES)", "00.LFL", "%.2u.LFL", GID_MANIAC,
// 	  1, 0xFF, 0xFF, GTCFMT_8SEP16, BFMT_SIZEONLY, 4, GF_OLD_BUNDLE | GF_NES },
	{ "zakv1", "Zak McKracken and the Alien Mindbenders (V1)", "00.LFL", "%.2u.LFL", GID_ZAK,
	  1, 0xFF, 0xFF, GTCFMT_8SEP16, BFMT_SIZEONLY, 4, GF_OLD_BUNDLE },
	{ "zakv2", "Zak McKracken and the Alien Mindbenders (V2)", "00.LFL", "%.2u.LFL", GID_ZAK,
	  2, 0xFF, 0xFF, GTCFMT_8SEP16, BFMT_SIZEONLY, 4, GF_OLD_BUNDLE },
	{ "zaktowns", "Zak McKracken and the Alien Mindbenders (FM-TOWNS)", "00.LFL", "%.2u.LFL", GID_ZAK,
	  3, 0x00, 0x00, GTCFMT_16MIX32, BFMT_SHORTTAG, 6, GF_FMTOWNS },
	{ "indy3", "Indiana Jones and the Last Crusade (EGA)", "00.LFL", "%.2u.LFL", GID_INDY3,
	  3, 0xFF, 0xFF, GTCFMT_8SEP16, BFMT_SIZEONLY, 4, GF_OLD_BUNDLE },
	{ "indy3mac", "Indiana Jones and the Last Crusade (Macintosh)", "00.LFL", "%.2u.LFL", GID_INDY3,
	  3, 0xFF, 0xFF, GTCFMT_8SEP16, BFMT_SIZEONLY, 4, GF_OLD_BUNDLE | GF_MACINTOSH },
	{ "indy3towns", "Indiana Jones and the Last Crusade (FM-TOWNS)", "00.LFL", "%.2u.LFL", GID_INDY3,
	  3, 0x00, 0x00, GTCFMT_16MIX32, BFMT_SHORTTAG, 6, GF_FMTOWNS },
	{ "indy3vga", "Indiana Jones and the Last Crusade (VGA)", "00.LFL", "%.2u.LFL", GID_INDY3,
	  3, 0x00, 0x00, GTCFMT_16MIX32, BFMT_SHORTTAG, 6, GF_NULL },
	{ "loom", "Loom (EGA)", "00.LFL", "%.2u.LFL", GID_LOOM,
	  3, 0xFF, 0xFF, GTCFMT_8SEP16, BFMT_SIZEONLY, 4, GF_OLD_BUNDLE },
	{ "loomtowns", "Loom (FM-TOWNS)", "00.LFL", "%.2u.LFL", GID_LOOM,
	  3, 0x00, 0x00, GTCFMT_16MIX32, BFMT_SHORTTAG, 6, GF_FMTOWNS },
	{ "loomcd", "Loom (VGA)", "000.LFL", "DISK%.2u.LEC", GID_LOOM,
	  4, 0x00, 0x69, GTCFMT_16MIX32, BFMT_SHORTTAG, 6, GF_NULL },
	{ "monkey", "The Secret of Monkey Island (EGA)", "000.LFL", "DISK%.2u.LEC", GID_MONKEY,
	  4, 0x00, 0x69, GTCFMT_16MIX32, BFMT_SHORTTAG, 6, GF_NULL },
	{ "monkeycd", "The Secret of Monkey Island (CD)", "MONKEY.000", "MONKEY.%.3u", GID_MONKEY,
	  5, 0x69, 0x69, GTCFMT_16SEP32, BFMT_LONGTAG, 8, GF_NULL },
	{ "monkeycdalt", "The Secret of Monkey Island (CD, MONKEY1.000)", "MONKEY1.000", "MONKEY1.%.3u", GID_MONKEY,
	  5, 0x69, 0x69, GTCFMT_16SEP32, BFMT_LONGTAG, 8, GF_NULL },
	{ "monkeysega", "The Secret of Monkey Island (SEGA)", "GAME.000", "GAME.%.3u", GID_MONKEY,
	  5, 0x69, 0x69, GTCFMT_16SEP32, BFMT_LONGTAG, 8, GF_NULL },
	{ "monkey2", "Monkey Island 2: LeChuck's Revenge", "MONKEY2.000", "MONKEY2.%.3u", GID_MONKEY2,
	  5, 0x69, 0x69, GTCFMT_16SEP32, BFMT_LONGTAG, 8, GF_NULL },
	{ "atlantis", "Indiana Jones and the Fate of Atlantis", "ATLANTIS.000", "ATLANTIS.%.3u", GID_INDY4,
	  5, 0x69, 0x69, GTCFMT_16SEP32, BFMT_LONGTAG, 8, GF_NULL },
	{ "tentacle", "Day of the Tentacle", "TENTACLE.000", "TENTACLE.%.3u", GID_TENTACLE,
	  6, 0x69, 0x69, GTCFMT_16SEP32, BFMT_LONGTAG, 8, GF_NULL },
	{ "samnmax", "Sam & Max Hit the Road", "SAMNMAX.000", "SAMNMAX.%.3u", GID_SAMNMAX,
	  6, 0x69, 0x69, GTCFMT_16SEP32, BFMT_LONGTAG, 8, GF_NULL },
	{ "samnmaxalt", "Sam & Max Hit the Road (SAMNMAX.SM0)", "SAMNMAX.SM0", "SAMNMAX.SM%u", GID_SAMNMAX,
	  6, 0x69, 0x69, GTCFMT_16SEP32, BFMT_LONGTAG, 8, GF_NULL },
	{ "ft", "Full Throttle", "FT.LA0", "FT.LA%u", GID_FT,
	  7, 0x00, 0x00, GTCFMT_16SEP32, BFMT_LONGTAG, 8, GF_NULL },
	{ "dig", "The Dig", "DIG.LA0", "DIG.LA%u", GID_DIG,
	  7, 0x00, 0x00, GTCFMT_16SEP32, BFMT_LONGTAG, 8, GF_NULL },
// 	{ "comi", "The Curse of Monkey Island", "COMI.LA0", "COMI.LA%u", GID_CMI,
// 	  8, 0x00, 0x00, FNFMT_GAME_LA0, GTCFMT_16SEP32, BFMT_LONGTAG, 8, GF_NULL },
	//
	{ nullptr, nullptr, nullptr, nullptr, GID_NULL, 0, 0, 0, GTCFMT_NULL, BFMT_NULL, 0, GF_NULL } };

/*
 *
 */

GameDefinition ScummRp::_game = { nullptr, nullptr, nullptr, nullptr, GID_NULL, 0, 0, 0, GTCFMT_NULL, BFMT_NULL, 0, GF_NULL };

int ScummRp::_fileOptions = BlocksFile::BFOPT_AUTO;
int ScummRp::_options = ScummRp::OPT_NULL;
BackUp ScummRp::_backupSystem;
int ScummRp::_infoSlots = INF_GLOBAL;
TableOfContent *const *ScummRp::_tocs = ScummRp::_mainTocs;
uint32 ScummRp::_filterTag = 0;

ScummRp::TOCSet ScummRp::_mainTocSet;
ScummRp::TOCSet ScummRp::_tmpTocSet;
ScummRp::TOCSet ScummRp::_updTocSet;

TableOfContent *const ScummRp::_mainTocs[] =
{
	&ScummRp::_mainTocSet.roomToc, &ScummRp::_mainTocSet.scrpToc,
	&ScummRp::_mainTocSet.sounToc, &ScummRp::_mainTocSet.costToc,
	&ScummRp::_mainTocSet.charToc,
	nullptr
};
TableOfContent *const ScummRp::_tmpTocs[] =
{
	&ScummRp::_tmpTocSet.roomToc, &ScummRp::_tmpTocSet.scrpToc,
	&ScummRp::_tmpTocSet.sounToc, &ScummRp::_tmpTocSet.costToc,
	&ScummRp::_tmpTocSet.charToc,
	nullptr
};
TableOfContent *const ScummRp::_updTocs[] =
{
	&ScummRp::_updTocSet.roomToc, &ScummRp::_updTocSet.scrpToc,
	&ScummRp::_updTocSet.sounToc, &ScummRp::_updTocSet.costToc,
	&ScummRp::_updTocSet.charToc,
	nullptr
};
