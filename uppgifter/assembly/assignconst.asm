	LDR	R1,=c
	MOV	R0,#'A'
	STRB	R0,[R1]
	LDR	R1,=s
	LDR	R0,=1000
	STRH	R0,[R1]
	LDR 	R1,=i
	LDR	R0,=100000
	STR	R0,[R1]
	.ALIGN
i:	.SPACE	4
s:	.SPACE	2
c:	.SPACE	1
