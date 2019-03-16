/*
 * Robert Durst 2019
 * Sailfish Programming Language
 */
#pragma once
#include "../ast/Ast.h"
#include "../lexar/Lexar.h"
#include "../parser/Parser.h"
#include "../semantics/SemanticAnalyzer.h"
#include "../transpiler/Transpiler.h"
#include <iostream>
#include <stdlib.h>
#include <string>

int handleCommandLine(int, char* const* argv);