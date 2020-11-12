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
 * Note: the last "official" binary was built on: 2003-10-12 13:54:10.
 * Various changes were made in 2004 and in 2005, but were never released,
 * except maybe to some translation team members. Look for the SCUMMTR_
 * #ifdef's to learn more about the changes.
 */

#include "scummtr.hpp"
#include "backup.hpp"
#include "trblock.hpp"
#include "io.hpp"

#include <cstring>

using namespace std;

/*
 * ScummTr
 */

const char *const ScummTr::NAME    = "ScummTr";
const char *const ScummTr::VERSION = "0.4";
const char *const ScummTr::AUTHOR  = "Thomas Combeleran";

const ScummRp::Parameter ScummTr::_trParameters[] = {
	{ 'g', ScummRp::_paramGameId, sizeof ScummRp::_paramGameId, false },
	{ 'f', ScummTr::_paramTextFile, sizeof ScummTr::_paramTextFile, true },
	{ 'l', ScummTr::_paramLanguage, sizeof ScummTr::_paramLanguage, false },
	{ 'p', ScummRp::_paramGameDir, sizeof ScummRp::_paramGameDir, true },
	{ 'a', ScummTr::_paramPaddedRsc, sizeof ScummTr::_paramPaddedRsc, false },
	{ 'A', ScummTr::_paramPaddedRsc, sizeof ScummTr::_paramPaddedRsc, false },
	{ 0, 0, 0 } };

char ScummTr::_paramTextFile[512] = "./text";
char ScummTr::_paramLanguage[3] = "en";
char ScummTr::_paramPaddedRsc[16] = "";

int ScummTr::_textOptions = Text::TXT_NULL;
int ScummTr::_paddedRsc = 0;

ScummTr::RscNameLimits ScummTr::_rscNameLimits = { { 0, 0, 0 } };
bool ScummTr::_exportWithPadding = false;
bool ScummTr::_maxPadding = false;

void ScummTr::_explore(TreeBlock &tree, int action, Text &text)
{
	TreeBlockPtr blockPtr;

	tree.firstBlock();
	while ((blockPtr = tree.nextBlock()) != 0)
	{
		if (blockPtr.is<LFLFPack>() || blockPtr.is<RoomBlock>())
			ScummTr::_explore(*blockPtr, action, text);
		else
			switch (blockPtr->getTag())
			{
			case MKTAG4('L','E','C','F'):
			case MKTAG2('L','E'):
				{
					TreeBlockPtr lecf;

					lecf = new LECFPack(*blockPtr);
					ScummTr::_explore(*lecf, action, text);
				}
				break;
			case MKTAG4('S','C','R','P'):
			case MKTAG2('S','C'):
			case MKTAG4('S','C','v','3'):
			case MKTAG4('S','C','v','2'):
			case MKTAG4('S','C','v','1'):
			case MKTAG4('E','N','C','D'):
			case MKTAG4('E','X','C','D'):
			case MKTAG2('E','N'):
			case MKTAG2('E','X'):
			case MKTAG4('E','N','v','3'):
			case MKTAG4('E','X','v','3'):
			case MKTAG4('E','N','v','2'):
			case MKTAG4('E','X','v','2'):
			case MKTAG4('E','N','v','1'):
			case MKTAG4('E','X','v','1'):
			case MKTAG4('L','S','v','3'):
				{
					if (action == ScummRp::ACT_IMPORT)
						ScriptBlock(*blockPtr).importText(text);
					else if (action == ScummRp::ACT_EXPORT)
						ScriptBlock(*blockPtr).exportText(text, ScummTr::_exportWithPadding);
					else if (action == ScummTr::ACT_RSCNAMELIMITS)
						ScriptBlock(*blockPtr).getRscNameLimits();
				}
				break;
			case MKTAG4('L','S','C','R'):
			case MKTAG2('L','S'):
				{
					if (action == ScummRp::ACT_IMPORT)
						ScriptBlock(*blockPtr, (ScummRp::_game.version == 7) ? 2 : 1).importText(text);
					else if (action == ScummRp::ACT_EXPORT)
						ScriptBlock(*blockPtr, (ScummRp::_game.version == 7) ? 2 : 1).exportText(text, ScummTr::_exportWithPadding);
					else if (action == ScummTr::ACT_RSCNAMELIMITS)
						ScriptBlock(*blockPtr, (ScummRp::_game.version == 7) ? 2 : 1).getRscNameLimits();
				}
				break;
			case MKTAG4('O','B','C','D'):
				ScummTr::_explore(*blockPtr, action, text);
				break;
			case MKTAG4('O','B','N','A'):
				{
					if (action == ScummRp::ACT_IMPORT)
						ObjectNameBlock(*blockPtr).importText(text);
					else if (action == ScummRp::ACT_EXPORT)
						ObjectNameBlock(*blockPtr).exportText(text, ScummTr::_exportWithPadding);
					else if (action == ScummTr::ACT_RSCNAMELIMITS)
						ObjectNameBlock(*blockPtr).getRscNameLimits();
				}
				break;
			case MKTAG4('V','E','R','B'):
				{
					if (action == ScummRp::ACT_IMPORT)
						ObjectCodeBlock(*blockPtr).importText(text);
					else if (action == ScummRp::ACT_EXPORT)
						ObjectCodeBlock(*blockPtr).exportText(text, ScummTr::_exportWithPadding);
					else if (action == ScummTr::ACT_RSCNAMELIMITS)
						ObjectCodeBlock(*blockPtr).getRscNameLimits();
				}
				break;
			case MKTAG2('O','C'):
			case MKTAG4('O','C','v','3'):
				{
					if (action == ScummRp::ACT_IMPORT)
						OldObjectCodeBlock(*blockPtr).importText(text);
					else if (action == ScummRp::ACT_EXPORT)
						OldObjectCodeBlock(*blockPtr).exportText(text, ScummTr::_exportWithPadding);
					else if (action == ScummTr::ACT_RSCNAMELIMITS)
						OldObjectCodeBlock(*blockPtr).getRscNameLimits();
				}
				break;
			case MKTAG4('O','C','v','2'):
			case MKTAG4('O','C','v','1'):
				{
					if (action == ScummRp::ACT_IMPORT)
						OldObjectCodeBlockV1(*blockPtr).importText(text);
					else if (action == ScummRp::ACT_EXPORT)
						OldObjectCodeBlockV1(*blockPtr).exportText(text, ScummTr::_exportWithPadding);
					else if (action == ScummTr::ACT_RSCNAMELIMITS)
						OldObjectCodeBlockV1(*blockPtr).getRscNameLimits();
				}
				break;
			}
	}
}

