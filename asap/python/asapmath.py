import re
from asap.scantable import scantable
from asap.parameters import rcParams
from asap.logging import asaplog, asaplog_post_dec
from asap.selector import selector
from asap.asapplotter import new_asaplot
from matplotlib import rc as rcp

@asaplog_post_dec
def average_time(*args, **kwargs):
    """
    Return the (time) average of a scan or list of scans. [in channels only]
    The cursor of the output scan is set to 0
    Parameters:
        one scan or comma separated  scans or a list of scans
        mask:     an optional mask (only used for 'var' and 'tsys' weighting)
        scanav:   True averages each scan separately.
                  False (default) averages all scans together,
        weight:   Weighting scheme.
                    'none'     (mean no weight)
                    'var'      (1/var(spec) weighted)
                    'tsys'     (1/Tsys**2 weighted)
                    'tint'     (integration time weighted)
                    'tintsys'  (Tint/Tsys**2)
                    'median'   ( median averaging)
        align:    align the spectra in velocity before averaging. It takes
                  the time of the first spectrum in the first scantable
                  as reference time.
        compel:   True forces to average overwrapped IFs.
    Example:
        # return a time averaged scan from scana and scanb
        # without using a mask
        scanav = average_time(scana,scanb)
        # or equivalent
        # scanav = average_time([scana, scanb])
        # return the (time) averaged scan, i.e. the average of
        # all correlator cycles
        scanav = average_time(scan, scanav=True)
    """
    scanav = False
    if kwargs.has_key('scanav'):
       scanav = kwargs.get('scanav')
    weight = 'tint'
    if kwargs.has_key('weight'):
       weight = kwargs.get('weight')
    mask = ()
    if kwargs.has_key('mask'):
        mask = kwargs.get('mask')
    align = False
    if kwargs.has_key('align'):
        align = kwargs.get('align')
    compel = False
    if kwargs.has_key('compel'):
        compel = kwargs.get('compel')
    varlist = vars()
    if isinstance(args[0],list):
        lst = args[0]
    elif isinstance(args[0],tuple):
        lst = list(args[0])
    else:
        lst = list(args)

    del varlist["kwargs"]
    varlist["args"] = "%d scantables" % len(lst)
    # need special formatting here for history...

    from asap._asap import stmath
    stm = stmath()
    for s in lst:
        if not isinstance(s,scantable):
            msg = "Please give a list of scantables"
            raise TypeError(msg)
    if scanav: scanav = "SCAN"
    else: scanav = "NONE"
    alignedlst = []
    if align:
        refepoch = lst[0].get_time(0)
        for scan in lst:
            alignedlst.append(scan.freq_align(refepoch,insitu=False))
    else:
        alignedlst = lst
    if weight.upper() == 'MEDIAN':
        # median doesn't support list of scantables - merge first
        merged = None
        if len(alignedlst) > 1:
            merged = merge(alignedlst)
        else:
            merged = alignedlst[0]
        s = scantable(stm._averagechannel(merged, 'MEDIAN', scanav))
        del merged
    else:
        #s = scantable(stm._average(alignedlst, mask, weight.upper(), scanav))
        s = scantable(stm._new_average(alignedlst, compel, mask, 
                                       weight.upper(), scanav))
    s._add_history("average_time",varlist)

    return s

@asaplog_post_dec
def quotient(source, reference, preserve=True):
    """
    Return the quotient of a 'source' (signal) scan and a 'reference' scan.
    The reference can have just one scan, even if the signal has many. Otherwise
    they must have the same number of scans.
    The cursor of the output scan is set to 0
    Parameters:
        source:        the 'on' scan
        reference:     the 'off' scan
        preserve:      you can preserve (default) the continuum or
                       remove it.  The equations used are
                       preserve:  Output = Toff * (on/off) - Toff
                       remove:    Output = Toff * (on/off) - Ton
    """
    varlist = vars()
    from asap._asap import stmath
    stm = stmath()
    stm._setinsitu(False)
    s = scantable(stm._quotient(source, reference, preserve))
    s._add_history("quotient",varlist)
    return s

@asaplog_post_dec
def dototalpower(calon, caloff, tcalval=0.0):
    """
    Do calibration for CAL on,off signals.
    Adopted from GBTIDL dototalpower
    Parameters:
        calon:         the 'cal on' subintegration
        caloff:        the 'cal off' subintegration
        tcalval:       user supplied Tcal value
    """
    varlist = vars()
    from asap._asap import stmath
    stm = stmath()
    stm._setinsitu(False)
    s = scantable(stm._dototalpower(calon, caloff, tcalval))
    s._add_history("dototalpower",varlist)
    return s

