#include "Parser2.h"

// -------- Parser Helper Code --------- //
void
Parser2::advanceAndCheckToken(const TokenKind& k)
{
    // first check value and kind
    if (currentToken->kind != k)
    {
        errorhandler->handle(
            new Error2(currentToken->col, currentToken->line,
                       "Expected a token of type: " + displayKind(k),
                       "Received: ", currentToken->value,
                       " of type " + displayKind(currentToken->kind) + "."));
    }
    currentToken = lexar->getNextToken();

    // catch errors from the lexar
    if (currentToken->kind == TokenKind::ERROR)
    {
        errorhandler->handle(new Error2(currentToken->col, currentToken->line,
                                        "Lexar Error.",
                                        "Error: ", currentToken->value, ""));
    }

    while (currentToken->kind == TokenKind::COMMENT ||
           currentToken->kind == TokenKind::COMMA)
    {
        currentToken = lexar->getNextToken();

        // catch errors from the lexar
        if (currentToken->kind == TokenKind::ERROR)
        {
            errorhandler->handle(
                new Error2(currentToken->col, currentToken->line,
                           "Lexar Error.", "Error: ", currentToken->value, ""));
        }
    }
}

// constructor
Parser2::Parser2(const std::string& filename)
{
    lexar = std::make_unique<Lexar2>(filename);
    currentToken = lexar->getNextToken();
    errorhandler = std::make_unique<Parser2ErrorHandler>(Parser2ErrorHandler());
}

// public interface method
std::shared_ptr<NodeLexeme>
Parser2::parse()
{
    return parseProgram();
}

// -------- Parser Parse Methods --------- //
/**
 * Program := Source
 */
std::shared_ptr<NodeLexeme>
Parser2::parseProgram()
{
    return parseSource();
}
/**
 * Source := Import Source | SourcePart
 */
std::shared_ptr<NodeLexeme>
Parser2::parseSource()
{
    auto imports = getChain(false, TokenKind::IMPORT, OP::IMPORT,
                            [this]() -> std::shared_ptr<NodeLexeme> {
                                return this->parseImportInfo();
                            });
    auto sourcepart = parseSourcePart();
    return makeNode(OP::SOURCE, imports, sourcepart);
}

/**
 * ImportInfo := UDName Location
 */
std::shared_ptr<NodeLexeme>
Parser2::parseImportInfo()
{
    advanceAndCheckToken(TokenKind::IMPORT); // eat 'Import'
    auto name = parseUDName();
    advanceAndCheckToken(TokenKind::COLON); // eat colon
    auto loc = parseLocation();
    return makeNode(OP::IMPORT, name, loc);
}

/**
 * UDName := Identifier
 */
std::shared_ptr<LeafLexeme>
Parser2::parseUDName()
{
    return parseIdentifier();
}

/**
 * Location := String
 */
std::shared_ptr<LeafLexeme>
Parser2::parseLocation()
{
    return parseString();
}

/**
 * SourcePart := UDT | UDA | Script
 */
std::shared_ptr<NodeLexeme>
Parser2::parseSourcePart()
{
    switch (currentToken->kind)
    {
    case TokenKind::UAT:
        return parseUDT();
    default:
        return parseScript();
    }
}

/**
 * UDT := UserDefinedType
 */
std::shared_ptr<NodeLexeme>
Parser2::parseUDT()
{
    return parseUserDefinedType();
}

/**
 * UserDefinedType := Attributes Methods
 */
std::shared_ptr<NodeLexeme>
Parser2::parseUserDefinedType()
{
    auto attributes = parseAttributes();
    auto methods = parseMethods();
    return makeNode(OP::UDT, attributes, methods);
}

/**
 * Attributes := [Identifier Identifier]*
 */
std::shared_ptr<NodeLexeme>
Parser2::parseAttributes()
{
    advanceAndCheckToken(TokenKind::UAT);     // consume uat
    advanceAndCheckToken(TokenKind::LCURLEY); // consume l curley
    auto topattribute = getChain(true, TokenKind::RCURLEY, OP::ATTRIBUTE,
                                 [this]() -> std::shared_ptr<NodeLexeme> {
                                     return this->parseVariable();
                                 });
    advanceAndCheckToken(TokenKind::RCURLEY); // consume r curley
    return makeNode(OP::ATTRIBUTE, topattribute);
}

