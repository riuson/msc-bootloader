Demonstration project that implements a bootloader for the MCU with a pseudo-file system interface.

The file system is FAT16 without the support of long names.
Microcontroller is [STM32F429ZIT6](https://www.st.com/en/microcontrollers/stm32f429zi.html) on board [STM32F429I-DISCO](https://www.st.com/en/evaluation-tools/32f429idiscovery.html).

When starting, the user's firmware is checked.
If it is present, then it is launched. Otherwise, the bootloader is started.
If the user's button is pressed, bootloader also starts.

The loader implements the USB MSC. The file system of the drive contains 2 files:
  *  *readme.txt*: a file that is read-only;
  *  *firmware.bin*: this allows you to copy the user firmware from the device to the PC, and also download it from the PC to the device, simply by copying the file.

Thanks to
  * http://s-engineer.ru/opisanie-fajlovoj-sistemy-fat16/
  * http://www.beginningtoseethelight.org/fat16/index.htm
  * http://www.pvsm.ru/stm32/263347

[Video demonstration](https://www.youtube.com/watch?v=drFkwgE9rDg)
