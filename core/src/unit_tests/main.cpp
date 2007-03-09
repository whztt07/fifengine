/***************************************************************************
*   Copyright (C) 2005-2007 by the FIFE Team                              *
*   fife-public@lists.sourceforge.net                                     *
*   This file is part of FIFE.                                            *
*                                                                         *
*   FIFE is free software; you can redistribute it and/or modify          *
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
*   51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA              *
***************************************************************************/

// Standard C++ library includes
#include <iostream>
#include <string>

// 3rd party library includes
#include <guichan.hpp>

// FIFE includes
// These includes are split up in two parts, separated by one empty line
// First block: files included from the FIFE root src directory
// Second block: files included from the same folder
#include "map/mapfactory.h"
#include "script/scriptbackendmanager.h"
#include "video/rendermanager.h"
#include "video/renderbackend.h"
#include "debugutils.h"
#include "engine.h"
#include "exception.h"
#include "gamestatemanager.h"
#include "settingsmanager.h"

int main(int argc, char* argv[]) {

	try {
		FIFE::Engine* engine = new FIFE::Engine(argc,argv);

		FIFE::SettingsManager* settings = FIFE::SettingsManager::instance();
		FIFE::ScriptBackendManager::instance()->select("Lua");

		unsigned int swidth = settings->read("ScreenWidth", 800);
		unsigned int sheight = settings->read("ScreenHeight", 600);
		unsigned char bitsPerPixel = settings->read("BitsPerPixel", 0);
		bool fullscreen = settings->read("FullScreen", false);

		FIFE::CRenderBackend()->createMainScreen(swidth, sheight, bitsPerPixel, fullscreen);

		// it seems this *MUST* be done after setting the video mode!
		SDL_EnableUNICODE(1);

		engine->start();

		delete engine;
	} catch (const FIFE::Exception& exception) {
		std::cerr << exception.getMessage() << std::endl;
		return 1;
	} catch (const gcn::Exception& exception) {
		std::cerr << exception.getMessage() << std::endl;
	}


	return 0;
}
