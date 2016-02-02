#Pinplay Compile, Record and Replay Commands

##For Compilation
g++   -D_FILE_OFFSET_BITS=64 -I/home/pinplay/PinPlay/latest/source/tools/InstLib -I/home/pinplay/PinPlay/latest/extras/pinplay//include -I/home/pinplay/PinPlay/latest/source/tools/PinPoints -DSLICING   -O2    -I/home/pinplay/PinPlay/latest/source/include/pin -I/home/pinplay/PinPlay/latest/source/include/pin/gen -I/home/pinplay/PinPlay/latest/extras/components/include -I/home/pinplay/PinPlay/latest/extras/xed-intel64/include -I/home/pinplay/PinPlay/latest/source/tools/InstLib -DBIGARRAY_MULTIPLIER=1 -Wall -Werror -Wno-unknown-pragmas -fno-stack-protector -DTARGET_IA32E -DHOST_IA32E -fPIC -DTARGET_LINUX  -I/home/pinplay/PinPlay/latest/source/include/pin -I/home/pinplay/PinPlay/latest/source/include/pin/gen -I/home/pinplay/PinPlay/latest/extras/components/include -I/home/pinplay/PinPlay/latest/extras/xed-intel64/include -I/home/pinplay/PinPlay/latest/source/tools/InstLib -O3 -fomit-frame-pointer -fno-strict-aliasing    -c -o obj-intel64//inscount.o inscount.cpp

g++ -shared -Wl,--hash-style=sysv -Wl,-Bsymbolic -Wl,--version-script=/home/pinplay/PinPlay/latest/source/include/pin/pintool.ver    -o obj-intel64//inscount.so obj-intel64//inscount.o /home/pinplay/PinPlay/latest/extras/pinplay//lib/intel64/libslicing.a /home/pinplay/PinPlay/latest/extras/pinplay//lib/intel64/libpinplay.a /home/pinplay/PinPlay/latest/extras/pinplay//lib-ext/intel64/libbz2.a /home/pinplay/PinPlay/latest/extras/pinplay//lib-ext/intel64/libz.a /home/pinplay/PinPlay/latest/source/tools/InstLib/obj-intel64/controller.a obj-intel64//pinplay-debugger-shell.o  -L/home/pinplay/PinPlay/latest/intel64/lib -L/home/pinplay/PinPlay/latest/intel64/lib-ext -L/home/pinplay/PinPlay/latest/intel64/runtime/glibc -L/home/pinplay/PinPlay/latest/extras/xed-intel64/lib -lpin -lxed -lpindwarf -ldl


##Log a Program Execution
/home/pinplay/PinPlay/latest/pin -t /home/pinplay/PinPlay/latest/extras/pinplay/examples/obj-intel64/inscount.so -log -log:basename pinball/foo -log:compressed bzip2  -- ./hello64

##Replay Program Execution
/home/pinplay/PinPlay/latest/pin -xyzzy -reserve_memory pinball/foo.address -t /home/pinplay/PinPlay/latest/extras/pinplay/examples/obj-intel64/inscount.so -replay -replay:basename pinball/foo -- /home/pinplay/PinPlay/latest/extras/pinplay//bin/intel64/nullapp

/home/pinplay/PinPlay/latest/pin -xyzzy -reserve_memory whole_program.test.txt/sjeng.test.txt_2655.address -t /home/pinplay/PinPlay/latest/extras/pinplay/examples/obj-intel64/malloctrace.so -replay -replay:basename whole_program.test.txt/sjeng.test.txt_2655 -- /home/pinplay/PinPlay/latest/extras/pinplay//bin/intel64/nullapp

pin -xyzzy -reserve_memory pinball/foo.address -t your-tool.so
-replay -replay:basename pinball/foo -- $PIN_ROOT/extrans/pinplay/bin/intel64/nullapp
    [for intel64 pinballs]


/home/pinplay/PinPlay/latest/pin -xyzzy -reserve_memory sjeng.test.txt_4860.pp/sjeng.test.txt_4860_t0r5_warmup1001500_prolog0_region30000037_epilog0_005_0-20588.0.address -t /home/pinplay/PinPlay/latest/extras/pinplay/examples/obj-intel64/inscount4.so -replay -replay:basename sjeng.test.txt_4860.pp/sjeng.test.txt_4860_t0r5_warmup1001500_prolog0_region30000037_epilog0_005_0-20588.0 -- /home/pinplay/PinPlay/latest/extras/pinplay//bin/intel64/nullapp > log_region5.txt


