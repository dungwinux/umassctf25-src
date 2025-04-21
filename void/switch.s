.intel_syntax noprefix

.global special

special:
  push rbx
  push rdi
  push rsi
  push rbp

  # Input stack (the env is too small so we need this)
  mov rax, 0x0101010101010101
  push rax
  push rax
  push rax
  push rax
  # ^esp+0xf8
  mov rax, 0x1919191919191919
  push rax
  push rax
  push rax
  push rax
  # ^esp+0xd8
  mov rax, 0x6060606060606060
  push rax
  push rax
  push rax
  push rax
  # ^esp+0xb8
  mov rax, 0x0909090909090909
  push rax
  push rax
  push rax
  push rax
  # ^esp+0x98
  mov rax, 0x0606060606060606
  push rax
  push rax
  push rax
  push rax
  # ^esp+0x78
  mov rax, 0x3030303030303030
  push rax
  push rax
  push rax
  push rax
  # ^esp+0x58
  mov rax, 0x8080808080808080
  push rax
  push rax
  push rax
  push rax
  # ^esp+0x38
  mov rax, 0x0f0f0f0f0f0f0f0f
  push rax
  push rax
  push rax
  push rax
  # ^esp+0x18

  # Setup return
  lea rax, [rip + special_exit]
  mov r9, 0x33
  shl r9, 32
  add rax, r9
  push rax

  # Setup entry
  push 0x23
  lea rax, [rip + special_body]
  push rax
  mov rbp, r8

  # Other
  vmovdqa ymm3, ymmword ptr [rip + this_is_useful]
  vpbroadcastw ymm2, word ptr [rip + lol]
  vpbroadcastb ymm1, byte ptr [rip + lol + 2]
 
  retfq
special_body:
.code32
  vpxor ymm0, ymm0, ymm0
  sub esp, 0x10
  push ecx
  push edx
  mov eax, 1
  cpuid
  and ecx, 0x18000000
  cmp ecx, 0x18000000
  jne check_exit
  mov eax, 7
  mov ecx, 0
  cpuid
  and ebx, 0x20
  cmp ebx, 0x20
  jne check_exit
  mov ecx, 0
  xgetbv
  and eax, 6
  cmp eax, 6
  jne check_exit

  pop edx
  pop ecx
  jmp vectorize
check_exit:
  add esp, 8
  mov eax, -1
  jmp exit

vectorize:
  xor esi, esi
  xor edi, edi
  # force no-vectorize if needed
  # jmp end_vectorize
start_vectorize:
  mov edi, ebp
  mov dword ptr [esp], esi
  and byte ptr [esp], 0x70
  vpbroadcastb ymm4, byte ptr [esp]
  vpor ymm4, ymm4, ymm3
  sub edi, esi
  cmp edi, 32
  jl end_vectorize

  # set new starting base (0x30) and cap at 75
  vmovdqu ymm5, ymmword ptr [edx + esi]
  vpsubb ymm5, ymm5, ymmword ptr [esp + 0x58]
  vpaddusb ymm5, ymm5, ymm1
  vpsubb ymm5, ymm5, ymm1

  # convert to packed BCD
  vpunpcklbw ymm6, ymm5, ymm0
  vpmullw ymm6, ymm6, ymm2
  vpsrlw ymm6, ymm6, 11
  vpunpckhbw ymm7, ymm5, ymm0
  vpmullw ymm7, ymm7, ymm2
  vpsrlw ymm7, ymm7, 11
  vpackuswb ymm7, ymm6, ymm7
  vpaddb ymm7, ymm7, ymm7
  vpsllw ymm6, ymm7, 3
  vpsubb ymm5, ymm5, ymm7
  vpaddb ymm7, ymm7, ymm7
  vpaddb ymm7, ymm7, ymm7
  vpsubb ymm5, ymm5, ymm7
  vpor ymm7, ymm5, ymm6

  # adding spice (and cook)
  vpaddb ymm5, ymm4, ymm7
  # Check if (x & 0xF > 9)
  vpand ymm6, ymm5, ymmword ptr [esp + 0x18]
  vpcmpgtb ymm6, ymm6, ymmword ptr [esp + 0x98]
  # Check if (AF == 1)
  vpxor ymm7, ymm4, ymm7
  vpxor ymm7, ymm5, ymm7
  vpsrlw ymm7, ymm7, 4
  vpand ymm7, ymm7, ymmword ptr [esp + 0xf8]
  vpsubb ymm7, ymm0, ymm7

  vpor ymm6, ymm7, ymm6
  vpand ymm6, ymm6, ymmword ptr [esp + 0x78]
  vpaddb ymm7, ymm5, ymm6

  vpxor ymm6, ymm7, ymmword ptr [esp + 0x38]
  vpcmpgtb ymm6, ymm6, ymmword ptr [esp + 0xd8]
  vpand ymm6, ymm6, ymmword ptr [esp + 0xb8]
  vpaddb ymm5, ymm6, ymm7

  # saving result
  vmovdqu ymmword ptr [ecx + esi], ymm5

  add esi, 32
  jmp start_vectorize
end_vectorize:

  vzeroupper
  xor eax, eax
  xchg esi, edx
  xchg edi, ecx
  mov cx, 0xa04
loop:
  cmp edx, ebp
  jge end_loop

  # set new starting base
  movzx ebx, byte ptr [esi + edx]
  sub bl, 0x30
  mov ax, 75
  cmp al, bl
  cmova eax, ebx

  # convert to packed BCD
  div ch
  shl al, cl
  or al, ah

  # adding spice (and cook)
  mov bl, al
  mov bh, dl
  and bh, 0x70
  mov al, dl
  and al, 0xf
  xor ah, ah
  div ch
  or bh, ah
  mov al, bl
  add al, bh
  daa

  # saving result
  mov byte ptr [edi + edx], al

  inc edx
  jmp loop
end_loop:

  xor eax, eax
exit:
  add esp, 0x10
  retf
.code64
special_exit:

  add rsp, 0x100

  pop rbp
  pop rsi
  pop rdi
  pop rbx
  ret

lol:
  .short 205
  .byte 180

.align 32
this_is_useful:
  .byte 0
  .byte 1
  .byte 2
  .byte 3
  .byte 4
  .byte 5
  .byte 6
  .byte 7
  .byte 8
  .byte 9
  .byte 0
  .byte 1
  .byte 2
  .byte 3
  .byte 4
  .byte 5
  .byte 0x10
  .byte 0x11
  .byte 0x12
  .byte 0x13
  .byte 0x14
  .byte 0x15
  .byte 0x16
  .byte 0x17
  .byte 0x18
  .byte 0x19
  .byte 0x10
  .byte 0x11
  .byte 0x12
  .byte 0x13
  .byte 0x14
  .byte 0x15
