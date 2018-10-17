PREFIX   = ../../../install
CROSS    = arm-linux-
ROOT     = $(PREFIX)/$(CROSS:%-=%)

DEFAULT=$(shell if [ -e DEFAULT ]; then ls DEFAULT; fi)
SUBMODULE=$(shell ls $(PRJ_FILENAME))
DEFAULTMODULE=$(filter-out $(SUBMODULE), $(DEFAULT))

all:
	@bash -c 'for mod in $(SUBMODULE); \
                do                      \
                        if [ -e $(PRJ_FILENAME)/$$mod/Makefile ]; then \
				make -C $(PRJ_FILENAME)/$$mod CROSS=$(CROSS) PROJECT=$(PROJECT) all; \
				if [ $$? != 0 ]; then exit $$?; fi \
                        fi; \
                done;'
	@bash -c 'for mod in $(DEFAULTMODULE); \
                do                      \
			for comp in $(COMPLIST); \
			do	\
				if [ $$mod == $$comp ]; then	\
                        		if [ -e DEFAULT/$$mod/Makefile ]; then \
						make -C DEFAULT/$$mod CROSS=$(CROSS) PROJECT=$(PROJECT) all; \
						if [ $$? != 0 ]; then exit $$?; fi \
                        		fi; \
				fi;	\
			done; \
                done;'

clean:
	@bash -c 'for mod in $(SUBMODULE); \
                do                      \
                        if [ -e $(PRJ_FILENAME)/$$mod/Makefile ]; then \
				make -C $(PRJ_FILENAME)/$$mod clean; \
				if [ $$? != 0 ]; then exit $$?; fi \
                        fi; \
                done;'
	@bash -c 'for mod in $(DEFAULTMODULE); \
                do                      \
			for comp in $(COMPLIST); \
			do	\
				if [ $$mod == $$comp ]; then	\
                        		if [ -e DEFAULT/$$mod/Makefile ]; then \
						make -C DEFAULT/$$mod clean; \
						if [ $$? != 0 ]; then exit $$?; fi \
                        		fi; \
				fi;	\
			done; \
                done;'
	rm -rf rootfs
	rm -rf initramfs

install:
	@bash -c 'for mod in $(SUBMODULE); \
                do                      \
                        if [ -e $(PRJ_FILENAME)/$$mod/Makefile ]; then \
				make -C $(PRJ_FILENAME)/$$mod CROSS=$(CROSS) ROOT=$(ROOT) PREFIX=$(PREFIX) install; \
				if [ $$? != 0 ]; then exit $$?; fi \
                        fi; \
                done;'
	@bash -c 'for mod in $(DEFAULTMODULE); \
                do                      \
			for comp in $(COMPLIST); \
			do	\
				if [ $$mod == $$comp ]; then	\
                        		if [ -e DEFAULT/$$mod/Makefile ]; then \
						make -C DEFAULT/$$mod CROSS=$(CROSS) ROOT=$(ROOT) PREFIX=$(PREFIX) install; \
						if [ $$? != 0 ]; then exit $$?; fi \
                        		fi; \
				fi;	\
			done; \
                done;'

pack_rootfs:
	@bash -c 'for mod in $(SUBMODULE); \
                do                      \
                        if [ -e $(PRJ_FILENAME)/$$mod/Makefile ]; then \
				make -C $(PRJ_FILENAME)/$$mod CROSS=$(CROSS) PACK_TO=$(PACK_TO) PACK_INITRAMFS_TO=$(PACK_INITRAMFS_TO) PROJECT=$(PROJECT) pack_rootfs; \
				if [ $$? != 0 ]; then exit $$?; fi \
                        fi; \
                done;'
	@bash -c 'for mod in $(DEFAULTMODULE); \
                do                      \
			for comp in $(COMPLIST); \
			do	\
				if [ $$mod == $$comp ]; then	\
                        		if [ -e DEFAULT/$$mod/Makefile ]; then \
						make -C DEFAULT/$$mod CROSS=$(CROSS) PACK_TO=$(PACK_TO) PACK_INITRAMFS_TO=$(PACK_INITRAMFS_TO) PROJECT=$(PROJECT) pack_rootfs; \
						if [ $$? != 0 ]; then exit $$?; fi \
                        		fi; \
				fi;	\
			done; \
                done;'
