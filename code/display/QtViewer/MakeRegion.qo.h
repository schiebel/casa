//# MakeRegion.qo.h: Qt implementation of viewer region maker window.
//# (This is the gui part only; it connects to region functions within
//# QtDisplayPanel).
//# Copyright (C) 2005
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$

#ifndef MAKEREGION_H
#define MAKEREGION_H

#include <casa/aips.h>
#include <casa/Containers/Record.h>
#include <display/QtViewer/QtDisplayPanel.qo.h>
#include <display/DisplayDatas/DisplayData.h>

#include <graphics/X11/X_enter.h>
#include <QtCore>
#include <QtGui>
#include <QList>
#include <QPushButton>
#include <QComboBox>
#include <graphics/X11/X_exit.h>

namespace casa {

	class RSComposite;
	class WCUnion;

	class MakeRegion : public QWidget {

		Q_OBJECT

	public:

		MakeRegion(QtDisplayPanel* qdp);
		~MakeRegion() {}

	public slots:

		//handle double click that announces a box is ready
		void drawRegion(Record mousereg, WorldCanvasHolder *wch);

		//React to new region creation in display panel.
		void newRegion_(String imgFilename);

		//handle switch display axis
		void changeAxis(String, String, String, int);

		//handle single click to select a box
		void activate(Record);

		//this is not used, it emits for any wc change, too often
		void wcChanged(const String,
		               const Vector<Double>, const Vector<Double>);

		virtual void closeEvent(QCloseEvent* event);

	signals:
		void hideRegionInImage();

	protected slots:

		//load masks from the image
		void loadRegionFromImage();

		//save masks to the image
		void saveRegionToImage();

		//delete region from the image
		void deleteRegionFromImage();

		//delete all masks
		void deleteAll();

		//toggle show or hide, but does not delete any
		void showHideAll();

		//toggle show or hide, but does not delete any
		void showHideGroup();

		//use a different color
		void colorAll(const QString&);

		//the work horse for all the edit commands
		void doIt();

		//show help text
		void showHelp();

		//create new box of this shape
		void reShape(const QString&);

		void zPlaneChanged();
		void pPlaneChanged();
	protected:

		void rotateBox(int cb);
		void addBox(RegionShape*);
		void deleteBox(RegionShape*);
		void reDraw();

		void addRegionsToShape(RSComposite*& theShapes,
		                       const WCRegion*& wcreg);

		WCUnion* unfoldCompositeRegionToSimpleUnion(const WCRegion*& wcreg);
		void unfoldIntoSimpleRegionPtrs(PtrBlock<const WCRegion*>& outRegPtrs,
		                                const WCRegion*& wcreg);

		//convert region to shape
		RSComposite *regionToShape(
		    QtDisplayData* qdd, const ImageRegion* wcreg);

		bool planeAllowed(String, String);

	private:

		QtDisplayPanel* qdp_;

		PtrBlock<const ImageRegion*> unionRegions_p;
		DisplayData* regData;

		RegionShape* activeShape;

		QTimer* timer;
		bool flash;

		int cb;
		int zIndex;
		int pIndex;
		String zAxis;

		QGroupBox* tGroup;
		QPushButton* removeAll;
		QMenu *showHideMenu;
		QPushButton* showHide;
		QComboBox* color;

		QLineEdit* chan;
		QLineEdit* corr;
		QLineEdit* name;
		QPushButton* load;
		QPushButton* save;

		/*
		QGroupBox* eGroup;
		QComboBox* shape;
		QPushButton* remove;
		QPushButton* rotateL;
		QPushButton* rotateR;
		QPushButton* left;
		QPushButton* right;
		QPushButton* up;
		QPushButton* down;
		QPushButton* wider;
		QPushButton* narrower;
		QPushButton* taller;
		QPushButton* shorter;
		*/

	};


}

#endif
