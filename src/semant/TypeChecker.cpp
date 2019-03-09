/*
 * Robert Durst 2019
 * Sailfish Programming Language
 */
#include "TypeChecker.h"

// ------- Ugly/Hacky Helper Functions ------- //

ast::Expression*
binaryExpressionHelper(ast::BinaryExpression* node)
{
    switch (node->getBinaryExpressionType())
    {
    case ast::BinaryExpression::Exponentiation:
    {
        ast::Exponentiation* subnode = dynamic_cast<ast::Exponentiation*>(node);
        return subnode->getLeftExpr();
    }
    case ast::BinaryExpression::Multiplication:
    {
        ast::Multiplication* subnode = dynamic_cast<ast::Multiplication*>(node);
        return subnode->getLeftExpr();
    }
    case ast::BinaryExpression::Division:
    {
        ast::Division* subnode = dynamic_cast<ast::Division*>(node);
        return subnode->getLeftExpr();
    }
    case ast::BinaryExpression::Modulo:
    {
        ast::Modulo* subnode = dynamic_cast<ast::Modulo*>(node);
        return subnode->getLeftExpr();
    }
    case ast::BinaryExpression::Addition:
    {
        ast::Addition* subnode = dynamic_cast<ast::Addition*>(node);
        return subnode->getLeftExpr();
    }
    case ast::BinaryExpression::Subtraction:
    {
        ast::Subtraction* subnode = dynamic_cast<ast::Subtraction*>(node);
        return subnode->getLeftExpr();
    }
    case ast::BinaryExpression::BinaryGreaterThan:
    {
        ast::BinaryGreaterThan* subnode =
            dynamic_cast<ast::BinaryGreaterThan*>(node);
        return subnode->getLeftExpr();
    }
    case ast::BinaryExpression::BinaryLessThan:
    {
        ast::BinaryLessThan* subnode = dynamic_cast<ast::BinaryLessThan*>(node);
        return subnode->getLeftExpr();
    }
    case ast::BinaryExpression::BinaryGreaterThanOrEqual:
    {
        ast::BinaryGreaterThanOrEqual* subnode =
            dynamic_cast<ast::BinaryGreaterThanOrEqual*>(node);
        return subnode->getLeftExpr();
    }
    case ast::BinaryExpression::BinaryLessThanOrEqual:
    {
        ast::BinaryLessThanOrEqual* subnode =
            dynamic_cast<ast::BinaryLessThanOrEqual*>(node);
        return subnode->getLeftExpr();
    }
    case ast::BinaryExpression::EquivalenceComparison:
    {
        ast::EquivalenceComparison* subnode =
            dynamic_cast<ast::EquivalenceComparison*>(node);
        return subnode->getLeftExpr();
    }
    case ast::BinaryExpression::NonEquivalenceComparison:
    {
        ast::NonEquivalenceComparison* subnode =
            dynamic_cast<ast::NonEquivalenceComparison*>(node);
        return subnode->getLeftExpr();
    }
    case ast::BinaryExpression::AndComparison:
    {
        ast::AndComparison* subnode = dynamic_cast<ast::AndComparison*>(node);
        return subnode->getLeftExpr();
    }
    case ast::BinaryExpression::OrComparison:
    {
        ast::OrComparison* subnode = dynamic_cast<ast::OrComparison*>(node);
        return subnode->getLeftExpr();
    }
    case ast::BinaryExpression::Assignment:
    {
        ast::Assignment* subnode = dynamic_cast<ast::Assignment*>(node);
        return subnode->getLeftExpr();
    }
    case ast::BinaryExpression::FunctionCallExpression:
    {
        ast::FunctionCall* subnode = dynamic_cast<ast::FunctionCall*>(node);
        return subnode->getExpr();
    }
    case ast::BinaryExpression::MemberAccess:
    {
        ast::MemberAccess* subnode = dynamic_cast<ast::MemberAccess*>(node);

        switch (subnode->getMemberAccessType())
        {
        case ast::MemberAccess::AttributeAccess:
        {
            ast::AttributeAccess* subsubnode =
                dynamic_cast<ast::AttributeAccess*>(subnode);
            return subsubnode->getExpression();
        }
        case ast::MemberAccess::MethodAccess:
        {
            ast::MethodAccess* subsubnode =
                dynamic_cast<ast::MethodAccess*>(subnode);
            return subsubnode->getExpression();
        }
        }
        break;
    }
    case ast::BinaryExpression::ExpressionOnlyStatement:
    {
        ast::ExpressionOnlyStatement* subnode =
            dynamic_cast<ast::ExpressionOnlyStatement*>(node);
        return subnode->getExpression();
    }
    }
}