void ScummTr::_processGameFilesV123()
{
	int i;
	char dataFileName[32];
	string indexPath(ScummRp::_paramGameDir);
	TreeBlockPtr index;
	Text::Charset charset;

	charset = ScummTr::_selectCharset();
	indexPath += '/';
	indexPath += ScummRp::_game.indexFileName;
	index = ScummRp::_newIndex(indexPath.c_str());
	ScummRp::_exploreIndex<ScummRp::ACT_LOAD>(*index);
	{
		Text text(ScummTr::_paramTextFile, ScummTr::_textOptions, charset);

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
					ScummTr::_explore(*room, ScummRp::ACT_IMPORT, text);
				else
				{
					if (ScummTr::_exportWithPadding)
					{
						ScummRpIO::setQuiet(true);
						ScummTr::_explore(*room, ScummTr::ACT_RSCNAMELIMITS, text);
						ScummRpIO::setQuiet(false);
					}
					ScummTr::_explore(*room, ScummRp::ACT_EXPORT, text);
				}
			}
		}
	}
	if (ScummRp::_options & ScummRp::OPT_IMPORT)
	{
		ScummRpIO::setQuiet(true);
		ScummRp::_exploreIndex<ScummRp::ACT_SAVE>(*index);
		ScummRpIO::setQuiet(false);
	}
}

void ScummTr::_processGameFilesV4567()
{
	int i;
	char dataFileName[32];
	string indexPath(ScummRp::_paramGameDir);
	TreeBlockPtr index;
	int numberOfDisks;
	Text::Charset charset;

	charset = ScummTr::_selectCharset();
	indexPath += '/';
	indexPath += ScummRp::_game.indexFileName;
	index = ScummRp::_newIndex(indexPath.c_str());
	ScummRp::_exploreIndex<ScummRp::ACT_LOAD>(*index);
	numberOfDisks = ScummRp::_mainTocSet.roomToc.numberOfDisks();
	ScummRp::_prepareTmpIndex();
	{
		Text text(ScummTr::_paramTextFile, ScummTr::_textOptions, charset);

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
				ScummTr::_explore(*disk, ScummRp::ACT_IMPORT, text);
			else
			{
				if (ScummTr::_exportWithPadding)
				{
					ScummRpIO::setQuiet(true);
					ScummTr::_explore(*disk, ScummTr::ACT_RSCNAMELIMITS, text);
					ScummRpIO::setQuiet(false);
				}
				ScummTr::_explore(*disk, ScummRp::ACT_EXPORT, text);
			}
			ScummRp::_mergeTmpIndex();
		}
	}
	ScummRp::_updateMainIndex();
	if (ScummRp::_options & ScummRp::OPT_IMPORT)
	{
		ScummRpIO::setQuiet(true);
		ScummRp::_exploreIndex<ScummRp::ACT_SAVE>(*index);
		ScummRpIO::setQuiet(false);
	}
}

