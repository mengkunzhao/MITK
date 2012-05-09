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


#ifndef BERRYACTIONCONTRIBUTIONITEM_H
#define BERRYACTIONCONTRIBUTIONITEM_H

#include "berryContributionItem.h"

namespace berry {

class Action;

/**
 * A contribution item which delegates to an action.
 * <p>
 * This class may be instantiated; it is not intended to be subclassed.
 * </p>
 * @noextend This class is not intended to be subclassed by clients.
 */
class ActionContributionItem : public ContributionItem
{

public:

  /**
   * Creates a new contribution item from the given action. The id of the
   * action is used as the id of the item.
   *
   * @param action
   *            the action
   */
  ActionContributionItem(Action* action);

  ~ActionContributionItem();

  /**
   * Compares this action contribution item with another object. Two action
   * contribution items are equal if they refer to the identical Action.
   */
  bool operator==(const Object* o) const;

  std::size_t HashCode() const;

  /**
   * The <code>ActionContributionItem</code> implementation of this
   * <code>IContributionItem</code> method creates an SWT
   * <code>Button</code> for the action using the action's style. If the
   * action's checked property has been set, the button is created and primed
   * to the value of the checked property.
   */
  void Fill(QStatusBar* parent);

  /**
   * The <code>ActionContributionItem</code> implementation of this
   * <code>IContributionItem</code> method creates an SWT
   * <code>MenuItem</code> for the action using the action's style. If the
   * action's checked property has been set, a button is created and primed to
   * the value of the checked property. If the action's menu creator property
   * has been set, a cascading submenu is created.
   */
  QAction* Fill(QMenu* parent, QAction *before);

  /**
   * The <code>ActionContributionItem</code> implementation of this ,
   * <code>IContributionItem</code> method creates an SWT
   * <code>ToolItem</code> for the action using the action's style. If the
   * action's checked property has been set, a button is created and primed to
   * the value of the checked property. If the action's menu creator property
   * has been set, a drop-down tool item is created.
   */
  QAction* Fill(QToolBar* parent, int index);

  /**
   * Returns the action associated with this contribution item.
   *
   * @return the action
   */
  Action* GetAction() const;

  /**
   * Returns the presentation mode, which is the bitwise-or of the
   * <code>MODE_*</code> constants. The default mode setting is 0, meaning
   * that for menu items, both text and image are shown (if present), but for
   * tool items, the text is shown only if there is no image.
   *
   * @return the presentation mode settings
   */
  Modes GetMode() const;

  /**
   * The action item implementation of this <code>IContributionItem</code>
   * method returns <code>true</code> for menu items and <code>false</code>
   * for everything else.
   */
  bool IsDynamic() const;

  /*
   * Method declared on IContributionItem.
   */
  bool IsEnabled() const;

  /**
   * The <code>ActionContributionItem</code> implementation of this
   * <code>ContributionItem</code> method extends the super implementation
   * by also checking whether the command corresponding to this action is
   * active.
   */
  bool IsVisible() const;

  /**
   * Sets the presentation mode, which is the bitwise-or of the
   * <code>MODE_*</code> constants.
   *
   * @param mode
   *            the presentation mode settings
   */
  void SetMode(Modes mode);

  /**
   * The action item implementation of this <code>IContributionItem</code>
   * method calls <code>update(null)</code>.
   */
  void Update();

  /**
   * Synchronizes the UI with the given property.
   *
   * @param propertyName
   *            the name of the property, or <code>null</code> meaning all
   *            applicable properties
   */
  void Update(const QString& propertyName);

protected:

  /**
   * Returns <code>true</code> if this item is allowed to enable,
   * <code>false</code> otherwise.
   *
   * @return if this item is allowed to be enabled
   */
  bool IsEnabledAllowed() const;

  /**
   * Shorten the given text <code>t</code> so that its length doesn't exceed
   * the width of the given ToolItem.The default implementation replaces
   * characters in the center of the original string with an ellipsis ("...").
   * Override if you need a different strategy.
   *
   * @param textValue
   *            the text to shorten
   * @param item
   *            the tool item the text belongs to
   * @return the shortened string
   *
   */
  //QString ShortenText(const QString& textValue, QToolButton* item);

private:

  /**
   * Returns the listener for SWT tool item widget events.
   *
   * @return a listener for tool item events
   */
  //Listener GetToolItemListener();

  /**
   * Handles a widget dispose event for the widget corresponding to this item.
   */
  //void HandleWidgetDispose(Event e);

  /**
   * Handles a widget selection event.
   */
  //void HandleWidgetSelection(Event e, bool selection);

  /**
   * Returns whether the given action has any images.
   *
   * @param actionToCheck
   *            the action
   * @return <code>true</code> if the action has any images,
   *         <code>false</code> if not
   */
  //bool HasImages(Action* actionToCheck) const;

  /**
   * Returns whether the command corresponding to this action is active.
   */
  bool IsCommandActive() const;

  /**
   * Updates the images for this action.
   *
   * @param forceImage
   *            <code>true</code> if some form of image is compulsory, and
   *            <code>false</code> if it is acceptable for this item to have
   *            no image
   * @return <code>true</code> if there are images for this action,
   *         <code>false</code> if not
   */
  //bool UpdateImages(bool forceImage);

  /**
   * Dispose any images allocated for this contribution item
   */
  //void DisposeOldImages();

  /**
   * Handle show and hide on the proxy menu for IAction.AS_DROP_DOWN_MENU
   * actions.
   *
   * @return the appropriate listener
   */
  //Listener getMenuCreatorListener();

  /**
   * The proxy menu is being shown, we better get the real menu.
   *
   * @param proxy
   *            the proxy menu
   */
  //void HandleShowProxy(QMenu* proxy);

  /**
   * Create MenuItems in the proxy menu that can execute the real menu items
   * if selected. Create proxy menus for any real item submenus.
   *
   * @param realMenu
   *            the real menu to copy from
   * @param proxy
   *            the proxy menu to populate
   */
  //void CopyMenu(QMenu* realMenu, QMenu* proxy);

  /**
   * The proxy menu is being hidden, so we need to make it go away.
   *
   * @param proxy
   *            the proxy menu
   */
  //void HandleHideProxy(QMenu* proxy);

private:

  /**
   * This is the easiest way to hold the menu until we can swap it in to the
   * proxy.
   */
  //QMenu* holdMenu = null;

  //bool menuCreatorCalled = false;


  /** a string inserted in the middle of text that has been shortened */
  //static const QString ellipsis = "...";

  /**
   * Stores the result of the action. False when the action returned failure.
   */
  bool result; // = null;

  /**
   * The presentation mode.
   */
  Modes mode; // = 0;

  /**
   * The action.
   */
  Action* action;

};

}

#endif // BERRYACTIONCONTRIBUTIONITEM_H
