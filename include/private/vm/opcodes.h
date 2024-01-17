yy(OP_LDLOC, int32_t, i32, "%d", ldloc, index)
yy(OP_STLOC, int32_t, i32, "%d", stloc, index)
yy(OP_LDARG, int32_t, i32, "%d", ldarg, index)
yy(OP_LDFRAME, int32_t, i32, "%d", ldframe, index)
yy(OP_LDSTR, int32_t, i32, "%d", ldstr, index)
yy(OP_LDGLOB, int32_t, i32, "%d", ldglob, index)
yy(OP_STGLOB, int32_t, i32, "%d", stglob, index)
yy(OP_PUSHSD, double, flt64, "%lf", pushsd, flt64)
yy(OP_PUSHSL, mc_bool_t, byte, "%u", pushsl, logical)
yy(OP_PUSHR, uint64_t, u64, "%lu", pushr, obj_ref)
xx(OP_POP, pop)
xx(OP_CCOWR, ccowr)
yy(OP_CALLI, int32_t, i32, "%d", calli, inargc)
yy(OP_CALL, int32_t, i32, "%d", call,  inargc)
xx(OP_DISP  , disp)
xx(OP_RET   , ret)
	
xx(OP_TRANS	, trans)
xx(OP_HCAT	, hcat)
xx(OP_VCAT	, vcat)
xx(OP_RANGE	, range)
/*
	SUB2D (Create 2D Subscript)

	STACK[-1] = Matrix to be subscripted
	STACK[-2] = Subscript Row
	STACK[-3] = Subscript Column

	STACK[-1] = SUB2D(STACK[-1], STACK[-2], STACK[-3])

	After the execution, STACK[-1] is subscript matrix.
*/
xx(OP_SUB2D	, sub2d)
/*
	SUBG (Subscript Get)
	STACK[-1] = Matrix to be subscripted
	STACK[-2] = Subscript Matrix 
	STACK[-1] = SUBG(STACK[-1], STACK[-2])
*/
xx(OP_SUBG	, subg)
/*
	SUBG (Subscript Set) 
	STACK[-1] = Matrix to be subscripted
	STACK[-2] = Subscript Matrix
	STACK[-3] = Value
	STACK[-1] = SUBS(STACK[-1], STACK[-2], STACK[-3])
*/
xx(OP_SUBS	, subs)
/*
	ADD (Add operands)
	STACK[-1] = Left
	STACK[-2] = Right 
	STACK[-1] = STACK[-1] + STACK[-2]
*/
xx(OP_ADD	, add)
/*
	ADD (Add operands)
	STACK[-1] = Left
	STACK[-2] = Right
	STACK[-1] = STACK[-1] - STACK[-2]
*/
xx(OP_SUB	, sub)
/*
	ADD (Add operands)
	STACK[-1] = Left
	STACK[-2] = Right
	STACK[-1] = STACK[-1] * STACK[-2]
*/
xx(OP_MUL	, mul)
/*
	ADD (Add operands)
	STACK[-1] = Left
	STACK[-2] = Right
	STACK[-1] = STACK[-1] / STACK[-2]
*/
xx(OP_DIV	, div)
/*
	ADD (Add operands)
	STACK[-1] = Left
	STACK[-2] = Right
	STACK[-1] = STACK[-1] ^ STACK[-2]
*/
xx(OP_POW	, pow)
/*
	ADD (Add operands)
	STACK[-1] = Left
	STACK[-2] = Right
	STACK[-1] = STACK[-1] .* STACK[-2]
*/
xx(OP_DOTMUL, dotmul)
/*
	ADD (Add operands)
	STACK[-1] = Left
	STACK[-2] = Right
	STACK[-1] = STACK[-1] ./ STACK[-2]
*/
xx(OP_DOTDIV, dotdiv)
/*
	ADD (Add operands)
	STACK[-1] = Left
	STACK[-2] = Right
	STACK[-1] = STACK[-1] .^ STACK[-2]
*/
xx(OP_DOTPOW, dotpow)
/*
	ADD (Add operands)
	STACK[-1] = Op 
	STACK[-1] = -STACK[-1]
*/
xx(OP_NEG   , neg)

/*
*	Generic, type-agnostic comparison instruction that takes comparison type as argument.
*   The valid arguments for the instruction is as follow:
*	0: STACK[-1] == STACK[-2]
*	1: STACK[-1] != STACK[-2]
*	2: STACK[-1] < STACK[-2]
*	3: STACK[-1] <= STACK[-2]
*	4: STACK[-1] > STACK[-2]
*	5: STACK[-1] >= STACK[-2]
*
*/
yy(OP_CMP   , int32_t, i32, "%d", cmp, index)
/*
* If both operands is non-negative number or true, then it pushes a true (Numeric 1) value.
* STACK = STACK[-1] && STACK[-2]
*/
xx(OP_AND   , and)
/*
* If one of the operands is non-negative number or true, then it pushes a true (Numeric 1) value.
* STACK = STACK[-1] || STACK[-2]
*/
xx(OP_OR    , or)

/*
yy(OP_JMP8)
yy(OP_JMP16)
*/
/*
* JMP (Jumps to Relative Address. Displacement is an 32-bit signed integer, which means it can 
* jump back and forth.)
* 
* New IP = IP After Execution Of JMP + Displacement 
*/
yy(OP_JMP,		int32_t, i32, "%d", jmp,   disp)
/*
* JMPIF (Jumps to Relative Address and pops the operand at top if the operand at top is false. 
* Displacement is an 32-bit signed integer, which means it can jump back and forth.)
*
* if(STACK[-1] == False)
*	POP
*	New IP = IP After Execution Of JMP + Displacement
* end
*/
yy(OP_JMPIF,	int32_t, i32, "%d", jmpif, disp)
/*
* JMPIF (Jumps to Relative Address and pops the operand at top if the operand at top is true.
* Displacement is an 32-bit signed integer, which means it can jump back and forth.)
*
* if(STACK[-1] == True)
*	POP
*	New IP = IP After Execution Of JMP + Displacement
* end
*/
yy(OP_JMPIT,	int32_t, i32, "%d", jmpit, disp)

xx(OP_GITER,	giter)
yy(OP_FOR,      int32_t, i32, "%d", for, disp)

/*
*  NOP (No operation. Sometimes, it is used to align the code.)
*  VM just ignores it.
*/
xx(OP_NOP, nop)
// Halts the program immediately and hence VM itself.
xx(OP_HALT, halt)

//xx(OP_MAX, max)