#
# CASA - Common Astronomy Software Applications
# Copyright (C) 2010 by ESO (in the framework of the ALMA collaboration)
#
# This file is part of CASA.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

casa_add_library( dish
  SDIterators/SDFITSDataIterator.cc
  SDIterators/SDIterator.cc
  SDIterators/SDIterAxes.cc
  SDIterators/SDIterFieldCopier.cc
  SDIterators/SDMSIterator.cc
  SDIterators/SDRecord.cc
  SDIterators/SDTableIterator.cc
  SDIterators/SDTabRecCopier.cc
   )

install(FILES
	SDIterators/SDFITSDataIterator.h
	SDIterators/SDIterAxes.h
	SDIterators/SDIterFieldCopier.h
	SDIterators/SDIterator.h
	SDIterators/SDMSIterator.h
	SDIterators/SDRecord.h
	SDIterators/SDTabRecCopier.h
	SDIterators/SDTableIterator.h
	DESTINATION include/casacode/dish/SDIterators
	)

casa_add_assay( dish SDIterators/test/tSDMSIterator.cc )
casa_add_assay( dish SDIterators/test/tSDRecord.cc )
casa_add_assay( dish SDIterators/test/tSDTableIterator.cc )
