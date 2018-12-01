all: clean left right receiver

clean:
	make -C mitosis-keyboard-basic/custom/armgcc HALF=left clean cleanobj
left:
	make -C mitosis-keyboard-basic/custom/armgcc HALF=left nrf51822_xxac
right:
	make -C mitosis-keyboard-basic/custom/armgcc HALF=right nrf51822_xxac
receiver:
	make -C mitosis-receiver-basic/custom/armgcc

