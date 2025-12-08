###############################################################################
# THIS TEST AIM TO:
# - check simple inline task declaration (all brace variants)
###############################################################################

@main
task T1() {}

task T2(){}

task T3(){echo T3}

task T4()               {                   echo T4                 }

task T5() { echo T5 }

task T6() {echo T6 }

task T7() { echo T7}