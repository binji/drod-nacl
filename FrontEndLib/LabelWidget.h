// $Id: LabelWidget.h 9117 2008-08-05 04:04:05Z mrimer $

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

#ifndef LABELWIDGET_H
#define LABELWIDGET_H

#include "FontManager.h"
#include "Widget.h"

#include <string>
using std::string;

//******************************************************************************
class CLabelWidget : public CWidget
{
public:
	 enum TEXTALIGN
	 {
		 TA_Left = 0,
		 TA_CenterGroup = 2  //Multiple lines of text are not individually centered, but the
							 //group of lines as a whole are centered within the label area.
	 };

	CLabelWidget(const UINT dwSetTagNo, const int nSetX, const int nSetY, const UINT wSetW,
			const UINT wSetH, const UINT eSetFontType, const WCHAR *pwczSetText,
			const bool bResizeToFit=false, const UINT wFirstIndent=0, const WIDGETTYPE eType=WT_Label);

	UINT           GetFontType() const {return this->eFontType;}
	UINT           GetTextWidthHeight(UINT &wW, UINT &wH, const UINT wFirstIndent=0) const;
	UINT           GetFirstIndent() const {return this->wFirstIndent;}
	UINT           GetLastWidth() const {UINT dummy; return wLastWidth ? wLastWidth :
												GetTextWidthHeight(dummy,dummy,this->wFirstIndent);}
	WSTRING        GetText() const {return this->wstrText;}
	virtual bool   ContainsCoords(const int nX, const int nY) const;
	virtual void   Paint(bool bUpdateRect = true);
	void           SetClickable(const bool bVal) {this->bClickable = bVal;}
	void           SetFontType(const UINT eSetFontType) {this->eFontType = eSetFontType;}
	void           SetAlign(const TEXTALIGN eSetAlign) {this->eTextAlign = eSetAlign;}
	void           SetText(const WCHAR *pwczSetText, const bool bResizeToFit=false,
			const UINT wFirstIndent=0, const bool bFitWidth=false);

private:
	WSTRING           wstrText;
	UINT              eFontType;
	TEXTALIGN         eTextAlign;
	UINT              wFirstIndent;
	mutable UINT      wLastWidth;  //Cached; always use GetLastWidth() to read
	bool              bClickable; //whether clicks on label will be caught and handled
};

#endif //#ifndef LABELWIDGET_H
