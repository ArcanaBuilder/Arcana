###############################################################################
# THIS TEST AIM TO:
# - check task declaration with complex recursive then [NEGATIVE]
###############################################################################


@main
@then T2
task T1()
{
    echo T1
}

@then T3
task T2() 
{
    echo T2
}

@then T1
task T3() 
{
    echo T3
}