#include <Core/System/MemoryMgr.h>

#include "string.h"

namespace std {

	string::string() {
		myLength = 0;
		myDataSize = 0;
		myData = 0;
	}

	string::~string() {
		if (myData != 0) {
			delete[] myData;
		}
	}

	// TODO: optimize (use palm-specific functions)
	void string::reserve(size_t minSize) {
		if (minSize > myDataSize) {
			if (minSize | 0xf) {
				minSize = (minSize & (size_t)-0x10) + 0x10;
			}
			char *d = new char[minSize];
			if (myLength > 0) {
				MemMove(d, myData, myLength);
			}
			if (myData != 0) {
				delete[] myData;
			}
			myData = d;
		}
	}
};
