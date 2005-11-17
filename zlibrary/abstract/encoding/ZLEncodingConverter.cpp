/*
 * Copyright (C) 2004, 2005 Nikolay Pultsin <geometer@mawhrin.net>
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

#include <algorithm>

#include "../filesystem/ZLFSManager.h"
#include "../filesystem/ZLDir.h"

#include "ZLEncodingConverter.h"
#include "EncodingConverters.h"
#include "EncodingReader.h"

std::string ZLEncodingConverter::ourEncodingDescriptionPath;
std::vector<std::string> ZLEncodingConverter::ourKnownEncodings;

void ZLEncodingConverter::setEncodingDescriptionPath(const std::string &path) {
	ourEncodingDescriptionPath = path;
	ourKnownEncodings.clear();
	shared_ptr<ZLDir> dir = ZLFile(ourEncodingDescriptionPath).directory();
	if (!dir.isNull()) {
		dir->collectFiles(ourKnownEncodings, true);
	}
	ourKnownEncodings.push_back("US-ASCII");
	ourKnownEncodings.push_back("UTF-8");
	std::sort(ourKnownEncodings.begin(), ourKnownEncodings.end());
}

shared_ptr<ZLEncodingConverter> ZLEncodingConverter::createConverter(const std::string &encoding) {
	if (!encoding.empty()) {
		const std::vector<std::string> &encodingList = knownEncodings();
		std::vector<std::string>::const_iterator it;
		for (it = encodingList.begin(); it != encodingList.end(); it++) {
			if (strcasecmp(encoding.c_str(), it->c_str()) == 0) {
				break;
			}
		}

		if (it != encodingList.end()) {
			EncodingCharReader er(*it);
			char **encodingMap = er.createTable();
			if (encodingMap != 0) {
				if (er.bytesNumber() == 1) {
					return new OneByteEncodingConverter(encodingMap);
				} else if (er.bytesNumber() == 2) {
					return new TwoBytesEncodingConverter(encodingMap);
				}
			}
		}
	}

	return new DummyEncodingConverter();
}

ZLEncodingConverter::ZLEncodingConverter() : myExtensionNumber(0) {
}

ZLEncodingConverter::~ZLEncodingConverter() {
}

void ZLEncodingConverter::registerExtension(char ch, const shared_ptr<ZLControlSequenceExtension> extension) {
	if (myExtensions[(unsigned char)ch].isNull()) {
		myExtensionNumber++;
	}
	myExtensions[(unsigned char)ch] = extension;
}

void ZLEncodingConverter::reset() {
}

DummyEncodingConverter::DummyEncodingConverter() {
}

DummyEncodingConverter::~DummyEncodingConverter() {
}

void DummyEncodingConverter::convert(std::string &dst, const char *srcStart, const char *srcEnd) {
	if (myExtensionNumber == 0) {
		dst.append(srcStart, srcEnd - srcStart);
	} else {
		dst.reserve(dst.length() + srcEnd - srcStart);
		for (const char *ptr = srcStart; ptr != srcEnd; ptr++) {
			if (myActiveExtension.isNull()) {
				myActiveExtension = myExtensions[(unsigned char)*ptr];
				if (!myActiveExtension.isNull()) {
					myActiveExtension->start();
				}
			}
			if (myActiveExtension.isNull()) {
				dst += *ptr;
			} else {
				if (myActiveExtension->parseCharacter(*ptr)) {
					dst += myActiveExtension->buffer();
					myActiveExtension = 0;
				}
			}
		}
	}
}

OneByteEncodingConverter::OneByteEncodingConverter(char **encodingMap) : myEncodingMap(encodingMap) {
	for (int i = 0; i < 256; i++) {
		if (myEncodingMap[i] == 0) {
			myEncodingMap[i] = new char[2];
			myEncodingMap[i][0] = i;
			myEncodingMap[i][1] = '\0';
		}
	}
}

OneByteEncodingConverter::~OneByteEncodingConverter() {
	for (int i = 0; i < 256; i++) {
		delete[] myEncodingMap[i];
	}
	delete[] myEncodingMap;
}

void OneByteEncodingConverter::convert(std::string &dst, const char *srcStart, const char *srcEnd) {
	dst.reserve(dst.length() + 3 * (srcEnd - srcStart));
	for (const char *ptr = srcStart; ptr != srcEnd; ptr++) {
		if (myExtensionNumber > 0) {
			if (myActiveExtension.isNull()) {
				myActiveExtension = myExtensions[(unsigned char)*ptr];
				if (!myActiveExtension.isNull()) {
					myActiveExtension->start();
				}
			}
			if (!myActiveExtension.isNull()) {
				if (myActiveExtension->parseCharacter(*ptr)) {
					dst += myActiveExtension->buffer();
					myActiveExtension = 0;
				}
				continue;
			}
		}

		dst += myEncodingMap[(unsigned char)*ptr];
	}
}

TwoBytesEncodingConverter::TwoBytesEncodingConverter(char **encodingMap) : myEncodingMap(encodingMap), myLastCharIsNotProcessed(false) {
}

TwoBytesEncodingConverter::~TwoBytesEncodingConverter() {
	for (int i = 0; i < 32768; i++) {
		if (myEncodingMap[i] != 0) {
			delete[] myEncodingMap[i];
		}
	}
	delete[] myEncodingMap;
}

void TwoBytesEncodingConverter::convert(std::string &dst, const char *srcStart, const char *srcEnd) {
	if (srcStart == srcEnd) {
		return;
	}

	dst.reserve(dst.length() + 3 * (srcEnd - srcStart) / 2);
	if (myLastCharIsNotProcessed) {
		const char *utf8 = myEncodingMap[0x100 * (myLastChar & 0x7F) + (unsigned char)*srcStart];
		if (utf8 != 0) {
			dst += utf8;
		}
		srcStart++;
		myLastCharIsNotProcessed = false;
	}
	for (const char *ptr = srcStart; ptr != srcEnd; ptr++) {
		if (myExtensionNumber > 0) {
			if (myActiveExtension.isNull()) {
				myActiveExtension = myExtensions[(unsigned char)*ptr];
				if (!myActiveExtension.isNull()) {
					myActiveExtension->start();
				}
			}
			if (!myActiveExtension.isNull()) {
				if (myActiveExtension->parseCharacter(*ptr)) {
					dst += myActiveExtension->buffer();
					myActiveExtension = 0;
				}
				continue;
			}
		}

		if (((*ptr) & 0x80) == 0) {
			dst += *ptr;
		} else if (ptr + 1 == srcEnd) {
			myLastChar = *ptr;
			myLastCharIsNotProcessed = true;
		} else {
			const char *utf8 = myEncodingMap[0x100 * ((*ptr) & 0x7F) + (unsigned char)*(ptr + 1)];
			if (utf8 != 0) {
				dst += utf8;
			}
			ptr++;
		}
	}
}

void TwoBytesEncodingConverter::reset() {
	myLastCharIsNotProcessed = false;
}
