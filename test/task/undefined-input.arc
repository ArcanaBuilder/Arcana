###############################################################################
# THIS TEST AIM TO:
# - check task declaration with arcana glob with undefined input [NEGATIVE]
###############################################################################


VAR = **.arc


@main
task T1(VAR1) 
{
    cat {arc:list:VAR}
}
