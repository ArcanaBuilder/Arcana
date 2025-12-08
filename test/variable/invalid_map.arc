###############################################################################
# THIS TEST AIM TO:
# - check if variable can be mapped into another one [NEGATIVE]
###############################################################################


VARIABLE = ./**.arc

@map VARIABLE
VARIABLE2 = ciao



@pub; @main; task DUMMY() {}