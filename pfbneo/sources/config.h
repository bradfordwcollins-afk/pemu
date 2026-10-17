//
// Created by cpasjuste on 29/05/18.
//

#ifndef PFBA_CONFIG_H
#define PFBA_CONFIG_H

#include "c2dui_config.h"

class PFBAConfig : public c2dui::Config {

public:

    PFBAConfig(c2d::Io *io, int version);

    // adds the "DIPSWITCHES" menu (all dipswitches of the game) to the rom options
    void loadRomOptions(const ss_api::Game &game) override;

};

#endif //PFBA_CONFIG_H
