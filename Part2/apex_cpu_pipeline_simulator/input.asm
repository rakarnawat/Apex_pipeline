MOVC R1,#0
MOVC R2,#1
MOVC R3,#6
MOVC R4,#5
MOVC R5,#7
MOVC R6,#0
STORE R3,R1,#3
STORE R4,R1,#4
STORE R5,R1,#2
EXOR R7,R6,R2
CMP R7,R1
BNZ #16
LOAD R8,R1,#2
MUL R8,R8,R6
STR R8,R1,R2
LOAD R8,R1,#4
MUL R8,R8,R6
STORE R8,R1,#3
NOP 
ADD R9,R8,R4
STORE R9,R1,#5
ADDL R6,R6,#1
CMP R6,R2
BZ #-56
HALT 