@asaplog_post_dec
def dosigref(sig, ref, smooth, tsysval=0.0, tauval=0.0):
    """
    Calculate a quotient (sig-ref/ref * Tsys)
    Adopted from GBTIDL dosigref
    Parameters:
        sig:         on source data
        ref:         reference data
        smooth:      width of box car smoothing for reference
        tsysval:     user specified Tsys (scalar only)
        tauval:      user specified Tau (required if tsysval is set)
    """
    varlist = vars()
    from asap._asap import stmath
    stm = stmath()
    stm._setinsitu(False)
    s = scantable(stm._dosigref(sig, ref, smooth, tsysval, tauval))
    s._add_history("dosigref",varlist)
    return s

@asaplog_post_dec
def calps(scantab, scannos, smooth=1, tsysval=0.0, tauval=0.0, tcalval=0.0, verify=False):
    """
    Calibrate GBT position switched data
    Adopted from GBTIDL getps
    Currently calps identify the scans as position switched data if source
    type enum is pson or psoff. The data must contains 'CAL' signal
    on/off in each integration. To identify 'CAL' on state, the source type 
    enum of poncal and poffcal need to be present.

    Parameters:
        scantab:       scantable
        scannos:       list of scan numbers
        smooth:        optional box smoothing order for the reference
                       (default is 1 = no smoothing)
        tsysval:       optional user specified Tsys (default is 0.0,
                       use Tsys in the data)
        tauval:        optional user specified Tau
        tcalval:       optional user specified Tcal (default is 0.0,
                       use Tcal value in the data)
        verify:        Verify calibration if true
    """
    varlist = vars()
    # check for the appropriate data
