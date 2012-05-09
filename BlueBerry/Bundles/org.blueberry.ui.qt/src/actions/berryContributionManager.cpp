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

#include <berryLog.h>

#include "berryContributionManager.h"

#include "berryIContributionManagerOverrides.h"
#include "berryIContributionItem.h"
//#include "berryActionContributionItem.h"

namespace berry {

//void ContributionManager::Add(Action *action)
//{
//  Q_ASSERT_X(action, "nullcheck", "Action must not be null");
//  IContributionItem::Pointer item(new ActionContributionItem(action));
//  Add(item);
//}

void ContributionManager::Add(const SmartPointer<IContributionItem>& item)
{
  Q_ASSERT_X(item, "nullcheck", "Item must not be null");
  if (AllowItem(item.GetPointer()))
  {
    contributions.append(item);
    ItemAdded(item);
  }
}

//void ContributionManager::AppendToGroup(const QString& groupName, Action *action)
//{
//  IContributionItem::Pointer item(new ActionContributionItem(action));
//  AddToGroup(groupName, item, true);
//}

void ContributionManager::AppendToGroup(const QString& groupName, const SmartPointer<IContributionItem>& item)
{
  AddToGroup(groupName, item, true);
}

SmartPointer<IContributionItem> ContributionManager::Find(const QString& id) const
{
  QListIterator<IContributionItem::Pointer> e(contributions);
  while (e.hasNext())
  {
    IContributionItem::Pointer item = e.next();
    QString itemId = item->GetId();
    if (itemId.compare(id, Qt::CaseInsensitive) != 0)
    {
      return item;
    }
  }
  return IContributionItem::Pointer(0);
}

QList<SmartPointer<IContributionItem> > ContributionManager::GetItems() const
{
  return contributions;
}

int ContributionManager::GetSize()
{
  return contributions.size();
}

SmartPointer<IContributionManagerOverrides> ContributionManager::GetOverrides()
{
  if (overrides.IsNull())
  {
    struct _DefaultOverride : public IContributionManagerOverrides {
      int GetEnabled(IContributionItem* /*item*/) const {
        return -1;
      }

      int GetVisible(IContributionItem* /*item*/) const {
        return -1;
      }
    };
    overrides = new _DefaultOverride;
  }
  return overrides;
}

int ContributionManager::IndexOf(const QString& id)
{
  int i = 0;
  foreach(IContributionItem::Pointer item, contributions)
  {
    QString itemId = item->GetId();
    if (item->GetId().compare(id, Qt::CaseInsensitive) == 0)
    {
      return i;
    }
    ++i;
  }
  return -1;
}

void ContributionManager::Insert(int index, const SmartPointer<IContributionItem>& item)
{
  if (index > contributions.size())
  {
    QString msg = QString("inserting ")  + item->GetId() + " at " + QString::number(index);
    throw std::invalid_argument(msg.toStdString());
  }
  if (AllowItem(item.GetPointer()))
  {
    contributions.insert(index, item);
    ItemAdded(item);
  }
}

//void ContributionManager::InsertAfter(const QString& ID, Action *action)
//{
//  IContributionItem::Pointer item(new ActionContributionItem(action));
//  InsertAfter(ID, item);
//}

void ContributionManager::InsertAfter(const QString& ID, const SmartPointer<IContributionItem>& item)
{
  IContributionItem::Pointer ci = Find(ID);
  if (ci.IsNull())
  {
    throw std::invalid_argument(std::string("can't find ID") + ID.toStdString());
  }
  int ix = contributions.indexOf(ci);
  if (ix >= 0)
  {
    // BERRY_INFO << "insert after: " << ix;
    if (AllowItem(item.GetPointer()))
    {
      contributions.insert(ix + 1, item);
      ItemAdded(item);
    }
  }
}

//void ContributionManager::InsertBefore(const QString& ID, Action *action)
//{
//  IContributionItem::Pointer item(new ActionContributionItem(action));
//  InsertBefore(ID, item);
//}

void ContributionManager::InsertBefore(const QString& ID, const SmartPointer<IContributionItem>& item)
{
  IContributionItem::Pointer ci = Find(ID);
  if (ci.IsNull())
  {
    throw std::invalid_argument(std::string("can't find ID ") + ID.toStdString());
  }
  int ix = contributions.indexOf(ci);
  if (ix >= 0)
  {
    // BERRY_INFO << "insert before: " << ix;
    if (AllowItem(item.GetPointer()))
    {
      contributions.insert(ix, item);
      ItemAdded(item);
    }
  }
}

bool ContributionManager::IsDirty() const
{
  if (isDirty)
  {
    return true;
  }
  if (HasDynamicItems())
  {
    foreach (IContributionItem::Pointer item, contributions)
    {
      if (item->IsDirty())
      {
        return true;
      }
    }
  }
  return false;
}

bool ContributionManager::IsEmpty() const
{
  return contributions.empty();
}

void ContributionManager::MarkDirty()
{
  SetDirty(true);
}

//void ContributionManager::PrependToGroup(const QString& groupName, Action *action)
//{
//  IContributionItem::Pointer item(new ActionContributionItem(action));
//  AddToGroup(groupName, item, false);
//}

void ContributionManager::PrependToGroup(const QString& groupName, const SmartPointer<IContributionItem>& item)
{
  AddToGroup(groupName, item, false);
}

SmartPointer<IContributionItem> ContributionManager::Remove(const QString& ID)
{
  IContributionItem::Pointer ci = Find(ID);
  if (ci.IsNull())
  {
    return ci;
  }
  return Remove(ci);
}

SmartPointer<IContributionItem> ContributionManager::Remove(const SmartPointer<IContributionItem>& item)
{
  if (contributions.removeAll(item))
  {
    ItemRemoved(item);
    return item;
  }
  return IContributionItem::Pointer(0);
}

void ContributionManager::RemoveAll()
{
  QList<IContributionItem::Pointer> items = GetItems();
  contributions.clear();
  foreach (IContributionItem::Pointer item, items)
  {
    ItemRemoved(item);
  }
  dynamicItems = 0;
  MarkDirty();
}

bool ContributionManager::ReplaceItem(const QString& identifier,
                 const SmartPointer<IContributionItem>& replacementItem)
{
  if (identifier.isNull())
  {
    return false;
  }

  const int index = IndexOf(identifier);
  if (index < 0)
  {
    return false; // couldn't find the item.
  }

  // Remove the old item.
  const IContributionItem::Pointer oldItem = contributions.at(index);
  ItemRemoved(oldItem);

  // Add the new item.
  contributions.replace(index, replacementItem);
  ItemAdded(replacementItem); // throws NPE if (replacementItem == null)

  // Go through and remove duplicates.
  QMutableListIterator<IContributionItem::Pointer> i(contributions);
  i.toBack();
  while (i.hasPrevious())
  {
    IContributionItem::Pointer item = i.previous();
    if ((item.IsNotNull()) && (identifier == item->GetId()))
    {
//      if (Policy.TRACE_TOOLBAR) {
//        System.out
//            .println("Removing duplicate on replace: " + identifier);
//      }
      i.remove();
      ItemRemoved(item);
    }
  }

  return true; // success
}

void ContributionManager::SetOverrides(const SmartPointer<IContributionManagerOverrides>& newOverrides)
{
  overrides = newOverrides;
}

ContributionManager::ContributionManager()
  : isDirty(true), dynamicItems(0)
{
  // Do nothing.
}

bool ContributionManager::AllowItem(IContributionItem* /*itemToAdd*/)
{
  return true;
}

void ContributionManager::DumpStatistics()
{
  int size = contributions.size();

  BERRY_INFO << this->ToString();
  BERRY_INFO << "   Number of elements: " << size;
  int sum = 0;
  for (int i = 0; i < size; i++)
  {
    if (contributions.at(i)->IsVisible())
    {
      ++sum;
    }
  }
  BERRY_INFO << "   Number of visible elements: " << sum;
  BERRY_INFO << "   Is dirty: " << IsDirty();
}

bool ContributionManager::HasDynamicItems() const
{
  return (dynamicItems > 0);
}

int ContributionManager::IndexOf(const SmartPointer<IContributionItem>& item) const
{
  return contributions.indexOf(item);
}

void ContributionManager::ItemAdded(const SmartPointer<IContributionItem>& item)
{
  item->SetParent(this);
  MarkDirty();
  if (item->IsDynamic())
  {
    dynamicItems++;
  }
}

void ContributionManager::ItemRemoved(const SmartPointer<IContributionItem>& item)
{
  item->SetParent(0);
  MarkDirty();
  if (item->IsDynamic())
  {
    dynamicItems--;
  }
}

void ContributionManager::SetDirty(bool dirty)
{
  isDirty = dirty;
}

void ContributionManager::InternalSetItems(const QList<SmartPointer<IContributionItem> >& items)
{
  contributions.clear();
  for (int i = 0; i < items.size(); ++i)
  {
    if (AllowItem(items[i].GetPointer()))
    {
      contributions.append(items[i]);
    }
  }
}

void ContributionManager::AddToGroup(const QString& groupName, const SmartPointer<IContributionItem>& item,
                                     bool append)
{
  QMutableListIterator<IContributionItem::Pointer> items(contributions);
  for (int i = 0; items.hasNext(); ++i)
  {
    IContributionItem::Pointer o = items.next();
    if (o->IsGroupMarker())
    {
      QString id = o->GetId();
      if (id.compare(groupName, Qt::CaseInsensitive) == 0)
      {
        ++i;
        if (append)
        {
          for (; items.hasNext(); ++i)
          {
            IContributionItem::Pointer ci = items.next();
            if (ci->IsGroupMarker())
            {
              break;
            }
          }
        }
        if (AllowItem(item.GetPointer()))
        {
          contributions.insert(i, item);
          ItemAdded(item);
        }
        return;
      }
    }
  }
  throw std::invalid_argument(std::string("Group not found: ") + groupName.toStdString());
}

}
