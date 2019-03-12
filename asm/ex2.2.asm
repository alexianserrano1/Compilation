%include "io.asm"

carre : 
	pop ebx
	pop eax
	imul eax
	push eax
	push ebx
	ret
	
