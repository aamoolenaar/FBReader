/*
 * FBReader -- electronic book reader
 * Copyright (C) 2005 Nikolay Pultsin <geometer@mawhrin.net>
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __GTKFBREADER_H__
#define __GTKFBREADER_H__

#include <map>

#include <gtk/gtkwidget.h>
#include <gtk/gtkwindow.h>

#include "../common/fbreader/FBReader.h"

class GtkFBReader : public FBReader { 

private:
	static std::string ImageDirectory;
	
public:
	GtkFBReader();
	~GtkFBReader();

private:
	GtkWidget *addToolButton(GtkWidget *toolbar, const std::string &name, ActionCode code);

protected:
	void setWindowCaption(const std::string &caption) { gtk_window_set_title (myMainWindow, caption.c_str ()); }
	void setMode(ViewMode mode);
	void setButtonEnabled(ButtonId id, bool enable);
	void searchSlot();
	void cancelSlot();
	void fullscreenSlot() {}

public:
	gboolean handleKeySlot(GdkEventKey *);

	void close();

private:
	GtkWindow *myMainWindow;

	std::map<std::string,ActionCode> myKeyBindings;
	std::map<ButtonId,GtkWidget*> myButtons;
};

#endif /* __GTKFBREADER_H__ */

// vim:ts=2:sw=2:noet
