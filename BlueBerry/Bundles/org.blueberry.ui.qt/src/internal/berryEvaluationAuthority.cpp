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


#include "berryEvaluationAuthority.h"

#include "berryExpressionInfo.h"
#include "berryExpression.h"
#include "berryEvaluationReference.h"
#include "berryWorkbenchPlugin.h"

#include <berryObjects.h>
#include <berryISources.h>
#include <berryIEvaluationService.h>

#include <QStringList>
#include <QSet>

namespace berry {

const QString EvaluationAuthority::COMPONENT = "EVALUATION";


QStringList EvaluationAuthority::GetNames(const SmartPointer<IEvaluationReference>& ref) const
{
  ExpressionInfo info;
  ref->GetExpression()->CollectExpressionInfo(&info);
  QSet<QString> allNames;
  std::set<std::string> names = info.GetAccessedVariableNames();
  for (std::set<std::string>::iterator name = names.begin(); name != names.end(); ++name)
  {
    allNames << QString::fromStdString(*name);
  }
  if (info.HasDefaultVariableAccess())
  {
    allNames << ISources::ACTIVE_CURRENT_SELECTION_NAME();
  }

  std::set<std::string> pnames = info.GetAccessedPropertyNames();
  for (std::set<std::string>::iterator pname = pnames.begin(); pname != pnames.end(); ++pname)
  {
    allNames << QString::fromStdString(*pname);
  }
  return allNames.toList();
}

void EvaluationAuthority::RefsWithSameExpression(const QList<SmartPointer<EvaluationReference> >& refs)
{
  int k = 0;
  while (k < refs.size() && !refs[k]->IsPostingChanges())
  {
    k++;
  }
  if (k >= refs.size())
  {
    return;
  }
  EvaluationReference::Pointer ref = refs[k];
  bool oldValue = Evaluate(ref);
  ref->ClearResult();
  const bool newValue = Evaluate(ref);
  if (oldValue != newValue)
  {
    FirePropertyChange(ref, ValueOf(oldValue), ValueOf(newValue));
  }
  for (k++; k < refs.size(); k++)
  {
    ref = refs[k];
    // this is not as expensive as it looks
    if (ref->IsPostingChanges())
    {
      oldValue = Evaluate(ref);
      if (oldValue != newValue)
      {
        ref->SetResult(newValue);
        FirePropertyChange(ref, ValueOf(oldValue),
                           ValueOf(newValue));
      }
    }
  }
}

void EvaluationAuthority::StartSourceChange(const QStringList& sourceNames)
{
  notifying++;
  if (notifying == 1)
  {
    FireServiceChange(IEvaluationService::PROP_NOTIFYING, ValueOf(false),
                      ValueOf(true));
  }
}

void EvaluationAuthority::EndSourceChange(const QStringList& sourceNames)
{
  if (notifying == 1) {
    FireServiceChange(IEvaluationService::PROP_NOTIFYING, ValueOf(true),
                      ValueOf(false));
  }
  notifying--;
}

void EvaluationAuthority::FirePropertyChange(const SmartPointer<IEvaluationReference>& ref,
                                             Object::Pointer oldValue, Object::Pointer newValue)
{
  PropertyChangeEvent::Pointer event(new PropertyChangeEvent(ref, ref->GetProperty().toStdString(), oldValue,
                                                             newValue));
  ref->GetListener()->PropertyChange(event);
}

void EvaluationAuthority::FireServiceChange(const QString& property, Object::Pointer oldValue,
                                            Object::Pointer newValue)
{
  PropertyChangeEvent::Pointer event(
        new PropertyChangeEvent(Object::Pointer(this), property.toStdString(), oldValue, newValue));
  serviceListeners.propertyChange(event);
}

void EvaluationAuthority::ServiceChangeException(const std::exception &exc)
{
  WorkbenchPlugin::Log(exc.what());
}

Object::Pointer EvaluationAuthority::ValueOf(bool result)
{
  return ObjectBool::Pointer(new ObjectBool(result));
}

void EvaluationAuthority::SourceChanged(int sourcePriority)
{
  // no-op, we want the other one
}

void EvaluationAuthority::SourceChanged(const QStringList& sourceNames)
{
  struct SourceChangeScope {
    EvaluationAuthority* const ea;
    const QStringList& sourceNames;
    SourceChangeScope(EvaluationAuthority* ea, const QStringList& sourceNames)
      : ea(ea), sourceNames(sourceNames)
    {
      ea->StartSourceChange(sourceNames);
    }
    ~SourceChangeScope()
    {
      ea->EndSourceChange(sourceNames);
    }
  };

  SourceChangeScope(this, sourceNames);
  // evaluations to recompute
  for (int i = 0; i < sourceNames.size(); i++)
  {
    if (cachesBySourceName.contains(sourceNames[i]))
    {
      const ExprToEvalsMapType& cachesByExpression = cachesBySourceName[sourceNames[i]];
      QList<QSet<EvaluationReference::Pointer> > expressionCaches = cachesByExpression.values();
      for (int j = 0; j < expressionCaches.size(); j++)
      {
        if (!(expressionCaches[j].isEmpty()))
        {
          QList<EvaluationReference::Pointer> refs = expressionCaches[j].toList();
          RefsWithSameExpression(refs);
        }
      }
    }
  }
}

EvaluationAuthority::EvaluationAuthority()
  : serviceExceptionHandler(this, &EvaluationAuthority::ServiceChangeException), notifying(0)
{
  serviceListeners.propertyChange.SetExceptionHandler(serviceExceptionHandler);
}

SmartPointer<Shell> EvaluationAuthority::GetActiveShell() const
{
  return GetVariable(ISources::ACTIVE_SHELL_NAME()).Cast<Shell>();
}

void EvaluationAuthority::AddEvaluationListener(const SmartPointer<IEvaluationReference>& ref)
{
  // we update the source priority bucket sort of activations.
  QStringList sourceNames = GetNames(ref);
  for (int i = 0; i < sourceNames.size(); i++)
  {
    ExprToEvalsMapType& cachesByExpression = cachesBySourceName[sourceNames[i]];
    const Expression::Pointer expression = ref->GetExpression();
    cachesByExpression[expression].insert(ref.Cast<EvaluationReference>());
  }

  bool result = Evaluate(ref);
  FirePropertyChange(ref, Object::Pointer(0), ValueOf(result));
}

void EvaluationAuthority::RemoveEvaluationListener(const SmartPointer<IEvaluationReference>& ref)
{
  // Next we update the source priority bucket sort of activations.
  QStringList sourceNames = GetNames(ref);
  for (int i = 0; i < sourceNames.size(); i++)
  {
    if (cachesBySourceName.contains(sourceNames[i]))
    {
      ExprToEvalsMapType& cachesByExpression = cachesBySourceName[sourceNames[i]];
      if (cachesByExpression.contains(ref->GetExpression()))
      {
        QSet<EvaluationReference::Pointer>& caches = cachesByExpression[ref->GetExpression()];
        caches.remove(ref.Cast<EvaluationReference>());
        if (caches.isEmpty())
        {
          cachesByExpression.remove(ref->GetExpression());
        }
      }
      if (cachesByExpression.isEmpty())
      {
        cachesBySourceName.remove(sourceNames[i]);
      }
    }
  }
  bool result = Evaluate(ref);
  FirePropertyChange(ref, ValueOf(result), Object::Pointer(0));
}

void EvaluationAuthority::AddServiceListener(const SmartPointer<IPropertyChangeListener>& listener)
{
  serviceListeners.AddListener(listener);
}

void EvaluationAuthority::RemoveServiceListener(const SmartPointer<IPropertyChangeListener>& listener)
{
  serviceListeners.RemoveListener(listener);
}

}
