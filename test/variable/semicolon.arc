###############################################################################
# THIS TEST AIM TO:
# - check if semicolon can be used after a statement
###############################################################################

using profiles P1 P2;

VAR0 = value;
VAR1 = ./**.arc;

@map VAR1;
VAR3 = ./**.sh

@profile P1;
VAR4 = value

@pub; @main; task DUMMY() {}