##    s = scantab.get_scan('*_ps*')
##     if s is None:
##         msg = "The input data appear to contain no position-switch mode data."
##         raise TypeError(msg)
    s = scantab.copy()
    from asap._asap import srctype
    sel = selector()
    sel.set_types( srctype.pson )
    try:
        scantab.set_selection( sel )
    except Exception, e:
        msg = "The input data appear to contain no position-switch mode data."
        raise TypeError(msg)
    s.set_selection()
    sel.reset()
    ssub = s.get_scan(scannos)
    if ssub is None:
        msg = "No data was found with given scan numbers!"
        raise TypeError(msg)
    #ssubon = ssub.get_scan('*calon')
    #ssuboff = ssub.get_scan('*[^calon]')
    sel.set_types( [srctype.poncal,srctype.poffcal] )
    ssub.set_selection( sel )
    ssubon = ssub.copy()
    ssub.set_selection()
    sel.reset()
    sel.set_types( [srctype.pson,srctype.psoff] )
    ssub.set_selection( sel )
    ssuboff = ssub.copy()
    ssub.set_selection()
    sel.reset()
    if ssubon.nrow() != ssuboff.nrow():
        msg = "mismatch in numbers of CAL on/off scans. Cannot calibrate. Check the scan numbers."
        raise TypeError(msg)
    cals = dototalpower(ssubon, ssuboff, tcalval)
    #sig = cals.get_scan('*ps')
    #ref = cals.get_scan('*psr')
    sel.set_types( srctype.pson )
    cals.set_selection( sel )
    sig = cals.copy()
    cals.set_selection()
    sel.reset()
    sel.set_types( srctype.psoff )
    cals.set_selection( sel )
    ref = cals.copy()
    cals.set_selection()
    sel.reset()
    if sig.nscan() != ref.nscan():
        msg = "mismatch in numbers of on/off scans. Cannot calibrate. Check the scan numbers."
        raise TypeError(msg)

    #for user supplied Tsys
    if tsysval>0.0:
        if tauval<=0.0:
            msg = "Need to supply a valid tau to use the supplied Tsys"
            raise TypeError(msg)
        else:
            sig.recalc_azel()
            ref.recalc_azel()
            #msg = "Use of user specified Tsys is not fully implemented yet."
            #raise TypeError(msg)
            # use get_elevation to get elevation and
            # calculate a scaling factor using the formula
            # -> tsys use to dosigref

    #ress = dosigref(sig, ref, smooth, tsysval)
    ress = dosigref(sig, ref, smooth, tsysval, tauval)
    ###
    if verify:
        # get data
        import numpy
        precal={}
        postcal=[]
        keys=['ps','ps_calon','psr','psr_calon']
        types=[srctype.pson,srctype.poncal,srctype.psoff,srctype.poffcal]
        ifnos=list(ssub.getifnos())
        polnos=list(ssub.getpolnos())
        sel=selector()
        for i in range(2):
            #ss=ssuboff.get_scan('*'+keys[2*i])
            ll=[]
            for j in range(len(ifnos)):
                for k in range(len(polnos)):
                    sel.set_ifs(ifnos[j])
                    sel.set_polarizations(polnos[k])
                    sel.set_types(types[2*i])
                    try:
                        #ss.set_selection(sel)
                        ssuboff.set_selection(sel)
                    except:
                        continue
                    #ll.append(numpy.array(ss._getspectrum(0)))
                    ll.append(numpy.array(ssuboff._getspectrum(0)))
                    sel.reset()
                    ssuboff.set_selection()
            precal[keys[2*i]]=ll
            #del ss
            #ss=ssubon.get_scan('*'+keys[2*i+1])
            ll=[]
            for j in range(len(ifnos)):
                for k in range(len(polnos)):
                    sel.set_ifs(ifnos[j])
                    sel.set_polarizations(polnos[k])
                    sel.set_types(types[2*i+1])
                    try:
                        #ss.set_selection(sel)
                        ssubon.set_selection(sel)
                    except:
                        continue
                    #ll.append(numpy.array(ss._getspectrum(0)))
                    ll.append(numpy.array(ssubon._getspectrum(0)))
                    sel.reset()
                    ssubon.set_selection()
            precal[keys[2*i+1]]=ll
            #del ss
        for j in range(len(ifnos)):
            for k in range(len(polnos)):
                sel.set_ifs(ifnos[j])
                sel.set_polarizations(polnos[k])
                try:
                    ress.set_selection(sel)
                except:
                    continue
                postcal.append(numpy.array(ress._getspectrum(0)))
                sel.reset()
                ress.set_selection()
        del sel
        # plot
        asaplog.post()
        asaplog.push('Plot only first spectrum for each [if,pol] pairs to verify calibration.')
        asaplog.post('WARN')
        p=new_asaplot()
        rcp('lines', linewidth=1)
        #nr=min(6,len(ifnos)*len(polnos))
        nr=len(ifnos)*len(polnos)
        titles=[]
        btics=[]
        if nr<4:
            p.set_panels(rows=nr,cols=2,nplots=2*nr,ganged=False)
            for i in range(2*nr):
                b=False
                if i >= 2*nr-2:
                    b=True
                btics.append(b)
        elif nr==4:
            p.set_panels(rows=2,cols=4,nplots=8,ganged=False)
            for i in range(2*nr):
                b=False
                if i >= 2*nr-4:
                    b=True
                btics.append(b)
        elif nr<7:
            p.set_panels(rows=3,cols=4,nplots=2*nr,ganged=False)
            for i in range(2*nr):
                if i >= 2*nr-4:
                    b=True
                btics.append(b)
        else:
            asaplog.post()
            asaplog.push('Only first 6 [if,pol] pairs are plotted.')
            asaplog.post('WARN')
            nr=6
            for i in range(2*nr):
                b=False
                if i >= 2*nr-4:
                    b=True
                btics.append(b)
            p.set_panels(rows=3,cols=4,nplots=2*nr,ganged=False)
        for i in range(nr):
            p.subplot(2*i)
            p.color=0
            title='raw data IF%s POL%s' % (ifnos[int(i/len(polnos))],polnos[i%len(polnos)])
            titles.append(title)
            #p.set_axes('title',title,fontsize=40)
            ymin=1.0e100
            ymax=-1.0e100
            nchan=s.nchan(ifnos[int(i/len(polnos))])
            edge=int(nchan*0.01)
            for j in range(4):
                spmin=min(precal[keys[j]][i][edge:nchan-edge])
                spmax=max(precal[keys[j]][i][edge:nchan-edge])
                ymin=min(ymin,spmin)
                ymax=max(ymax,spmax)
            for j in range(4):
                if i==0:
                    p.set_line(label=keys[j])
                else:
                    p.legend()
                p.plot(precal[keys[j]][i])
            p.axes.set_ylim(ymin-0.1*abs(ymin),ymax+0.1*abs(ymax))
            if not btics[2*i]:
                p.axes.set_xticks([])
            p.subplot(2*i+1)
            p.color=0
            title='cal data IF%s POL%s' % (ifnos[int(i/len(polnos))],polnos[i%len(polnos)])
            titles.append(title)
            #p.set_axes('title',title)
            p.legend()
            ymin=postcal[i][edge:nchan-edge].min()
            ymax=postcal[i][edge:nchan-edge].max()
            p.plot(postcal[i])
            p.axes.set_ylim(ymin-0.1*abs(ymin),ymax+0.1*abs(ymax))
            if not btics[2*i+1]:
                p.axes.set_xticks([])
        for i in range(2*nr):
            p.subplot(i)
            p.set_axes('title',titles[i],fontsize='medium')
        x=raw_input('Accept calibration ([y]/n): ' )
        if x.upper() == 'N':
            p.quit()
            del p
            return scantab
        p.quit()
        del p
    ###
    ress._add_history("calps", varlist)
    return ress

