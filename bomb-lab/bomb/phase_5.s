0x401062 <+0>:     push   %rbx
0x401063 <+1>:     sub    $0x20,%rsp
0x401067 <+5>:     mov    %rdi,%rbx
0x40106a <+8>:     mov    %fs:0x28,%rax
0x401073 <+17>:    mov    %rax,0x18(%rsp)

0x401078 <+22>:    xor    %eax,%eax
0x40107a <+24>:    call   0x40131b <string_length>
0x40107f <+29>:    cmp    $0x6,%eax
0x401082 <+32>:    je     0x4010d2 <phase_5+112>
0x401084 <+34>:    call   0x40143a <explode_bomb>
0x401089 <+39>:    jmp    0x4010d2 <phase_5+112>

0x40108b <+41>:    movzbl (%rbx,%rax,1),%ecx # Get 1st byte of input string
0x40108f <+45>:    mov    %cl,(%rsp)
0x401092 <+48>:    mov    (%rsp),%rdx
0x401096 <+52>:    and    $0xf,%edx # Mask last 4 bit of character
0x401099 <+55>:    movzbl 0x4024b0(%rdx),%edx # Get byte of STRING

0x4010a0 <+62>:    mov    %dl,0x10(%rsp,%rax,1)
0x4010a4 <+66>:    add    $0x1,%rax
0x4010a8 <+70>:    cmp    $0x6,%rax
0x4010ac <+74>:    jne    0x40108b <phase_5+41>
0x4010ae <+76>:    movb   $0x0,0x16(%rsp)

0x4010b3 <+81>:    mov    $0x40245e,%esi
0x4010b8 <+86>:    lea    0x10(%rsp),%rdi
0x4010bd <+91>:    call   0x401338 <strings_not_equal>

0x4010c2 <+96>:    test   %eax,%eax
0x4010c4 <+98>:    je     0x4010d9 <phase_5+119>
0x4010c6 <+100>:   call   0x40143a <explode_bomb>
0x4010cb <+105>:   nopl   0x0(%rax,%rax,1)
0x4010d0 <+110>:   jmp    0x4010d9 <phase_5+119>
0x4010d2 <+112>:   mov    $0x0,%eax
0x4010d7 <+117>:   jmp    0x40108b <phase_5+41>
0x4010d9 <+119>:   mov    0x18(%rsp),%rax
0x4010de <+124>:   xor    %fs:0x28,%rax
0x4010e7 <+133>:   je     0x4010ee <phase_5+140>
0x4010e9 <+135>:   call   0x400b30 <__stack_chk_fail@plt>
0x4010ee <+140>:   add    $0x20,%rsp
0x4010f2 <+144>:   pop    %rbx
0x4010f3 <+145>:   ret
