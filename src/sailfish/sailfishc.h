/*
 * Robert Durst 2019
 * Sailfish Programming Language
 *
 * Quite literally the bread, the butter, and the jam of the sailfishc compiler.
 * This single pass traversal of the source text parses, does semantic analysis,
 * and transpiles the code into C without any intermediate representation of
 * structure. While this may seem cool, it was honestly a really poor design
 * decision. This beast has become rather tightly coupled with a few more global
 * variables than I would like. This is partially a product of experimentation
 * for experimentation sake, partially because it is only my second time working
 * on a project in this domain and doing anything large in C++ and also because
 * this was mostly written in binges between the hours of 10pm and 5am.
 */
#pragma once
#include "../common/display.h"
#include "../errorhandler/Error.h"
#include "../errorhandler/ParserErrorHandler.h"
#include "../errorhandler/SemanticAnalyzerErrorHandler.h"
#include "../lexar/Lexar.h"
#include "../lexar/Token.h"
#include "../semantics/SymbolTable.h"
#include "../semantics/UDTTable.h"
#include "../stdlib_c/stdlib_c.h"
#include <cstdarg>
#include <deque>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <tuple>
#include <unordered_map>
#include <variant>
#include <vector>

using UdtFlagAndBufer =
    std::tuple<std::shared_ptr<UDTTable>, bool, std::string>;

const static std::string OUTPUT_HEADER =
    "/**"
    "\n * Do not alter! This code is generated by the sailfishc"
    "\n * compiler and might/will break if anything changed."
    "\n "
    "\n * This program is free software: you can redistribute it and/or "
    "modify"
    "\n * it under the terms of the GNU General Public License as "
    "published by"
    "\n * the Free Software Foundation, either version 3 of the License, or"
    "\n * (at your option) any later version."
    "\n * "
    "\n * This program is distributed in the hope that it will be useful,"
    "\n * but WITHOUT ANY WARRANTY; without even the implied warranty of"
    "\n * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the"
    "\n * GNU General Public License for more details."
    "\n * You should have received a copy of the GNU General Public License"
    "\n *"
    "\n * \"Many men go fishing all their lives without knowing it is"
    "\n * not the fish they are after\" - Henry David Thoreau\n */"
    "\n"
    "\n#include <stdio.h>"
    "\n#include <stdlib.h>"
    "\n#include <string.h>"
    "\n";

class sailfishc
{
  private:
    // state of compiler
    std::unique_ptr<Lexar> lexar;
    std::unique_ptr<Token> currentToken;
    std::unique_ptr<ParserErrorHandler> errorhandler;
    std::unique_ptr<SemanticAnalyzerErrorHandler> semanticerrorhandler;
    std::shared_ptr<SymbolTable> symboltable;
    std::unique_ptr<UDTTable> udttable;
    std::string filename;
    bool isUdt;
    bool inGrouping = false;
    std::string decName = "";
    std::string decType = "";
    int currentTabs = 0;
    std::string targetBuffer = "";
    std::string methodAccessName = "";
    std::string attributeAccessName = "";
    bool shouldDisplayErrors;

    // ------- Transpiler -------- //
    std::ofstream output;
    // some transpiler helpers
    std::string getTabs();

    // helper for simplifying redundancy of recursive loops
    template <typename F>
    void
    recursiveParse(bool eq, TokenKind k, const F& f)
    {
        if (((currentToken->kind == k) && (eq)) ||
            ((currentToken->kind != k) && (!eq)))
        {
            return;
        }
        else if (currentToken->kind == TokenKind::EOF_)
        {
            errorhandler->handle(std::make_unique<Error>(
                Error(currentToken->col, currentToken->line,
                      "Unexpected end of file.",
                      "Expected to receive an end of token delimiter such "
                      "as '(' or '}'",
                      "", "")));
            return;
        }
        else
        {
            f();
            recursiveParse(eq, k, f); // call recursively instead of vectorize
            return;
        }
    }

