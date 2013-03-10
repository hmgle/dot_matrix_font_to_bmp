#!/bin/sh

SAVEIFS=$IFS
IFS=;
read -r line
echo "$line" |./utf8togb2312 |./gb2312tobmps |./bmpsall2bmp > null0.tmp~
while read -r line
do
	echo "$line" |./utf8togb2312 |./gb2312tobmps |./bmpsall2bmp > null1.tmp~
	cat null0.tmp~ null1.tmp~ | ./combin_v_3_test > null3.tmp~
	cat null3.tmp~ > null0.tmp~
done

IFS=$SAVEIFS
cat null3.tmp~
if [ -f null3.tmp~ ]; then
	rm null3.tmp~
fi

if [ -f null1.tmp~ ]; then
	rm null1.tmp~
fi

if [ -f null0.tmp~ ]; then
	rm null0.tmp~
fi
