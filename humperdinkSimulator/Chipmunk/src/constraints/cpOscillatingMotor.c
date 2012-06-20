/* Copyright (c) 2007 Scott Lembcke
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdlib.h>
#include <math.h>

#include "chipmunk.h"
#include "constraints/util.h"

static void
preStep(cpOscillatingMotor *joint, cpFloat dt, cpFloat dt_inv)
{
	cpBody *a = joint->constraint.a;
	cpBody *b = joint->constraint.b;
	
	// calculate moment of inertia coefficient.
	joint->iSum = 1.0f/(a->i_inv + b->i_inv);
	
	
	// compute max impulse
	joint->jMax = J_MAX(joint, dt);

	// apply joint torque
	a->w -= joint->jAcc*a->i_inv;
	b->w += joint->jAcc*b->i_inv;
	
	// accumulate time
	joint->t += dt;
	
	// time modulo (2*pi)/frequency (avoid large t)
	if (joint->frequency == 0.0f) {
		joint->t = 0.0f;
	} else {
		cpFloat tLimit = (2.0*M_PI)/joint->frequency;
		if (joint->t > tLimit) joint->t -= tLimit;
	}
}

static void
applyImpulse(cpOscillatingMotor *joint)
{
	cpBody *a = joint->constraint.a;
	cpBody *b = joint->constraint.b;
	
	cpFloat rate;
	if (joint->amplitude == 0.0f || joint->frequency == 0.0f) {
		rate = 0.0f;
	} else {
		// compute rate according to time-step
		rate = joint->frequency * joint->amplitude * cos(joint->frequency * joint->t + joint->phaseShift);
	}
	
	
	// compute relative rotational velocity
	cpFloat wr = b->w - a->w + rate;
	
	// compute normal impulse	
	cpFloat j = -wr*joint->iSum;
	cpFloat jOld = joint->jAcc;
	joint->jAcc = cpfclamp(jOld + j, -joint->jMax, joint->jMax);
	j = joint->jAcc - jOld;
	
	// apply impulse
	a->w -= j*a->i_inv;
	b->w += j*b->i_inv;
}

static cpFloat
getImpulse(cpOscillatingMotor *joint)
{
	return cpfabs(joint->jAcc);
}

static const cpConstraintClass klass = {
	(cpConstraintPreStepFunction)preStep,
	(cpConstraintApplyImpulseFunction)applyImpulse,
	(cpConstraintGetImpulseFunction)getImpulse,
};
CP_DefineClassGetter(cpOscillatingMotor)

cpOscillatingMotor *
cpOscillatingMotorAlloc(void)
{
	return (cpOscillatingMotor *)cpmalloc(sizeof(cpOscillatingMotor));
}

cpOscillatingMotor *
cpOscillatingMotorInit(cpOscillatingMotor *joint, cpBody *a, cpBody *b, cpFloat frequency, cpFloat amplitude, cpFloat phaseShift)
{
	cpConstraintInit((cpConstraint *)joint, &klass, a, b);
	
	joint->frequency = frequency;
	joint->amplitude = amplitude;
	joint->phaseShift = phaseShift;
	joint->t = 0.0f;
	joint->jAcc = 0.0f;
	
	return joint;
}

cpConstraint *
cpOscillatingMotorNew(cpBody *a, cpBody *b, cpFloat frequency, cpFloat amplitude, cpFloat phaseShift)
{
	return (cpConstraint *)cpOscillatingMotorInit(cpOscillatingMotorAlloc(), a, b, frequency, amplitude, phaseShift);
}
