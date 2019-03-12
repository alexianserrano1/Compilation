%include "io.asm"
section .bss
buffer resd 1 ;mémoire pour l'entier

section .text
global _start
_start: mov eax, buffer
	call readline
	call atoi
	imul eax
	call iprintLF
	mov eax, 1
	int 0x80
