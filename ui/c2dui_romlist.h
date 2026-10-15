//
// Created by cpasjuste on 19/10/16.
//

#ifndef C2DUI_ROMLIST_H
#define C2DUI_ROMLIST_H

#include <vector>
#include <string>

namespace c2dui {

    class UiMain;

    class RomList {

    public:

        RomList(UiMain *ui, const std::string &emuVersion, const std::vector<std::string> &filters);

        virtual ~RomList();

        virtual void build(bool addArcadeSystem = false);

        void addFav(const ss_api::Game &game);

        void removeFav(const ss_api::Game &game);

        void setLoadingText(const char *format, ...);

        // autoload helpers
        static std::string getAutoLoadName(const ss_api::Game &game);

        static std::string getAutoLoadId(const ss_api::Game &game);

        // find an available game from an id returned by getAutoLoadId ("systemId:path" or "path")
        ss_api::Game findGameByAutoLoadId(const std::string &id);

        // find an available game from a name returned by getAutoLoadName
        ss_api::Game findGameByAutoLoadName(const std::string &name);

        UiMain *ui;
        c2d::RectangleShape *rect;
        c2d::Text *text;
        ss_api::GameList *gameList = nullptr;
        ss_api::GameList *gameListFav = nullptr;
        std::vector<std::string> paths;
        char icon_path[1024];
        float time_start = 0;

    protected:
        std::vector<std::string> filters;
    };
}

#endif //C2DUI_ROMLIST_H
