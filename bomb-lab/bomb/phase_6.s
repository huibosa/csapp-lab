0x4010f4 <+0>:     push   %r14
0x4010f6 <+2>:     push   %r13
0x4010f8 <+4>:     push   %r12
0x4010fa <+6>:     push   %rbp
0x4010fb <+7>:     push   %rbx
0x4010fc <+8>:     sub    $0x50,%rsp
0x401100 <+12>:    mov    %rsp,%r13 # $r13 = $rsp
0x401103 <+15>:    mov    %rsp,%rsi
0x401106 <+18>:    call   0x40145c <read_six_numbers>


0x40110b <+23>:    mov    %rsp,%r14 # $r13 = $1
0x40110e <+26>:    mov    $0x0,%r12d # $r12d = 0
0x401114 <+32>:    mov    %r13,%rbp # $rbp = $r13 = $rsp = $1

# If val > 6, then call explode_bomb
0x401117 <+35>:    mov    0x0(%r13),%eax # $eax = $1
0x40111b <+39>:    sub    $0x1,%eax # $eax = $1 - 1
0x40111e <+42>:    cmp    $0x5,%eax
0x401121 <+45>:    jbe    0x401128 <phase_6+52> 
0x401123 <+47>:    call   0x40143a <explode_bomb>

# If any number equals 1st number, then explode_bomb
# $r12d and $ebx stores index
0x401128 <+52>:    add    $0x1,%r12d # $r12d = 1
0x40112c <+56>:    cmp    $0x6,%r12d 
0x401130 <+60>:    je     0x401153 <phase_6+95> # if i == 6, then jump
0x401132 <+62>:    mov    %r12d,%ebx
0x401135 <+65>:    movslq %ebx,%rax
0x401138 <+68>:    mov    (%rsp,%rax,4),%eax
0x40113b <+71>:    cmp    %eax,0x0(%rbp)
0x40113e <+74>:    jne    0x401145 <phase_6+81>
0x401140 <+76>:    call   0x40143a <explode_bomb>
0x401145 <+81>:    add    $0x1,%ebx
0x401148 <+84>:    cmp    $0x5,%ebx
0x40114b <+87>:    jle    0x401135 <phase_6+65>
0x40114d <+89>:    add    $0x4,%r13 # ($r13) = $2
0x401151 <+93>:    jmp    0x401114 <phase_6+32>

# val[i] = 7 - val[i]
0x401153 <+95>:    lea    0x18(%rsp),%rsi # $rsi = $6
0x401158 <+100>:   mov    %r14,%rax       # $rax = $rsp
0x40115b <+103>:   mov    $0x7,%ecx
0x401160 <+108>:   mov    %ecx,%edx
0x401162 <+110>:   sub    (%rax),%edx # 7 - $1
0x401164 <+112>:   mov    %edx,(%rax) # ($rsp) = 7 - $1
0x401166 <+114>:   add    $0x4,%rax
0x40116a <+118>:   cmp    %rsi,%rax
0x40116d <+121>:   jne    0x401160 <phase_6+108>

0x40116f <+123>:   mov    $0x0,%esi
0x401174 <+128>:   jmp    0x401197 <phase_6+163>
0x401176 <+130>:   mov    0x8(%rdx),%rdx
0x40117a <+134>:   add    $0x1,%eax
0x40117d <+137>:   cmp    %ecx,%eax
0x40117f <+139>:   jne    0x401176 <phase_6+130>
0x401181 <+141>:   jmp    0x401188 <phase_6+148>

# $rsi: index, step 0x4, upper limit 0x18
# $rdx: addresses
0x401183 <+143>:   mov    $0x6032d0,%edx
0x401188 <+148>:   mov    %rdx,0x20(%rsp,%rsi,2)
0x40118d <+153>:   add    $0x4,%rsi
0x401191 <+157>:   cmp    $0x18,%rsi
0x401195 <+161>:   je     0x4011ab <phase_6+183>

0x401197 <+163>:   mov    (%rsp,%rsi,1),%ecx
0x40119a <+166>:   cmp    $0x1,%ecx
0x40119d <+169>:   jle    0x401183 <phase_6+143>
0x40119f <+171>:   mov    $0x1,%eax
0x4011a4 <+176>:   mov    $0x6032d0,%edx
0x4011a9 <+181>:   jmp    0x401176 <phase_6+130>

# This segment store 6 addr *0x006032d0
# $rax: store index
# $rsi: store index upper limit
# $rbx: store start addr
0x4011ab <+183>:   mov    0x20(%rsp),%rbx
0x4011b0 <+188>:   lea    0x28(%rsp),%rax
0x4011b5 <+193>:   lea    0x50(%rsp),%rsi
0x4011ba <+198>:   mov    %rbx,%rcx
0x4011bd <+201>:   mov    (%rax),%rdx
0x4011c0 <+204>:   mov    %rdx,0x8(%rcx)
0x4011c4 <+208>:   add    $0x8,%rax
0x4011c8 <+212>:   cmp    %rsi,%rax
0x4011cb <+215>:   je     0x4011d2 <phase_6+222>
0x4011cd <+217>:   mov    %rdx,%rcx
0x4011d0 <+220>:   jmp    0x4011bd <phase_6+201>

0x4011d2 <+222>:   movq   $0x0,0x8(%rdx)
0x4011da <+230>:   mov    $0x5,%ebp
0x4011df <+235>:   mov    0x8(%rbx),%rax
0x4011e3 <+239>:   mov    (%rax),%eax
0x4011e5 <+241>:   cmp    %eax,(%rbx)
0x4011e7 <+243>:   jge    0x4011ee <phase_6+250>
0x4011e9 <+245>:   call   0x40143a <explode_bomb>
0x4011ee <+250>:   mov    0x8(%rbx),%rbx
0x4011f2 <+254>:   sub    $0x1,%ebp
0x4011f5 <+257>:   jne    0x4011df <phase_6+235>

0x4011f7 <+259>:   add    $0x50,%rsp
0x4011fb <+263>:   pop    %rbx
0x4011fc <+264>:   pop    %rbp
0x4011fd <+265>:   pop    %r12
0x4011ff <+267>:   pop    %r13
0x401201 <+269>:   pop    %r14
0x401203 <+271>:   ret
