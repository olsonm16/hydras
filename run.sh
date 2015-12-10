function makeScript {
     filename=$1
     bcc -ansi -c -o $filename.o $filename.c
     ld86 -o $filename -d $filename.o lib.o userlib.o
     ./loadFile $filename
}

source ../PathSetter.bash

bcc -ansi -c -o proc.o proc.c

bcc -ansi -c -o kernel.o kernel.c
as86 kernel.asm -o kernel_asm.o
ld86 -o kernel -d kernel.o proc.o kernel_asm.o

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

for prog in shell txtedt uprog2 hello bye hello5 bye5
     do
         echo $prog
         makeScript $prog # use of above function
     done

bochs -q -f opsys.bxrc




