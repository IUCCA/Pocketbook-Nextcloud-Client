//------------------------------------------------------------------
// eventHandler.cpp
//
// Author:           JuanJakobo
// Date:             04.08.2020
//
//-------------------------------------------------------------------

#include "inkview.h"
#include "eventHandler.h"
#include "menuHandler.h"
#include "listView.h"
#include "item.h"

#include <string>
#include <memory>

using std::string;

EventHandler *EventHandler::_eventHandlerStatic;

EventHandler::EventHandler()
{
    //create a event to create handlers
    _eventHandlerStatic = this;

    _menu = new MenuHandler("nextcloud");
    _nextcloud = new Nextcloud();
    _loginView = nullptr;
    _listView = nullptr;

    if (iv_access(NEXTCLOUD_CONFIG_PATH.c_str(), W_OK) == 0)
    {
        if (_nextcloud->login())
        {
            _listView = new ListView(_menu->getContentRect(), _nextcloud->getItems());
            FullUpdate();
            return;
        }
    }

    _loginView = new LoginView(_menu->getContentRect());
    _loginView->drawLoginView();
    FullUpdate();
}
EventHandler::~EventHandler()
{
    delete _nextcloud;
    delete _listView;
    delete _menu;
}

int EventHandler::eventDistributor(const int type, const int par1, const int par2)
{
    if (ISPOINTEREVENT(type))
        return EventHandler::pointerHandler(type, par1, par2);

    return 0;
}

void EventHandler::mainMenuHandlerStatic(const int index)
{
    _eventHandlerStatic->mainMenuHandler(index);
}

void EventHandler::mainMenuHandler(const int index)
{
    switch (index)
    {
    //Logout
    case 101:
        _nextcloud->logout();
        delete _listView;
        _listView = nullptr;
        _loginView = new LoginView(_menu->getContentRect());
        _loginView->drawLoginView();
        FullUpdate();
        break;
    //Exit
    case 102:
        CloseApp();
        break;
    default:
        break;
    }
}

int EventHandler::pointerHandler(const int type, const int par1, const int par2)
{
    if (type == EVT_POINTERDOWN)
    {
        if (IsInRect(par1, par2, _menu->getMenuButtonRect()) == 1)
        {
            return _menu->createMenu(_nextcloud->isLoggedIn(), EventHandler::mainMenuHandlerStatic);
        }
        else if (_listView != nullptr)
        {
            int itemID = _listView->listClicked(par1, par2);
            if (itemID != -1)
            {
                if (_nextcloud->getItems()[itemID].getType() == Itemtype::IFOLDER)
                {
                    FillAreaRect(_menu->getContentRect(), WHITE);
                        irect loadingScreenRect = iRect(_menu->getContentRect()->w / 2 - 100, _menu->getContentRect()->h / 2 - 50, 200, 100, ALIGN_CENTER);
                    DrawTextRect2(&loadingScreenRect, "Loading...");
                    PartialUpdate(loadingScreenRect.x, loadingScreenRect.y, loadingScreenRect.w, loadingScreenRect.h);

                    string tempPath = _nextcloud->getItems()[itemID].getPath();

                    if (!tempPath.empty())
                        _nextcloud->getDataStructure(tempPath);

                    delete _listView;
                    _listView = new ListView(_menu->getContentRect(), _nextcloud->getItems());
                    _listView->drawHeader(tempPath.substr(NEXTCLOUD_ROOT_PATH.length()));

                    PartialUpdate(_menu->getContentRect()->x, _menu->getContentRect()->y, _menu->getContentRect()->w, _menu->getContentRect()->h);
                }
                else
                {
                    //TODO  --> download in new thread and show progress and when back on the site show progress... pop up if not on site and download finshed
                    //warning if downloading and leaving programm
                    _nextcloud->downloadItem(itemID);
                }
            }

            return 1;
        }
        else if (_loginView != nullptr)
        {
            if (_loginView->logginClicked(par1, par2) == 2)
            {
                if (_nextcloud->login(_loginView->getURL(), _loginView->getUsername(), _loginView->getPassword()))
                {
                    _listView = new ListView(_menu->getContentRect(), _nextcloud->getItems());
                    delete _loginView;
                    FullUpdate();
                }
                return 1;
            }
        }
    }
    return 0;
}