/**
 * Methods := [FunctionDefinition]*
 */
std::shared_ptr<NodeLexeme>
Parser2::parseMethods()
{
    advanceAndCheckToken(TokenKind::UFN);     // consume ufn
    advanceAndCheckToken(TokenKind::LCURLEY); // consume l curley
    auto topmethod = getChain(true, TokenKind::RCURLEY, OP::METHOD,
                              [this]() -> std::shared_ptr<NodeLexeme> {
                                  return this->parseFunctionDefinition();
                              });
    advanceAndCheckToken(TokenKind::RCURLEY); // consume r curley
    return makeNode(OP::METHOD, topmethod);
}

std::shared_ptr<NodeLexeme>
Parser2::parseScript()
{
    return parseScript_();
}

std::shared_ptr<NodeLexeme>
Parser2::parseScript_()
{
    auto func = getChain(true, TokenKind::START, OP::FUNCTION,
                         [this]() -> std::shared_ptr<NodeLexeme> {
                             return this->parseFunctionDefinition();
                         });
    auto start = parseStart();
    return makeNode(OP::SCRIPT, func, start);
}

/**
 * FunctionDefinition := '(' 'fun' identifier FunctionInfo ')'
 */
std::shared_ptr<NodeLexeme>
Parser2::parseFunctionDefinition()
{
    advanceAndCheckToken(TokenKind::LPAREN); // consume l paren
    advanceAndCheckToken(TokenKind::FUN);    // consume fun
    auto id = parseIdentifier();
    auto info = parseFunctionInfo();
    advanceAndCheckToken(TokenKind::RPAREN); // consume r paren
    return makeNode(OP::FUNCTION, id, info);
}

/**
 * FunctionInfo := FunctionInOut Block
 */
std::shared_ptr<NodeLexeme>
Parser2::parseFunctionInfo()
{
    auto inout = parseFunctionInOut();
    auto block = parseBlock();
    return makeNode(OP::FUNCTION_INFO, inout, block);
}

/**
 * FunctionInOut := FunctionInputs* FunctionOutputs
 *  FunctionInputs :=  '(' Variable [',' Variable]* ')'
 */
std::shared_ptr<NodeLexeme>
Parser2::parseFunctionInOut()
{
    // inputs
    advanceAndCheckToken(TokenKind::LPAREN); // consume l paren
                                             // TODO: check for multiple voids
    auto topinput = getChain(true, TokenKind::RPAREN, OP::FUNCTION_INPUT,
                             [this]() -> std::shared_ptr<NodeLexeme> {
                                 return this->parseVariable();
                             });
    advanceAndCheckToken(TokenKind::RPAREN); // consume r paren

    // outputs
    advanceAndCheckToken(TokenKind::LPAREN); // consume l paren
    auto output = parseType();
    advanceAndCheckToken(TokenKind::RPAREN); // consume r paren
    return makeNode(OP::FUNCTION_IN_OUT, topinput, output);
}

/**
 * Start := 'start' Block
 */
std::shared_ptr<NodeLexeme>
Parser2::parseStart()
{
    advanceAndCheckToken(TokenKind::START);
    auto block = parseBlock();
    return makeNode(OP::START, makeLeaf(LIT::IDENTIFIER, "start"), block);
}

/**
 * Block = '{' Statement* '}'
 */
std::shared_ptr<NodeLexeme>
Parser2::parseBlock()
{
    advanceAndCheckToken(TokenKind::LCURLEY); // eat '{'
    auto topstatement =
        getChain(true, TokenKind::RCURLEY, OP::STATEMENT,
                 [this]() -> Lexeme { return this->parseStatement(); });
    std::shared_ptr<NodeLexeme> parseBlockRecurse();

    advanceAndCheckToken(TokenKind::RCURLEY); // eat '}'
    return makeNode(OP::BLOCK, topstatement);
}

