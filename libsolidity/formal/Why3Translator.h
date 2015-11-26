/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * @author Christian <c@ethdev.com>
 * @date 2015
 * Component that translates Solidity code into the why3 programming language.
 */

#pragma once

#include <libsolidity/ast/ASTVisitor.h>
#include <libsolidity/interface/Exceptions.h>
#include <string>

namespace dev
{
namespace solidity
{

class SourceUnit;

/**
 * Simple translator from Solidity to Why3.
 *
 * @todo detect side effects in sub-expressions and limit them to one per statement.
 * @todo `x = y = z`
 * @todo implicit and explicit type conversion
 */
class Why3Translator: private ASTConstVisitor
{
public:
	Why3Translator(ErrorList& _errors): m_lines{{std::string(), 0}}, m_errors(_errors) {}

	/// Appends formalisation of the given source unit to the output.
	/// @returns false on error.
	bool process(SourceUnit const& _source);

	std::string translation() const;

private:
	/// Returns an error.
	void error(ASTNode const& _node, std::string const& _description);
	/// Reports a fatal error and throws.
	void fatalError(ASTNode const& _node, std::string const& _description);

	/// Appends imports and constants use throughout the formal code.
	void appendPreface();

	/// @returns a string representation of the corresponding formal type or the empty string
	/// if the type is not supported.
	std::string toFormalType(Type const& _type) const;

	void indent() { newLine(); m_lines.back().indentation++; }
	void unindent();
	void addLine(std::string const& _line);
	void add(std::string const& _str);
	void newLine();
	void appendSemicolon();

	virtual bool visit(SourceUnit const&) override { return true; }
	virtual bool visit(ContractDefinition const& _contract) override;
	virtual void endVisit(ContractDefinition const& _contract) override;
	virtual bool visit(FunctionDefinition const& _function) override;
	virtual void endVisit(FunctionDefinition const& _function) override;
	virtual bool visit(Block const&) override;
	virtual bool visit(IfStatement const& _node) override;
	virtual bool visit(WhileStatement const& _node) override;
	virtual bool visit(Return const& _node) override;
	virtual bool visit(VariableDeclarationStatement const& _node) override;
	virtual bool visit(ExpressionStatement const&) override;
	virtual bool visit(Assignment const& _node) override;
	virtual bool visit(TupleExpression const& _node) override;
	virtual void endVisit(TupleExpression const&) override { add(")"); }
	virtual bool visit(UnaryOperation const& _node) override;
	virtual bool visit(BinaryOperation const& _node) override;
	virtual bool visit(FunctionCall const& _node) override;
	virtual bool visit(MemberAccess const& _node) override;
	virtual bool visit(IndexAccess const& _node) override;
	virtual bool visit(Identifier const& _node) override;
	virtual bool visit(Literal const& _node) override;

	virtual bool visitNode(ASTNode const& _node) override
	{
		error(_node, "Code not supported for formal verification.");
		return false;
	}

	bool isStateVariable(VariableDeclaration const* _var) const;
	bool isStateVariable(std::string const& _name) const;
	bool isLocalVariable(VariableDeclaration const* _var) const;
	bool isLocalVariable(std::string const& _name) const;

	/// Visits the givin statement and indents it unless it is a block
	/// (which does its own indentation).
	void visitIndentedUnlessBlock(Statement const& _statement);

	void addSourceFromDocStrings(DocumentedAnnotation const& _annotation);
	/// Transforms substring like `#varName` and `#stateVarName` to code that evaluates to their value.
	std::string transformVariableReferences(std::string const& _annotation);

	/// True if we have already seen a contract. For now, only a single contract
	/// is supported.
	bool m_seenContract = false;
	bool m_errorOccured = false;

	std::vector<ASTPointer<VariableDeclaration>> const* m_stateVariables = nullptr;
	std::map<std::string, VariableDeclaration const*> m_localVariables;

	struct Line
	{
		std::string contents;
		unsigned indentation;
	};
	std::vector<Line> m_lines;
	ErrorList& m_errors;
};


}
}
