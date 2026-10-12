/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2026 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#include "SDL_internal.h"

#ifdef SDL_JOYSTICK_XSDK

// The Xbox 360's controllers through xsdk: four users, read with XamInputGetState and rumbled with XamInputSetState.

#include "xsdk/kernel.h"

#include "../SDL_sysjoystick.h"
#include "../SDL_joystick_c.h"
#include "../usb_ids.h"

#define XSDK_USERS 4
#define XSDK_AXES  6

// In SDL's gamepad button order, so the mapping below is one to one.
static const Uint16 button_bits[] = {
    INPUT_GAMEPAD_A,
    INPUT_GAMEPAD_B,
    INPUT_GAMEPAD_X,
    INPUT_GAMEPAD_Y,
    INPUT_GAMEPAD_BACK,
    INPUT_GAMEPAD_GUIDE,
    INPUT_GAMEPAD_START,
    INPUT_GAMEPAD_LEFT_THUMB,
    INPUT_GAMEPAD_RIGHT_THUMB,
    INPUT_GAMEPAD_LEFT_SHOULDER,
    INPUT_GAMEPAD_RIGHT_SHOULDER,
    INPUT_GAMEPAD_DPAD_UP,
    INPUT_GAMEPAD_DPAD_DOWN,
    INPUT_GAMEPAD_DPAD_LEFT,
    INPUT_GAMEPAD_DPAD_RIGHT,
};

struct joystick_hwdata
{
    Uint32 user;
    Uint32 last_packet;
};

// The instance of the controller each user holds, or 0.
static SDL_JoystickID user_instances[XSDK_USERS];

// The user behind the device_index'th connected controller.
static int XSDK_UserOf(int device_index)
{
    for (int user = 0; user < XSDK_USERS; user++) {
        if (user_instances[user] && device_index-- == 0) {
            return user;
        }
    }
    return -1;
}

static void XSDK_JoystickDetect(void)
{
    for (int user = 0; user < XSDK_USERS; user++) {
        INPUT_STATE state;
        const bool connected = XamInputGetState(user, 0, &state) == ERROR_SUCCESS;
        if (connected && !user_instances[user]) {
            user_instances[user] = SDL_GetNextObjectID();
            SDL_PrivateJoystickAdded(user_instances[user]);
        } else if (!connected && user_instances[user]) {
            SDL_PrivateJoystickRemoved(user_instances[user]);
            user_instances[user] = 0;
        }
    }
}

static bool XSDK_JoystickInit(void)
{
    XSDK_JoystickDetect();
    return true;
}

static int XSDK_JoystickGetCount(void)
{
    int count = 0;
    for (int user = 0; user < XSDK_USERS; user++) {
        count += user_instances[user] ? 1 : 0;
    }
    return count;
}

static bool XSDK_JoystickIsDevicePresent(Uint16 vendor_id, Uint16 product_id, Uint16 version, const char *name)
{
    return false;
}

static const char *XSDK_JoystickGetDeviceName(int device_index)
{
    return "Xbox 360 Controller";
}

static const char *XSDK_JoystickGetDevicePath(int device_index)
{
    return NULL;
}

static int XSDK_JoystickGetDeviceSteamVirtualGamepadSlot(int device_index)
{
    return -1;
}

static int XSDK_JoystickGetDevicePlayerIndex(int device_index)
{
    return XSDK_UserOf(device_index);
}

static void XSDK_JoystickSetDevicePlayerIndex(int device_index, int player_index)
{
}

static SDL_GUID XSDK_JoystickGetDeviceGUID(int device_index)
{
    // Microsoft's wired Xbox 360 controller, so SDL takes it for one.
    return SDL_CreateJoystickGUID(SDL_HARDWARE_BUS_UNKNOWN, USB_VENDOR_MICROSOFT, USB_PRODUCT_XBOX360_WIRED_CONTROLLER,
                                  0, NULL, XSDK_JoystickGetDeviceName(device_index), 'x', 0);
}

static SDL_JoystickID XSDK_JoystickGetDeviceInstanceID(int device_index)
{
    const int user = XSDK_UserOf(device_index);
    return user < 0 ? 0 : user_instances[user];
}

static bool XSDK_JoystickOpen(SDL_Joystick *joystick, int device_index)
{
    const int user = XSDK_UserOf(device_index);
    if (user < 0) {
        return SDL_SetError("No controller at index %d", device_index);
    }
    joystick->hwdata = (struct joystick_hwdata *)SDL_calloc(1, sizeof(*joystick->hwdata));
    if (!joystick->hwdata) {
        return false;
    }
    joystick->hwdata->user = (Uint32)user;
    joystick->hwdata->last_packet = SDL_MAX_UINT32;
    joystick->nbuttons = SDL_arraysize(button_bits);
    joystick->naxes = XSDK_AXES;
    joystick->nhats = 0;
    SDL_SetBooleanProperty(SDL_GetJoystickProperties(joystick), SDL_PROP_JOYSTICK_CAP_RUMBLE_BOOLEAN, true);
    return true;
}

static bool XSDK_JoystickRumble(SDL_Joystick *joystick, Uint16 low_frequency_rumble, Uint16 high_frequency_rumble)
{
    // The left motor is the heavy, low-frequency one.
    const INPUT_VIBRATION vibration = { low_frequency_rumble, high_frequency_rumble };
    const Uint32 result = XamInputSetState(joystick->hwdata->user, 0, &vibration);
    if (result != ERROR_SUCCESS) {
        return SDL_SetError("XamInputSetState failed: 0x%08X", (unsigned)result);
    }
    return true;
}

