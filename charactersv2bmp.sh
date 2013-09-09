#!/bin/sh

# 要utf-8方式编码的输入才能正常运行
# cygwin的运行环境需要设置 Options->Text->Character set:选择utf-8
# 以垂直方向合并位图

help()
{
	cat <<EOF

	USAGE: $0 [-h] "要生成位图的字符" > 位图存放路径

	EXAMPLES:
	$0 "汉字" > ./output.bmp
EOF
}

if { [ -z "$1" ] && [ -t 0 ] ; } || [ "$1" = '-h' ]
then
	help
	exit 0
fi
echo "$1"|./utf8togb2312 |./gb2312tobmps |./bmpsallv2bmp
