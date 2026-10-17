//
// Created by cpasjuste on 29/05/18.
//

#include <string>
#include <set>
#include "c2dui.h"
#include "config.h"
#include "burner.h"
#include "retro_input_wrapper.h"

int PFBAGetDriverIndex(const ss_api::Game &game);

using namespace c2d;
using namespace c2dui;

PFBAConfig::PFBAConfig(c2d::Io *io, int version) : Config(io, version, "arcade/") {
    printf("PFBAConfig(%s, v%i)\n", getConfigPath().c_str(), version);

#ifndef __PFBN_NO_CONSOLES__
    // add fba default roms paths
    roms_paths.emplace_back(io->getDataPath() + "channelf/");
    roms_paths.emplace_back(io->getDataPath() + "coleco/");
    roms_paths.emplace_back(io->getDataPath() + "fds/");
    roms_paths.emplace_back(io->getDataPath() + "gamegear/");
    roms_paths.emplace_back(io->getDataPath() + "megadrive/");
    roms_paths.emplace_back(io->getDataPath() + "msx/");
    roms_paths.emplace_back(io->getDataPath() + "nes/");
    roms_paths.emplace_back(io->getDataPath() + "ngp/");
    roms_paths.emplace_back(io->getDataPath() + "pce/");
    roms_paths.emplace_back(io->getDataPath() + "sg1000/");
    roms_paths.emplace_back(io->getDataPath() + "sgx/");
    roms_paths.emplace_back(io->getDataPath() + "sms/");
    roms_paths.emplace_back(io->getDataPath() + "spectrum/");
    roms_paths.emplace_back(io->getDataPath() + "tg16/");
#endif

    ////////////////////////////////////////////////////////////
    /// pfba custom config
    ////////////////////////////////////////////////////////////

    /// MAIN OPTIONS
    get(Option::Id::GUI_SHOW_ZIP_NAMES)->setValueBool(false);

    /// ROMS OPTIONS
    add(Option::Id::ROM_FILTER, "FORCE_60HZ",
        {"OFF", "ON"}, 1, Option::Id::ROM_FORCE_60HZ, Option::Flags::BOOLEAN);
    get(Option::Id::ROM_FORCE_60HZ)->setInfo("YOU NEED TO RESTART EMULATION AFTER CHANGING THIS OPTION");

    // audio
    add(Option::Id::ROM_FORCE_60HZ, "AUDIO_FREQUENCY",
        {"11025", "22050", "32000", "44100", "48000"}, 3, Option::Id::ROM_AUDIO_FREQ, Option::Flags::STRING);
    get(Option::Id::ROM_AUDIO_FREQ)->setInfo("YOU NEED TO RESTART EMULATION AFTER CHANGING THIS OPTION");

    add(Option::Id::ROM_AUDIO_FREQ, "AUDIO_INTERPOLATION",
        {"0", "1", "3"}, 2, Option::Id::ROM_AUDIO_INTERPOLATION, Option::Flags::STRING);
    get(Option::Id::ROM_AUDIO_INTERPOLATION)->setInfo(
            "YOU NEED TO RESTART EMULATION AFTER CHANGING THIS OPTION");
    add(Option::Id::ROM_AUDIO_INTERPOLATION, "AUDIO_FM_INTERPOLATION",
        {"0", "1", "3"}, 2, Option::Id::ROM_AUDIO_FMINTERPOLATION, Option::Flags::STRING);
    get(Option::Id::ROM_AUDIO_FMINTERPOLATION)->setInfo(
            "YOU NEED TO RESTART EMULATION AFTER CHANGING THIS OPTION");

#ifdef __VITA__
    add(Option::Id::ROM_AUDIO_FMINTERPOLATION, "ROTATION",
        {"OFF", "ON", "FLIP", "CAB MODE"}, 1, Option::Id::ROM_ROTATION, Option::Flags::STRING);
#else
    add(Option::Id::ROM_AUDIO_FMINTERPOLATION, "ROTATION",
        {"OFF", "ON", "FLIP"}, 1, Option::Id::ROM_ROTATION, Option::Flags::STRING);
#endif
    add(Option::Id::ROM_ROTATION, "NEOBIOS",
        {"UNIBIOS_4_0", "UNIBIOS_3_3", "UNIBIOS_3_2", "UNIBIOS_3_1",
         "MVS_ASIA_EUR_V6S1", "MVS_ASIA_EUR_V5S1", "MVS_ASIA_EUR_V3S4",
         "MVS_USA_V5S2", "MVS_USA_V5S4", "MVS_USA_V5S6",
         "MVS_JPN_V6", "MVS_JPN_V5", "MVS_JPN_V3S4", "MVS_JPN_J3",
         "AES_ASIA", "AES_JAPAN",
         "NEO_MVH_MV1CA", "NEO_MVH_MV1CJ",
         "DECK_V6", "DEVKIT"},
        0, Option::Id::ROM_NEOBIOS, Option::Flags::STRING);
    get(Option::Id::ROM_NEOBIOS)->setInfo("YOU NEED TO RESTART EMULATION AFTER CHANGING THIS OPTION");
    // dipswitch: cabinet type (only affects games having a "2 Players" / "4 Players" dipswitch)
    add(Option::Id::ROM_NEOBIOS, "CABINET_PLAYERS",
        {"DEFAULT", "2 PLAYERS", "4 PLAYERS"}, 0, Option::Id::ROM_DIP_CABINET, Option::Flags::STRING);
    get(Option::Id::ROM_DIP_CABINET)->setInfo(
            "ONLY FOR GAMES WITH A 2/4 PLAYERS DIPSWITCH - RESTART EMULATION AFTER CHANGING THIS OPTION");
    // game speed (applied when the game loads)
    add(Option::Id::ROM_DIP_CABINET, "GAME_SPEED",
        {"100%", "102%", "104%", "106%", "108%", "110%", "112%", "114%", "116%", "118%", "120%"},
        0, Option::Id::ROM_GAME_SPEED, Option::Flags::STRING);
    get(Option::Id::ROM_GAME_SPEED)->setInfo(
            "RESTART EMULATION AFTER CHANGING THIS OPTION - AUDIO PITCH WILL INCREASE WITH SPEED");
#ifdef __VITA__
    // do not use unibios as default on vita for cyclone asm compatibility
    get(Option::Id::ROM_NEOBIOS)->setIndex(4);

    add(Option::Id::ROM_NEOBIOS, "FRAMESKIP",
        {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9"},
        0, Option::Id::ROM_FRAMESKIP, Option::Flags::STRING);
#endif

#ifdef __PS4__
    // PS4: force 48000hz audio output
    get(Option::Id::ROM_AUDIO_FREQ)->setIndex(4);
    get(Option::Id::ROM_AUDIO_FREQ)->setFlags(Option::Flags::STRING | Option::Flags::HIDDEN);
#endif

    // "c2dui_romlist" will also reload config, but we need new roms paths
    reset();
    load();
}

// make a valid and readable config key from a dipswitch group name (ie: "Coinage Source" -> "DIP_COINAGE_SOURCE")
static std::string getDipOptionName(const std::string &groupName, std::set<std::string> &used) {
    std::string name = "DIP_";
    bool lastUnderscore = true;
    for (char c: groupName) {
        if (isalnum((unsigned char) c)) {
            name += (char) toupper((unsigned char) c);
            lastUnderscore = false;
        } else if (!lastUnderscore) {
            name += '_';
            lastUnderscore = true;
        }
    }
    while (!name.empty() && name.back() == '_') name.pop_back();
    if (name.size() > 28) name.resize(28);
    if (name == "DIP") name = "DIP_OPTION";

    std::string unique = name;
    for (int n = 2; used.count(unique); n++) {
        unique = name + "_" + std::to_string(n);
    }
    used.insert(unique);

    return unique;
}

void PFBAConfig::loadRomOptions(const ss_api::Game &game) {
    int driver = PFBAGetDriverIndex(game);
    if (driver < 0) {
        return;
    }

    // dipswitch info is read from the driver, the active driver must be restored (game may be running)
    UINT32 previous = nBurnDrvActive;
    nBurnDrvActive = driver;
    std::vector<DipGroupInfo> groups = InpDIPSWGetGroups();
    nBurnDrvActive = previous;

    if (groups.empty()) {
        return;
    }

    std::vector<Option> dips;
    dips.emplace_back("DIPSWITCHES", std::vector<std::string>{"DIPSWITCHES"}, 0,
                      PFBA_DIP_MENU_ID, Option::Flags::MENU);

    std::set<std::string> usedNames;
    for (size_t g = 0; g < groups.size(); g++) {
        std::vector<std::string> values;
        // if the driver default doesn't match any option, add a "DEFAULT" entry (does not change the dipswitch)
        if (groups[g].defaultIndex < 0) {
            values.emplace_back("DEFAULT");
        }
        for (const auto &opt: groups[g].options) {
            values.push_back(opt.text);
        }
        int defaultIndex = groups[g].defaultIndex < 0 ? 0 : groups[g].defaultIndex;

        dips.emplace_back(getDipOptionName(groups[g].name, usedNames), values, defaultIndex,
                          PFBA_DIP_BASE + (int) g, Option::Flags::STRING);
        dips.back().setInfo("RESTART EMULATION AFTER CHANGING THIS OPTION");
    }

    // insert the dipswitch menu before the joypad menu
    std::vector<Option> *opts = getRomOptionsList();
    auto pos = opts->end();
    for (auto it = opts->begin(); it != opts->end(); ++it) {
        if (it->getId() == Option::Id::MENU_JOYPAD) {
            pos = it;
            break;
        }
    }
    opts->insert(pos, dips.begin(), dips.end());
}
