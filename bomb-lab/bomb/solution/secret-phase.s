0x401242 <+0>:     push   %rbx
0x401243 <+1>:     call   0x40149e <read_line>
0x401248 <+6>:     mov    $0xa,%edx
0x40124d <+11>:    mov    $0x0,%esi
0x401252 <+16>:    mov    %rax,%rdi
0x401255 <+19>:    call   0x400bd0 <strtol@plt>

# val must <= 1001
0x40125a <+24>:    mov    %rax,%rbx
0x40125d <+27>:    lea    -0x1(%rax),%eax
0x401260 <+30>:    cmp    $0x3e8,%eax
0x401265 <+35>:    jbe    0x40126c <secret_phase+42>
0x401267 <+37>:    call   0x40143a <explode_bomb>

0x40126c <+42>:    mov    %ebx,%esi
0x40126e <+44>:    mov    $0x6030f0,%edi
0x401273 <+49>:    call   0x401204 <fun7>
0x401278 <+54>:    cmp    $0x2,%eax
0x40127b <+57>:    je     0x401282 <secret_phase+64>
0x40127d <+59>:    call   0x40143a <explode_bomb>

# Print message and return
0x401282 <+64>:    mov    $0x402438,%edi
0x401287 <+69>:    call   0x400b10 <puts@plt>
0x40128c <+74>:    call   0x4015c4 <phase_defused>
0x401291 <+79>:    pop    %rbx
0x401292 <+80>:    ret
