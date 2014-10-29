# This is the canonical source of the MFM version number!
MFM_VERSION_MAJOR:=2
MFM_VERSION_MINOR:=0
MFM_VERSION_REV:=11


################## NOTHING BELOW HERE SHOULD NEED TO CHANGE ##################

MFM_VERSION_NUMBER:=$(MFM_VERSION_MAJOR).$(MFM_VERSION_MINOR).$(MFM_VERSION_REV)
MFM_IS_DEVEL_VERSION:=$(strip $(filter-out %0 %2 %4 %6 %8, $(MFM_VERSION_NUMBER)))
