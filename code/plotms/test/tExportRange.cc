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


void exportPlot(PlotMSApp& app, String outFile, PlotExportFormat::Type type,
                String strType, int minSize, int maxSize ){
	tUtil::clearFile( outFile+"_Scan1,2,3,4" );
	tUtil::clearFile( outFile+"_Scan5,6,7_2" );

	PlotExportFormat format(type, outFile );
	format.resolution = PlotExportFormat::SCREEN;
	bool ok = app.save(format);
	cout << "tExportRange:: "<<strType.c_str()<<" Result of save="<<ok<<endl;

	ok = tUtil::checkFile( outFile+"_Scan1,2,3,4", minSize, maxSize, -1 );
	cout << "tExportRange:: "<<strType.c_str()<<" Result of first save file check="<<ok<<endl;

	//There should be 2 output files.
	if ( ok ){
		ok = tUtil::checkFile( outFile+"_Scan5,6,7_2", minSize, maxSize, -1 );
		cout << "tExportRange:: "<<strType.c_str()<<" Result of second save file check="<<ok<<endl;
	}
}

void exportAsJPG( PlotMSApp& app ){

    String outFile( "/tmp/plotMSExportRangeJPGTest");

    PlotExportFormat::Type type = PlotExportFormat::JPG;
    exportPlot( app, outFile, type, "JPG", 80000, 110000);
}

void exportAsPNG( PlotMSApp& app ){

    String outFile( "/tmp/plotMSExportRangePNGTest");

    PlotExportFormat::Type type = PlotExportFormat::PNG;
    exportPlot(  app, outFile, type, "PNG", 20000, 30000 );

}

void exportAsPS( PlotMSApp& app ){

    String outFile( "/tmp/plotMSExportRangePSTest");

    PlotExportFormat::Type type = PlotExportFormat::PS;
    exportPlot(  app, outFile, type, "PS", 700000, 920000);

}

void exportAsPDF( PlotMSApp& app ){

    String outFile( "/tmp/plotMSExportRangePDFTest");

    PlotExportFormat::Type type = PlotExportFormat::PDF;
    cout << "Exporting PDF"<<endl;
    exportPlot(  app, outFile, type, "PDF", 60000, 78000);

}

void exportAsText( PlotMSApp& app ){

    String outFile( "/tmp/plotMSExportRangeTextTest");
    cout << "Exporting Text"<<endl;
    PlotExportFormat::Type type = PlotExportFormat::TEXT;
    exportPlot(  app, outFile, type, "TEXT", 20000, 30000 );

}


/**
 * Tests whether an iteration plot, with two pages, can be exported
 * in a single pass.
 */

int main(int /*argc*/, char** /*argv[]*/) {

	String dataPath = tUtil::getFullPath( "pm_ngc5921.ms" );
    cout << "tExportRange using data from "<<dataPath.c_str()<<endl;

    // Set up plotms object.
    PlotMSApp app(false, false);

    //Make a 2x2 grid
    PlotMSParameters& overallParams = app.getParameters();
    overallParams.setRowCount( 2 );
    overallParams.setColCount( 2 );

    // Set up parameters for plot.
    PlotMSPlotParameters plotParams = PlotMSPlot::makeParameters(&app);

    PMS_PP_MSData* ppdata = plotParams.typedGroup<PMS_PP_MSData>();
    if (ppdata == NULL) {
        plotParams.setGroup<PMS_PP_MSData>();
        ppdata = plotParams.typedGroup<PMS_PP_MSData>();
    }
    ppdata->setFilename( dataPath );

    //We are going to iterate over scan, using a shared axis and global
    //scale with a 2 x 2 grid for the iteration.
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

    //We want to print all (2) pages in the output.
    PlotMSExportParam& exportParams = app.getExportParameters();
    exportParams.setExportRange( PMS::PAGE_ALL );

    //Make the plot.
    app.addOverPlot( &plotParams );

    exportAsJPG( app );
    exportAsPNG( app );

    //Too much data to export as text.
    //exportAsText( app );
    exportAsPDF( app );
    exportAsPS( app );

    return tUtil::exitMain( false );
}

