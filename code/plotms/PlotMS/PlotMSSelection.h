//# PlotMSSelection.h: MS Selection parameters.
//# Copyright (C) 2009
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
//# $Id: $
#ifndef PLOTMSSELECTION_H_
#define PLOTMSSELECTION_H_

#include <casa/Containers/Record.h>
#include <ms/MSSel/MSSelection.h>
#include <plotms/PlotMS/PlotMSConstants.h>

#include <map>

#include <casa/namespace.h>
using namespace std;

namespace casa {

//# Forward declarations
class MeasurementSet;
class NewCalTable;

// Specifies an MS selection.  See the mssSetData method in
// ms/MSSel/MSSelectionTools.h for details.
class PlotMSSelection {
public:
    // Static //
    
    // Enum and methods to define the different fields for an MS selection.
    // **If these are changed, also update: convenience methods below,
    // xmlcasa/implement/plotms/plotms*, xmlcasa/tasks/plotms.xml,
    // xmlcasa/scripts/task_plotms.py.**
    // <group>
    PMS_ENUM1(Field, fields, fieldStrings, field,
              FIELD, SPW, TIMERANGE, UVRANGE, ANTENNA, SCAN,
              CORR, ARRAY, OBSERVATION, INTENT, MSSELECT)
    PMS_ENUM2(Field, fields, fieldStrings, field,
              "field", "spw", "timerange", "uvrange", "antenna", "scan",
              "corr", "array", "observation", "intent", "msselect")
    // </group>
              
    // Returns the default value for the given selection field.  Returns an
    // empty String except for FIELD which returns "2".
    static String defaultValue(Field f);
    
    
    // Non-Static //
    
    // Default constructor.
    PlotMSSelection();
    
    // Copy constructor.  See operator=().
    PlotMSSelection(const PlotMSSelection& copy);
    
    // Destructor.
    ~PlotMSSelection();
    
    
    // Converts this object to/from a record.  The record keys are the values
    // of the Field enum in String form, and the values are the String values.
    // <group>
    void fromRecord(const RecordInterface& record);
    Record toRecord() const;
    // </group>
    
    //Print out an abbreviated summary of the selection.
    String toStringShort() const;

    // Applies this selection using the first MS into the second MS.  (See the
    // mssSetData method in ms/MSSel/MSSelectionTools.h for details.)
    void apply(MeasurementSet& ms, MeasurementSet& selectedMS,
               Vector<Vector<Slice> >& chansel,
	       Vector<Vector<Slice> >& corrsel);
        
    // Applies this selection to a NewCaltable
    void apply(NewCalTable& ct, NewCalTable& selectedCT,
               Vector<Vector<Slice> >& chansel,
    	       Vector<Vector<Slice> >& corrsel);
        
    // Gets/Sets the value for the given selection field.
    // <group>
    const String& getValue(Field f) const;
    void getValue(Field f, String& value) const { value = getValue(f); }
    void setValue(Field f, const String& value);
    // </group>
    
    // Convenience methods for returning the standard selection fields.
    // <group>
    const String& field() const     { return getValue(FIELD);     }
    const String& spw() const       { return getValue(SPW);       }
    const String& timerange() const { return getValue(TIMERANGE); }
    const String& uvrange() const   { return getValue(UVRANGE);   }
    const String& antenna() const   { return getValue(ANTENNA);   }
    const String& scan() const      { return getValue(SCAN);      }
    const String& corr() const      { return getValue(CORR);      }
    const String& array() const     { return getValue(ARRAY);     }
    const String& observation() const {return getValue(OBSERVATION);}
    const String& intent() const    { return getValue(INTENT);}
    const String& msselect() const  { return getValue(MSSELECT);  }
    const Int& forceNew() const     { return forceNew_; }
    // </group>
    
    // Convenience methods for setting the standard selection fields.
    // <group>
    void setField(const String& v)     { setValue(FIELD, v);     }
    void setSpw(const String& v)       { setValue(SPW, v);       }
    void setTimerange(const String& v) { setValue(TIMERANGE, v); }
    void setUvrange(const String& v)   { setValue(UVRANGE, v);   }
    void setAntenna(const String& v)   { setValue(ANTENNA, v);   }
    void setScan(const String& v)      { setValue(SCAN, v);      }
    void setCorr(const String& v)      { setValue(CORR, v);      }
    void setArray(const String& v)     { setValue(ARRAY, v);     }
    void setObservation(const String& v) {setValue(OBSERVATION, v); }
    void setIntent(const String& v)    { setValue(INTENT, v); }
    void setMsselect(const String& v)  { setValue(MSSELECT, v);  }
    void setForceNew(const Int& forcenew) { forceNew_ = forcenew; }
    // </group>
    
    // Equality operators.
    // <group>
    bool operator==(const PlotMSSelection& other) const;
    bool operator!=(const PlotMSSelection& other) const {
        return !(operator==(other)); }

    bool fieldsEqual(const PlotMSSelection& other) const;
    bool fieldsNotEqual(const PlotMSSelection& other) const {
        return !fieldsEqual(other); }

    // </group>
    
    // Copy operator.
    PlotMSSelection& operator=(const PlotMSSelection& copy);
    
    //Return lists of antennas that have been selected.
    Vector<int> getSelectedAntennas1();
    Vector<int> getSelectedAntennas2();

    //Returns whether or not any selections have been made.
    bool isEmpty() const;
private:    
    // Selection field values.
    map<Field, String> itsValues_;



    // Force appearance of new selection (even when not new)
    Int forceNew_;
	Vector<int> selAnts;
	Vector<int> selAnts2;
    
    // Initializes the values to their defaults.
    void initDefaults();

};

}

#endif /* PLOTMSSELECTION_H_ */
