#!/bin/bash

RDIR=`pwd`
TEST_DIR="$RDIR/TestFiles"
TEST_FILES="test1.txt"
REPORT_DIR="$RDIR/Reports"

# Количество повторений
let COUNTER_VAR=50

# Запуск задач
for i in $TEST_FILES ; do
    echo "Start thread programm for test file $i"
    $RDIR/progThread 4 $TEST_DIR/$i > $REPORT_DIR/"$i".result.thread
    echo "Start mpi programm for test file $i"
    mpirun -np 4 $RDIR/progMPI $TEST_DIR/$i > $REPORT_DIR/"$i".result.mpi
done

# Многократный запуск для последующего рассчета СКО и т.д.

TEST_FILE=test1.txt

# Подготовка
find -name *.repeate | xargs rm -f

# Параллельная программа с 1 потоком
COUNTER=0

echo "Start pthreads programm with 1 thread repeating..."
while [ $COUNTER -lt $COUNTER_VAR ] ; do
    $RDIR/progThread 1 $TEST_DIR/$TEST_FILE | head -n 1 >> $REPORT_DIR/result.threads.1.repeate
    let COUNTER=COUNTER+1
done
echo "Done"
echo ""

# Параллельная программа с 2 потоками
COUNTER=0

echo "Start pthreads programm with 2 thread repeating..."
while [ $COUNTER -lt $COUNTER_VAR ] ; do
    $RDIR/progThread 2 $TEST_DIR/$TEST_FILE | head -n 1 >> $REPORT_DIR/result.threads.2.repeate
    let COUNTER=COUNTER+1
done
echo "Done"
echo ""

# Параллельная программа с 4 потоками
COUNTER=0

echo "Start pthreads programm with 4 thread repeating..."
while [ $COUNTER -lt $COUNTER_VAR ] ; do
    $RDIR/progThread 4 $TEST_DIR/$TEST_FILE | head -n 1 >> $REPORT_DIR/result.threads.4.repeate
    let COUNTER=COUNTER+1
done
echo "Done"
echo ""

# Параллельная программа с 8 потоками
COUNTER=0

echo "Start pthreads programm with 8 thread repeating..."
while [ $COUNTER -lt $COUNTER_VAR ] ; do
    $RDIR/progThreads 8 $TEST_DIR/$TEST_FILE | head -n 1 >> $REPORT_DIR/result.threads.8.repeate
    let COUNTER=COUNTER+1
done
echo "Done"
echo ""

# Параллельная программа с 16 потоками
COUNTER=0

echo "Start pthreads programm with 16 thread repeating..."
while [ $COUNTER -lt $COUNTER_VAR ] ; do
    $RDIR/progThreads 16 $TEST_DIR/$TEST_FILE | head -n 1 >> $REPORT_DIR/result.threads.16.repeate
    let COUNTER=COUNTER+1
done
echo "Done"
echo ""

# MPI с 1 рабочим процессом
COUNTER=0

echo "Start MPI programm with 2 process repeating..."
while [ $COUNTER -lt $COUNTER_VAR ] ; do
    mpirun -np 2 $RDIR/progMPI $TEST_DIR/$TEST_FILE | head -n 1 >> $REPORT_DIR/result.mpi.1.repeate
    let COUNTER=COUNTER+1
done
echo "Done"
echo ""

# MPI с 2 рабочими процессами
COUNTER=0

echo "Start MPI programm with 3 process repeating..."
while [ $COUNTER -lt $COUNTER_VAR ] ; do
    mpirun -np 3 $RDIR/progMPI $TEST_DIR/$TEST_FILE | head -n 1 >> $REPORT_DIR/result.mpi.2.repeate
    let COUNTER=COUNTER+1
done
echo "Done"
echo ""

# MPI с 4 рабочими процессами
COUNTER=0

echo "Start MPI programm with 4 process repeating..."
while [ $COUNTER -lt $COUNTER_VAR ] ; do
    mpirun -np 5 $RDIR/progMPI $TEST_DIR/$TEST_FILE | head -n 1 >> $REPORT_DIR/result.mpi.4.repeate
    let COUNTER=COUNTER+1
done
echo "Done"
echo ""

#COUNTER=0
#
#echo "Start MPI programm with 5 process repeating..."
#while [ $COUNTER -lt $COUNTER_VAR ] ; do
#    mpirun -np 6 $RDIR/progMPI $TEST_DIR/$TEST_FILE | head -n 1 >> $REPORT_DIR/result.mpi.5.repeate
#    let COUNTER=COUNTER+1
#done
#echo "Done"
#echo ""
#
#COUNTER=0
#
#echo "Start MPI programm with 9 process repeating..."
#while [ $COUNTER -lt $COUNTER_VAR ] ; do
#    mpirun -np 10 $RDIR/progMPI $TEST_DIR/$TEST_FILE | head -n 1 >> $REPORT_DIR/result.mpi.9.repeate
#    let COUNTER=COUNTER+1
#done
#echo "Done"
#echo ""
#
#COUNTER=0
#
#echo "Start MPI programm with 15 process repeating..."
#while [ $COUNTER -lt $COUNTER_VAR ] ; do
#    mpirun -np 16 $RDIR/progMPI $TEST_DIR/$TEST_FILE | head -n 1 >> $REPORT_DIR/result.mpi.16.repeate
#    let COUNTER=COUNTER+1
#done
#echo "Done"
#echo ""

