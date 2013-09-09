#!/bin/sh

# 要utf-8方式编码的输入才能正常运行
# cygwin的运行环境需要设置 Options->Text->Character set:选择utf-8

help()
{
	cat <<EOF

	USAGE: $0 [-c] [-h] "要生成位图的字符" > 位图存放路径

	EXAMPLES:
	$0 "汉字" > ./output.bmp
EOF
}

color_anti_flag=0
while getopts "c" opt
do
	case $opt in
		c)
			color_anti_flag=1
			;;
		?)
			echo "argv error"
			exit 1;;
	esac
done

if { [ -z "$1" ] && [ -t 0 ] ; } || [ "$1" = '-h' ]
then
	help
	exit 0
fi
if [ "$color_anti_flag" -eq "0" ]
then
	echo "$1"|./utf8togb2312 |./gb2312tobmps |./bmpsall2bmp
else
	echo "$2"|./utf8togb2312 |./gb2312tobmps -c |./bmpsall2bmp
fi
