  .syntax unified
	.cpu cortex-m4
	.fpu softvfp
	.thumb

    .section .rodata
    .global readme_txt
    .type   readme_txt, %object
    .align  4
readme_txt:
    .incbin "readme.txt"
readme_txt_end:
    .global readme_txt_size
    .type   readme_txt_size, %object
    .align  4
readme_txt_size:
    .int    readme_txt_end - readme_txt
