###############################################################################
# THIS TEST AIM TO:
# - check if variable can be duplicated with profile attribute [NEGATIVE]
###############################################################################

@profile P1
VARIABLE = V1

@profile P2
VARIABLE = V2

@pub; @main; task DUMMY() {}