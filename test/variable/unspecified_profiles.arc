###############################################################################
# THIS TEST AIM TO:
# - check if variable can be duplicated with empty profile attribute [NEGATIVE]
###############################################################################

using profiles P1 P2;

@profile P1
VARIABLE = V1

@profile
VARIABLE = V2

@pub; @main; task DUMMY() {}