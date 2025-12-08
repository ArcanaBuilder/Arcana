###############################################################################
# THIS TEST AIM TO:
# - check task declaration with complex recursive requires [NEGATIVE]
###############################################################################


@main
@requires T2
task T1()
{
    echo T1
}

@requires T3
task T2() 
{
    echo T2
}

@requires T1
task T3() 
{
    echo T3
}