#!/bin/tcsh

# runtest calls this script with the following arguments:
set RESULTSFILE = ${1}
set SRCDIR = ${2}
set BINDIR = ${3}

rm -f tmpresults_1.txt
cat <<EOF > ./com_parallel_test_batch.csh
#!/bin/tcsh
#
#PBS -V
#PBS -l nodes=2:ppn=8
#PBS -l walltime=00:30:00 
#PBS -j oe
#PBS -o com_parallel_test_batch_output
#PBS -A MPINFRA

cd \${PBS_O_WORKDIR}
mpirun -np 16 -machinefile \${PBS_NODEFILE} ${BINDIR}/com_parallel_test -o tmpresults_1.txt
EOF
qsub com_parallel_test_batch.csh
@ i = 1
while($i <= 720)
    @ i += 1
    if( -e tmpresults_1.txt ) then
        @ i += 721;
    else
        sleep 10;
    endif
end
sleep 10
cat tmpresults_1.txt >> ${RESULTSFILE}
rm -f tmpresults_1.txt
rm -f ./com_parallel_test_batch.csh
rm -f com_parallel_test_batch_output
exit 0
