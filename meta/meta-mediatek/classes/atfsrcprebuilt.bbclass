python () {

    import os.path

    srcpath = d.getVar('ATF_SRC', True)
    prebuiltpath = d.getVar('ATF_PREBUILT', True)

    if os.path.exists(srcpath+'/.git'):
        d.setVar('ATF_SRC', srcpath)
    elif os.path.exists(prebuiltpath+'/.git'):
        d.setVar('ATF_SRC', prebuiltpath)
    else:
        bb.fatal("Both %s and %s aren't existed" % (srcpath, prebuiltpath) )

    # d.setVar('B', d.getVar('TZ_SRC', True) )
    # d.setVar('S', d.getVar('TZ_SRC', True) )
    # bb.warn("TZ_SRC           = %s" % d.getVar('TZ_SRC', True) )
    # bb.warn("EXTERNALSRC       = %s" % d.getVar('EXTERNALSRC', True))
    # bb.warn("EXTERNALSRC_BUILD = %s" %  d.getVar('EXTERNALSRC_BUILD', True) )

}
