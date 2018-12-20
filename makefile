
ARCH=arm-none-eabi

CC=${ARCH}-gcc
AS=${ARCH}-as
OBJCOPY=${ARCH}-objcopy
OBJDUMP=${ARCH}-objdump
SIZE=${ARCH}-size

ST_FLASH=st-flash

LINKER_SCRIPTS=stm32.ld

CSOURCE=$(shell ls *.c)
ASOURCE=$(shell ls *.S)
ELF=main.elf
HEX=main.hex
BIN=main.bin
DISA_LIST=main.lst



OUTPUT_FORMAT=ihex


OBJS=${CSOURCE:.c=.o} ${ASOURCE:.S=.o}

PORT=-mcpu=cortex-m0  -mthumb # -mlittle-endian
CPUCLK=48000000
INCLUDES=-I./cmsis_include -DSTM32F051x8
OPT=-O2
CFLAGS= ${PORT} -D CPUCLK=${CPUCLK} ${OPT} -Wall ${INCLUDES}


LFLAGS= -T ${LINKER_SCRIPTS} -nostartfiles -nostdlib --specs=nosys.specs
all:${HEX} size
size:
	${SIZE} ${ELF}
${BIN}:${ELF}
	${OBJCOPY} -O binary $< $@
${HEX}:${ELF}
	${OBJCOPY} -O ihex $< $@
${ELF}:${OBJS}
	${CC} -o $@ ${LFLAGS} ${OBJS} 
.c.o:
	${CC} ${CFLAGS} -c $< -o $@
.S.o:
	${AS} ${PORT} -g $< -o $@
disa:${DISA_LIST}
${DISA_LIST}:${HEX}
	${OBJDUMP} -marm -b ihex -M force-thumb -M reg-names-std -EL -D $< > $@

clean:
	rm -rf *.o ${ELF} ${HEX} ${DISA_LIST} ${BIN} *~

burn:${HEX}
	${ST_FLASH} erase
	${ST_FLASH} --format ${OUTPUT_FORMAT} write $<

read:
# read 8k at offset 0
	${ST_FLASH} --format binary --flash=64k read x.bin 0 0x2000
