###############################################################################
# THIS TEST AIM TO:
# - check task declaration with recursive Requires [NEGATIVE]
###############################################################################


@main
@requires T1
task T1()
{
    echo T1
}
