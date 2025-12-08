###############################################################################
# THIS TEST AIM TO:
# - check if variable can be mapped into another one
###############################################################################


VARIABLE = ./**.arc

@map VARIABLE
VARIABLE2 = ./**.test



@pub; @main; task DUMMY() {}