shr $1, %r1
shl $1, %r1
ror $6, %r2
rol $6, %r2
sar $1, %r1
sal $1, %r1
f:
ret
call f
hlt
