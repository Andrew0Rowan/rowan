# Copyright (C) 2016 Mediatek
# Author: Richard Sun
# Some code and influence taken from externalsrc.bbclass:
# Copyright (C) 2012 Linux Foundation
# Author: Richard Purdie
# Some code and influence taken from srctree.bbclass:
# Copyright (C) 2009 Chris Larson <clarson@kergoth.com>
# Released under the MIT license (see COPYING.MIT for the terms)
#
# workonsrc.bbclass enables use of an existing source tree, usually workon to 
# the build system to build a piece of software rather than the usual fetch/unpack/patch
# process.
#
# To use, add workonsrc to the global inherit and set WORKONSRC to point at the
# directory you want to use containing the sources e.g. from local.conf for a recipe
# called "myrecipe" you would do:
#
# INHERIT += "workonsrc"
# WORKONSRC_pn-myrecipe = "/path/to/my/source/tree"
#
# In order to make this class work for both target and native versions (or with
# multilibs/cross or other BBCLASSEXTEND variants), B is set to point to a separate
# directory under the work directory (split source and build directories). This is
# the default, but the build directory can be set to the source directory if
# circumstances dictate by setting WORKONSRC_BUILD to the same value, e.g.:
#
# WORKONSRC_BUILD_pn-myrecipe = "/path/to/my/source/tree"
#

SRCTREECOVEREDTASKS ?= "do_patch do_unpack do_fetch"
EXTERNALSRC_SYMLINKS ?= "oe-workdir:${WORKDIR} oe-logs:${T}"

python () {
    import subprocess, os.path

    pn = d.getVar('PN', True)
    d.appendVarFlag('do_populate_lic', 'depends', ' %s:do_configure' % pn)
    workonsrc = d.getVar('WORKONSRC', True)
    workonprebuilt = workonsrc.replace("build/../src/", "build/../prebuilt/")
    if not os.path.exists(workonsrc):
        if os.path.exists(workonprebuilt):
            workonsrc = workonprebuilt
        else:
            bb.warn("Both %s and %s aren't existed" % (workonsrc, workonprebuilt) )

    # If this is the base recipe and WORKONSRC is set for it or any of its
    # derivatives, then enable BB_DONT_CACHE to force the recipe to always be
    # re-parsed so that the file-checksums function for do_compile is run every
    # time.
    bpn = d.getVar('BPN', True)
    if bpn == d.getVar('PN', True):
        classextend = (d.getVar('BBCLASSEXTEND', True) or '').split()
        if (workonsrc or
                ('native' in classextend and
                 d.getVar('WORKONSRC_pn-%s-native' % bpn, True)) or
                ('nativesdk' in classextend and
                 d.getVar('WORKONSRC_pn-nativesdk-%s' % bpn, True)) or
                ('cross' in classextend and
                 d.getVar('WORKONSRC_pn-%s-cross' % bpn, True))):
            d.setVar('BB_DONT_CACHE', '1')

    if workonsrc:
        d.setVar('S', workonsrc)
        workonsrcbuild = d.getVar('WORKONSRC_BUILD', True)
        if workonsrcbuild:
            d.setVar('B', workonsrcbuild)
        else:
            d.setVar('B', '${WORKDIR}/${BPN}-${PV}/')
            workonsrcbuild = d.getVar('B', True)

        if workonsrc != workonsrcbuild:
            d.setVar('S', workonsrcbuild)

        local_srcuri = []
        fetch = bb.fetch2.Fetch((d.getVar('SRC_URI', True) or '').split(), d)
        for url in fetch.urls:
            url_data = fetch.ud[url]
            parm = url_data.parm
            if (url_data.type == 'file' or
                    'type' in parm and parm['type'] == 'kmeta'):
                local_srcuri.append(url)

        d.setVar('SRC_URI', ' '.join(local_srcuri))

        if '{SRCPV}' in d.getVar('PV', False):
            # Dummy value because the default function can't be called with blank SRC_URI
            d.setVar('SRCPV', '999')

        tasks = filter(lambda k: d.getVarFlag(k, "task", True), d.keys())

        for task in tasks:
            if task.endswith("_setscene"):
                # sstate is never going to work for workon source trees, disable it
                bb.build.deltask(task, d)
            else:
                # Since configure will likely touch ${S}, ensure only we lock so one task has access at a time
                d.appendVarFlag(task, "lockfiles", " ${S}/singletask.lock")

            # We do not want our source to be wiped out, ever (kernel.bbclass does this for do_clean)
            cleandirs = (d.getVarFlag(task, 'cleandirs', False) or '').split()
            setvalue = False
            for cleandir in cleandirs[:]:
                if d.expand(cleandir) == workonsrc:
                    cleandirs.remove(cleandir)
                    setvalue = True
            if setvalue:
                d.setVarFlag(task, 'cleandirs', ' '.join(cleandirs))

        fetch_tasks = ['do_fetch', 'do_unpack']
        # If we deltask do_patch, there's no dependency to ensure do_unpack gets run, so add one
        # Note that we cannot use d.appendVarFlag() here because deps is expected to be a list object, not a string
        d.setVarFlag('do_configure', 'deps', (d.getVarFlag('do_configure', 'deps', False) or []) + ['do_unpack'])

        for task in d.getVar("SRCTREECOVEREDTASKS", True).split():
            if local_srcuri and task in fetch_tasks:
                continue
            bb.build.deltask(task, d)

        d.prependVarFlag('do_compile', 'prefuncs', "workonsrc_compile_prefunc ")
        d.prependVarFlag('do_configure', 'prefuncs', "workonsrc_configure_prefunc ")

        d.setVarFlag('do_compile', 'file-checksums', '${@srctree_hash_files(d)}')
        d.setVarFlag('do_configure', 'file-checksums', '${@srctree_configure_hash_files(d)}')

        # We don't want the workdir to go away
        d.appendVar('RM_WORK_EXCLUDE', ' ' + d.getVar('PN', True))

        # If B=S the same builddir is used even for different architectures.
        # Thus, use a shared CONFIGURESTAMPFILE and STAMP directory so that
        # change of do_configure task hash is correctly detected and stamps are
        # invalidated if e.g. MACHINE changes.
        if d.getVar('S', True) == d.getVar('B', True):
            configstamp = '${TMPDIR}/work-shared/${PN}/${EXTENDPE}${PV}-${PR}/configure.sstate'
            d.setVar('CONFIGURESTAMPFILE', configstamp)
            d.setVar('STAMP', '${STAMPS_DIR}/work-shared/${PN}/${EXTENDPE}${PV}-${PR}')
}

