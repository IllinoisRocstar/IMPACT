#!/bin/tcsh

set OutFile=$1
set SRCDIR=$2
set BINDIR=$3

set TmpOut=${OutFile}_tmp.txt
printf "This is a test of a serial example program\n" >> test_sep_in.txt
printf "All it does is copy this file.\n" >> test_sep_in.txt
# Test the serial example program output
${BINDIR}/sep -o test_sep_out.txt test_sep_in.txt 
set STEST=`diff test_sep_in.txt test_sep_out.txt`
rm -f test_sep_out.txt test_sep_in.txt
printf "ExampleProgram:Works=" >> ${TmpOut}
if( "$STEST" == "") then
  printf "1\n" >> ${TmpOut}
else
  printf "0\n" >> ${TmpOut}
endif
cat ${TmpOut} >> ${OutFile}
rm -f ${TmpOut}
exit 0
