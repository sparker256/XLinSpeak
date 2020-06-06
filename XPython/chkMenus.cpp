#include <iostream>
#include <map>
#include <vector>
#include <list>
#include <dlfcn.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <sstream>
#include <XPLM/XPLMDefs.h>
#include <XPLM/XPLMDataAccess.h>
#include <XPLM/XPLMMenus.h>

#include "chk_helper.h"


static const XPLMMenuID mainMenuID = (void *)0x12345678;
#if defined(XPLM300)
static const XPLMMenuID aircraftMenuID = (void *)0x11223344;
#endif
static const XPLMMenuID newMenuID = (void *)0x87654321;
static std::string name;
static int item;
static int lang;
static XPLMMenuCheck check;
static XPLMMenuHandler_f menuHandler;
static void *menuRef;
static std::list<XPLMDataRef> d;

void initMenusModule()
{
  d.push_back(registerROAccessor("name", name));
  d.push_back(registerROAccessor("item", item));
  d.push_back(registerROAccessor("lang", lang));
  d.push_back(registerROAccessor("check", check));
}

void cleanupMenusModule()
{
  for(std::list<XPLMDataRef>::iterator i = d.begin(); i != d.end(); ++i){
    XPLMUnregisterDataAccessor(*i);
  }
  d.empty();
}


XPLMMenuID XPLMFindPluginsMenu(void)
{
  return mainMenuID;
}

#if defined(XPLM300)
XPLMMenuID XPLMFindAircraftMenu(void)
{
  return aircraftMenuID;
}
#endif

XPLMMenuID XPLMCreateMenu(const char *inName, XPLMMenuID inParentMenu, int inParentItem, XPLMMenuHandler_f inHandler,
                          void *inMenuRef)
{
  (void)(inParentMenu);
  name = inName;
  assert(inParentMenu == mainMenuID);
  item = inParentItem;
  menuHandler = inHandler;
  menuRef = inMenuRef;
  return newMenuID;
}

void XPLMDestroyMenu(XPLMMenuID inMenuID)
{
  (void)(inMenuID);
  assert(inMenuID == newMenuID);
  name = "destroyed";
}

void XPLMClearAllMenuItems(XPLMMenuID inMenuID)
{
  (void)(inMenuID);
  assert(inMenuID == newMenuID);
  item = 0;
}

int XPLMAppendMenuItem(XPLMMenuID inMenu, const char *inItemName, void *inItemRef, int inForceEnglish)
{
  (void)(inMenu);
  assert((inMenu == mainMenuID) || (inMenu == newMenuID));
  name = inItemName;
  lang = inForceEnglish;
  if(menuHandler){
    menuHandler(menuRef, inItemRef);
  }
  return ++item;
}

#if defined(XPLM300)
int XPLMAppendMenuItemWithCommand(XPLMMenuID inMenu, const char *inItemName, XPLMCommandRef inCommandToExecute)
{
  assert(inMenu == aircraftMenuID);
  name = inItemName;
  XPLMCommandOnce(inCommandToExecute);
  return ++item;
}
#endif

void XPLMAppendMenuSeparator(XPLMMenuID inMenu)
{
  (void)(inMenu);
  assert(inMenu == newMenuID);
  name = "separator";
}

void XPLMSetMenuItemName(XPLMMenuID inMenu, int inIndex, const char *inItemName, int inForceEnglish)
{
  (void)(inMenu);
  assert(inMenu == newMenuID);
  item = inIndex;
  name = inItemName;
  lang = inForceEnglish;
}

void XPLMCheckMenuItem(XPLMMenuID inMenu, int inIndex, XPLMMenuCheck inCheck)
{
  (void)(inMenu);
  assert(inMenu == newMenuID);
  item = inIndex;
  check = inCheck;
}

void XPLMCheckMenuItemState(XPLMMenuID inMenu, int inIndex, XPLMMenuCheck *outCheck)
{
  (void)(inMenu);
  assert(inMenu == newMenuID);
  item = inIndex;
  *outCheck = check;
}

void XPLMEnableMenuItem(XPLMMenuID inMenu, int inIndex, int enabled)
{
  (void)(inMenu);
  assert(inMenu == newMenuID);
  item = inIndex;
  lang = enabled;
}

#if defined(XPLM210)
void XPLMRemoveMenuItem(XPLMMenuID inMenu, int inIndex)
{
  (void)(inMenu);
  assert(inMenu == newMenuID);
  item = inIndex;
  name = "removed";
}
#endif
