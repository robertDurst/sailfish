/*
 * Robert Durst 2019
 * Sailfish Programming Language
 */
#pragma once
#include "../errorhandler/Error2.h"
#include "../errorhandler/Parser2ErrorHandler.h"
#include "../errorhandler/SemanticAnalyzerErrorHandler.h"
#include "../lexar/Lexar2.h"
#include "../lexar/Token2.h"
#include "../semantics/SymbolTable.h"
#include "../semantics/UDTTable.h"
#include "../stdlib_c/stdlib_c.h"
#include "Lexeme.h"
#include <cstdarg>
#include <deque>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <tuple>
#include <variant>
#include <vector>

using LSandFlag = std::tuple<Lexeme, std::string, std::string>;
using SandS = std::tuple<std::string, std::string>;
using LandS = std::tuple<Lexeme, std::string>;
using LandV = std::tuple<Lexeme, std::variant<LIT, OP>>;
using LandSt = std::tuple<Lexeme, std::shared_ptr<SymbolTable>>;
using LSandS = std::tuple<Lexeme, std::string, std::string>;
using UdtFlagAndBufer =
    std::tuple<std::unique_ptr<UDTTable>, bool, std::string>;

class Parser2
{
  private:
    std::unique_ptr<Lexar2> lexar;
    std::unique_ptr<Token2> currentToken;
    std::unique_ptr<Parser2ErrorHandler> errorhandler;
    std::unique_ptr<SemanticAnalyzerErrorHandler> semanticerrorhandler;
    std::shared_ptr<SymbolTable> symboltable;
    std::unique_ptr<UDTTable> udttable;
    std::string filename;
    bool isUdt;
    bool inGrouping = false;
    std::string decName = "";
    int currentTabs = 0;
    std::string udtBuffer = "";
    std::string methodAccessName = "";

    // ------- Transpiler -------- //
    const std::string OUTPUT_HEADER =
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
    std::ofstream output;
    // some transpiler helpers
    std::string getTabs();

    // helper for simplifying redundancy of recursive loops
    template <typename F>
    NodePtr
    getChain(bool eq, TokenKind k, OP op, const F& f)
    {
        if (((currentToken->kind == k) && (eq)) ||
            ((currentToken->kind != k) && (!eq)))
        {
            return makeNullNode();
        }
        else if (currentToken->kind == TokenKind::EOF_)
        {
            errorhandler->handle(std::make_unique<Error2>(
                Error2(currentToken->col, currentToken->line,
                       "Unexpected end of file.",
                       "Expected to receive an end of token delimiter such "
                       "as '(' or '}'",
                       "", "")));
            return makeNullNode(); // unreachable
        }
        else
        {
            auto childL = f();
            auto childR =
                getChain(eq, k, op, f); // call recursively instead of vectorize
            return makeNode(op, std::move(childL), std::move(childR));
        }
    }

    // helper method for easy token advancement and
    // catching errors
    void advanceAndCheckToken(const TokenKind&);
    LIT kindToLIT(TokenKind);
    std::string tokenToType(const TokenKind&, const std::string&);

    // some work to simplify all the expression parsing
    template <typename G>
    LandS
    simpleExpr(TokenKind tk, OP op, const std::string& symbol,
               const std::string& T0, const G& g)
    {
        advanceAndCheckToken(tk); // consume token
        output << " " + symbol + " ";
        udtBuffer += " " + symbol + " ";
        auto a = parseE0();
        auto e0 = std::move(std::get<0>(a));
        auto type = std::get<1>(a);

        auto actualType = g(T0, type);

        return std::make_tuple(makeNode(op, T0, std::move(e0)), actualType);
    }

    template <typename F, typename G>
    LandS
    parseExpr(const std::string& T0, const F& f, const G& g)
    {

        auto b = f(T0);
        auto t0 = std::move(std::get<0>(b));
        auto type = std::get<1>(b);

        return std::make_tuple(std::move(t0), type);
    }

