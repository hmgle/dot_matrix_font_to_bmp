#!/bin/sh

(./characters2bmp.sh "红豆生南国，" \
&& ./characters2bmp.sh "春来发几枝？" \
&& ./characters2bmp.sh "愿君多采撷，" \
&& ./characters2bmp.sh "此物最想思。" \
&& ./characters2bmp.sh "      " \
&& ./characters2bmp.sh "  -王维 ") \
| ./bmpsallv2bmp > poem.bmp