std::string
primaryHelper(ast::Primary* primary)
{
    ast::Primary::PrimaryType type = primary->getPrimaryType();

    switch (type)
    {
    case ast::Primary::Identifier:
    {
        ast::Identifier* subnode = dynamic_cast<ast::Identifier*>(primary);
        return subnode->getValue();
    }
    case ast::Primary::StringLiteral:
    {
        ast::StringLiteral* subnode =
            dynamic_cast<ast::StringLiteral*>(primary);
        return "str";
    }
    case ast::Primary::BooleanLiteral:
    {
        ast::BooleanLiteral* subnode =
            dynamic_cast<ast::BooleanLiteral*>(primary);
        return "bool";
    }
    case ast::Primary::IntegerLiteral:
    {
        ast::IntegerLiteral* subnode =
            dynamic_cast<ast::IntegerLiteral*>(primary);
        return "int";
    }
    case ast::Primary::FloatLiteral:
    {
        ast::FloatLiteral* subnode = dynamic_cast<ast::FloatLiteral*>(primary);
        return "flt";
    }
    }
}

std::string
expressionHelper(ast::Expression* node, ErrorHandler* seh)
{
    switch (node->getExpressionType())
    {
    case ast::Expression::NewExpression:
    {
        ast::NewExpression* subnode = dynamic_cast<ast::NewExpression*>(node);

        switch (subnode->getNewVal()->getNewType())
        {
        case ast::New::DictionaryLiteral:
            return "dictionary";
        case ast::New::ListLiteral:
            return "list";
        case ast::New::UserDefinedType:
            return "udt";
        }
        break;
    }
    case ast::Expression::GroupingExpression:
    {
        ast::GroupingExpression* subnode =
            dynamic_cast<ast::GroupingExpression*>(node);

        seh->handle(new Error(subnode->getLineNum(),
                              "Unexpected grouping inside binary expression. "
                              "Groupings cannot be nested."));
        // return list type to continue semantic analysis
        return "list";
    }
    case ast::Expression::ArrayExpression:
    {
        ast::ArrayExpression* subnode =
            dynamic_cast<ast::ArrayExpression*>(node);

        seh->handle(new Error(subnode->getLineNum(),
                              "Unexpected list inside binary expression."));
        // return list type to continue semantic analysis
        return "list";
    }
    case ast::Expression::PrimaryExpression:
    {
        ast::PrimaryExpression* subnode =
            dynamic_cast<ast::PrimaryExpression*>(node);

        return primaryHelper(subnode->getPrimary());
    }
    case ast::Expression::UnaryExpression:
    {
        ast::UnaryExpression* subnode =
            dynamic_cast<ast::UnaryExpression*>(node);

        // must be a boolean so fine to return boolean here
        return "bool";
    }
    }
}

std::string
getRightExpressionType(ast::ExpressionStatement* node,
                       ErrorHandler* errorHandler)
{
    return expressionHelper(binaryExpressionHelper(node->getBinaryExpression()),
                            errorHandler);
}

// ------- END ------- //

// ------- Additions to the symbol table ------- //
void
TypeChecker::check(ast::Start* root)
{
    visit(root);
}

