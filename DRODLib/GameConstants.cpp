// $Id: GameConstants.cpp 9637 2011-07-18 19:28:55Z mrimer $

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
 * Portions created by the Initial Developer are Copyright (C) 1995, 1996,
 * 1997, 2000, 2001, 2002, 2005, 2011 Caravel Software. All Rights Reserved.
 *
 * Contributor(s):
 *
 * ***** END LICENSE BLOCK ***** */

#include "GameConstants.h"

const char szCompanyName[] = "Caravel Games";

//Current game version.  Update as needed.

//DROD: Gunthro Budkin
const char szDROD[] = "drod"; //allows import/export of DROD-compatible XML
const WCHAR wszDROD[] = { We('d'),We('r'),We('o'),We('d'),We(0) };
const char szDROD_VER[] = "4_0";
const WCHAR wszDROD_VER[] = { We('4'),We('_'),We('0'),We(0) };
const char szUserspaceFolder[] = "DROD 4";

const UINT VERSION_NUMBER = 400; //data format version -- increment whenever export format changes
const UINT NEXT_VERSION_NUMBER = 500;

const WCHAR wszVersionReleaseNumber[] = {
	We('4'),We('.'),We('0'),We('.'),We('1'),We('.'),We('1'),We('1'),We('0'),We(0)   // 4.0.1.110
};
