/***************************************************************************
 *   Copyright (C) 2005-2008 by the FIFE team                              *
 *   http://www.fifengine.de                                               *
 *   This file is part of FIFE.                                            *
 *                                                                         *
 *   FIFE is free software; you can redistribute it and/or                 *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2.1 of the License, or (at your option) any later version.    *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA          *
 ***************************************************************************/

// Standard C++ library includes

// 3rd party library includes

// FIFE includes
// These includes are split up in two parts, separated by one empty line
// First block: files included from the FIFE root src directory
// Second block: files included from the same folder
#include "util/base/exception.h"
#include "util/log/logger.h"

#include "pool.h"

namespace FIFE {
	static Logger _log(LM_POOL);
	
	Pool::Pool(const std::string& name): 
		m_entries(),
		m_location_to_entry(),
		m_listeners(),
		m_loaders(),
		m_curind(0),
		m_name(name)
	{
	}

	Pool::~Pool() {
		FL_LOG(_log, LMsg("Pool destroyed: ") << m_name);
		printStatistics();
		clear();
		std::vector<ResourceLoader*>::iterator loader;
		for (loader = m_loaders.begin(); loader != m_loaders.end(); loader++) {
			delete (*loader);
		}
	}
	
	void Pool::clear() {
		std::vector<IPoolListener*>::iterator listener;
		for (listener = m_listeners.begin(); listener != m_listeners.end(); listener++) {
			(*listener)->poolCleared();
		}
		std::vector<PoolEntry*>::iterator entry;
		for (entry = m_entries.begin(); entry != m_entries.end(); entry++) {
			// Warn about leaks, but at least display ALL of them
			// Instead of bailing out with an exception in the FifeClass destructor.
			if( (*entry)->resource && (*entry)->resource->getRefCount() > 0 ) {
				FL_WARN(_log, LMsg(m_name + " leak: ") << (*entry)->location->getFilename());
				(*entry)->resource = 0;
			}
			delete (*entry);
		}
		m_entries.clear();
		m_location_to_entry.clear();
	}

	void Pool::addResourceLoader(ResourceLoader* loader) {
		m_loaders.push_back(loader);
	}

	int Pool::addResourceFromLocation(const ResourceLocation& loc) {
		ResourceLocationToEntry::const_iterator it = m_location_to_entry.find(loc);
		if (it != m_location_to_entry.end()) {
			return (*it).second;
		}
		
		PoolEntry* entry = new PoolEntry();
		entry->location = loc.clone();
		m_entries.push_back(entry);
		size_t index = m_entries.size() - 1;
		m_location_to_entry[loc] = index;
		return index;
	}

	int Pool::addResourceFromFile(const std::string& filename) {
		return addResourceFromLocation(ResourceLocation(filename));
	}

	IResource& Pool::get(unsigned int index, bool inc) {
		if (index >= m_entries.size()) {
			FL_ERR(_log, LMsg("Tried to get with index ") << index << ", only " << m_entries.size() << " items in pool " + m_name);
			throw IndexOverflow( __FUNCTION__ );
		}
		IResource* res = NULL;
		PoolEntry* entry = m_entries[index];
		if (entry->resource) {
			res = entry->resource;
		} else {
			if (!entry->loader) {
				findAndSetProvider(*entry);
			} else {
				entry->resource = entry->loader->loadResource(*entry->location);
			}

			if (!entry->loader) {
				LMsg msg("No suitable loader was found for resource ");
				msg << "#" << index << "<" << entry->location->getFilename()
				    << "> in pool " << m_name;
				FL_ERR(_log, msg);
	      
				throw NotFound(msg.str);
			}

			if (!entry->resource) {
				LMsg msg("No loader was able to load the requested resource ");
				msg << "#" << index << "<" << entry->location->getFilename()
				    << "> in pool " << m_name;
				FL_ERR(_log, msg);
				throw NotFound(msg.str);
			}
			res = entry->resource;
		}
		if (inc) {
			res->addRef();
		}
		res->setPoolId(index);
		return *res;
	}
	
	int Pool::getIndex(const std::string& filename) {
		// create resource
		return addResourceFromFile(filename);
	}

	void Pool::release(unsigned int index, bool dec) {
		if (index >= m_entries.size()) {
			throw IndexOverflow( __FUNCTION__ );
		}

		IResource* res = NULL;
		PoolEntry* entry = m_entries[index];
		if (entry->resource) {
			res = entry->resource;
			if (dec) {
				res->decRef();
			}
			if(res->getRefCount() == 0) {
				delete entry->resource;
				entry->resource = 0;
			}
		}
	}

	int Pool::getResourceCount(int status) {
		int amount = 0;
		std::vector<PoolEntry*>::iterator entry;
		for (entry = m_entries.begin(); entry != m_entries.end(); entry++) {
			if (status & RES_LOADED) {
				if ((*entry)->resource) {
					amount++;
				}
			}
			if (status & RES_NON_LOADED) {
				if (!(*entry)->resource) {
					amount++;
				}
			}
		}
		return amount;
	}

	void Pool::addPoolListener(IPoolListener* listener) {
		m_listeners.push_back(listener);
	}

	void Pool::removePoolListener(IPoolListener* listener) {
		std::vector<IPoolListener*>::iterator i = m_listeners.begin();
		while (i != m_listeners.end()) {
			if ((*i) == listener) {
				m_listeners.erase(i);
				return;
			}
			++i;
		}
	}

	void Pool::findAndSetProvider(PoolEntry& entry) {
		std::vector<ResourceLoader*>::iterator it = m_loaders.begin();
		std::vector<ResourceLoader*>::iterator end = m_loaders.end();
		if( it == end ) {
			FL_PANIC(_log, "no loader constructors given for resource pool");
		}
		for(; it != end; ++it) {
			IResource* res = (*it)->loadResource(*entry.location);
			if (res) {
				entry.resource = res;
				entry.loader = *it;
				return;
			}
		};
	}

	void Pool::printStatistics() {
		FL_LOG(_log, LMsg("Pool not loaded =") << getResourceCount(RES_NON_LOADED));
		FL_LOG(_log, LMsg("Pool loaded     =") << getResourceCount(RES_LOADED));
		int amount = 0;
		std::vector<PoolEntry*>::iterator entry;
		for (entry = m_entries.begin(); entry != m_entries.end(); entry++) {
			if ((*entry)->resource) {
				if ((*entry)->resource->getRefCount() > 0) {
					amount++;
				}
			}
		}
		FL_LOG(_log, LMsg("Pool locked     =") << amount);
		FL_LOG(_log, LMsg("Pool total size =") << m_entries.size());
	}
}
