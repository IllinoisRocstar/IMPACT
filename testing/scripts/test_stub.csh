#!/bin/tcsh
# This is a testing stub - it is not really a test. There are no 
# real general platform-specific tests right now.
# runtest calls this script with the following arguments:
set RESULTSFILE = ${1}
set SRCDIR = ${2}
set BINDIR = ${3}

printf "TestStubWorks=1" >> ${RESULTSFILE}
exit 0
