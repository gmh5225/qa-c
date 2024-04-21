### A C

Toy C compiler to practice C++ and learn about compilers.

#### Progress

Compiles

```
int main() {
    
    int result =0; 
    for (int i = 0; i < 5; i = i + 1  ) {
        result = result + 1; 
    }
    return result;
}
```
to 
```
section .text
global _start
main:
	push rbp
	mov rbp, rsp
	sub rsp, 16
	mov dword [rbp - 4], 0
	mov dword [rbp - 8], 0
	jmp .L0
.L1:
	mov eax, dword [rbp - 4]
	add eax, 1
	mov dword [rbp - 4], eax
	mov eax, dword [rbp - 8]
	add eax, 1
	mov dword [rbp - 8], eax
.L0:
	mov eax, dword [rbp - 8]
	cmp eax, 5
	sete al
	movzx eax, al
	jl .L1
	jmp .L2
.L2:
	mov eax, dword [rbp - 4]
	jmp .end
.end:
	leave
	ret
_start:
	call main
	mov edi, eax
	mov eax, 60
	syscall
```