Text::Charset ScummTr::_selectCharset()
{
	if (ScummRp::_game.version <= 2)
	{
		if (strcmp(ScummTr::_paramLanguage, "en") == 0)
			return Text::CHS_V1EN;
		if (strcmp(ScummTr::_paramLanguage, "de") == 0)
			return Text::CHS_V1DE;
		if (strcmp(ScummTr::_paramLanguage, "it") == 0)
			return Text::CHS_V1IT;
		if (strcmp(ScummTr::_paramLanguage, "fr") == 0)
			return Text::CHS_V1FR;
		ScummRpIO::warning(xsprintf("Unknown language code %s", ScummTr::_paramLanguage));
		return Text::CHS_NULL;
	}
	return Text::CHS_V3ANSI;
}

void ScummTr::setRscNameMaxLengh(ScummTr::RscType t, int32 id, int32 l)
{
	if (((1 << t) & ScummTr::_paddedRsc) == 0)
		return;
	if (id < 0)
	{
		if (ScummTr::_maxPadding && ScummTr::_rscNameLimits.anyRsc[t] < l)
			ScummTr::_rscNameLimits.anyRsc[t] = l;
	}
	else
	{
		if ((int32)ScummTr::_rscNameLimits.rsc[t].size() <= id)
			ScummTr::_rscNameLimits.rsc[t].resize(id + 1, 0);
		if (ScummTr::_rscNameLimits.rsc[t][id] < l)
			ScummTr::_rscNameLimits.rsc[t][id] = l;
	}
}

int32 ScummTr::getRscNameMaxLengh(ScummTr::RscType t, int32 id)
{
	if (((1 << t) & ScummTr::_paddedRsc) == 0)
		return 0;
	if (id < 0)
		return ScummTr::_rscNameLimits.anyRsc[t];
	else
		if (ScummTr::_rscNameLimits.anyRsc[t] < ScummTr::_rscNameLimits.rsc[t][id])
			return ScummTr::_rscNameLimits.rsc[t][id];
		else
			return ScummTr::_rscNameLimits.anyRsc[t];
}

int ScummTr::main(int argc, const char **argv)
{
	int r, g, i;

	r = 0;
	ScummTr::_getOptions(argc, argv, ScummTr::_trParameters);
	ScummRpIO::setInfoSlots(ScummRp::_infoSlots);
	ScummRpIO::info(INF_GLOBAL, xsprintf("%s %s by %s", ScummTr::NAME, ScummTr::VERSION, ScummTr::AUTHOR));
	ScummRpIO::info(INF_GLOBAL, "");
	if (ScummRp::_options & ScummRp::OPT_LIST)
	{
		ScummTr::_listGames();
		return 0;
	}
	if (ScummTr::_invalidOptions())
	{
		ScummTr::_usage();
		return 0;
	}
	ScummTr::_paddedRsc = 0;
	for (i = 0; ScummTr::_paramPaddedRsc[i] != 0; ++i)
		switch (ScummTr::_paramPaddedRsc[i] | 0x20)
		{
		case 'a':
			ScummTr::_paddedRsc |= 1 << ScummTr::RSCT_ACTOR;
			break;
		case 'o':
			ScummTr::_paddedRsc |= 1 << ScummTr::RSCT_OBJECT;
			break;
		case 'v':
			ScummTr::_paddedRsc |= 1 << ScummTr::RSCT_VERB;
			break;
		}
	g = ScummRp::_findGameDef(ScummRp::_paramGameId);
	if (g == -1)
	{
		ScummRp::_listGames();
		return 0;
	}
	ScummRp::_game = ScummRp::_gameDef[g];
	if (ScummRp::_options & ScummRp::OPT_IMPORT)
		ScummRp::_fileOptions |= BlocksFile::BFOPT_BACKUP;
	else
	{
		ScummRp::_fileOptions |= BlocksFile::BFOPT_READONLY;
		ScummTr::_textOptions |= Text::TXT_OUT;
	}
	if (ScummRp::_game.version < 4)
		ScummTr::_processGameFilesV123();
	else
		ScummTr::_processGameFilesV4567();
	ScummRp::_backupSystem.applyChanges();
	return 0;
}

