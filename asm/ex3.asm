%include "io.asm"
section .bss
buffer resd 1 ;pour lire l'entree
somme resd 10 ;somme des 10 entiers
min resd 1    ;minimum des 10 entiers
max resd 1    ;maximum des 10 entiers

section .text
global _start
_start: 


