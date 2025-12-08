###############################################################################
# THIS TEST AIM TO:
# - check task declaration with arcana glob read as simple value
###############################################################################


VAR = **.arc


@main
task T1()
{
    rm {arc:VAR}
}