void
TypeChecker::visit(ast::NewVariableDefinition* node)
{
    bool isGood = true;

    // capture variable information to add to the symbol table
    ast::Variable* var = node->getVariable();
    std::string name = var->getName()->getValue();
    std::string type = var->getType()->getType();

    std::string adjustedName = "V" + type;

    // make sure the name is not a reserved word, a primitive name, or a UDT
    if (isPrimitive(name) || isKeyword(name) || udtTable->hasUDT(name))
    {
        semanticErrorHandler->handle(new Error(
            node->getLineNum(), "Declared variable named: " + name +
                                    " illegally shares its name with a "
                                    "type or a keyword/reserved word."));
    }

    // make sure the type is either primitive or a udt
    if (!isPrimitive(type) && !udtTable->hasUDT(type))
    {
        semanticErrorHandler->handle(new Error(
            node->getLineNum(), "Declared type: " + type +
                                    " for variable named: " + name +
                                    " is not a legal or known type."));
    }

    bool isUnique = symbolTable->addSymbol(name, adjustedName);

    // symbol table entry must be unique for current scope
    if (!isUnique)
    {
        symbolTableErrorHandler->handle(new Error(
            node->getLineNum(),
            "Invalid redecleration of variable with name: " + name + "."));
    }

    // visit expression
    visit(node->getExpressionStatement());

    std::string exprType = getRightExpressionType(
        node->getExpressionStatement(), semanticErrorHandler);

    // make sure the type of the assignment is the same as the declared type
    if (exprType != type)
    {
        semanticErrorHandler->handle(new Error(
            node->getLineNum(),
            "Declared type: " + type + " for variable named: " + name +
                " does not match assigned expression type of: " + exprType +
                "."));
    }
}

void
TypeChecker::visit(ast::ListDefinition* node)
{
    std::string name = node->getName()->getValue();
    std::string type = node->getType()->getType();

    std::string adjustedName = "L" + type;

    bool isUnique = symbolTable->addSymbol(name, adjustedName);

    // symbol table entry must be unique for current scope
    if (!isUnique)
    {
        symbolTableErrorHandler->handle(new Error(
            node->getLineNum(),
            "Invalid redecleration of list with name: " + name + "."));
    }

    // visit expression
    visit(node->getExpression());
}

void
TypeChecker::visit(ast::DictionaryDefinition* node)
{
    std::string name = node->getName()->getValue();
    std::string keyType = node->getKeyType()->getType();
    std::string valueType = node->getValueType()->getType();

    // make sure the name is not a reserved word, a primitive name, or a UDT
    if (isPrimitive(name) || isKeyword(name) || udtTable->hasUDT(name))
    {
        semanticErrorHandler->handle(new Error(
            node->getLineNum(), "Declared definition named: " + name +
                                    " illegally shares its name with a "
                                    "type or a keyword/reserved word."));
    }

    std::string adjustedName = "D" + keyType + "_" + valueType;

    bool isUnique = symbolTable->addSymbol(name, adjustedName);

    // symbol table entry must be unique for current scope
    if (!isUnique)
    {
        symbolTableErrorHandler->handle(new Error(
            node->getLineNum(),
            "Invalid redecleration of dictionary with name: " + name + "."));
    }

    // ensure the keyType exists, i.e. is a primitive or udt
    if (!isPrimitive(keyType) && !symbolTable->hasVariable(keyType))
    {
        semanticErrorHandler->handle(
            new Error(node->getName()->getLineNum(),
                      "Defined dictionary's key type of: " + keyType +
                          " for dictionary: " + name + " does not exist."));
    }

    // ensure the valueType exists, i.e. is a primitive or udt
    if (!isPrimitive(valueType) && !symbolTable->hasVariable(valueType))
    {
        semanticErrorHandler->handle(
            new Error(node->getName()->getLineNum(),
                      "Defined dictionary's value type of: " + valueType +
                          " for dictionary: " + name + " does not exist."));
    }

    // visit expression
    visit(node->getExpression());

    std::string exprType =
        expressionHelper(node->getExpression(), semanticErrorHandler);

    // make sure the type of the assignment is the same as the declared type
    // TODO: make sure that the dictionary expression key and values actually
    // match, not just that it is a dictionary
    if (exprType != "dictionary")
    {
        semanticErrorHandler->handle(new Error(
            node->getLineNum(),
            "Declared type of dictionary for variable named: " + name +
                " does not match assigned expression type of: " + exprType +
                "."));
    }
}