/**
 * Statement := Tree | Return | Declaration | E0
 */
Lexeme
Parser2::parseStatement()
{
    switch (currentToken->kind)
    {
    case TokenKind::TREE:
        return parseTree();
    case TokenKind::RETURN:
        return parseReturn();
    case TokenKind::DEC:
        return parseDeclaration();
    default:
        return parseE0();
    }
}

/**
 * Tree := 'tree' (' Branch* ')'
 */
std::shared_ptr<NodeLexeme>
Parser2::parseTree()
{

    advanceAndCheckToken(TokenKind::TREE);   // eat 'tree'
    advanceAndCheckToken(TokenKind::LPAREN); // eat '('
    std::vector<std::shared_ptr<NodeLexeme>> branches;

    auto topbranch = getChain(true, TokenKind::RPAREN, OP::BRANCH,
                              [this]() -> std::shared_ptr<NodeLexeme> {
                                  return this->parseBranch();
                              });
    advanceAndCheckToken(TokenKind::RPAREN); // eat ')'
    return makeNode(OP::TREE, topbranch);
}

/**
 * Branch := '(' Grouping Block')'
 */
std::shared_ptr<NodeLexeme>
Parser2::parseBranch()
{
    advanceAndCheckToken(TokenKind::LPAREN); // eat '('
    auto grouping = parseGrouping();
    auto block = parseBlock();
    advanceAndCheckToken(TokenKind::RPAREN); // eat ')'
    return makeNode(OP::BRANCH, grouping, block);
}

/**
 * Grouping := '|' E0 '|'
 */
Lexeme
Parser2::parseGrouping()
{
    advanceAndCheckToken(TokenKind::PIPE); // eat '|'
    auto condition = parseE0();
    advanceAndCheckToken(TokenKind::PIPE); // eat '|'
    return condition;
}

/**
 * Return := 'return' T
 */
std::shared_ptr<NodeLexeme>
Parser2::parseReturn()
{
    advanceAndCheckToken(TokenKind::RETURN); // consume 'return'
    auto t = parseT();
    return makeNode(OP::RETURN, t);
}

/**
 * Declaration :=  'dec' Variable '=' E0
 */
std::shared_ptr<NodeLexeme>
Parser2::parseDeclaration()
{
    advanceAndCheckToken(TokenKind::DEC); // consume 'dec'
    auto var = parseVariable();
    advanceAndCheckToken(TokenKind::ASSIGNMENT); // consume '='
    auto e0 = parseE0();
    return makeNode(OP::DECLARATION, var, e0);
}

/**
 * E0 := T0 E1
 */
Lexeme
Parser2::parseE0()
{
    auto t0 = parseT();
    return parseE1(t0);
}

/**
 * E1 := ** E0 | E2
 */
Lexeme
Parser2::parseE1(Lexeme T0)
{
    if (currentToken->kind == TokenKind::EXPONENTIATION)
    {
        advanceAndCheckToken(TokenKind::EXPONENTIATION); // consume "**"
        auto e0 = parseE0();
        return makeNode(OP::EXPONENT, T0, e0);
    }
    return parseE2(T0);
}

/**
 * E2 := * E0 | / E0 | % E0 | E3
 */
Lexeme
Parser2::parseE2(Lexeme T1)
{
    if (currentToken->kind == TokenKind::MULTIPLICATION ||
        currentToken->kind == TokenKind::DIVISION ||
        currentToken->kind == TokenKind::MODULO)
    {
        switch (currentToken->kind)
        {
        case TokenKind::MULTIPLICATION:
        {
            advanceAndCheckToken(TokenKind::MULTIPLICATION); // consume "*"
            auto e0 = parseE0();
            return makeNode(OP::MULTIPLICATION, T1, e0);
        }
        case TokenKind::DIVISION:
        {
            advanceAndCheckToken(TokenKind::DIVISION); // consume "/"
            auto e0 = parseE0();
            return makeNode(OP::DIVISION, T1, e0);
        }
        case TokenKind::MODULO:
        {
            advanceAndCheckToken(TokenKind::MODULO); // consume "%"
            auto e0 = parseE0();
            return makeNode(OP::MODULO, T1, e0);
        }
        }
    }
    return parseE3(T1);
}

