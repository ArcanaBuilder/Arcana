###############################################################################
# THIS TEST AIM TO:
# - check task declaration with same then and requires
###############################################################################


@main
@requires T2
@then T2
task T1()
{
    echo T1
}

task T2()
{
    echo T1
}