void
TypeChecker::visit(ast::FunctionDefinition* node)
{
    // if we hit errors, do not add it to the symbol table
    bool isGood = true;

    // ensure scope is global since functions cannot be in blocks
    if (!symbolTable->isGlobalScope())
    {
        symbolTableErrorHandler->handle(
            new Error(node->getName()->getLineNum(),
                      "Functions can only be declared at the global level, "
                      "i.e. cannot be nested!"));
    }

    std::string name = node->getName()->getValue();

    // make sure the name is not a reserved word, a primitive name, or a UDT
    if (isPrimitive(name) || isKeyword(name) || udtTable->hasUDT(name))
    {
        semanticErrorHandler->handle(new Error(
            node->getLineNum(), "Declared function named: " + name +
                                    " illegally shares its name with a "
                                    "type or a keyword/reserved word."));
    }

    std::string adjustedName = "F" + name + "(";

    //  capture for the addition of the function to the symbol table
    for (auto const& input : node->getInputList())
    {
        ast::Variable* var = input->getInput();
        std::string inp_type = var->getType()->getType();
        std::string inp_name = var->getName()->getValue();

        // ensure the type exists, i.e. is a primitive or udt
        if (!isPrimitive(inp_type) && !symbolTable->hasVariable(inp_type))
        {
            semanticErrorHandler->handle(
                new Error(node->getName()->getLineNum(),
                          "Functions input type of:  " + inp_type +
                              " for function: " + name + " does not exist."));
        }

        // make sure the name is not a reserved word, a primitive name, or a
        // UDT, however edge case where is void exists
        if (inp_name != "void" &&
            (isPrimitive(inp_name) || isKeyword(inp_name) ||
             udtTable->hasUDT(inp_name)))
        {
            semanticErrorHandler->handle(
                new Error(node->getLineNum(),
                          "Declared function input named: " + inp_name +
                              " illegally shares its name with a "
                              "type or a keyword/reserved word."));
        }

        adjustedName += "_" + inp_type;
    }

    adjustedName += ")(";

    // capture for the addition of the function to the symbol table
    for (auto const& output : node->getOutputList())
    {
        ast::Typename* var = output->getOutput();
        std::string out_type = var->getType();

        if (!isPrimitive(out_type) && !symbolTable->hasVariable(out_type))
        {
            semanticErrorHandler->handle(
                new Error(node->getName()->getLineNum(),
                          "Functions output type of:  " + out_type +
                              " for function: " + name + " does not exist."));
        }

        adjustedName += "_" + out_type;
    }

    adjustedName += ")";

    if (isGood)
    {
        bool isUnique = symbolTable->addSymbol(name, adjustedName);

        // symbol table entry must be unique for current scope
        if (!isUnique)
        {
            symbolTableErrorHandler->handle(new Error(
                node->getLineNum(),
                "Invalid redecleration of function with name: " + name + "."));
        }
    }

    // enter a scope for this function, starting with the parameters
    symbolTable->enterScope();

    // visit the function body
    visit(node->getBody());

    // exit scope once we exit the body
    symbolTable->exitScope();
}

