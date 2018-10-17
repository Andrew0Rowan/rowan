python () {

    import os.path

    srcpath = d.getVar('MTK_SRC', True)
    prebuiltpath = srcpath.replace("build/../src/", "build/../prebuilt/")
    if os.path.exists(srcpath+'/.git'):
        d.setVar('MTK_SRC', srcpath)
    elif os.path.exists(prebuiltpath+'/.git'):
        d.setVar('MTK_SRC', prebuiltpath)
    else:
        bb.warn("Both %s and %s aren't existed" % (srcpath, prebuiltpath) )

    d.setVar('B', d.getVar('MTK_SRC', True) )
    d.setVar('S', d.getVar('MTK_SRC', True) )
#    bb.warn("MTK_SRC           = %s" % d.getVar('MTK_SRC', True) )
#    bb.warn("EXTERNALSRC       = %s" % d.getVar('EXTERNALSRC', True))
#    bb.warn("EXTERNALSRC_BUILD = %s" %  d.getVar('EXTERNALSRC_BUILD', True) )

}
