# %rdi: pointer
0x401204 <+0>:     sub    $0x8,%rsp
0x401208 <+4>:     test   %rdi,%rdi
0x40120b <+7>:     je     0x401238 <fun7+52> # Check if NULL pointer

0x40120d <+9>:     mov    (%rdi),%edx # $edx = 36
0x40120f <+11>:    cmp    %esi,%edx
0x401211 <+13>:    jle    0x401220 <fun7+28>
0x401213 <+15>:    mov    0x8(%rdi),%rdi
0x401217 <+19>:    call   0x401204 <fun7>
0x40121c <+24>:    add    %eax,%eax
0x40121e <+26>:    jmp    0x40123d <fun7+57>
0x401220 <+28>:    mov    $0x0,%eax
0x401225 <+33>:    cmp    %esi,%edx
0x401227 <+35>:    je     0x40123d <fun7+57>
0x401229 <+37>:    mov    0x10(%rdi),%rdi
0x40122d <+41>:    call   0x401204 <fun7>
0x401232 <+46>:    lea    0x1(%rax,%rax,1),%eax
0x401236 <+50>:    jmp    0x40123d <fun7+57>
0x401238 <+52>:    mov    $0xffffffff,%eax
0x40123d <+57>:    add    $0x8,%rsp
0x401241 <+61>:    ret
