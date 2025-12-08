###############################################################################
# THIS TEST AIM TO:
# - check task declaration with arcana glob var 
#   with undefined expander [NEGATIVE]
###############################################################################


VAR = **.arc


@main
task T1()
{
    rm {arc:undefined:VAR}
}
