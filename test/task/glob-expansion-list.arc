###############################################################################
# THIS TEST AIM TO:
# - check task declaration with arcana glob var list
###############################################################################


VAR = **.arc


@main
task T1()
{
    cat {arc:list:VAR}
}