@asaplog_post_dec
def calnod(scantab, scannos=[], smooth=1, tsysval=0.0, tauval=0.0, tcalval=0.0, verify=False):
    """
    Do full (but a pair of scans at time) processing of GBT Nod data
    calibration.
    Adopted from  GBTIDL's getnod
    Parameters:
        scantab:     scantable
        scannos:     a pair of scan numbers, or the first scan number of the pair
        smooth:      box car smoothing order
        tsysval:     optional user specified Tsys value
        tauval:      optional user specified tau value (not implemented yet)
        tcalval:     optional user specified Tcal value
        verify:       Verify calibration if true
    """
    varlist = vars()
    from asap._asap import stmath
    from asap._asap import srctype
    stm = stmath()
    stm._setinsitu(False)

    # check for the appropriate data
##     s = scantab.get_scan('*_nod*')
##     if s is None:
##         msg = "The input data appear to contain no Nod observing mode data."
##         raise TypeError(msg)
    s = scantab.copy()
    sel = selector()
    sel.set_types( srctype.nod )
    try:
        s.set_selection( sel )
    except Exception, e:
        msg = "The input data appear to contain no Nod observing mode data."
        raise TypeError(msg)
    sel.reset()
    del sel
    del s

    # need check correspondance of each beam with sig-ref ...
    # check for timestamps, scan numbers, subscan id (not available in
    # ASAP data format...). Assume 1st scan of the pair (beam 0 - sig
    # and beam 1 - ref...)
    # First scan number of paired scans or list of pairs of
    # scan numbers (has to have even number of pairs.)

    #data splitting
    scan1no = scan2no = 0

    if len(scannos)==1:
        scan1no = scannos[0]
        scan2no = scannos[0]+1
        pairScans = [scan1no, scan2no]
    else:
        #if len(scannos)>2:
        #    msg = "calnod can only process a pair of nod scans at time."
        #    raise TypeError(msg)
        #
        #if len(scannos)==2:
        #    scan1no = scannos[0]
        #    scan2no = scannos[1]
        pairScans = list(scannos)

    if tsysval>0.0:
        if tauval<=0.0:
            msg = "Need to supply a valid tau to use the supplied Tsys"
            raise TypeError(msg)
        else:
            scantab.recalc_azel()
    resspec = scantable(stm._donod(scantab, pairScans, smooth, tsysval,tauval,tcalval))
    ###
    if verify:
        # get data
        import numpy
        precal={}
        postcal=[]
        keys=['','_calon']
        types=[srctype.nod,srctype.nodcal]
        ifnos=list(scantab.getifnos())
        polnos=list(scantab.getpolnos())
        sel=selector()
        ss = scantab.copy()
        for i in range(2):
            #ss=scantab.get_scan('*'+keys[i])
            ll=[]
            ll2=[]
            for j in range(len(ifnos)):
                for k in range(len(polnos)):
                    sel.set_ifs(ifnos[j])
                    sel.set_polarizations(polnos[k])
                    sel.set_scans(pairScans[0])
                    sel.set_types(types[i])
                    try:
                        ss.set_selection(sel)
                    except:
                        continue
                    ll.append(numpy.array(ss._getspectrum(0)))
                    sel.reset()
                    ss.set_selection()
                    sel.set_ifs(ifnos[j])
                    sel.set_polarizations(polnos[k])
                    sel.set_scans(pairScans[1])
                    sel.set_types(types[i])
                    try:
                        ss.set_selection(sel)
                    except:
                        ll.pop()
                        continue
                    ll2.append(numpy.array(ss._getspectrum(0)))
                    sel.reset()
                    ss.set_selection()
            key='%s%s' %(pairScans[0],keys[i])
            precal[key]=ll
            key='%s%s' %(pairScans[1],keys[i])
            precal[key]=ll2
            #del ss
        keys=precal.keys()
        for j in range(len(ifnos)):
            for k in range(len(polnos)):
                sel.set_ifs(ifnos[j])
                sel.set_polarizations(polnos[k])
                sel.set_scans(pairScans[0])
                try:
                    resspec.set_selection(sel)
                except:
                    continue
                postcal.append(numpy.array(resspec._getspectrum(0)))
                sel.reset()
                resspec.set_selection()
        del sel
        # plot
        asaplog.post()
        asaplog.push('Plot only first spectrum for each [if,pol] pairs to verify calibration.')
        asaplog.post('WARN')
        p=new_asaplot()
        rcp('lines', linewidth=1)
        #nr=min(6,len(ifnos)*len(polnos))
        nr=len(ifnos)*len(polnos)
        titles=[]
        btics=[]
        if nr<4:
            p.set_panels(rows=nr,cols=2,nplots=2*nr,ganged=False)
            for i in range(2*nr):
                b=False
                if i >= 2*nr-2:
                    b=True
                btics.append(b)
        elif nr==4:
            p.set_panels(rows=2,cols=4,nplots=8,ganged=False)
            for i in range(2*nr):
                b=False
                if i >= 2*nr-4:
                    b=True
                btics.append(b)
        elif nr<7:
            p.set_panels(rows=3,cols=4,nplots=2*nr,ganged=False)
            for i in range(2*nr):
                if i >= 2*nr-4:
                    b=True
                btics.append(b)
        else:
            asaplog.post()
            asaplog.push('Only first 6 [if,pol] pairs are plotted.')
            asaplog.post('WARN')
            nr=6
            for i in range(2*nr):
                b=False
                if i >= 2*nr-4:
                    b=True
                btics.append(b)
            p.set_panels(rows=3,cols=4,nplots=2*nr,ganged=False)
        for i in range(nr):
            p.subplot(2*i)
            p.color=0
            title='raw data IF%s POL%s' % (ifnos[int(i/len(polnos))],polnos[i%len(polnos)])
            titles.append(title)
            #p.set_axes('title',title,fontsize=40)
            ymin=1.0e100
            ymax=-1.0e100
            nchan=scantab.nchan(ifnos[int(i/len(polnos))])
            edge=int(nchan*0.01)
            for j in range(4):
                spmin=min(precal[keys[j]][i][edge:nchan-edge])
                spmax=max(precal[keys[j]][i][edge:nchan-edge])
                ymin=min(ymin,spmin)
                ymax=max(ymax,spmax)
            for j in range(4):
                if i==0:
                    p.set_line(label=keys[j])
                else:
                    p.legend()
                p.plot(precal[keys[j]][i])
            p.axes.set_ylim(ymin-0.1*abs(ymin),ymax+0.1*abs(ymax))
            if not btics[2*i]:
                p.axes.set_xticks([])
            p.subplot(2*i+1)
            p.color=0
            title='cal data IF%s POL%s' % (ifnos[int(i/len(polnos))],polnos[i%len(polnos)])
            titles.append(title)
            #p.set_axes('title',title)
            p.legend()
            ymin=postcal[i][edge:nchan-edge].min()
            ymax=postcal[i][edge:nchan-edge].max()
            p.plot(postcal[i])
            p.axes.set_ylim(ymin-0.1*abs(ymin),ymax+0.1*abs(ymax))
            if not btics[2*i+1]:
                p.axes.set_xticks([])
        for i in range(2*nr):
            p.subplot(i)
            p.set_axes('title',titles[i],fontsize='medium')
        x=raw_input('Accept calibration ([y]/n): ' )
        if x.upper() == 'N':
            p.quit()
            del p
            return scantab
        p.quit()
        del p
    ###
    resspec._add_history("calnod",varlist)
    return resspec

