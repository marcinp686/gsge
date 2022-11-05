.data
	
.code
	
; sine function calculation with sin(x) = x*(4/pi) - (abs(x)*x*4)/(pi*pi)
; B=1.27323954473516, C=0.40528473456935 
; which is: sin(x) = x*B - x*abs(x)*C
; pi=3.14159265358979 = 40490fdbh

; The x64 ABI considers the registers RAX, RCX, RDX, R8, R9, R10, R11, and XMM0-XMM5 volatile.
; When present, the upper portions of YMM0-YMM15 and ZMM0-ZMM15 are also volatile

updateTransformMatrix proc
	
	mov r10, 0000000100000000H
	movq xmm0,r10
	vbroadcastsd ymm0,xmm0
	mov r11, qword ptr [rcx+16]
	vpermps ymm1,ymm0,ymmword ptr [rdx]
	
	
	vbroadcastf128 ymm2, xmmword ptr [rcx+32]		; load scale vector to ymm2
	vbroadcastf128 ymm3, xmmword ptr [rcx+16]		; load rotation xyz0xyz0 for sine and cosine calculation to ymm3
	
	; store abs(x) in ymm6
	mov r10d, 7fffffffh							; mask to have absolute value of x
	movq xmm6, r10
	vbroadcastss ymm6, xmm6
	vpand ymm6, ymm3, ymm6						; absolute value of rotations(ymm3) stored in ymm6
	
	; check rotation bounds to be within -pi to +pi
	mov r10d, 40490fdbh							; PI as hex
	movd xmm9, r10d
	vbroadcastss ymm9,xmm9						; broadcast pi to ymm9
	vdivps ymm11, ymm3, ymm9					; divide x / PI
	vroundps ymm12,ymm11, 00000011b
	vsubps ymm13,ymm11,ymm12
	vmulps	ymm14,ymm9,ymm13
	vmovaps xmmword ptr [rcx+16], xmm14
	
	; begin calculation on sine 		
	vmulps ymm4,ymm3,ymm4						; multiply x*B, store in ymm4
	vmulps ymm5,ymm3,ymm5						; multiply x*C, store in ymm5

	; VFNMADDPS
	vmulps ymm5, ymm6, ymm5
	vsubps  ymm7, ymm4, ymm5
	; end sine calculation



	;pop r10
	ret
updateTransformMatrix endp
end

updateTransformMatrix_1 proc
	;push r10 
	
	mov r10, 03fa2f983h						; B = 1.27323954473516 as hex
	movd xmm4, r10
	vbroadcastss ymm4, xmm4						; broadcast B to ymm4
	
	mov r10d, 03ecf817bh						; C = 0.40528473456935 as hex
	movq xmm5, r10
	vbroadcastss ymm5, xmm5						; broadcast C to ymm
	
	vbroadcastf128 ymm2, xmmword ptr [rcx+32]		; load scale vector to ymm2
	vbroadcastf128 ymm3, xmmword ptr [rcx+16]		; load rotation xyz0xyz0 for sine and cosine calculation to ymm3
	
	; store abs(x) in ymm6
	mov r10d, 7fffffffh							; mask to have absolute value of x
	movq xmm6, r10
	vbroadcastss ymm6, xmm6
	vpand ymm6, ymm3, ymm6						; absolute value of rotations(ymm3) stored in ymm6
	
	; check rotation bounds to be within -pi to +pi
	mov r10d, 40490fdbh							; PI as hex
	movd xmm9, r10d
	vbroadcastss ymm9,xmm9						; broadcast pi to ymm9
	vdivps ymm11, ymm3, ymm9					; divide x / PI
	vroundps ymm12,ymm11, 00000011b
	vsubps ymm13,ymm11,ymm12
	vmulps	ymm14,ymm9,ymm13
	vmovaps xmmword ptr [rcx+16], xmm14
	
	; begin calculation on sine 		
	vmulps ymm4,ymm3,ymm4						; multiply x*B, store in ymm4
	vmulps ymm5,ymm3,ymm5						; multiply x*C, store in ymm5

	; VFNMADDPS
	vmulps ymm5, ymm6, ymm5
	vsubps  ymm7, ymm4, ymm5
	; end sine calculation



	;pop r10
	ret
updateTransformMatrix_1 endp
end