/*
 * SPDX-License-Identifier: MIT
 *
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

#include "common/types.hpp"
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
	~TextBlock() override = 0;

public:
	TextBlock(const TreeBlock &block);
	TextBlock &operator=(const TreeBlock &block) override;
};

/*
 * ScriptBlock
 */

class ScriptBlock : public TextBlock
{
protected:
	Script *_script;

public:
	void importText(Text &input) override;
	void exportText(Text &output, bool pad = false) override;
	void getRscNameLimits() override;

public:
	ScriptBlock(int32 subHeaderSize = 0);
	~ScriptBlock() override;

private: // Not copiable
	ScriptBlock(const ScriptBlock &);
	ScriptBlock &operator=(const ScriptBlock &);

public: // TreeBlock part still copiable
	ScriptBlock(const TreeBlock &block, int32 subHeaderSize = 0);
	ScriptBlock &operator=(const TreeBlock &block) override;
};

/*
 * ObjectNameBlock
 */

class ObjectNameBlock : public TextBlock
{
public:
	void importText(Text &input) override;
	void exportText(Text &output, bool pad = false) override;
	void getRscNameLimits() override;

public:
	ObjectNameBlock();
	~ObjectNameBlock() override;

private: // Not copiable
	ObjectNameBlock(const ObjectNameBlock &);
	ObjectNameBlock &operator=(const ObjectNameBlock &);

public: // TreeBlock part still copiable
	ObjectNameBlock(const TreeBlock &block);
	ObjectNameBlock &operator=(const TreeBlock &block) override;
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
	void _importText(Text &input, int32 oldSize, int32 scriptOffset);
	virtual void _listVerbs(std::list<int32> &l, int32 scriptOffset);
	virtual void _updateVerbs(const std::list<int32> &l, int32 scriptOffset, int n);
	virtual int32 _findScriptOffset();

public:
	void importText(Text &input) override;
	void exportText(Text &output, bool pad = false) override;
	void getRscNameLimits() override;

public:
	ObjectCodeBlock();
	~ObjectCodeBlock() override;

private: // Not copiable
	ObjectCodeBlock(const ObjectCodeBlock &);
	ObjectCodeBlock &operator=(const ObjectCodeBlock &);

public: // TreeBlock part still copiable
	ObjectCodeBlock(const TreeBlock &block);
	ObjectCodeBlock &operator=(const TreeBlock &block) override;
};

/*
 * OldObjectCodeBlock
 */

class OldObjectCodeBlock : public ObjectCodeBlock
{
protected:
	void _listVerbs(std::list<int32> &l, int32 scriptOffset) override;
	void _updateVerbs(const std::list<int32> &l, int32 scriptOffset, int n) override;
	int32 _findScriptOffset() override;
	template <int I> void _exportName(Text &output, bool pad = false);
	template <int I> void _importName(Text &input, int32 &scriptOffset);
public:
	void importText(Text &input) override;
	void exportText(Text &output, bool pad = false) override;
	void getRscNameLimits() override;

public:
	OldObjectCodeBlock();
	~OldObjectCodeBlock() override;

private: // Not copiable
	OldObjectCodeBlock(const OldObjectCodeBlock &);
	OldObjectCodeBlock &operator=(const OldObjectCodeBlock &);

public: // TreeBlock part still copiable
	OldObjectCodeBlock(const TreeBlock &block);
	OldObjectCodeBlock &operator=(const TreeBlock &block) override;
};

/*
 * OldObjectCodeBlockV1
 */

class OldObjectCodeBlockV1 : public OldObjectCodeBlock
{
protected:
	void _listVerbs(std::list<int32> &l, int32 scriptOffset) override;
	void _updateVerbs(const std::list<int32> &l, int32 scriptOffset, int n) override;
	int32 _findScriptOffset() override;

public:
	void importText(Text &input) override;
	void exportText(Text &output, bool pad = false) override;
	void getRscNameLimits() override;

public:
	OldObjectCodeBlockV1();
	~OldObjectCodeBlockV1() override;

private: // Not copiable
	OldObjectCodeBlockV1(const OldObjectCodeBlockV1 &);
	OldObjectCodeBlockV1 &operator=(const OldObjectCodeBlockV1 &);

public: // TreeBlock part still copiable
	OldObjectCodeBlockV1(const TreeBlock &block);
	OldObjectCodeBlockV1 &operator=(const TreeBlock &block) override;
};

#endif