@asaplog_post_dec
def calfs(scantab, scannos=[], smooth=1, tsysval=0.0, tauval=0.0, tcalval=0.0, verify=False):
    """
    Calibrate GBT frequency switched data.
    Adopted from GBTIDL getfs.
    Currently calfs identify the scans as frequency switched data if source
    type enum is fson and fsoff. The data must contains 'CAL' signal
    on/off in each integration. To identify 'CAL' on state, the source type 
    enum of foncal and foffcal need to be present.

    Parameters:
        scantab:       scantable
        scannos:       list of scan numbers
        smooth:        optional box smoothing order for the reference
                       (default is 1 = no smoothing)
        tsysval:       optional user specified Tsys (default is 0.0,
                       use Tsys in the data)
        tauval:        optional user specified Tau
        verify:        Verify calibration if true
    """
    varlist = vars()
    from asap._asap import stmath
    from asap._asap import srctype
    stm = stmath()
    stm._setinsitu(False)

#    check = scantab.get_scan('*_fs*')
#    if check is None:
#        msg = "The input data appear to contain no Nod observing mode data."
#        raise TypeError(msg)
    s = scantab.get_scan(scannos)
    del scantab

    resspec = scantable(stm._dofs(s, scannos, smooth, tsysval,tauval,tcalval))
    ###
    if verify:
        # get data
        ssub = s.get_scan(scannos)
        #ssubon = ssub.get_scan('*calon')
        #ssuboff = ssub.get_scan('*[^calon]')
        sel = selector()
        sel.set_types( [srctype.foncal,srctype.foffcal] )
        ssub.set_selection( sel )
        ssubon = ssub.copy()
        ssub.set_selection()
        sel.reset()
        sel.set_types( [srctype.fson,srctype.fsoff] )
        ssub.set_selection( sel )
        ssuboff = ssub.copy()
        ssub.set_selection()
        sel.reset()
        import numpy
        precal={}
        postcal=[]
        keys=['fs','fs_calon','fsr','fsr_calon']
        types=[srctype.fson,srctype.foncal,srctype.fsoff,srctype.foffcal]
        ifnos=list(ssub.getifnos())
        polnos=list(ssub.getpolnos())
        for i in range(2):
            #ss=ssuboff.get_scan('*'+keys[2*i])
            ll=[]
            for j in range(len(ifnos)):
                for k in range(len(polnos)):
                    sel.set_ifs(ifnos[j])
                    sel.set_polarizations(polnos[k])
                    sel.set_types(types[2*i])
                    try:
                        #ss.set_selection(sel)
                        ssuboff.set_selection(sel)
                    except:
                        continue
                    ll.append(numpy.array(ss._getspectrum(0)))
                    sel.reset()
                    #ss.set_selection()
                    ssuboff.set_selection()
            precal[keys[2*i]]=ll
            #del ss
            #ss=ssubon.get_scan('*'+keys[2*i+1])
            ll=[]
            for j in range(len(ifnos)):
                for k in range(len(polnos)):
                    sel.set_ifs(ifnos[j])
                    sel.set_polarizations(polnos[k])
                    sel.set_types(types[2*i+1])
                    try:
                        #ss.set_selection(sel)
                        ssubon.set_selection(sel)
                    except:
                        continue
                    ll.append(numpy.array(ss._getspectrum(0)))
                    sel.reset()
                    #ss.set_selection()
                    ssubon.set_selection()
            precal[keys[2*i+1]]=ll
            #del ss
        #sig=resspec.get_scan('*_fs')
        #ref=resspec.get_scan('*_fsr')
        sel.set_types( srctype.fson )
        resspec.set_selection( sel )
        sig=resspec.copy()
        resspec.set_selection()
        sel.reset()
        sel.set_type( srctype.fsoff )
        resspec.set_selection( sel )
        ref=resspec.copy()
        resspec.set_selection()
        sel.reset()
        for k in range(len(polnos)):
            for j in range(len(ifnos)):
                sel.set_ifs(ifnos[j])
                sel.set_polarizations(polnos[k])
                try:
                    sig.set_selection(sel)
                    postcal.append(numpy.array(sig._getspectrum(0)))
                except:
                    ref.set_selection(sel)
                    postcal.append(numpy.array(ref._getspectrum(0)))
                sel.reset()
                resspec.set_selection()
        del sel
        # plot
        asaplog.post()
        asaplog.push('Plot only first spectrum for each [if,pol] pairs to verify calibration.')
        asaplog.post('WARN')
        p=new_asaplot()
        rcp('lines', linewidth=1)
        #nr=min(6,len(ifnos)*len(polnos))
        nr=len(ifnos)/2*len(polnos)
        titles=[]
        btics=[]
        if nr>3:
            asaplog.post()
            asaplog.push('Only first 3 [if,pol] pairs are plotted.')
            asaplog.post('WARN')
            nr=3
        p.set_panels(rows=nr,cols=3,nplots=3*nr,ganged=False)
        for i in range(3*nr):
            b=False
            if i >= 3*nr-3:
                b=True
            btics.append(b)
        for i in range(nr):
            p.subplot(3*i)
            p.color=0
            title='raw data IF%s,%s POL%s' % (ifnos[2*int(i/len(polnos))],ifnos[2*int(i/len(polnos))+1],polnos[i%len(polnos)])
            titles.append(title)
            #p.set_axes('title',title,fontsize=40)
            ymin=1.0e100
            ymax=-1.0e100
            nchan=s.nchan(ifnos[2*int(i/len(polnos))])
            edge=int(nchan*0.01)
            for j in range(4):
                spmin=min(precal[keys[j]][i][edge:nchan-edge])
                spmax=max(precal[keys[j]][i][edge:nchan-edge])
                ymin=min(ymin,spmin)
                ymax=max(ymax,spmax)
            for j in range(4):
                if i==0:
                    p.set_line(label=keys[j])
                else:
                    p.legend()
                p.plot(precal[keys[j]][i])
            p.axes.set_ylim(ymin-0.1*abs(ymin),ymax+0.1*abs(ymax))
            if not btics[3*i]:
                p.axes.set_xticks([])
            p.subplot(3*i+1)
            p.color=0
            title='sig data IF%s POL%s' % (ifnos[2*int(i/len(polnos))],polnos[i%len(polnos)])
            titles.append(title)
            #p.set_axes('title',title)
            p.legend()
            ymin=postcal[2*i][edge:nchan-edge].min()
            ymax=postcal[2*i][edge:nchan-edge].max()
            p.plot(postcal[2*i])
            p.axes.set_ylim(ymin-0.1*abs(ymin),ymax+0.1*abs(ymax))
            if not btics[3*i+1]:
                p.axes.set_xticks([])
            p.subplot(3*i+2)
            p.color=0
            title='ref data IF%s POL%s' % (ifnos[2*int(i/len(polnos))+1],polnos[i%len(polnos)])
            titles.append(title)
            #p.set_axes('title',title)
            p.legend()
            ymin=postcal[2*i+1][edge:nchan-edge].min()
            ymax=postcal[2*i+1][edge:nchan-edge].max()
            p.plot(postcal[2*i+1])
            p.axes.set_ylim(ymin-0.1*abs(ymin),ymax+0.1*abs(ymax))
            if not btics[3*i+2]:
                p.axes.set_xticks([])
        for i in range(3*nr):
            p.subplot(i)
            p.set_axes('title',titles[i],fontsize='medium')
        x=raw_input('Accept calibration ([y]/n): ' )
        if x.upper() == 'N':
            p.quit()
            del p
            return scantab
        p.quit()
        del p
    ###
    resspec._add_history("calfs",varlist)
    return resspec

