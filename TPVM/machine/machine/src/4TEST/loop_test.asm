mov $10, %r3
mov $0, %r1
rep:
print %r1
add $1, %r1
loop rep
hlt
