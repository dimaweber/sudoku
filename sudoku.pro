TEMPLATE=subdirs

SUBDIRS += src \
		   libsudoku \
#		   tests

src.depends   = libsudoku
#tests.depends = libsudoku

OTHER_FILES += \
			   puzzles/*.sdm
			   puzzles/*.fsdm

