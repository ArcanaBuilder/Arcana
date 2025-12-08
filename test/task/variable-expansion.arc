###############################################################################
# THIS TEST AIM TO:
# - check task declaration with arcana var
###############################################################################


VAR = 'some value'


@main
task T1()
{
    echo {arc:VAR}
}
