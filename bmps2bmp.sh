#!/bin/sh

./bmps2bmp > null.tmp~
ret=$?
while [ $ret != 0 ]; do
	cat null.tmp~ | ./bmps2bmp > null2.tmp~
	ret=$?
	cat null2.tmp~ > null.tmp~
done
cat null.tmp~
if [ -f null2.tmp~ ]; then
	rm null2.tmp~
fi
rm null.tmp~
exit
