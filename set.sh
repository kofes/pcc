#! /bin/bash
for line in `find test/ -name *.in`
do
  line="${line%.*}"
  ./main $line.in > $line.out
done