void
TypeChecker::visit(ast::UserDefinedTypeDefinition* node)
{

    // ensure scope is global since functions cannot be in blocks
    if (!symbolTable->isGlobalScope())
    {
        symbolTableErrorHandler->handle(
            new Error(node->getAttributes()->getName()->getLineNum(),
                      "UDT's can only be declared at the global level, "
                      "i.e. cannot be nested!"));
    }

    ast::UserDefinedTypeAttributes* attributes = node->getAttributes();
    ast::UserDefinedTypeMethods* methods = node->getMethods();

    // capture the name
    std::string udt_name = attributes->getName()->getValue();

    // create a symbol table for the attributes
    SymbolTable* st_a = new SymbolTable();

    // temporarilly set the symbolTable field to the attribute one so that I can
    // properly create this
    SymbolTable* tempTable = symbolTable;
    symbolTable = st_a;

    for (auto const& var : attributes->getAttributes())
    {
        std::string name = var->getName()->getValue();
        std::string type = var->getType()->getType();

        std::string adjustedName = "V" + type;

        bool isUnique = symbolTable->addSymbol(name, adjustedName);

        // symbol table entry must be unique for current scope
        if (!isUnique)
        {
            symbolTableErrorHandler->handle(new Error(
                node->getLineNum(),
                "Invalid redecleration of method with name: " + name + "."));
        }
    }

    // capture the decorated symbol table and reset the class field back
    st_a = symbolTable;
    symbolTable = tempTable;

    // create a symbol table for the methods
    SymbolTable* st_m = new SymbolTable();

    // temporarilly set the symbolTable field to the attribute one so that I can
    // properly create this
    tempTable = symbolTable;
    symbolTable = st_m;

    for (auto const& func : methods->getMethods())
    {
        visit(func);
    }

    // capture the decorated symbol table and reset the class field back
    st_m = symbolTable;
    symbolTable = tempTable;

    std::string adjustedName = "U" + udt_name;

    // add to both symbol table and udt table
    bool isUnique = symbolTable->addSymbol(udt_name, adjustedName);

    // symbol table entry must be unique for current scope
    if (!isUnique)
    {
        symbolTableErrorHandler->handle(new Error(
            node->getLineNum(),
            "Invalid redecleration of udt with name: " + udt_name + "."));
    }

    isUnique = udtTable->addUDT(udt_name, st_a, st_m);

    // udt table entries MUST be unique unlike symbol table (for now)
    if (!isUnique)
    {
        symbolTableErrorHandler->handle(new Error(
            node->getLineNum(),
            "Invalid redecleration of udt with name: " + udt_name + "."));
    }
}

// ------- END ------- //

void
TypeChecker::visit(ast::InitialExecutionBody* node)
{
    // enter and exit scope for initial execution body
    symbolTable->enterScope();
    visit(node->getBody());
    symbolTable->exitScope();
}

void
TypeChecker::visit(ast::IfStatement* node)
{
    visit(node->getIfConditional());

    // enter and exit scope for if statement body
    symbolTable->enterScope();
    visit(node->getIfStatements());
    symbolTable->exitScope();

    // enter and exit scope for else statement body
    symbolTable->enterScope();
    visit(node->getElseStatements());
    symbolTable->exitScope();
}

void
TypeChecker::visit(ast::Negation* node)
{
    visit(node->getExpressionStatement());

    // ensure that all negations are boolean
    std::string type = getRightExpressionType(node->getExpressionStatement(),
                                              semanticErrorHandler);
    if (type != "Boolean")
    {
        semanticErrorHandler->handle(new Error(
            node->getLineNum(),
            "Expected boolean type following negation. Instead received: " +
                type + "."));
    }
}