/**
 * E3 := + E0 | - E0 | E4
 */
Lexeme
Parser2::parseE3(Lexeme T2)
{
    if (currentToken->kind == TokenKind::ADDITION ||
        currentToken->kind == TokenKind::SUBTRACTION)
    {

        switch (currentToken->kind)
        {
        case TokenKind::ADDITION:
        {
            advanceAndCheckToken(TokenKind::ADDITION); // consume "+"
            auto e0 = parseE0();
            return makeNode(OP::ADDITION, T2, e0);
        }
        case TokenKind::SUBTRACTION:
        {
            advanceAndCheckToken(TokenKind::SUBTRACTION); // consume "-"
            auto e0 = parseE0();
            return makeNode(OP::SUBTRACTION, T2, e0);
        }
        }
    }
    return parseE4(T2);
}

/**
 * E4 := < E0 | > E0 | <= E0 | >= E0 | E5
 */
Lexeme
Parser2::parseE4(Lexeme T3)
{
    if (currentToken->kind == TokenKind::LESS_THAN ||
        currentToken->kind == TokenKind::LESS_THAN_OR_EQUALS ||
        currentToken->kind == TokenKind::GREATER_THAN ||
        currentToken->kind == TokenKind::GREATER_THAN_OR_EQUALS)
    {
        switch (currentToken->kind)
        {
        case TokenKind::LESS_THAN:
        {
            advanceAndCheckToken(TokenKind::LESS_THAN); // consume "<"
            auto e0 = parseE0();
            return makeNode(OP::LESS_THAN, T3, e0);
        }
        case TokenKind::LESS_THAN_OR_EQUALS:
        {
            advanceAndCheckToken(TokenKind::LESS_THAN_OR_EQUALS); // consume "<"
            auto e0 = parseE0();
            return makeNode(OP::LESS_THAN_OR_EQUALS, T3, e0);
        }
        case TokenKind::GREATER_THAN:
        {
            advanceAndCheckToken(TokenKind::GREATER_THAN); // consume ">"
            auto e0 = parseE0();
            return makeNode(OP::GREATER_THAN, T3, e0);
        }
        case TokenKind::GREATER_THAN_OR_EQUALS:
        {
            advanceAndCheckToken(
                TokenKind::GREATER_THAN_OR_EQUALS); // consume ">="
            auto e0 = parseE0();
            return makeNode(OP::GREATER_THAN_OR_EQUALS, T3, e0);
        }
        }
    }
    return parseE5(T3);
}

/**
 * E5 := == E0 | != E0 | E6
 */
Lexeme
Parser2::parseE5(Lexeme T4)
{
    if (currentToken->kind == TokenKind::EQUIVALENCE ||
        currentToken->kind == TokenKind::NONEQUIVALENCE)
    {

        switch (currentToken->kind)
        {
        case TokenKind::EQUIVALENCE:
        {
            advanceAndCheckToken(TokenKind::EQUIVALENCE); // consume "=="
            auto e0 = parseE0();
            return makeNode(OP::EQUIVALENCE, T4, e0);
        }
        case TokenKind::NONEQUIVALENCE:
        {
            advanceAndCheckToken(TokenKind::NONEQUIVALENCE); // consume "!="
            auto e0 = parseE0();
            return makeNode(OP::NONEQUIVALENCE, T4, e0);
        }
        }
    }
    return parseE6(T4);
}

/**
 * E6 := 'and' E0 | 'or' E0 | E7
 */
Lexeme
Parser2::parseE6(Lexeme T5)
{
    if (currentToken->kind == TokenKind::AND ||
        currentToken->kind == TokenKind::OR)
    {
        switch (currentToken->kind)
        {
        case TokenKind::AND:
        {
            advanceAndCheckToken(TokenKind::AND); // consume 'and'
            auto e0 = parseE0();
            return makeNode(OP::AND, T5, e0);
        }
        case TokenKind::OR:
        {
            advanceAndCheckToken(TokenKind::OR); // consume 'or'
            auto e0 = parseE0();
            return makeNode(OP::OR, T5, e0);
        }
        }
    }
    return parseE7(T5);
}

