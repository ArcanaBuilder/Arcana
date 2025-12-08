###############################################################################
# THIS TEST AIM TO:
# - check if semicolon can be used everywhere [NEGATIVE]
###############################################################################

using profiles P1 P2;

VAR0 = value;
VAR1 = ./**.arc;

; # <-- NOT ALLOWED

@map VAR1;
VAR3 = ./**.sh

@profile P1;
VAR4 = value


@pub; @main; task DUMMY() {}