python workonsrc_configure_prefunc() {
    srctree_rsync_files(d)
    # Create desired symlinks
    symlinks = (d.getVar('EXTERNALSRC_SYMLINKS', True) or '').split()
    for symlink in symlinks:
        symsplit = symlink.split(':', 1)
        lnkfile = os.path.join(d.getVar('S', True), symsplit[0])
        target = d.expand(symsplit[1])
        if len(symsplit) > 1:
            if os.path.islink(lnkfile):
                # Link already exists, leave it if it points to the right location already
                if os.readlink(lnkfile) == target:
                    continue
                os.unlink(lnkfile)
            elif os.path.exists(lnkfile):
                # File/dir exists with same name as link, just leave it alone
                continue
            os.symlink(target, lnkfile)

}

python workonsrc_compile_prefunc() {
    srctree_rsync_files(d)
    # Make it obvious that this is happening, since forgetting about it could lead to much confusion
    bb.plain('NOTE: %s: compiling from workon source tree %s' % (d.getVar('PN', True), d.getVar('WORKONSRC', True)))
}

def srctree_rsync_files(d):
    import subprocess, os.path

    workonsrc = d.getVar('WORKONSRC', True)
    workonprebuilt = workonsrc.replace("build/../src/", "build/../prebuilt/")
    if not os.path.exists(workonsrc):
        if os.path.exists(workonprebuilt):
            workonsrc = workonprebuilt
        else:
            bb.warn("Both %s and %s aren't existed" % (workonsrc, workonprebuilt) )

    if workonsrc:
        d.setVar('S', workonsrc)
        workonsrcbuild = d.getVar('WORKONSRC_BUILD', True)
        if workonsrcbuild:
            d.setVar('B', workonsrcbuild)
        else:
            d.setVar('B', '${WORKDIR}/${BPN}-${PV}/')
            workonsrcbuild = d.getVar('B', True)

        if workonsrc != workonsrcbuild:
            cmd = "mkdir -p %s" % (workonsrcbuild)
            subprocess.call(cmd, shell=True)

            if os.path.exists(workonsrc):
                cmd = "rsync -aL %s/* %s" % (workonsrc, workonsrcbuild)
                ret = subprocess.call(cmd, shell=True)
                if ret != 0:
                    bb.fatal("rsync -aL %s/* %s failed." % (workonsrc, workonsrcbuild))
            d.setVar('S', workonsrcbuild)

def srctree_hash_files(d):
    import shutil
    import subprocess
    import tempfile

    s_dir = d.getVar('WORKONSRC', True)
    git_dir = os.path.join(s_dir, '.git')
    oe_hash_file = os.path.join(git_dir, 'oe-devtool-tree-sha1')

    ret = " "
    if os.path.exists(git_dir):
        with tempfile.NamedTemporaryFile(dir=git_dir, prefix='oe-devtool-index') as tmp_index:
            # Clone index
            shutil.copy2(os.path.join(git_dir, 'index'), tmp_index.name)
            # Update our custom index
            env = os.environ.copy()
            env['GIT_INDEX_FILE'] = tmp_index.name
            subprocess.check_output(['git', 'add', '.'], cwd=s_dir, env=env)
            sha1 = subprocess.check_output(['git', 'write-tree'], cwd=s_dir, env=env).decode("utf-8")
        with open(oe_hash_file, 'w') as fobj:
            fobj.write(sha1)
        ret = oe_hash_file + ':True'
    else:
        ret = d.getVar('WORKONSRC', True) + '/*:True'
    return ret

def srctree_configure_hash_files(d):
    """
    Get the list of files that should trigger do_configure to re-execute,
    based on the value of CONFIGURE_FILES
    """
    in_files = (d.getVar('CONFIGURE_FILES', True) or '').split()
    out_items = []
    search_files = []
    for entry in in_files:
        if entry.startswith('/'):
            out_items.append('%s:%s' % (entry, os.path.exists(entry)))
        else:
            search_files.append(entry)
    if search_files:
        s_dir = d.getVar('WORKONSRC', True)
        for root, _, files in os.walk(s_dir):
            for f in files:
                if f in search_files:
                    out_items.append('%s:True' % os.path.join(root, f))
    return ' '.join(out_items)