@asaplog_post_dec
def merge(*args, **kwargs):
    """
    Merge a list of scanatables, or comma-sperated scantables into one
    scnatble.
    Parameters:
        A list [scan1, scan2] or scan1, scan2.
        freq_tol: frequency tolerance for merging IFs. numeric values
                  in units of Hz (1.0e6 -> 1MHz) and string ('1MHz')
                  is allowed.
    Example:
        myscans = [scan1, scan2]
        allscans = merge(myscans)
        # or equivalent
        sameallscans = merge(scan1, scan2)
        # with freqtol
        allscans = merge(scan1, scan2, freq_tol=1.0e6)
        # or equivalently
        allscans = merge(scan1, scan2, freq_tol='1MHz')
    """
    varlist = vars()
    if isinstance(args[0],list):
        lst = tuple(args[0])
    elif isinstance(args[0],tuple):
        lst = args[0]
    else:
        lst = tuple(args)
    if kwargs.has_key('freq_tol'):
        freq_tol = str(kwargs['freq_tol'])
        if len(freq_tol) > 0 and re.match('.+[GMk]Hz$', freq_tol) is None:
            freq_tol += 'Hz'
    else:
        freq_tol = ''
    varlist["args"] = "%d scantables" % len(lst)
    # need special formatting her for history...
    from asap._asap import stmath
    stm = stmath()
    for s in lst:
        if not isinstance(s,scantable):
            msg = "Please give a list of scantables"
            raise TypeError(msg)
    s = scantable(stm._merge(lst, freq_tol))
    s._add_history("merge", varlist)
    return s

