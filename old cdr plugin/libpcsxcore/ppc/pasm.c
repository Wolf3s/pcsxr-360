//#include "PsxCommon.h"
#include "ppc.h"
/**
Lorsque le compiler copile une function il ajoute un prologue pour securité
naked permet de gerer le prologue soit meme
mci [cOz]
**/

/*
Register R3-R10: Parameters

Register R3-R4: Results 
*/
void __declspec(naked) recRun(register void (*func)(), register u32 hw1, register u32 hw2)
{
	__asm{
		/* prologue code */
		mflr	r0
		stmw	r14, -72(r1) // -(32-14)*4
		stw		r0, 4(r1)
		stwu	r1, -80(r1) // -((32-14)*4+8)

		/* execute code */
		mtctr	r3			// load Count Register with address of func
		mr		r31, r4
		mr		r30, r5
		bctrl				//branch to contents of Count Register
	}
}

void __declspec(naked) returnPC()
{
	__asm{
		lwz		r0, 84(r1) // (32-14)*4+8+4
		addi	r1, r1, 80 //(32-14)*4+8
		mtlr	r0
		lmw		r14, -72(r1) // -(32-14)*4
		blr
	}
}
