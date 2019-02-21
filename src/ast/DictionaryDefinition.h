/*
 * Robert Durst 2019
 * Sailfish Programming Language
 */
#pragma once
#include "Expression.h"
#include "GeneralDefinition.h"
#include "Identifier.h"

namespace ast
{
class DictionaryDefinition : public GeneralDefinition
{
  private:
    Identifier* name;
    Expression* expr;

  public:
    // constructor
    DictionaryDefinition(Identifier* i, Expression* e)
    {
        name = i;
        expr = e;
    };
    // implement GeneralDefinition
    /* virtual */ GeneralDefinitionType
    getGeneralDefinitionType()
    {
        return GeneralDefinitionType::DictionaryDefinition;
    }
};
}