/**
 * E7 := = E0 | E8
 */
Lexeme
Parser2::parseE7(Lexeme T6)
{
    if (currentToken->kind == TokenKind::ASSIGNMENT)
    {
        advanceAndCheckToken(TokenKind::ASSIGNMENT); // consume "="
        auto e0 = parseE0();
        return makeNode(OP::ASSIGNMENT, T6, e0);
    }
    return parseE8(T6);
}

/**
 * E8 := [!, ++, --] E0 | E9
 */
Lexeme
Parser2::parseE8(Lexeme T7)
{
    if (currentToken->kind == TokenKind::NEGATION ||
        currentToken->kind == TokenKind::UNARYADD ||
        currentToken->kind == TokenKind::UNARYMINUS)
    {
        switch (currentToken->kind)
        {
        case TokenKind::NEGATION:
        {
            advanceAndCheckToken(TokenKind::NEGATION); // consume "!"
            auto e0 = parseE0();
            return makeNode(OP::NEGATION, T7, e0);
        }
        case TokenKind::UNARYADD:
        {
            advanceAndCheckToken(TokenKind::UNARYADD); // consume "++"
            auto e0 = parseE0();
            return makeNode(OP::UNARYADD, T7, e0);
        }
        case TokenKind::UNARYMINUS:
        {
            advanceAndCheckToken(TokenKind::UNARYMINUS); // consume "--"
            auto e0 = parseE0();
            return makeNode(OP::UNARYMINUS, T7, e0);
        }
        }
    }
    return parseE9(T7);
}

/**
 * E9 := [+=, -=, /=, *=] E0 | E10
 */
Lexeme
Parser2::parseE9(Lexeme T8)
{
    if (currentToken->kind == TokenKind::ADDTO ||
        currentToken->kind == TokenKind::SUBFROM ||
        currentToken->kind == TokenKind::DIVFROM ||
        currentToken->kind == TokenKind::MULTTO)
    {
        switch (currentToken->kind)
        {
        case TokenKind::ADDTO:
        {
            advanceAndCheckToken(TokenKind::ADDTO); // consume "+="
            auto e0 = parseE0();
            return makeNode(OP::ADDTO, T8, e0);
        }
        case TokenKind::SUBFROM:
        {
            advanceAndCheckToken(TokenKind::SUBFROM); // consume "-="
            auto e0 = parseE0();
            return makeNode(OP::SUBFROM, T8, e0);
        }
        case TokenKind::DIVFROM:
        {
            advanceAndCheckToken(TokenKind::DIVFROM); // consume "/="
            auto e0 = parseE0();
            return makeNode(OP::DIVFROM, T8, e0);
        }
        case TokenKind::MULTTO:
        {
            advanceAndCheckToken(TokenKind::MULTTO); // consume "*="
            auto e0 = parseE0();
            return makeNode(OP::MULTTO, T8, e0);
        }
        }
    }
    return parseE10(T8);
}

/**
 * E10 := MemberAccess E0 | E11
 */
Lexeme
Parser2::parseE10(Lexeme T9)
{
    if (currentToken->kind == TokenKind::DOT ||
        currentToken->kind == TokenKind::TRIPLE_DOT)
    {
        auto memberAccess = parseMemberAccess();
        auto e1 =
            parseE1(makeNullNode()); // like calling E0 where T0 does not exist
        return makeNode(OP::MEMBER, memberAccess, e1);
    }
    return parseE11(T9);
}

/**
 * E11 := New | E12
 */
Lexeme
Parser2::parseE11(Lexeme T10)
{
    if (currentToken->kind == TokenKind::NEW)
    {
        auto New = parseNew();
        return makeNode(OP::MEMBER, New);
    }
    return parseE12(T10);
}

