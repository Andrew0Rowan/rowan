package_arch = $(subst -,_,${PACKAGE_ARCH})

all:
install:
	find -name "*.$(package_arch).rpm" | while read i; do ${bindir}/rpm2cpio $${i} | cpio -idmv ; done
	make -f Makefile.libnvram install
	make -f Makefile.libfile_op install
	make -f Makefile.nvram_daemon install
	make -f Makefile.APDB install
	make -f Makefile.NvRAMUtils install
	make -f Makefile.wfbt_wr install
	
uninstall:
	make -f Makefile.libnvram uninstall
	make -f Makefile.libfile_op uninstall
	make -f Makefile.nvram_daemon uninstall
	make -f Makefile.APDB uninstall
	make -f Makefile.NvRAMUtils uninstall
	make -f Makefile.wfbt_wr uninstall
clean:
	rm -rf out/ sbin/ bin/ doc/ usr/ test/ ./$(base_libdir)/
	 