void
TypeChecker::visit(ast::BinaryGreaterThan* node)
{
    visit(node->getLeftExpr());
    visit(node->getRightExpr());

    std::string lType =
        expressionHelper(node->getLeftExpr(), semanticErrorHandler);
    std::string rType =
        getRightExpressionType(node->getRightExpr(), semanticErrorHandler);

    if (lType != rType)
    {
        semanticErrorHandler->handle(
            new Error(node->getLineNum(),
                      "Expected the same types on each side of greater than "
                      "comparison. Instead recevied: " +
                          lType + " and " + rType + "."));
    }
    else if (lType != "Integer" && lType != "Float")
    {
        semanticErrorHandler->handle(new Error(
            node->getLineNum(), "Expected either float or integer type on left "
                                "side of greater than "
                                "comparison. Instead received: " +
                                    lType + "."));
    }
    else if (rType != "Integer" && rType != "Float")
    {
        semanticErrorHandler->handle(
            new Error(node->getLineNum(),
                      "Expected boolean type on right side of greater than "
                      "comparison. Instead received: " +
                          rType + "."));
    }
}
void
TypeChecker::visit(ast::BinaryLessThan* node)
{
    visit(node->getLeftExpr());
    visit(node->getRightExpr());

    std::string lType =
        expressionHelper(node->getLeftExpr(), semanticErrorHandler);
    std::string rType =
        getRightExpressionType(node->getRightExpr(), semanticErrorHandler);

    if (lType != rType)
    {
        semanticErrorHandler->handle(
            new Error(node->getLineNum(),
                      "Expected the same types on each side of less than "
                      "comparison. Instead recevied: " +
                          lType + " and " + rType + "."));
    }
    else if (lType != "Integer" && lType != "Float")
    {
        semanticErrorHandler->handle(new Error(
            node->getLineNum(), "Expected either float or integer type on left "
                                "side of less than "
                                "comparison. Instead received: " +
                                    lType + "."));
    }
    else if (rType != "Integer" && rType != "Float")
    {
        semanticErrorHandler->handle(
            new Error(node->getLineNum(),
                      "Expected boolean type on right side of less than "
                      "comparison. Instead received: " +
                          rType + "."));
    }
}
void
TypeChecker::visit(ast::BinaryGreaterThanOrEqual* node)
{
    visit(node->getLeftExpr());
    visit(node->getRightExpr());

    std::string lType =
        expressionHelper(node->getLeftExpr(), semanticErrorHandler);
    std::string rType =
        getRightExpressionType(node->getRightExpr(), semanticErrorHandler);

    if (lType != rType)
    {
        semanticErrorHandler->handle(new Error(
            node->getLineNum(),
            "Expected the same types on each side of greater than or equal to "
            "comparison. Instead recevied: " +
                lType + " and " + rType + "."));
    }
    else if (lType != "Integer" && lType != "Float")
    {
        semanticErrorHandler->handle(new Error(
            node->getLineNum(), "Expected either float or integer type on left "
                                "side of greater than or equal to "
                                "comparison. Instead received: " +
                                    lType + "."));
    }
    else if (rType != "Integer" && rType != "Float")
    {
        semanticErrorHandler->handle(new Error(
            node->getLineNum(),
            "Expected boolean type on right side of greater than or equal to "
            "comparison. Instead received: " +
                rType + "."));
    }
}
void
TypeChecker::visit(ast::BinaryLessThanOrEqual* node)
{
    visit(node->getLeftExpr());
    visit(node->getRightExpr());

    std::string lType =
        expressionHelper(node->getLeftExpr(), semanticErrorHandler);
    std::string rType =
        getRightExpressionType(node->getRightExpr(), semanticErrorHandler);

    if (lType != rType)
    {
        semanticErrorHandler->handle(new Error(
            node->getLineNum(),
            "Expected the same types on each side of less than or equal to "
            "comparison. Instead recevied: " +
                lType + " and " + rType + "."));
    }
    else if (lType != "Integer" && lType != "Float")
    {
        semanticErrorHandler->handle(new Error(
            node->getLineNum(), "Expected either float or integer type on left "
                                "side of less than or equal to "
                                "comparison. Instead received: " +
                                    lType + "."));
    }
    else if (rType != "Integer" && rType != "Float")
    {
        semanticErrorHandler->handle(new Error(
            node->getLineNum(),
            "Expected boolean type on right side of less than or equal to "
            "comparison. Instead received: " +
                rType + "."));
    }
}
void
TypeChecker::visit(ast::EquivalenceComparison* node)
{
    visit(node->getLeftExpr());
    visit(node->getRightExpr());

    std::string lType =
        expressionHelper(node->getLeftExpr(), semanticErrorHandler);
    std::string rType =
        getRightExpressionType(node->getRightExpr(), semanticErrorHandler);

    if (lType != rType)
    {
        semanticErrorHandler->handle(
            new Error(node->getLineNum(),
                      "Expected the same types on each side of equivalence "
                      "comparison. Instead recevied: " +
                          lType + " and " + rType + "."));
    }
}
void
TypeChecker::visit(ast::NonEquivalenceComparison* node)
{
    visit(node->getLeftExpr());
    visit(node->getRightExpr());

    std::string lType =
        expressionHelper(node->getLeftExpr(), semanticErrorHandler);
    std::string rType =
        getRightExpressionType(node->getRightExpr(), semanticErrorHandler);

    if (lType != rType)
    {
        semanticErrorHandler->handle(
            new Error(node->getLineNum(),
                      "Expected the same types on each side of nonequivalence "
                      "comparison. Instead recevied: " +
                          lType + " and " + rType + "."));
    }
}
void
TypeChecker::visit(ast::AndComparison* node)
{
    visit(node->getLeftExpr());
    visit(node->getRightExpr());

    std::string lType =
        expressionHelper(node->getLeftExpr(), semanticErrorHandler);
    std::string rType =
        getRightExpressionType(node->getRightExpr(), semanticErrorHandler);

    if (lType != "Boolean")
    {
        semanticErrorHandler->handle(new Error(
            node->getLineNum(), "Expected boolean type on left side of and "
                                "comparison. Instead received: " +
                                    lType + "."));
    }

    else if (rType != "Boolean")
    {
        semanticErrorHandler->handle(new Error(
            node->getLineNum(), "Expected boolean type on right side of and "
                                "comparison. Instead received: " +
                                    rType + "."));
    }
}
void
TypeChecker::visit(ast::OrComparison* node)
{
    visit(node->getLeftExpr());
    visit(node->getRightExpr());

    std::string lType =
        expressionHelper(node->getLeftExpr(), semanticErrorHandler);
    std::string rType =
        getRightExpressionType(node->getRightExpr(), semanticErrorHandler);

    if (lType != "Boolean")
    {
        semanticErrorHandler->handle(new Error(
            node->getLineNum(), "Expected boolean type on left side of or "
                                "comparison. Instead received: " +
                                    lType + "."));
    }

    else if (rType != "Boolean")
    {
        semanticErrorHandler->handle(new Error(
            node->getLineNum(), "Expected boolean type on right side of or "
                                "comparison. Instead received: " +
                                    rType + "."));
    }
}

