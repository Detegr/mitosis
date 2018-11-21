all: left right receiver

left:
	make -C mitosis-keyboard-basic/custom/armgcc HALF=left
right:
	make -C mitosis-keyboard-basic/custom/armgcc HALF=right
receiver:
	make -C mitosis-receiver-basic/custom/armgcc

