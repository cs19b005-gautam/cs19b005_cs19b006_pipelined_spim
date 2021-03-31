	.data
array:  .word	11, 5, 18, 7, 9, 4, 56, 3, 7, 2, 8

	.text
main:
	la	$s0, array
	add	$s1, $zero, 10
	j	BubbleSort	

BubbleSort:
	beq	$zero, $s1, exit
	add	$s2, $s1, $zero
	j	swapLoop

swapLoop:
	beq	$zero, $s2, bubbleJump
	lw	$t0, 0($s0)
	lw	$t1, 4($s0)

	slt	$t2, $t0, $t1
	bne	$zero, $t2, jumpSwap
	sw	$t0, 4($s0)
	sw	$t1, 0($s0)
	j	jumpSwap

jumpSwap:
	add	$s0, $s0, 4
	sub	$s2, $s2, 1 
	j	swapLoop

bubbleJump:
	sub	$s1, $s1, 1 
	la	$s0, array
	j	BubbleSort 
	
exit:
	add	$s1, $zero, 11
	
terminate:
	li	$v0, 10
	syscall