void
TypeChecker::visit(ast::Exponentiation* node)
{
    visit(node->getLeftExpr());
    visit(node->getRightExpr());

    std::string lType =
        expressionHelper(node->getLeftExpr(), semanticErrorHandler);
    std::string rType =
        getRightExpressionType(node->getRightExpr(), semanticErrorHandler);

    if (lType != "Integer")
    {
        semanticErrorHandler->handle(new Error(
            node->getLineNum(), "Integer type on left "
                                "side of exponentiation. Instead received: " +
                                    lType + "."));
    }
    else if (rType != "Integer")
    {
        semanticErrorHandler->handle(new Error(
            node->getLineNum(), "Integer type on left "
                                "side of exponentiation. Instead received: " +
                                    rType + "."));
    }
}
void
TypeChecker::visit(ast::Multiplication* node)
{
    visit(node->getLeftExpr());
    visit(node->getRightExpr());

    std::string lType =
        expressionHelper(node->getLeftExpr(), semanticErrorHandler);
    std::string rType =
        getRightExpressionType(node->getRightExpr(), semanticErrorHandler);

    if (lType != rType)
    {
        semanticErrorHandler->handle(new Error(
            node->getLineNum(), "Expected the same types on each side of "
                                "multiplication. Instead recevied: " +
                                    lType + " and " + rType + "."));
    }
    else if (lType != "Integer" && lType != "Float")
    {
        semanticErrorHandler->handle(new Error(
            node->getLineNum(), "Expected either float or integer type on left "
                                "side of multiplication. Instead received: " +
                                    lType + "."));
    }
    else if (rType != "Integer" && rType != "Float")
    {
        semanticErrorHandler->handle(new Error(
            node->getLineNum(), "Expected either float or integer type on left "
                                "side of multiplication. Instead received: " +
                                    rType + "."));
    }
}
void
TypeChecker::visit(ast::Division* node)
{
    visit(node->getLeftExpr());
    visit(node->getRightExpr());

    std::string lType =
        expressionHelper(node->getLeftExpr(), semanticErrorHandler);
    std::string rType =
        getRightExpressionType(node->getRightExpr(), semanticErrorHandler);

    if (lType != rType)
    {
        semanticErrorHandler->handle(new Error(
            node->getLineNum(), "Expected the same types on each side of "
                                "division. Instead recevied: " +
                                    lType + " and " + rType + "."));
    }
    else if (lType != "Integer" && lType != "Float")
    {
        semanticErrorHandler->handle(new Error(
            node->getLineNum(), "Expected either float or integer type on left "
                                "side of division. Instead received: " +
                                    lType + "."));
    }
    else if (rType != "Integer" && rType != "Float")
    {
        semanticErrorHandler->handle(new Error(
            node->getLineNum(), "Expected either float or integer type on left "
                                "side of division. Instead received: " +
                                    rType + "."));
    }
}
void
TypeChecker::visit(ast::Modulo* node)
{
    visit(node->getLeftExpr());
    visit(node->getRightExpr());

    std::string lType =
        expressionHelper(node->getLeftExpr(), semanticErrorHandler);
    std::string rType =
        getRightExpressionType(node->getRightExpr(), semanticErrorHandler);

    if (lType != "Integer")
    {
        semanticErrorHandler->handle(
            new Error(node->getLineNum(), "Integer type on left "
                                          "side of modulo. Instead received: " +
                                              lType + "."));
    }
    else if (rType != "Integer")
    {
        semanticErrorHandler->handle(
            new Error(node->getLineNum(), "Integer type on left "
                                          "side of modulo. Instead received: " +
                                              rType + "."));
    }
}
void
TypeChecker::visit(ast::Addition* node)
{
    visit(node->getLeftExpr());
    visit(node->getRightExpr());

    std::string lType =
        expressionHelper(node->getLeftExpr(), semanticErrorHandler);
    std::string rType =
        getRightExpressionType(node->getRightExpr(), semanticErrorHandler);

    if (lType != rType)
    {
        semanticErrorHandler->handle(new Error(
            node->getLineNum(), "Expected the same types on each side of "
                                "addition. Instead recevied: " +
                                    lType + " and " + rType + "."));
    }
    else if (lType != "Integer" && lType != "Float")
    {
        semanticErrorHandler->handle(new Error(
            node->getLineNum(), "Expected either float or integer type on left "
                                "side of addition. Instead received: " +
                                    lType + "."));
    }
    else if (rType != "Integer" && rType != "Float")
    {
        semanticErrorHandler->handle(new Error(
            node->getLineNum(), "Expected either float or integer type on left "
                                "side of addition. Instead received: " +
                                    rType + "."));
    }
}
void
TypeChecker::visit(ast::Subtraction* node)
{
    visit(node->getLeftExpr());
    visit(node->getRightExpr());

    std::string lType =
        expressionHelper(node->getLeftExpr(), semanticErrorHandler);
    std::string rType =
        getRightExpressionType(node->getRightExpr(), semanticErrorHandler);

    if (lType != rType)
    {
        semanticErrorHandler->handle(new Error(
            node->getLineNum(), "Expected the same types on each side of "
                                "subtraction. Instead recevied: " +
                                    lType + " and " + rType + "."));
    }
    else if (lType != "Integer" && lType != "Float")
    {
        semanticErrorHandler->handle(new Error(
            node->getLineNum(), "Expected either float or integer type on left "
                                "side of subtraction. Instead received: " +
                                    lType + "."));
    }
    else if (rType != "Integer" && rType != "Float")
    {
        semanticErrorHandler->handle(new Error(
            node->getLineNum(), "Expected either float or integer type on left "
                                "side of subtraction. Instead received: " +
                                    rType + "."));
    }
}

void
TypeChecker::visit(ast::ExportDefinition* node)
{
    // ensure scope is global since functions cannot be in blocks
    if (!symbolTable->isGlobalScope())
    {
        symbolTableErrorHandler->handle(new Error(
            node->getLineNum(),
            "Export definitions can only be declared at the global level, "
            "i.e. cannot be nested!"));
    }

    ast::Exportable* exprt = node->getExport();
    ast::Exportable::ExportableType type = exprt->getExportableType();
    switch (type)
    {
    case ast::Exportable::GeneralDecleration:
    {
        ast::GeneralDecleration* subnode =
            dynamic_cast<ast::GeneralDecleration*>(exprt);
        visit(subnode);
        break;
    }
    case ast::Exportable::FunctionDefinition:
    {
        ast::FunctionDefinition* subnode =
            dynamic_cast<ast::FunctionDefinition*>(exprt);
        visit(subnode);
        break;
    }
    }
}
