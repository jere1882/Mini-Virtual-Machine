mov $5, 120
mov $3, 124
mov $1, 128
mov $-1, 132
mov $-5, 136
push $5
push $120
pop %r0       
pop %r3       
xor %r1, %r1  
rep:
lw  %r0, %r2
add %r2, %r1  
add $4, %r0
loop rep
print %r1
hlt
