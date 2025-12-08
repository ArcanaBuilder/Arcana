###############################################################################
# THIS TEST AIM TO:
# - check task declaration with unknown attribute [NEGATIVE]
###############################################################################


@main
@unknown_attribute
task T1()
{
    echo T1
}
