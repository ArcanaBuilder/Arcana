###############################################################################
# THIS TEST AIM TO:
# - check task declaration with undefined interpreter [NEGATIVE]
###############################################################################


@main
@interpreter /path/to/it
task T1()
{
    echo T1
}
