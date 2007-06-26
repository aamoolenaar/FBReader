/*
 * FBReader -- electronic book reader
 * Copyright (C) 2004-2007 Nikolay Pultsin <geometer@mawhrin.net>
 * Copyright (C) 2005 Mikhail Sobolev <mss@mawhrin.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef __ZLTEXTVIEW_H__
#define __ZLTEXTVIEW_H__

#include <vector>
#include <set>
#include <string>

#include <ZLOptions.h>
#include <ZLView.h>
#include <ZLDialogManager.h>

#include <ZLTextWord.h>
#include <ZLTextStyle.h>
#include <ZLTextParagraphCursor.h>
#include <ZLTextSelectionModel.h>
#include <ZLTextArea.h>

class ZLTextModel;
class ZLTextMark;

class LineInfo;
class LineInfoPtr;
class TreeNodeInfo;

class TextView : public ZLView {

public:
	enum ScrollingMode {
		NO_OVERLAPPING,
		KEEP_LINES,
		SCROLL_LINES,
		SCROLL_PERCENTAGE
	};

public:
	class PositionIndicator {

	public:
		PositionIndicator(TextView &textView);
		virtual ~PositionIndicator();

		virtual void draw();
		bool onStylusPress(int x, int y);

	protected:
		const TextView &textView() const;
		ZLPaintContext &context() const;
		int top() const;
		int bottom() const;
		int left() const;
		int right() const;

		const std::vector<size_t> &textSize() const;
		size_t startTextIndex() const;
		size_t endTextIndex() const;

	private:
		void drawExtraText(const std::string &text);
		std::string textPositionString() const;
		std::string timeString() const;
		size_t sizeOfParagraph(size_t paragraphNumber) const;
		size_t sizeOfTextBeforeParagraph(size_t paragraphNumber) const;
		size_t sizeOfTextBeforeCursor() const;

	private:
		TextView &myTextView;
		int myExtraWidth;
	};

	friend class TextView::PositionIndicator;
	
private:
	class ViewStyle {

	public:
		ViewStyle(ZLPaintContext &context);
		~ViewStyle();

		void reset();
		void setStyle(const ZLTextStylePtr style);
		void applyControl(const ControlElement &control);
		void applyControl(const ForcedControlElement &control);
		void applyControls(const WordCursor &begin, const WordCursor &end);

		const ZLPaintContext &context() const;
		const ZLTextStylePtr style() const;
		int elementWidth(const TextElement &element, unsigned int charNumber) const;
		int elementHeight(const TextElement &element) const;
		int elementDescent(const TextElement &element) const;
		int textAreaHeight() const;

		int wordWidth(const Word &word, int start = 0, int length = -1, bool addHyphenationSign = false) const;

	private:
		ZLTextStylePtr myStyle;
		ZLPaintContext &myContext;
		mutable int myWordHeight;
	};

protected:
	TextView(ZLApplication &application, ZLPaintContext &context);
	virtual ~TextView();

public:
	void clearCaches();

	void scrollPage(bool forward, ScrollingMode mode, unsigned int value);
	void scrollToHome();
	void scrollToStartOfText();
	void scrollToEndOfText();

	void gotoMark(ZLTextMark mark);
	virtual void gotoParagraph(int num, bool last = false);
	void gotoPosition(int paragraphNumber, int wordNumber, int charNumber);

	const WordCursor &startCursor() const;
	const WordCursor &endCursor() const;

	virtual void setModel(shared_ptr<ZLTextModel> model, const std::string &name);
	const shared_ptr<ZLTextModel> model() const;

	bool hasMultiSectionModel() const;
	void search(const std::string &text, bool ignoreCase, bool wholeText, bool backward, bool thisSectionOnly);
	bool canFindNext() const;
	void findNext();
	bool canFindPrevious() const;
	void findPrevious();

	void highlightParagraph(int paragraphNumber);

	SelectionModel &selectionModel();
	void copySelectedTextToClipboard(ZLDialogManager::ClipboardType type) const;
	
protected:
	bool onStylusPress(int x, int y);
	bool onStylusMovePressed(int x, int y);
	bool onStylusRelease(int x, int y);
	void activateSelection(int x, int y);

	virtual void paint();

	const std::string &fileName() const;

	int paragraphIndexByCoordinate(int y) const;
	const TextElementArea *elementByCoordinates(int x, int y) const;

	void rebuildPaintInfo(bool strong);
	virtual void preparePaintInfo();

	void setStartCursor(ParagraphCursorPtr cursor);

	bool empty() const;

	virtual shared_ptr<PositionIndicator> createPositionIndicator();

private:
	void moveStartCursor(int paragraphNumber, int wordNumber = 0, int charNumber = 0);
	void moveEndCursor(int paragraphNumber, int wordNumber = 0, int charNumber = 0);

	void clear();

	int areaLength(const ParagraphCursor &paragraph, const TextElementArea &area, int toCharNumber);
	LineInfoPtr processTextLine(const WordCursor &start, const WordCursor &end);
	void prepareTextLine(const LineInfo &info);
	void drawTextLine(const LineInfo &info, size_t from, size_t to);
	void drawWord(int x, int y, const Word &word, int start, int length, bool addHyphenationSign);
	void drawString(int x, int y, const char *str, int len, const Word::WordMark *mark, int shift);
	void drawTreeLines(const TreeNodeInfo &info, int height, int vSpaceAfter);

	bool pageIsEmpty() const;
	WordCursor findLineFromStart(unsigned int overlappingValue) const;
	WordCursor findLineFromEnd(unsigned int overlappingValue) const;
	WordCursor findPercentFromStart(unsigned int percent) const;

	enum SizeUnit {
		PIXEL_UNIT,
		LINE_UNIT
	};
	int infoSize(const LineInfo &info, SizeUnit unit);
	int paragraphSize(const WordCursor &cursor, bool beforeCurrentPosition, SizeUnit unit);
	void skip(WordCursor &paragraph, SizeUnit unit, int size);
	WordCursor findStart(const WordCursor &end, SizeUnit unit, int textHeight);

	WordCursor buildInfos(const WordCursor &start);

	std::vector<size_t>::const_iterator nextBreakIterator() const;

	PositionIndicator &positionIndicator();

private:
	shared_ptr<ZLTextModel> myModel;
	std::string myFileName;

	enum {
		NOTHING_TO_PAINT,
		READY,
		START_IS_KNOWN,
		END_IS_KNOWN,
		TO_SCROLL_FORWARD,
		TO_SCROLL_BACKWARD
	} myPaintState;
	WordCursor myStartCursor;
	WordCursor myEndCursor;
	std::vector<LineInfoPtr> myLineInfos;
	std::set<LineInfoPtr> myLineInfoCache;

	ScrollingMode myScrollingMode;
	unsigned int myOverlappingValue;

	int myOldWidth, myOldHeight;

	TextElementMap myTextElementMap;
	TreeNodeMap myTreeNodeMap;

	std::vector<size_t> myTextSize;
	std::vector<size_t> myTextBreaks;

	ViewStyle myStyle;
	SelectionModel mySelectionModel;

	shared_ptr<PositionIndicator> myPositionIndicator;

	bool myTreeStateIsFrozen;

friend class SelectionModel;
};

inline TextView::ViewStyle::~ViewStyle() {}
inline const ZLPaintContext &TextView::ViewStyle::context() const { return myContext; }
inline const ZLTextStylePtr TextView::ViewStyle::style() const { return myStyle; }

inline bool TextView::empty() const { return myPaintState == NOTHING_TO_PAINT; }
inline const WordCursor &TextView::startCursor() const { return myStartCursor; }
inline const WordCursor &TextView::endCursor() const { return myEndCursor; }
inline const std::string &TextView::fileName() const { return myFileName; }
inline const shared_ptr<ZLTextModel> TextView::model() const { return myModel; }
inline SelectionModel &TextView::selectionModel() { return mySelectionModel; }

#endif /* __ZLTEXTVIEW_H__ */