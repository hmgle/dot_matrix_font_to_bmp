#!/bin/sh

# 要utf-8的输入才能正常运行
# cygwin的运行环境需要设置 Options->Text->Character set:选择utf-8

help()
{
	cat <<EOF

	USAGE: $0 [-h] 要生成位图的汉字 > 位图存放路径

	EXAMPLES:
	$0 汉字 > ./output.bmp
EOF
}

if { [ -z "$1" ] && [ -t 0 ] ; } || [ "$1" == '-h' ]
then
	help
	exit 0
fi
echo $1|./utf8togb2312 |./gb2312tobmps |./bmps2bmp.sh
