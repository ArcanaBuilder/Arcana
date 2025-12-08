###############################################################################
# THIS TEST AIM TO:
# - check task declaration with arcana glob with input
###############################################################################


VAR = **.arc


@main
task T1(VAR) 
{
    cat {arc:list:VAR}
}
