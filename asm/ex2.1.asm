%include "ex2.2.asm"
section .bss
buffer resd 1 ;mémoire pour l'entier

section .text
global _start
_start: mov eax, buffer
	call readline
	call atoi
	push eax
	call carre
	pop eax
	call iprintLF
	mov eax, 1
	int 0x80