@asaplog_post_dec
def calibrate( scantab, scannos=[], calmode='none', verify=None ):
    """
    Calibrate data.

    Parameters:
        scantab:       scantable
        scannos:       list of scan number
        calmode:       calibration mode
        verify:        verify calibration
    """
    import re
    antname = scantab.get_antennaname()
    if ( calmode == 'nod' ):
        asaplog.push( 'Calibrating nod data.' )
        scal = calnod( scantab, scannos=scannos, verify=verify )
    elif ( calmode == 'quotient' ):
        asaplog.push( 'Calibrating using quotient.' )
        scal = scantab.auto_quotient( verify=verify )
    elif ( calmode == 'ps' ):
        asaplog.push( 'Calibrating %s position-switched data.' % antname )
        if ( antname.find( 'APEX' ) != -1 ):
            scal = apexcal( scantab, scannos, calmode, verify )
        elif ( antname.find( 'ALMA' ) != -1 or antname.find( 'OSF' ) != -1
               or re.match('DV[0-9][0-9]$',antname) is not None
               or re.match('PM[0-9][0-9]$',antname) is not None
               or re.match('CM[0-9][0-9]$',antname) is not None
               or re.match('DA[0-9][0-9]$',antname) is not None ):
            scal = almacal( scantab, scannos, calmode, verify )
        else:
            scal = calps( scantab, scannos=scannos, verify=verify )
    elif ( calmode == 'fs' or calmode == 'fsotf' ):
        asaplog.push( 'Calibrating %s frequency-switched data.' % antname )
        if ( antname.find( 'APEX' ) != -1 ):
            scal = apexcal( scantab, scannos, calmode, verify )
        elif ( antname.find( 'ALMA' ) != -1 or antname.find( 'OSF' ) != -1 ):
            scal = almacal( scantab, scannos, calmode, verify )
        else:
            scal = calfs( scantab, scannos=scannos, verify=verify )
    elif ( calmode == 'otf' ):
        asaplog.push( 'Calibrating %s On-The-Fly data.' % antname )
        scal = almacal( scantab, scannos, calmode, verify )
    else:
        asaplog.push( 'No calibration.' )
        scal = scantab.copy()

    return scal

def apexcal( scantab, scannos=[], calmode='none', verify=False ):
    """
    Calibrate APEX data

    Parameters:
        scantab:       scantable
        scannos:       list of scan number
        calmode:       calibration mode

        verify:        verify calibration
    """
    from asap._asap import stmath
    stm = stmath()
    antname = scantab.get_antennaname()
    selection=selector()
    selection.set_scans(scannos)
    orig = scantab.get_selection()
    scantab.set_selection(orig+selection)