static bool XSDK_JoystickRumbleTriggers(SDL_Joystick *joystick, Uint16 left_rumble, Uint16 right_rumble)
{
    return SDL_Unsupported();
}

static bool XSDK_JoystickSetLED(SDL_Joystick *joystick, Uint8 red, Uint8 green, Uint8 blue)
{
    return SDL_Unsupported();
}

static bool XSDK_JoystickSendEffect(SDL_Joystick *joystick, const void *data, int size)
{
    return SDL_Unsupported();
}

static bool XSDK_JoystickSetSensorsEnabled(SDL_Joystick *joystick, bool enabled)
{
    return SDL_Unsupported();
}

// XInput's axes are signed with up positive and triggers run 0 to 255; SDL's are down positive and full range.
static Sint16 XSDK_InvertAxis(Sint16 value)
{
    return (Sint16)~value;
}

static Sint16 XSDK_TriggerAxis(Uint8 value)
{
    return (Sint16)((int)value * 257 - 32768);
}

static void XSDK_JoystickUpdate(SDL_Joystick *joystick)
{
    INPUT_STATE state;
    if (XamInputGetState(joystick->hwdata->user, 0, &state) != ERROR_SUCCESS) {
        return;
    }
    if (state.packet_number == joystick->hwdata->last_packet) {
        return;
    }
    joystick->hwdata->last_packet = state.packet_number;

    const Uint64 timestamp = SDL_GetTicksNS();
    const INPUT_GAMEPAD *pad = &state.gamepad;
    for (int i = 0; i < (int)SDL_arraysize(button_bits); i++) {
        SDL_SendJoystickButton(timestamp, joystick, (Uint8)i, (pad->buttons & button_bits[i]) != 0);
    }
    SDL_SendJoystickAxis(timestamp, joystick, 0, pad->thumb_lx);
    SDL_SendJoystickAxis(timestamp, joystick, 1, XSDK_InvertAxis(pad->thumb_ly));
    SDL_SendJoystickAxis(timestamp, joystick, 2, pad->thumb_rx);
    SDL_SendJoystickAxis(timestamp, joystick, 3, XSDK_InvertAxis(pad->thumb_ry));
    SDL_SendJoystickAxis(timestamp, joystick, 4, XSDK_TriggerAxis(pad->left_trigger));
    SDL_SendJoystickAxis(timestamp, joystick, 5, XSDK_TriggerAxis(pad->right_trigger));
}

static void XSDK_JoystickClose(SDL_Joystick *joystick)
{
    SDL_free(joystick->hwdata);
    joystick->hwdata = NULL;
}

static void XSDK_JoystickQuit(void)
{
    SDL_zeroa(user_instances);
}

static bool XSDK_JoystickGetGamepadMapping(int device_index, SDL_GamepadMapping *out)
{
    *out = (SDL_GamepadMapping){
        .a = { EMappingKind_Button, 0 },
        .b = { EMappingKind_Button, 1 },
        .x = { EMappingKind_Button, 2 },
        .y = { EMappingKind_Button, 3 },
        .back = { EMappingKind_Button, 4 },
        .guide = { EMappingKind_Button, 5 },
        .start = { EMappingKind_Button, 6 },
        .leftstick = { EMappingKind_Button, 7 },
        .rightstick = { EMappingKind_Button, 8 },
        .leftshoulder = { EMappingKind_Button, 9 },
        .rightshoulder = { EMappingKind_Button, 10 },
        .dpup = { EMappingKind_Button, 11 },
        .dpdown = { EMappingKind_Button, 12 },
        .dpleft = { EMappingKind_Button, 13 },
        .dpright = { EMappingKind_Button, 14 },
        .misc1 = { EMappingKind_None, 255 },
        .right_paddle1 = { EMappingKind_None, 255 },
        .left_paddle1 = { EMappingKind_None, 255 },
        .right_paddle2 = { EMappingKind_None, 255 },
        .left_paddle2 = { EMappingKind_None, 255 },
        .leftx = { EMappingKind_Axis, 0 },
        .lefty = { EMappingKind_Axis, 1 },
        .rightx = { EMappingKind_Axis, 2 },
        .righty = { EMappingKind_Axis, 3 },
        .lefttrigger = { EMappingKind_Axis, 4 },
        .righttrigger = { EMappingKind_Axis, 5 },
    };
    return true;
}

SDL_JoystickDriver SDL_XSDK_JoystickDriver = {
    XSDK_JoystickInit,
    XSDK_JoystickGetCount,
    XSDK_JoystickDetect,
    XSDK_JoystickIsDevicePresent,
    XSDK_JoystickGetDeviceName,
    XSDK_JoystickGetDevicePath,
    XSDK_JoystickGetDeviceSteamVirtualGamepadSlot,
    XSDK_JoystickGetDevicePlayerIndex,
    XSDK_JoystickSetDevicePlayerIndex,
    XSDK_JoystickGetDeviceGUID,
    XSDK_JoystickGetDeviceInstanceID,
    XSDK_JoystickOpen,
    XSDK_JoystickRumble,
    XSDK_JoystickRumbleTriggers,
    XSDK_JoystickSetLED,
    XSDK_JoystickSendEffect,
    XSDK_JoystickSetSensorsEnabled,
    XSDK_JoystickUpdate,
    XSDK_JoystickClose,
    XSDK_JoystickQuit,
    XSDK_JoystickGetGamepadMapping,
};

#endif // SDL_JOYSTICK_XSDK
