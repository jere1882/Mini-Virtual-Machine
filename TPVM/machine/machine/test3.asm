push $0
push $0
push $0
push $0
push $123
push $0
push $-3
push $0
push $0
push $0
push $3
push $996
pop %r0
pop %r1
mov $0, %r2
next:
lw %r0, %r3
add $4, %r0
add %r3, %r2
sub $1, %r1
mul $-1, %r1
cmp $0, %r1
print $1
jmpl next
print %r2
hlt
