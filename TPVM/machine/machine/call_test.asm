read %r1
call fact
print %r1
hlt
fact:
cmp $0, %r1    
jmpe cero        
mov %r1, %r3
mov $1, %r1  
rep:
mul %r3, %r1
loop rep
ret
cero:
mov $1, %r1
ret
