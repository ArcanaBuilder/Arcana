###############################################################################
# THIS TEST AIM TO:
# - check task declaration with arcana glob var inline
###############################################################################


VAR = **.arc


@main
task T1()
{
    rm {arc:inline:VAR}
}
