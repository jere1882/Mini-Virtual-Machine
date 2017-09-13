mov $32, %r3    
xor %r2, %r2    
read %r0        
rep:        
push %r3
mov $1, %r1
dec %r3
shl %r3, %r1    
pop %r3
and %r0, %r1   
cmp $0, %r1   
jmpe apagado
inc %r2       
apagado: 
loop rep
print %r2   
hlt
