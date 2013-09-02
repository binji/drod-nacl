// $Id: $

/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Deadly Rooms of Death.
 *
 * The Initial Developer of the Original Code is
 * Caravel Software.
 * Portions created by the Initial Developer are Copyright (C) 2002, 2005
 * Caravel Software. All Rights Reserved.
 *
 * Contributor(s):
 *
 * ***** END LICENSE BLOCK ***** */

#include "Assert.h"
#include "Wchar.h"
#include <map>
#include <cstdlib>  // atoi

namespace MetaKey {
	extern const char MAX_EYE_CANDY[];
}

class Metadata {
private:
	typedef std::map<std::string, std::string> MetadataMap;
	static MetadataMap map;
	Metadata ();  // unconstructable

public:
	static const std::string& GetString  (const std::string &key) {
		MetadataMap::const_iterator it = map.find(key);
		if (it != map.end())
			return it->second;

		ASSERT(!"Unrecognized metadata key");
		static std::string default_val;
		return default_val;
	}
	static const WSTRING      GetWString (const std::string &key) {
		WSTRING value;

		MetadataMap::const_iterator it = map.find(key);
		if (it != map.end()) {
			CTextToUnicode(it->second.c_str(), value);
		} else {
			ASSERT(!"Unrecognized metadata key");
		}

		return value;
	}

	static int GetInt(const std::string &key) {
		const std::string str = GetString(key);
		return atoi(str.c_str());
	}

	static void Set (const std::string &key, const std::string &value)
	{
		ASSERT(map.count(key) == 0);
		map[key] = value;
	}
};
