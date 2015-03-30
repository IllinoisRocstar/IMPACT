#!/bin/tcsh

echo "modules test"

set OutFile=$1
set TmpOut=${OutFile}_tmp.txt
set SRCDIR=$2
set BINDIR=$3

echo "SRCDIR = $SRCDIR"

# ========= THE MODULES TEST ============
@ modules_err = 0
@ Cmodule_err = 0
@ Fmodule_err = 0

if( -e "ModulesTest") then
  printf "Removing old ModulesTest.\n"
  rm -rf ModulesTest
endif

mkdir ModulesTest
cd ModulesTest
ln -s ${BINDIR}/TestModuleDriver .
ln -s ${BINDIR}/TestModuleDriverF .

cp ${SRCDIR}/data/modules/TestModuleCorrect.dat .
echo ${SRCDIR}/data/modules/TestModuleCorrect.dat

./TestModuleDriver > TestModule.dat
set TESTSTATUS=$status

printf "TestModuleDriver:Runs=" >> ${TmpOut}
if( "${TESTSTATUS}" == "0" ) then
    printf "1\n" >> ${TmpOut}
else
    printf "0\n" >> ${TmpOut} 
    printf "TestModuleDriver:Passes=0\n" >> ${TmpOut}
    @ modules_err = 1
    @ Cmodule_err = 1
endif

if ( "${Cmodule_err}" == "0" ) then
    diff TestModuleCorrect.dat TestModule.dat
    if ($? == 1) then
        printf "TestModuleDriver:Passes=0\n" >> ${TmpOut}
        @ modules_err = 1
        @ Cmodule_err = 1
    else
        printf "TestModuleDriver:Passes=1\n" >> ${TmpOut}
    endif
endif

./TestModuleDriverF > TestModuleF.dat
set TESTSTATUS=$status

printf "TestModuleDriverF:Runs=" >> ${TmpOut}
if( "${TESTSTATUS}" == "0" ) then
    printf "1\n" >> ${TmpOut}
else
    printf "0\n" >> ${TmpOut} 
    printf "TestModuleDriverF:Passes=0\n" >> ${TmpOut}
    @ modules_err = 1
    @ Fmodule_err = 1
endif

if ( "${Fmodule_err}" == "0" ) then
    diff TestModuleCorrect.dat TestModuleF.dat
    if ($? == 1) then
        printf "TestModuleDriverF:Passes=0\n" >> ${TmpOut}
        @ modules_err = 1
        @ Fmodule_err = 1
    else
        printf "TestModuleDriverF:Passes=1\n" >> ${TmpOut}
    endif
endif

cat ${TmpOut} >> ../${OutFile}
cd ..
if ("${modules_err}" == 0) then
   printf "Modules tests passed, removing data.\n"
   rm -rf ModulesTest
endif

exit {$err}
