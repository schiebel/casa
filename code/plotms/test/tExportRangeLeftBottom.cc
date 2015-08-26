//# Copyright (C) 2008
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
//#
//# $Id$

#include <plotms/PlotMS/PlotMS.h>
#include <plotms/Plots/PlotMSPlotParameterGroups.h>
#include <plotms/Plots/PlotMSPlot.h>
#include <plotms/test/tUtil.h>


#include <iostream>
#include <msvis/MSVis/UtilJ.h>
#include <casa/namespace.h>
#include <QApplication>

/**
 * Tests whether an iteration plot, with two pages, can be exported
 * in a single pass.  We use a 2x3 grid and a common left,bottom axis
 */

int main(int /*argc*/, char** /*argv[]*/) {

	String dataPath = tUtil::getFullPath( "pm_ngc5921.ms" );
    cout << "tExportRangeLeftBottom using data from "<<dataPath.c_str()<<endl;

    // Set up plotms object.
    PlotMSApp app(false, false);

    //Make a 2x3 grid
    PlotMSParameters& overallParams = app.getParameters();
    overallParams.setRowCount( 2 );
    overallParams.setColCount( 3 );

    // Set up parameters for plot.
    PlotMSPlotParameters plotParams = PlotMSPlot::makeParameters(&app);

    PMS_PP_MSData* ppdata = plotParams.typedGroup<PMS_PP_MSData>();
    if (ppdata == NULL) {
        plotParams.setGroup<PMS_PP_MSData>();
        ppdata = plotParams.typedGroup<PMS_PP_MSData>();
    }
    ppdata->setFilename( dataPath );

    //We are going to iterate over scan, using a shared axis and global
    //scale for the iteration.
    PMS_PP_Iteration* iterationParams = plotParams.typedGroup<PMS_PP_Iteration>();
    if ( iterationParams == NULL ){
    	plotParams.setGroup<PMS_PP_Iteration>();
    	iterationParams = plotParams.typedGroup<PMS_PP_Iteration>();
    }
    iterationParams->setIterationAxis( PMS::SCAN );
    iterationParams->setGlobalScaleX( true );
    iterationParams->setGlobalScaleY( true );

    iterationParams->setCommonAxisX( true );
    iterationParams->setCommonAxisY( true );

    //Set a right y-axis and a top x-axis
    PMS_PP_Axes* axisParams = plotParams.typedGroup<PMS_PP_Axes>();
    if (axisParams == NULL) {
    	plotParams.setGroup<PMS_PP_Axes>();
        axisParams = plotParams.typedGroup<PMS_PP_Axes>();
    }
    axisParams->setAxes(X_BOTTOM, Y_LEFT, 0 );

    //We want to print all (2) pages in the output.
    PlotMSExportParam& exportParams = app.getExportParameters();
    exportParams.setExportRange( PMS::PAGE_ALL );

    //Make the plot.
    app.addOverPlot( &plotParams );

    String outFile( "/tmp/plotMSExportRangeLeftBottom");
    String outFile1( outFile+"_Scan1,2,3,4,5,6.jpg");
    String outFile2( outFile + "_Scan7_2.jpg");
    tUtil::clearFile( outFile1 );
    tUtil::clearFile( outFile2 );
    PlotExportFormat::Type type = PlotExportFormat::JPG;
	PlotExportFormat format(type, outFile+".jpg" );
	format.resolution = PlotExportFormat::SCREEN;
	bool ok = app.save(format);
	cout << "tExportRangeLeftBottom:: Result of save="<<ok<<endl;
    

	ok = tUtil::checkFile( outFile1, 80000, 125000, -1 );
	cout << "tExportRangeLeftBottom:: Result of first save file check="<<ok<<endl;

	//There should be 2 output files.
	if ( ok ){
		ok = tUtil::checkFile( outFile2, 40000, 50000, -1 );
		cout << "tExportRangeLeftBottom:  Result of second save file check="<<ok<<endl;
	}
	return tUtil::exitMain( false );
}

