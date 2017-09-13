read %r0 
cmp $0, %r0  
jmpl negativo
print %r0
jmp fin
negativo:
mul $-1, %r0
print %r0
fin:
hlt        
