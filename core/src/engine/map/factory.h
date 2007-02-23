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

#ifndef FIFE_MAP_FACTORY_H
#define FIFE_MAP_FACTORY_H

// Standard C++ library includes
#include <map>
#include <string>

// 3rd party library includes

// FIFE includes
// These includes are split up in two parts, separated by one empty line
// First block: files included from the FIFE root src directory
// Second block: files included from the same folder
#include "singleton.h"

namespace FIFE { namespace map {

	class Map;
	class Loader;
	class Archetype;
	class ObjectInfo;

	/** User-Interface to load any kind of supported mapfile.
	 * 
	 * The most important function is probably @c loadMap() which
	 * iterates through all supported @em MapLoader(s) until one
	 * succeeds.
	 *
	 * A @em MapLoader needs to be registered with the factory; 
	 * This is done in the constructor.
	 * 
	 * @see MapLoader
	 * @see Map
	 * 
	 * This class is derived from a @em singleton, you can access
	 * the object by calling the @c instance() member function.
	 * 
	 * @see DynamicSingleton
	 */
	class Factory : public DynamicSingleton<Factory> {
		public:
			/** Constructor.
			 * Called during Engine startup
			 */
			Factory();

			/** Destructor.
			 * Called during Engine shutdown
			 */
			virtual ~Factory();

			/** Attempts to load a mapfile.
			 *
			 * Tries all registered loaders until success.
			 *
			 * @param file Path to file to load.
			 * @return Pointer load @em Map instance (on success) or NULL pointer on failure.
			 * @note Exceptions from the maploaders are caught and @b not propagated.
			 * @bug Throwing an exception when all loaders fail might be a good idea.
			 */
			Map* loadMap(const std::string& path);

			/** List archetypes
			 */
			std::list<std::string> listArchetypes() const;

			/** Load an archetype (collection of tileid/prototypes) from a file.
			 */
			void loadArchetypes(const std::string& type, const std::string& filename);

			/** Add an archetype.
			 */
			void addArchetype(Archetype* archetype);

			/** Map type to an internal id
			 *  
			 */
			size_t getPrototypeId(const std::string& type);

			/** Map internal id to type name
			 *  
			 */
			const std::string& getPrototypeName(size_t proto_id);

			/** Load a prototype of an object
			 *  Loading a prototype of an object will set the objects
			 *  data to default values, that can be overridden.
			 */
			void loadPrototype(ObjectInfo* object, size_t proto_id);

			/** Get ImageCache ID if tile with id id
			 */
			size_t getTileImageId(size_t id);

		private:

			/** Registers a format-specific loader class with the factory.
			 *
			 * \param loader Pointer to a valid instance (@b no NULL pointers, please).
			 * \note Used internally
			 */
			void registerLoader(Loader* loader);
			
			/** Removes all registered loaders.
			 */
			void cleanup();

			typedef std::map<std::string, Loader*> type_loaders;
			// Registered maploaders.
			type_loaders m_loaders;
	};

} } //FIFE::map

#endif
/* vim: set noexpandtab: set shiftwidth=2: set tabstop=2: */