    template <typename F, typename G, typename T, typename... Rest>
    LandS
    parseExpr(const std::string& T0, const F& f, const G& g, T t, Rest... rest)
    {
        auto tk = std::get<0>(t);
        auto op = std::get<1>(t);
        auto symbol = std::get<2>(t);

        if (currentToken->kind == tk)
            return simpleExpr(tk, op, symbol, T0, g);

        return parseExpr(T0, f, g, rest...);
    }

    // extracts type from lexemes
    LandS extractType(Lexeme l);

    // semantic checker methods
    void checkType(const std::string&, const std::string&);
    void checkUnique(const std::string&);
    void checkExists(const std::string&);
    void checkUDTExists(const std::string&);
    LandS checkFunctionCall(const std::string&, std::shared_ptr<SymbolTable>);
    std::string parseFunctionReturnType(const std::string& s);
    std::vector<std::string> parseFunctionInputTypes(const std::string&);

    // parse methods
    NodePtr parseProgram();
    NodePtr parseSource();
    NodePtr parseSourcePart();
    NodePtr parseImportInfo();
    LeafPtr parseUDName();
    LeafPtr parseLocation();
    NodePtr parseUDT();
    NodePtr parseUserDefinedType();
    LandSt parseAttributes(std::shared_ptr<SymbolTable>);
    LandSt parseMethods(std::shared_ptr<SymbolTable>);
    NodePtr parseMethodsRecurse();
    NodePtr parseScript();
    NodePtr parseScript_();
    NodePtr parseFunctionDefinition();
    LandS parseFunctionInfo(const std::string&);
    LandS parseFunctionInOut(const std::string& name);
    NodePtr parseStart();
    LandS parseBlock();
    LSandFlag parseStatement();
    NodePtr parseTree();
    NodePtr parseBranch();
    Lexeme parseGrouping();
    LandS parseReturn();
    LandS parseDeclaration();
    LandS parseE0();
    LandS parseE1(const std::string&);
    LandS parseE2(const std::string&);
    LandS parseE3(const std::string&);
    LandS parseE4(const std::string&);
    LandS parseE5(const std::string&);
    LandS parseE6(const std::string&);
    LandS parseE7(const std::string&);
    LandS parseE8(const std::string&);
    LandS parseE9(const std::string&);
    LandS parseE10(const std::string&);
    LandS parseE11(const std::string&);
    LandS parseE12(const std::string&);
    LandS parseE13(const std::string&);
    LandS parseMemberAccess(const std::string&);
    LandS parseAttributeAccess(const std::string&, const std::string&);
    LandS parseMethodAccess(const std::string&, const std::string&);
    LandS parseFunctionCall();
    LandS parseNew();
    LandS parseUDTDec();
    LandS parseT();
    LandS parsePrimary();
    LSandS parseVariable();
    LeafPtr parseType();
    LeafPtr parseBoolean();
    LeafPtr parseNumber();
    LeafPtr parseInteger();
    LeafPtr parseFloat();
    LeafPtr parseString();
    LeafPtr parseOwnAccessor();
    LeafPtr parseEmpty();
    LeafPtr parseIdentifier();
    LandS parseList();
    std::tuple<LeafPtr, std::string> parseListType();

    // helper method for retreiving the Leaf/Node ptr from a Lexeme variant
    template <typename F>
    void
    lexemeIt(Lexeme l, F f)
    {
        auto i = l.index();
        if (i == 0)
            postorder(std::move(std::get<LeafPtr>(l)), f);
        else if (i == 1)
            postorder(std::move(std::get<NodePtr>(l)), f);
    }

  public:
    Parser2(const std::string& filename);
    NodePtr parse();

    template <typename F>
    void
    postorder(NodePtr n, F f)
    {
        if (n != nullptr && n->op != OP::NULL_VAL)
        {
            // root
            f(disp(n->op));
            // left
            lexemeIt(std::move(n->left), f);
            // right
            lexemeIt(std::move(n->right), f);
        }
    }

    template <typename F>
    void
    postorder(LeafPtr n, F f)
    {
        f(n->value);
    }

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
    getUDTBuffer()
    {
        return udtBuffer;
    }

    void transpile();
};
