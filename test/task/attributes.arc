###############################################################################
# THIS TEST AIM TO:
# - check task declaration with all attributes
###############################################################################

using profiles P1;

@profile P1
@pub
@always
@requires T2
@then     T3
@multithread
@main
@interpreter /bin/python3
@flushcache
@echo
task T1()
{
    echo T1
}


task T2() {}
task T3() {}
