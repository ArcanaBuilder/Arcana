###############################################################################
# THIS TEST AIM TO:
# - check if variable can be mapped into another one, no source mapping
#   specified [NEGATIVE]
###############################################################################


VARIABLE = ./**.arc

@map
VARIABLE2 = ./**.test



@pub; @main; task DUMMY() {}