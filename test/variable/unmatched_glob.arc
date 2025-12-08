###############################################################################
# THIS TEST AIM TO:
# - check if glob variable can not be expanded, no file .idk 
###############################################################################

VARIABLE = ./**.idk


@pub; @main; task DUMMY() {}