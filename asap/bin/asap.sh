#!/bin/sh
#--------------------------------------------------------------------------
# Wrapper script for ATNF Spectral Analysis Package (ASAP).
#---------------------------------------------------------------------------
# Copyright (C) 2004,2005
# ATNF
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the Free
# Software Foundation; either version 2 of the License, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
# Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 675 Massachusetts Ave, Cambridge, MA 02139, USA.
#
# Correspondence concerning this software should be addressed as follows:
#        Internet email: Malte.Marquarding@csiro.au
#        Postal address: Malte Marquarding,
#                        Australia Telescope National Facility,
#                        P.O. Box 76,
#                        Epping, NSW, 2121,
#                        AUSTRALIA
#
# $Id: asap.sh 2496 2012-05-11 05:19:31Z MalteMarquarding $
#---------------------------------------------------------------------------
[ -x /usr/bin/clear ] && /usr/bin/clear

# do not remove this line it gets replaced on install with custom moduledir
# **PYTHONPATH**

echo "Loading ASAP..."

ip="`which ipython`"
p="`which python`"

# no ipython installation - run without all the goodies
if [ ! -x "$ip" ]; then
    echo "Can't find, or no execute permissions for 'ipython'"
    echo "Running asap through 'python':"
    $p -c 'from asap import *'
else
    # if run for the first time set-up ipythonrc profile and ~/.asap
    if [ ! -d "${HOME}/.asap" ]; then
        $p -c "import asap"
    fi
    # now execute ipython using the profile
    $ip -ipythondir "${HOME}/.asap" $*
fi