/**
 * E12 := T12 | ε
 */
Lexeme
Parser2::parseE12(Lexeme T11)
{
    return T11;
}

/**
 * MemberAccess := AttributeAccess | MethodAccess
 */
std::shared_ptr<NodeLexeme>
Parser2::parseMemberAccess()
{
    switch (currentToken->kind)
    {
    case TokenKind::DOT:
        return parseAttributeAccess();
    case TokenKind::TRIPLE_DOT:
        return parseMethodAccess();
    default:
        errorhandler->handle(new Error2(
            currentToken->col, currentToken->line, "Expected a ... or . token.",
            "Received: ", currentToken->value,
            " of type " + displayKind(currentToken->kind) + "."));
        return makeNullNode(); //  unreachable
    }
}

/**
 * AttributeAccess := '.' Identifier
 */
std::shared_ptr<NodeLexeme>
Parser2::parseAttributeAccess()
{
    advanceAndCheckToken(TokenKind::DOT); // consume '.'
    auto attribute = parseIdentifier();
    return makeNode(OP::ATTRIBUTE_ACCESS, attribute);
}

/**
 * MethodAccess := '...' Identifier FunctionCall
 */
std::shared_ptr<NodeLexeme>
Parser2::parseMethodAccess()
{
    advanceAndCheckToken(TokenKind::TRIPLE_DOT); // consume '...'
    auto name = parseIdentifier();
    auto method = parseFunctionCall();
    return makeNode(OP::METHOD_ACCESS, method, name);
}

/**
 * FunctionCall := '(' [Identifier [',' Identifier]*] ')'
 */
std::shared_ptr<NodeLexeme>
Parser2::parseFunctionCall()
{
    advanceAndCheckToken(TokenKind::LPAREN); // consume l paren
    auto topinput = getChain(true, TokenKind::RPAREN, OP::INPUT,
                             [this]() -> std::shared_ptr<LeafLexeme> {
                                 return this->parseIdentifier();
                             });
    advanceAndCheckToken(TokenKind::RPAREN); // consume r paren
    return makeNode(OP::FUNCTION_CALL, topinput);
}

/**
 * New := UDTDec
 */
std::shared_ptr<NodeLexeme>
Parser2::parseNew()
{
    advanceAndCheckToken(TokenKind::NEW); // consume new
    switch (currentToken->kind)
    {
    case TokenKind::IDENTIFIER:
        return parseUDTDec();
    default:
        errorhandler->handle(
            new Error2(currentToken->col, currentToken->line,
                       "Expected a valid new declaration such as a UDT defined "
                       "by '{' '}'.",
                       "Received: ", currentToken->value,
                       " of type " + displayKind(currentToken->kind) + "."));
        return makeNullNode(); //  unreachable
    }
}

/**
 * UDTDec := Identifier '{' [UDTDecItem [',' UDTDecItem]*] '}'
 */
std::shared_ptr<NodeLexeme>
Parser2::parseUDTDec()
{
    auto name = parseIdentifier();
    advanceAndCheckToken(TokenKind::LCURLEY); // consume l curley
    auto topitem = getChain(true, TokenKind::RCURLEY, OP::UDTDECITEM,
                            [this]() -> std::shared_ptr<NodeLexeme> {
                                return this->parseUDTDecItem();
                            });
    advanceAndCheckToken(TokenKind::RCURLEY); // consume r curley
    return makeNode(OP::UDTDEC, name, topitem);
}

/**
 * UDTDecItem := Identifier ':' Primary
 */
std::shared_ptr<NodeLexeme>
Parser2::parseUDTDecItem()
{
    auto identifier = parseIdentifier();
    advanceAndCheckToken(TokenKind::COLON); // consume ':'
    auto primary = parsePrimary();
    return makeNode(OP::UDTDECITEM, identifier, primary);
}

/**
 * T := Primary | '(' E0 ')'
 */
