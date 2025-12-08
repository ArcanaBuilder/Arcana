###############################################################################
# THIS TEST AIM TO:
# - check if variable can be duplicated with profile attribute
###############################################################################

using profiles P1 P2;

@profile P1
VARIABLE = V1

@profile P2
VARIABLE = V2

@pub; @main; task DUMMY() {}