    // helper method for easy token advancement and
    // catching errors
    void advanceAndCheckToken(const TokenKind&);
    void advanceToken();
    std::string tokenToType(const TokenKind&, const std::string&);

    // some work to simplify all the expression parsing
    template <typename G>
    std::string
    simpleExpr(TokenKind tk, const std::string& symbol, const std::string& T0,
               const G& g)
    {
        advanceAndCheckToken(tk); // consume token
        targetBuffer += " " + symbol + " ";
        auto type = parseE0();

        return g(T0, type);
    }

    template <typename F, typename G>
    std::string
    parseExpr(const std::string& T0, const F& f, const G& g)
    {
        return f(T0);
    }

    template <typename F, typename G, typename T, typename... Rest>
    std::string
    parseExpr(const std::string& T0, const F& f, const G& g, T t, Rest... rest)
    {
        auto tk = std::get<0>(t);
        auto symbol = std::get<1>(t);

        if (currentToken->kind == tk)
            return simpleExpr(tk, symbol, T0, g);

        return parseExpr(T0, f, g, rest...);
    }

    // semantic checker methods
    void checkType(const std::string&, const std::string&);
    void checkUnique(const std::string&);
    void checkExists(const std::string&);
    void checkUDTExists(const std::string&);
    std::string checkFunctionCall(const std::string&,
                                  std::shared_ptr<SymbolTable>);
    std::string parseFunctionReturnType(const std::string& s);
    std::vector<std::string> parseFunctionInputTypes(const std::string&);

    // parse methods
    void parseProgram();
    void parseSource();
    void parseSourcePart();
    void parseImportInfo();
    std::string parseUDName();
    std::string parseLocation();
    void parseUDT();
    void parseUserDefinedType();
    void parseAttributes(std::shared_ptr<SymbolTable>);
    void parseMethods(std::shared_ptr<SymbolTable>);
    void parseScript();
    void parseFunctionDefinition();
    void parseFunctionInfo(const std::string&);
    std::string parseFunctionInOut(const std::string& name);
    void parseStart();
    std::string parseBlock();
    std::tuple<std::string, std::string> parseStatement();
    void parseTree();
    void parseBranch();
    void parseGrouping();
    std::string parseReturn();
    std::string parseDeclaration();
    std::string parseE0();
    std::string parseE1(const std::string&);
    std::string parseE2(const std::string&);
    std::string parseE3(const std::string&);
    std::string parseE4(const std::string&);
    std::string parseE5(const std::string&);
    std::string parseE6(const std::string&);
    std::string parseE7(const std::string&);
    std::string parseE8(const std::string&);
    std::string parseE9(const std::string&);
    std::string parseE10(const std::string&);
    std::string parseE11(const std::string&);
    std::string parseE12(const std::string&);
    std::string parseE13(const std::string&);
    std::string parseMemberAccess(const std::string&);
    std::string parseAttributeAccess(const std::string&, const std::string&);
    std::string parseMethodAccess(const std::string&, const std::string&);
    std::string parseFunctionCall();
    std::string parseNew();
    std::string parseUDTDec();
    std::string parseT();
    std::string parsePrimary();
    std::tuple<std::string, std::string> parseVariable();
    std::string parseType();
    std::string parseBoolean();
    std::string parseNumber();
    std::string parseInteger();
    std::string parseFloat();
    std::string parseString();
    std::string parseOwnAccessor();
    std::string parseEmpty();
    std::string parseIdentifier();
    std::string parseList();
    std::string parseListType();

  public:
    void parse();
    sailfishc(const std::string& file, bool shouldDisplayErrors);

    std::shared_ptr<SymbolTable>
    getSymbolTable()
    {
        return symboltable;
    }

    std::unique_ptr<UDTTable>
    getUDTTable()
    {
        return std::move(udttable);
    }

    bool
    getIsUDTFlag()
    {
        return isUdt;
    }

    std::string
    getTargetBuffer()
    {
        return targetBuffer;
    }

    void transpile();
    std::vector<std::shared_ptr<Error>> getErrors();
};
