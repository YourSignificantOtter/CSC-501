	.text
	.globl zfunction
zfunction:
	MOV 0x4(%ESP), %EAX
	ANDL $0xFFFC01FF, %EAX
	SHRL $0x8, %EAX
	ORL $0xFF000000, %EAX
	ret
