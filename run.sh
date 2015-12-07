source ../PathSetter.bash
bcc -ansi -c -o kernel.o kernel.c
as86 kernel.asm -o kernel_asm.o
ld86 -o kernel -d kernel.o kernel_asm.o
nasm bootload.asm
dd if=/dev/zero of=floppya.img bs=512 count=2880
dd if=bootload of=floppya.img bs=512 count=1 seek=0 conv=notrunc 
dd if=map.img of=floppya.img bs=512 count=1 seek=1 conv=notrunc
dd if=dir.img of=floppya.img bs=512 count=1 seek=2 conv=notrunc
dd if=kernel of=floppya.img bs=512 conv=notrunc seek=3
gcc -o loadFile loadFile.c
./loadFile message.txt

bcc -ansi -c -o userlib.o userlib.c

as86 lib.asm -o lib.o

bcc -ansi -c -o txtedt.o txtedt.c
ld86 -o txtedt -d txtedt.o userlib.o lib.o
./loadFile txtedt

bcc -ansi -c -o shell.o shell.c
ld86 -o shell -d shell.o userlib.o lib.o
./loadFile shell

bochs -q -f opsys.bxrc
