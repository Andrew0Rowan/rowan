
python() {
        d.appendVarFlag('do_cleansstate', 'depends', ' systemd:do_cleansstate')
        d.appendVarFlag('do_compile', 'depends', ' systemd:do_compile')
}
