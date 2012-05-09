/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "berryWithExpression.h"

#include "berryExpressions.h"
#include "berryEvaluationContext.h"

#include <berryIConfigurationElement.h>
#include <berryPlatformException.h>

namespace berry {

const QString WithExpression::ATT_VARIABLE= "variable";

const uint WithExpression::HASH_INITIAL= qHash("berry::WithExpression");

WithExpression::WithExpression(const IConfigurationElement::Pointer& configElement)
{
  fVariable = configElement->GetAttribute(ATT_VARIABLE);
  Expressions::CheckAttribute(ATT_VARIABLE, fVariable);
}

WithExpression::WithExpression(Poco::XML::Element* element)
{
  fVariable = QString::fromStdString(element->getAttribute(ATT_VARIABLE.toStdString()));
  Expressions::CheckAttribute(ATT_VARIABLE, fVariable.length() > 0 ? fVariable : QString());
}

WithExpression::WithExpression(const QString& variable)
 : fVariable(variable)
{

}

bool
WithExpression::operator==(const Object* object) const
{
  if (const WithExpression* that = dynamic_cast<const WithExpression*>(object))
  {
    return this->fVariable == that->fVariable &&
            this->Equals(this->fExpressions, that->fExpressions);
  }
  return false;
}

EvaluationResult::ConstPointer
WithExpression::Evaluate(IEvaluationContext* context) const
{
  Object::Pointer variable(context->GetVariable(fVariable));
  if (variable.IsNull())
  {
    throw CoreException(QString("Variable not defined: ") + fVariable);
  }
  EvaluationContext::Pointer scope(new EvaluationContext(context, variable));
  return this->EvaluateAnd(scope.GetPointer());
}

void
WithExpression::CollectExpressionInfo(ExpressionInfo* info)
{
  ExpressionInfo* other = new ExpressionInfo();
  CompositeExpression::CollectExpressionInfo(other);
  if (other->HasDefaultVariableAccess())
  {
    info->AddVariableNameAccess(fVariable);
  }
  info->MergeExceptDefaultVariable(other);
}

uint
WithExpression::ComputeHashCode()
{
  return HASH_INITIAL * HASH_FACTOR + this->HashCode(fExpressions)
  * HASH_FACTOR + qHash(fVariable);
}

}