##     ssub = scantab.get_scan( scannos )
##     scal = scantable( stm.cwcal( ssub, calmode, antname ) )
    scal = scantable( stm.cwcal( scantab, calmode, antname ) )
    scantab.set_selection(orig)
    return scal

def almacal( scantab, scannos=[], calmode='none', verify=False ):
    """
    Calibrate ALMA data

    Parameters:
        scantab:       scantable
        scannos:       list of scan number
        calmode:       calibration mode

        verify:        verify calibration
    """
    from asap._asap import stmath
    stm = stmath()
    selection=selector()
    selection.set_scans(scannos)
    orig = scantab.get_selection()
    scantab.set_selection(orig+selection)
##     ssub = scantab.get_scan( scannos )
##     scal = scantable( stm.almacal( ssub, calmode ) )
    scal = scantable( stm.almacal( scantab, calmode ) )
    scantab.set_selection(orig)
    return scal

@asaplog_post_dec
def splitant(filename, outprefix='',overwrite=False, getpt=True):
    """
    Split Measurement set by antenna name, save data as a scantables,
    and return a list of filename. Note that frequency reference frame
    is imported as it is in Measurement set.
    Notice this method can only be available from CASA.
    Prameter
       filename:    the name of Measurement set to be read.
       outprefix:   the prefix of output scantable name.
                    the names of output scantable will be
                    outprefix.antenna1, outprefix.antenna2, ....
                    If not specified, outprefix = filename is assumed.
       overwrite    If the file should be overwritten if it exists.
                    The default False is to return with warning
                    without writing the output. USE WITH CARE.
       getpt        Whether to import direction from MS/POINTING
                    table or not. Default is True (import direction).
    """
    # Import the table toolkit from CASA
    from taskinit import gentools
    from asap.scantable import is_ms
    tb = gentools(['tb'])[0]
    # Check the input filename
    if isinstance(filename, str):
        import os.path
        filename = os.path.expandvars(filename)
        filename = os.path.expanduser(filename)
        if not os.path.exists(filename):
            s = "File '%s' not found." % (filename)
            raise IOError(s)
        # check if input file is MS
        if not is_ms(filename):
            s = "File '%s' is not a Measurement set." % (filename)
            raise IOError(s)
    else:
        s = "The filename should be string. "
        raise TypeError(s)
    # Check out put file name
    outname=''
    if len(outprefix) > 0: prefix=outprefix+'.'
    else:
        prefix=filename.rstrip('/')
    # Now do the actual splitting.
    outfiles=[]
    tb.open(tablename=filename,nomodify=True)
    ant1=tb.getcol('ANTENNA1',0,-1,1)
    anttab=tb.getkeyword('ANTENNA').lstrip('Table: ')
    tb.close()
    tb.open(tablename=anttab,nomodify=True)
    nant=tb.nrows()
    antnames=tb.getcol('NAME',0,nant,1)
    tb.close()
    for antid in set(ant1):
        scan=scantable(filename,average=False,antenna=int(antid),getpt=getpt)
        outname=prefix+antnames[antid]+'.asap'
        scan.save(outname,format='ASAP',overwrite=overwrite)
        del scan
        outfiles.append(outname)
    return outfiles

@asaplog_post_dec
def _array2dOp( scan, value, mode="ADD", tsys=False, insitu=None, skip_flaggedrow=False):
    """
    This function is workaround on the basic operation of scantable
    with 2 dimensional float list.

    scan:    scantable operand
    value:   float list operand
    mode:    operation mode (ADD, SUB, MUL, DIV)
    tsys:    if True, operate tsys as well
    insitu:  if False, a new scantable is returned.
             Otherwise, the array operation is done in-sitsu.
    skip_flaggedrow: skip operation for row-flagged spectra.
    """
    if insitu is None: insitu = rcParams['insitu']
    nrow = scan.nrow()
    s = None
    from asap._asap import stmath
    stm = stmath()
    stm._setinsitu(insitu)
    if len( value ) == 1:
        s = scantable( stm._arrayop( scan, value[0], mode, tsys, skip_flaggedrow ) )
    elif len( value ) != nrow:
        raise ValueError( 'len(value) must be 1 or conform to scan.nrow()' )
    else:
        from asap._asap import stmath
        if not insitu:
            s = scan.copy()
        else:
            s = scan
        # insitu must be True as we go row by row on the same data
        stm._setinsitu( True )
        basesel = s.get_selection()
        # generate a new selector object based on basesel
        sel = selector(basesel)
        for irow in range( nrow ):
            sel.set_rows( irow )
            s.set_selection( sel )
            if len( value[irow] ) == 1:
                stm._unaryop( s, value[irow][0], mode, tsys, skip_flaggedrow )
            else:
                #stm._arrayop( s, value[irow], mode, tsys, 'channel' )
                stm._arrayop( s, value[irow], mode, tsys, skip_flaggedrow )
        s.set_selection(basesel)
    return s