Lexeme
Parser2::parseT()
{
    if (currentToken->kind == TokenKind::LPAREN)
    {
        advanceAndCheckToken(TokenKind::LPAREN); // consume l paren
        auto e0 = parseE0();
        advanceAndCheckToken(TokenKind::RPAREN); // consume r paren
        return e0;
    }

    else if (currentToken->kind == TokenKind::NEGATION ||
             currentToken->kind == TokenKind::UNARYADD ||
             currentToken->kind == TokenKind::UNARYMINUS ||
             currentToken->kind == TokenKind::NEW)
    {
        auto e1 = parseE1(makeNullNode()); // type less
        return e1;
    }

    auto primary = parsePrimary();
    return makeNode(OP::PRIMARY, primary);
}

/**
 * Primary := Bool | Integer |  String | Identifier
 */
std::shared_ptr<LeafLexeme>
Parser2::parsePrimary()
{
    switch (currentToken->kind)
    {
    case TokenKind::BOOL:
        return parseBoolean();
    case TokenKind::INTEGER:
    case TokenKind::FLOAT:
        return parseNumber();
    case TokenKind::STRING:
        return parseString();
    case TokenKind::IDENTIFIER:
        return parseIdentifier();
    case TokenKind::LIST:
        return parseList();
    default:
        errorhandler->handle(
            new Error2(currentToken->col, currentToken->line,
                       "Expected a valid primary token such as a boolean, "
                       "integer, float, string, identifier, or list.",
                       "Received: ", currentToken->value,
                       " of type " + displayKind(currentToken->kind) + "."));
        return parseIdentifier(); //  unreachable
    }
}

/**
 * Type := Identifier
 */
std::shared_ptr<LeafLexeme>
Parser2::parseType()
{
    if (currentToken->kind == TokenKind::LISTTYPE)
        return parseListType();
    return parseIdentifier();
}

/**
 * Variable := Type Identifier
 */
std::shared_ptr<NodeLexeme>
Parser2::parseVariable()
{
    auto type = parseType();

    // deal with void types
    if (type.get()->value == "void")
        return makeNode(OP::VARIABLE, type, type);

    auto name = parseIdentifier();
    return makeNode(OP::VARIABLE, type, name);
}

/**
 * Number := Integer | Float
 */
std::shared_ptr<LeafLexeme>
Parser2::parseNumber()
{
    auto v = currentToken->value;
    auto k = currentToken->kind;

    if (k == TokenKind::INTEGER)
    {
        advanceAndCheckToken(TokenKind::INTEGER); // eat integer
        return makeLeaf(LIT::INTEGER, v);
    }

    advanceAndCheckToken(TokenKind::FLOAT); // eat float
    return makeLeaf(LIT::FLOAT, v);
}

/**
 * Identifier := lexvalue
 */
std::shared_ptr<LeafLexeme>
Parser2::parseIdentifier()
{
    auto v = currentToken->value;
    advanceAndCheckToken(TokenKind::IDENTIFIER); // eat identifier
    return makeLeaf(LIT::IDENTIFIER, v);
}

/**
 * Bool := lexvalue
 */
std::shared_ptr<LeafLexeme>
Parser2::parseBoolean()
{
    auto v = currentToken->value;
    advanceAndCheckToken(TokenKind::BOOL); // eat identifier
    return makeLeaf(LIT::IDENTIFIER, v);
}

/**
 * String := lexvalue
 */
std::shared_ptr<LeafLexeme>
Parser2::parseString()
{
    auto v = currentToken->value;
    advanceAndCheckToken(TokenKind::STRING); // eat string
    return makeLeaf(LIT::STRING, v);
}

/**
 * List := lexvalue
 */
std::shared_ptr<LeafLexeme>
Parser2::parseList()
{
    auto v = currentToken->value;
    advanceAndCheckToken(TokenKind::LIST); // eat list
    return makeLeaf(LIT::LIST, v);
}

/**
 * ListType := lexvalue
 */
std::shared_ptr<LeafLexeme>
Parser2::parseListType()
{
    auto v = currentToken->value;
    advanceAndCheckToken(TokenKind::LISTTYPE); // eat list type
    return makeLeaf(LIT::LISTTYPE, v);
}