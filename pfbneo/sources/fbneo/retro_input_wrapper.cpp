//
// Created by cpasjuste on 24/02/2022.
//

#include <algorithm>
#include "retro_input.h"

unsigned nGameType = 0;
bool bIsNeogeoCartGame = false;
bool bLightgunHideCrosshairEnabled = true;
struct GameInp *pgi_reset = nullptr;
struct GameInp *pgi_diag = nullptr;
UINT8 *diag_input = nullptr;
struct GameInp *GameInp = nullptr;
UINT32 nGameInpCount = 0;
INT32 nAnalogSpeed = 0x0100;
INT32 nFireButtons = 0;
static int nDIPOffset;

int16_t input_cb(unsigned port, unsigned device, unsigned index, unsigned id) {
    if (port < PLAYER_MAX) {
        unsigned int buttons = c2d_renderer->getInput()->getPlayer((int) port)->buttons;
        //printf("input_cb(joy): port: %i, device: %i, index: %i, id: %x (keys: %x)\n",
        //     port, device, index, id, keys);

        if (device == RETRO_DEVICE_JOYPAD) {
            return buttons & id ? 1 : 0;
        }
    }

    return 0;
}

void poll_cb() {}

static void InpDIPSWGetOffset(void) {
    BurnDIPInfo bdi;
    nDIPOffset = 0;

    for (int i = 0; BurnDrvGetDIPInfo(&bdi, i) == 0; i++) {
        if (bdi.nFlags == 0xF0) {
            nDIPOffset = bdi.nInput;
            HandleMessage(RETRO_LOG_INFO, "DIP switches offset: %d.\n", bdi.nInput);
            break;
        }
    }
}

void InpDIPSWResetDIPs(void) {
    int i = 0;
    BurnDIPInfo bdi;
    struct GameInp *pgi = NULL;

    InpDIPSWGetOffset();

    while (BurnDrvGetDIPInfo(&bdi, i) == 0) {
        if (bdi.nFlags == 0xFF) {
            pgi = GameInp + bdi.nInput + nDIPOffset;
            if (pgi)
                pgi->Input.Constant.nConst = (pgi->Input.Constant.nConst & ~bdi.nMask) | (bdi.nSetting & bdi.nMask);
        }
        i++;
    }
}

bool InpDIPSWSetCabinetPlayers(int players) {
    if (players != 2 && players != 4) {
        return false;
    }

    InpDIPSWGetOffset();

    // dipswitch option text we are looking for, ie: "4 players"
    std::string wanted = std::to_string(players) + " player";
    bool found = false;
    BurnDIPInfo bdi;

    for (int i = 0; BurnDrvGetDIPInfo(&bdi, i) == 0; i++) {
        // skip offset (0xF0), group name (0xFE) and default (0xFF) entries
        if (bdi.nFlags == 0xF0 || bdi.nFlags == 0xFE || bdi.nFlags == 0xFF || bdi.szText == nullptr) {
            continue;
        }

        std::string text = bdi.szText;
        std::transform(text.begin(), text.end(), text.begin(), ::tolower);
        if (text.compare(0, wanted.size(), wanted) != 0) {
            continue;
        }

        int index = bdi.nInput + nDIPOffset;
        if (index < 0 || (UINT32) index >= nGameInpCount) {
            continue;
        }

        struct GameInp *pgi = GameInp + index;
        pgi->Input.Constant.nConst = (pgi->Input.Constant.nConst & ~bdi.nMask) | (bdi.nSetting & bdi.nMask);
        printf("InpDIPSWSetCabinetPlayers: set \"%s\" (input: %i, mask: 0x%02x, setting: 0x%02x)\n",
               bdi.szText, index, bdi.nMask, bdi.nSetting);
        found = true;
    }

    if (!found) {
        printf("InpDIPSWSetCabinetPlayers: no %i players dipswitch found for this game\n", players);
    }

    return found;
}
