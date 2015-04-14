/**
 * Document: MaxCompiler Tutorial (maxcompiler-tutorial)
 * Chapter: 3      Example: 1      Name: Moving Average Simple
 * MaxFile name: MovingAverageSimple
 * Summary:
 *   CPU code for the three point moving average design.
 */
#include "Maxfiles.h" 			// Includes .max files
#include <MaxSLiCInterface.h>	// Simple Live CPU interface

float dataIn[8] = { 1, 0, 2, 0, 4, 1, 8, 3 };
float dataOut[8];

typedef struct spread_s
{
    float spread;
    float leg_b;
    float leg_s;
} __attribute__ ((__packed__)) spread_t;

int main()
{
	printf("Running DFE\n");
	MovingAverageSimple(8, dataIn, dataOut);

	for (int i = 1; i < 7; i++) // Ignore edge values
		printf("dataOut[%d] = %f\n", i, dataOut[i]);

	return 0;
}