bool ScummTr::_readOption(const char *arg, char *pendingParams)
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
			case 'h':
				ScummTr::_textOptions |= Text::TXT_HEADER;
				break;
			case 'H':
				ScummTr::_textOptions |= Text::TXT_HEXA;
				break;
			case 'I':
				ScummTr::_textOptions |= Text::TXT_OPCODE;
				break;
			case 'A':
				ScummTr::_exportWithPadding = true;
				ScummTr::_maxPadding = true;
				ScummRp::_queueParam(pendingParams, c);
				break;
			case 'a':
				ScummTr::_exportWithPadding = true;
				ScummTr::_maxPadding = false;
				ScummRp::_queueParam(pendingParams, c);
				break;
			case 'w':
				ScummTr::_textOptions |= Text::TXT_CRLF;
				break;
			case 'c':
				ScummTr::_textOptions |= Text::TXT_USECHARSET;
				break;
			case 'b':
				ScummTr::_textOptions |= Text::TXT_BINARY;
				break;
			case 'o':
				ScummRp::_options |= ScummRp::OPT_EXPORT;
				break;
			case 'i':
				ScummRp::_options |= ScummRp::OPT_IMPORT;
				break;
			case 's':
				ScummRp::_fileOptions = BlocksFile::BFOPT_NULL;
				break;
			case 'm':
				ScummRp::_fileOptions &= ~BlocksFile::BFOPT_AUTO;
				ScummRp::_fileOptions |= BlocksFile::BFOPT_RAM;
				break;
			case 'O':
				ScummRp::_fileOptions &= ~BlocksFile::BFOPT_AUTO;
				ScummRp::_fileOptions |= BlocksFile::BFOPT_SEQFILE;
				break;
			case 'L':
				ScummRp::_options |= ScummRp::OPT_LIST;
				return false;
			case 'q':
				ScummRp::_infoSlots = INF_NULL; // intentionally not definitive
				break;
			case 'v':
				ScummRp::_infoSlots |= INF_DETAIL;
				break;
// 			case 'V':
// 				ScummRp::_infoSlots |= INF_LISTING;
// 				break;
			case 'g':
				ScummRp::_options |= ScummRp::OPT_GAME_FILES;
				ScummRp::_queueParam(pendingParams, c);
				break;
			case 'l':
			case 'p':
			case 'f':
				ScummRp::_queueParam(pendingParams, c);
				break;
			}
	return true;
}

void ScummTr::_getOptions(int argc, const char **argv, const ScummRp::Parameter *params)
{
	char pendingParams[MAX_PARAMS + 1];
	int i, j, k;

	pendingParams[0] = 0;
	for (i = 1; i < argc && argv[i] && ScummTr::_readOption(argv[i], pendingParams); ++i)
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

bool ScummTr::_invalidOptions()
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

void ScummTr::_usage()
{
	cout << "options:" << endl;
	cout << endl;
	cout << " -g gameid  " << "Select a game" << endl;
	cout << " -i         " << "Import text into the game files (Input)" << endl;
	cout << " -o         " << "Export text from the game files (Output)" << endl;
	cout << " -f path    " << "Path to the text file" << endl;
	cout << " -p path    " << "Path to the game directory" << endl;
	cout << " -w         " << "Use the CrLf format (for Windows)" << endl;
	cout << " -c         " << "Convert characters to the ANSI charset" << endl;
	cout << " -l xx      " << "Language (for V1 & V2 games): en, de, it, fr" << endl;
	cout << " -b         " << "Binary mode" << endl;
	cout << " -h         " << "Add line headers in exported text (with -o)" << endl;
	cout << " -H         " << "Hexadecimal char codes (with -o)" << endl;
	cout << " -I         " << "Show instruction opcode (with -o)" << endl;
	cout << " -a [oav]   " << "Pad Objects, Actors, Verbs with @ (with -o)" << endl;
	cout << " -A [oav]   " << "-a with variable ids taken into account (with -o)" << endl;
// 	cout << " -m         " << "Work in memory (whole game files are loaded in RAM)" << endl;
// 	cout << " -O         " << "Optimize for sequential access (with -i)" << endl;
// 	cout << " -s         " << "Slow mode (disable automatic -m or -O)" << endl;
	cout << " -L         " << "List supported games" << endl;
	cout << " -v         " << "Verbose" << endl;
// 	cout << " -V         " << "More verbose (lists blocks)" << endl;
	cout << " -q         " << "Quiet" << endl;
	cout << endl;
	cout << "Examples:" << endl;
	cout << "scummtr -cw -gp monkey2 ./mi2 -if ./mi2_fr.txt" << endl;
	cout << "scummtr -o -A ov -g zaktowns" << endl;
}