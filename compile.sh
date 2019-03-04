gcc -c -o InputHandler.o ./InputHandler.c
gcc -c -o chatmain.o ./chat.c
gcc -o chat ./InputHandler.o ./chatmain.o

rm ./InputHandler.o
rm ./chatmain.o