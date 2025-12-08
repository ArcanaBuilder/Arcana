###############################################################################
# THIS TEST AIM TO:
# - check if variable can be duplicated with profile attribute [NEGATIVE]
###############################################################################

using profiles P1 P2;

@pub; @main; task DUMMY() {}

@profile P1
VARIABLE = V1

@profile P2

