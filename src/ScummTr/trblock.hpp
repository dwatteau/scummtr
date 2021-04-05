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

#ifndef SCUMMTR_TRBLOCK_HPP
#define SCUMMTR_TRBLOCK_HPP

#include "ScummRp/block.hpp"

#include "text.hpp"
#include "script.hpp"

#include <list>

/*
 * TextBlock
 */

class TextBlock : public LeafBlock
{
protected:
	int _lflfId() const;
	int _ownId() const;

public:
	virtual void importText(Text &input) = 0;
	virtual void exportText(Text &output, bool pad = false) = 0;
	virtual void getRscNameLimits() = 0;

public:
	TextBlock();
	virtual ~TextBlock() = 0;

public:
	TextBlock(const TreeBlock &block);
	virtual TextBlock &operator=(const TreeBlock &block);
};

/*
 * ScriptBlock
 */

class ScriptBlock : public TextBlock
{
protected:
	Script *_script;

public:
	virtual void importText(Text &input);
	virtual void exportText(Text &output, bool pad = false);
	virtual void getRscNameLimits();

public:
	ScriptBlock(int32 subHeaderSize = 0);
	virtual ~ScriptBlock();

private: // Not copiable
	ScriptBlock(const ScriptBlock &);
	ScriptBlock &operator=(const ScriptBlock &);

public: // TreeBlock part still copiable
	ScriptBlock(const TreeBlock &block, int32 subHeaderSize = 0);
	virtual ScriptBlock &operator=(const TreeBlock &block);
};

/*
 * ObjectNameBlock
 */

class ObjectNameBlock : public TextBlock
{
public:
	virtual void importText(Text &input);
	virtual void exportText(Text &output, bool pad = false);
	virtual void getRscNameLimits();

public:
	ObjectNameBlock();
	virtual ~ObjectNameBlock();

private: // Not copiable
	ObjectNameBlock(const ObjectNameBlock &);
	ObjectNameBlock &operator=(const ObjectNameBlock &);

public: // TreeBlock part still copiable
	ObjectNameBlock(const TreeBlock &block);
	virtual ObjectNameBlock &operator=(const TreeBlock &block);
};

/*
 * ObjectCodeBlock
 */

class ObjectCodeBlock : public TextBlock
{
public:
	class Error : public std::runtime_error
	{
	public:
		Error(const std::string &message) : std::runtime_error(message) { }
	};

protected:
	Script *_script;

protected:
	template <class T, int I> void _tListVerbs(std::list<int32> &l, int32 scriptOffset);
	template <class T, int I> void _tUpdateVerbs(const std::list<int32> &l, int32 scriptOffset, int n);
	template <class T, int I> int32 _tFindScriptOffset();
	virtual void _listVerbs(std::list<int32> &l, int32 scriptOffset);
	virtual void _updateVerbs(const std::list<int32> &l, int32 scriptOffset, int n);
	virtual int32 _findScriptOffset();
	virtual void _importText(Text &input, int32 oldSize, int32 scriptOffset);

public:
	virtual void importText(Text &input);
	virtual void exportText(Text &output, bool pad = false);
	virtual void getRscNameLimits();

public:
	ObjectCodeBlock();
	virtual ~ObjectCodeBlock();

private: // Not copiable
	ObjectCodeBlock(const ObjectCodeBlock &);
	ObjectCodeBlock &operator=(const ObjectCodeBlock &);

public: // TreeBlock part still copiable
	ObjectCodeBlock(const TreeBlock &block);
	virtual ObjectCodeBlock &operator=(const TreeBlock &block);
};

/*
 * OldObjectCodeBlock
 */

class OldObjectCodeBlock : public ObjectCodeBlock
{
protected:
	virtual void _listVerbs(std::list<int32> &l, int32 scriptOffset);
	virtual void _updateVerbs(const std::list<int32> &l, int32 scriptOffset, int n);
	virtual int32 _findScriptOffset();
	template <int I> void _exportName(Text &output, bool pad = false);
	template <int I> void _importName(Text &input, int32 &scriptOffset);
public:
	virtual void importText(Text &input);
	virtual void exportText(Text &output, bool pad = false);
	virtual void getRscNameLimits();

public:
	OldObjectCodeBlock();
	virtual ~OldObjectCodeBlock();

private: // Not copiable
	OldObjectCodeBlock(const OldObjectCodeBlock &);
	OldObjectCodeBlock &operator=(const OldObjectCodeBlock &);

public: // TreeBlock part still copiable
	OldObjectCodeBlock(const TreeBlock &block);
	virtual OldObjectCodeBlock &operator=(const TreeBlock &block);
};

/*
 * OldObjectCodeBlockV1
 */

class OldObjectCodeBlockV1 : public OldObjectCodeBlock
{
protected:
	virtual void _listVerbs(std::list<int32> &l, int32 scriptOffset);
	virtual void _updateVerbs(const std::list<int32> &l, int32 scriptOffset, int n);
	virtual int32 _findScriptOffset();

public:
	virtual void importText(Text &input);
	virtual void exportText(Text &output, bool pad = false);
	virtual void getRscNameLimits();

public:
	OldObjectCodeBlockV1();
	virtual ~OldObjectCodeBlockV1();

private: // Not copiable
	OldObjectCodeBlockV1(const OldObjectCodeBlockV1 &);
	OldObjectCodeBlockV1 &operator=(const OldObjectCodeBlockV1 &);

public: // TreeBlock part still copiable
	OldObjectCodeBlockV1(const TreeBlock &block);
	virtual OldObjectCodeBlockV1 &operator=(const TreeBlock &block);
};

#endif
