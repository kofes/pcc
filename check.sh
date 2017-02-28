#! /bin/bash
for line in `find test/ -name *.in | sort`
do
  line="${line%.*}"
  ./main $line.in > $line.tmp
  if diff $line.out $line.tmp > /dev/null; then
    echo 'TEST '"${line##*/}"': [OK]'
  else
    echo 'TEST '"${line##*/}"': [BAD]'
  fi
  rm $line.tmp
done
