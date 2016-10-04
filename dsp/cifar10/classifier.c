/*
 * classifier.c
 *
 *  Created on: Dec 24, 2015
 *      Author: nachi
 */

#include <DSPF_sp_maxloc.h>
#include "DSPF_sp_dotprod.h"
#include "DSPF_sp_vec.h"

extern float* pHiddenNeuron;
extern float* pOutputNeuron;

void dummy_classifier(float *pInputNeurons, int nInputNeurons, int nHiddenNeurons, int nOutputNeurons, float *pInputWt, float *pHiddenBias, float *pOutputWt)
{

	int n, label;

	// Input layer computations
	for (n = 0; n < nHiddenNeurons; n++)
	{
		pHiddenNeuron[n] = DSPF_sp_dotprod(pInputNeurons,	pInputWt + n * nInputNeurons, nInputNeurons);//dot_prod(pInputNeurons,	pInputWt + n * nInputNeurons, nInputNeurons);
	}
	DSPF_sp_vec(pHiddenNeuron, pHiddenBias,pHiddenNeuron, nHiddenNeurons);

	// Nonlinear activation
	for ( n = 0; n < nHiddenNeurons; n++)
	{
		if(pHiddenNeuron[n] < 0)
			pHiddenNeuron[n] = 0;
	}

	// Output layer
	for ( n = 0; n < nOutputNeurons; n++)
	{
		pOutputNeuron[n] = DSPF_sp_dotprod(pHiddenNeuron, pOutputWt + n * nHiddenNeurons, nHiddenNeurons); //dot_prod(pHiddenNeuron, pOutputWt + n * nHiddenNeurons, nHiddenNeurons);
	}
	label = DSPF_sp_maxloc(pOutputNeuron,nOutputNeurons);
}


