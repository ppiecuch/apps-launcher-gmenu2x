/***************************************************************************
 *   Copyright (C) 2006 by Massimiliano Torromeo                           *
 *   massimiliano.torromeo@gmail.com                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <SDL.h>
#include <stdarg.h>

#include "debug.h"
#include "dumpevent.h"

enum {
    CAT_MISC,
    CAT_KEYB,
    CAT_MOUSE,
    CAT_JOY,
    CAT_CONTROLLER,

    MAX_CATEGORIES
};

static const char * catlabel[MAX_CATEGORIES] = {
    "MISC",
    "KEYB",
    "MOUSE",
    "JOY",
    "SDL_CONTROLLER",
};

#define MAX_LINELENGTH 256

// Event handlers
static int app_on_keydown(const SDL_Event *evt);
static int app_on_keyup(const SDL_Event *evt);
static int app_on_mousemove(const SDL_Event *evt);
static int app_on_mousebdown(const SDL_Event *evt);
static int app_on_mousebup(const SDL_Event *evt);
static int app_on_joyaxis(const SDL_Event *evt);
static int app_on_joyhat(const SDL_Event *evt);
static int app_on_joyball(const SDL_Event *evt);
static int app_on_joybdown(const SDL_Event *evt);
static int app_on_joybup(const SDL_Event *evt);

// Output functions
static int app_write(int category, const char *msg) {
    INFO("%15s | %s", catlabel[category], msg);
    return 0;
}

static int app_vfwrite(int category, const char *fmt, va_list vp) {
    char buf[MAX_LINELENGTH];
    int res = SDL_vsnprintf(buf, sizeof(buf), fmt, vp);
    app_write(category, buf);
    return res;
}

static int app_fwrite(int category, const char *fmt, ...) {
    va_list vp;
    va_start(vp, fmt);
    int retval = app_vfwrite(category, fmt, vp);
    va_end(vp);
    return retval;
}

// Main event decoder
int sdl_dump_events (const SDL_Event *evt) {
    switch (evt->type) {
        case SDL_QUIT:
            break;
        case SDL_KEYDOWN:
            app_on_keydown(evt);
            break;
        case SDL_KEYUP:
            app_on_keyup(evt);
            break;
        case SDL_MOUSEMOTION:
            app_on_mousemove(evt);
            break;
        case SDL_MOUSEBUTTONDOWN:
            app_on_mousebdown(evt);
            break;
        case SDL_MOUSEBUTTONUP:
            app_on_mousebup(evt);
            break;
        case SDL_JOYAXISMOTION:
            app_on_joyaxis(evt);
            break;
        case SDL_JOYHATMOTION:
            app_on_joyhat(evt);
            break;
        case SDL_JOYBALLMOTION:
            app_on_joyball(evt);
            break;
        case SDL_JOYBUTTONDOWN:
            app_on_joybdown(evt);
            break;
        case SDL_JOYBUTTONUP:
            app_on_joybup(evt);
            break;
        default:
            return 1;
            break;
    }
    return 0;
}

// Event handlers
static int app_on_keydown(const SDL_Event *evt) {
    const char *keyname = SDL_GetKeyName(evt->key.keysym.sym);
    if (!keyname) {
        char altname[32];
        SDL_snprintf(altname, sizeof(altname), "(%d)", evt->key.keysym.sym);
        keyname = altname;
    }
    app_fwrite(CAT_KEYB, "PRESS: %s", keyname);
    return 0;
}

static int app_on_keyup(const SDL_Event *evt) {
    const char * keyname = SDL_GetKeyName(evt->key.keysym.sym);
    if (!keyname) {
        char altname[32];
        SDL_snprintf(altname, sizeof(altname), "(%d)", evt->key.keysym.sym);
        keyname = altname;
    }
    app_fwrite(CAT_KEYB, "RELEASE: %s", keyname);
    return 0;
}

int app_on_mousemove (const SDL_Event *evt) {
    app_fwrite(CAT_MOUSE, "MV: %+d%+d:(%d,%d)",
        evt->motion.xrel,
        evt->motion.yrel,
        evt->motion.x,
        evt->motion.y
    );
  return 0;
}

static int app_on_mousebdown (const SDL_Event *evt) {
    app_fwrite(CAT_MOUSE, "PRESS: %d", evt->button.button);
    return 0;
}

static int app_on_mousebup(const SDL_Event *evt) {
    app_fwrite(CAT_MOUSE, "RELEASE: %d", evt->button.button);
    return 0;
}

static int app_on_joyaxis (const SDL_Event *evt) {
    app_fwrite(CAT_JOY, "%d/AXIS/%d: %d",
        evt->jaxis.which,
        evt->jaxis.axis,
        evt->jaxis.value
    );
    return 0;
}

static int app_on_joyhat (const SDL_Event *evt) {
    app_fwrite(CAT_JOY, "%d/HAT/%d: %d",
        evt->jhat.which,
        evt->jhat.hat,
        evt->jhat.value
    );
    return 0;
}

static int app_on_joyball (const SDL_Event *evt) {
    app_fwrite(CAT_JOY, "%d/BALL/%d: %+d%+d",
        evt->jball.which,
        evt->jball.ball,
        evt->jball.xrel,
        evt->jball.yrel
    );
    return 0;
}

static int app_on_joybdown (const SDL_Event *evt) {
    app_fwrite(CAT_JOY, "%d/PRESS: %d",
        evt->jbutton.which,
        evt->jbutton.button
    );
    return 0;
}

static int app_on_joybup (const SDL_Event *evt) {
    return app_fwrite(CAT_JOY, "%d/RELEASE: %d",
        evt->jbutton.which,
        evt->jbutton.button
    );
    return 0;
}
