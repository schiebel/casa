//
// C++ Implementation: STHistory
//
// Description:
//
//
// Author: Malte Marquarding <asap@atnf.csiro.au>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include <casa/Exceptions/Error.h>
#include <tables/Tables/TableDesc.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/TableParse.h>
#include <tables/Tables/TableRow.h>
#include <tables/Tables/TableCopy.h>

#include "STDefs.h"
#include "STHistory.h"
#include "MathUtils.h"

using namespace casa;

namespace asap {

const casa::String STHistory::name_ = "HISTORY";

STHistory::STHistory(const Scantable& parent ) :
  STSubTable( parent, name_ )
{
  setup();
}

asap::STHistory::STHistory( casa::Table tab ) : STSubTable(tab, name_)
{
  itemCol_.attach(table_,"ITEM");
}

STHistory::~STHistory()
{
}

STHistory& asap::STHistory::operator =( const STHistory & other )
{
  if (this != &other) {
    static_cast<STSubTable&>(*this) = other;
    itemCol_.attach(table_,"ITEM");
  }
  return *this;
}
void asap::STHistory::setup( )
{
  // add to base class table
  table_.addColumn(ScalarColumnDesc<String>("ITEM"));

  // new cached columns
  itemCol_.attach(table_,"ITEM");
}

uInt STHistory::addEntry( const String& item)
{
  uInt rno = table_.nrow();
  table_.addRow();
  itemCol_.put(rno, item);
  idCol_.put(rno, 0);
  return 0;
}

void asap::STHistory::getEntry( String& item, uInt id)
{
  Table t = table_(table_.col("ID") == Int(id), 1 );
  if (t.nrow() == 0 ) {
    throw(AipsError("STHistory::getEntry - id out of range"));
  }
  item = "";
}

void asap::STHistory::append( const STHistory & other )
{
  const Table& t = other.table();
  if (other.nrow() > 0) {
    addEntry(asap::SEPERATOR);
    TableCopy::copyRows(table_, t, table_.nrow(), 0, t.nrow());
    addEntry(asap::SEPERATOR);
  }

}

std::vector<std::string> asap::STHistory::getHistory( int nrow, 
						      int start) const
{
  if (nrow < 0) {
    nrow = this->nrow();
  }
  AlwaysAssert(nrow <= this->nrow(), AipsError);
  Vector<String> rows;
  Slicer slice(IPosition(1, start), IPosition(1, nrow));
  
  rows = itemCol_.getColumnRange(slice);
  return mathutil::tovectorstring(rows);
}

  void asap::STHistory::drop() {
    table_.removeRow(table_.rowNumbers());
  }

}
