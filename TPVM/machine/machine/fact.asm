read %r1    
call fact
print %r1
hlt
fact:
cmp $0, %r1
jmpe cero
cmp $1, %r1
jmpe uno
push %r1       
dec %r1
call fact
pop %r2
mul %r2, %r1
ret
cero:
mov $1, %r1
uno:
ret
