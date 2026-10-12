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

#ifdef SDL_FILESYSTEM_XSDK

// The Xbox 360 through xsdk: game:\ is the folder the title was launched from, and its settings live beside it.

#include "../SDL_sysfilesystem.h"

#include <errno.h>
#include <string.h>
#include <sys/stat.h>

#define XSDK_BASE_PATH "game:\\"

char *SDL_SYS_GetBasePath(void)
{
    return SDL_strdup(XSDK_BASE_PATH);
}

char *SDL_SYS_GetExeName(void)
{
    return NULL; // The title's own name isn't known to it.
}

char *SDL_SYS_GetPrefPath(const char *org, const char *app)
{
    const size_t len = SDL_strlen(XSDK_BASE_PATH) + SDL_strlen(org) + SDL_strlen(app) + 3;
    char *result = (char *)SDL_malloc(len);
    if (!result) {
        return NULL;
    }
    if (*org) {
        SDL_snprintf(result, len, "%s%s\\%s\\", XSDK_BASE_PATH, org, app);
    } else {
        SDL_snprintf(result, len, "%s%s\\", XSDK_BASE_PATH, app);
    }
    // Each folder after game:\ in turn.
    for (char *ptr = result + SDL_strlen(XSDK_BASE_PATH); *ptr; ptr++) {
        if (*ptr == '\\') {
            *ptr = '\0';
            if (mkdir(result, 0777) != 0 && errno != EEXIST) {
                SDL_SetError("Couldn't create %s: %s", result, strerror(errno));
                SDL_free(result);
                return NULL;
            }
            *ptr = '\\';
        }
    }
    return result;
}

char *SDL_SYS_GetUserFolder(SDL_Folder folder)
{
    SDL_Unsupported();
    return NULL;
}

#endif // SDL_FILESYSTEM_XSDK
