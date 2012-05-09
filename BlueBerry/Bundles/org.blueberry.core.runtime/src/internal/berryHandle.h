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


#ifndef BERRYHANDLE_H
#define BERRYHANDLE_H

#include <berryObject.h>

namespace berry {

class RegistryObject;
class RegistryObjectManager;

/**
 * A handle is the super class to all registry objects that are now served to users.
 * The handles never hold on to any "real" content of the object being represented.
 * A handle can become stale if its referenced object has been removed from the registry.
 */
class Handle : public Object
{
public:

  berryObjectMacro(berry::Handle)

  Handle(RegistryObjectManager* objectManager, int value);

  /**
   * Return the actual object corresponding to this handle.
   * @throws InvalidRegistryObjectException when the handle is stale.
   */
  virtual SmartPointer<RegistryObject> GetObject() const = 0;

  int GetId() const;

  bool operator==(const Object* object) const;

  uint HashCode() const;

protected:

  RegistryObjectManager* const objectManager;

private:

  int objectId;

};

}

#endif // BERRYHANDLE_H
