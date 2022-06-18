#!/bin/sh

# build and run program snippets in localwork
wk_space="$1"
prog_name="$2"

# gcc -ggdb -fsanitize=address -fno-omit-frame-pointer -o ${prog_name} ${wk_space}/c/${prog_name}.c -lpthread
gcc -o ${prog_name} ${wk_space}/c/${prog_name}.c -lpthread

./${prog_name}