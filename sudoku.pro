TEMPLATE=subdirs
  
SUBDIRS += src \
    	tests


tests.depends = src 

OTHER_FILES += \
               puzzles/*.sdm
               puzzles/*.fsdm

