/*
 * measurement.h
 *
 * utility functions for measurements
 *
 *  Created on: 07.07.2015
 *      Author: arn85980
 */

#ifndef BASE_DLPMATH_MEASUREMENT_MEASUREMENT_C_
#define BASE_DLPMATH_MEASUREMENT_MEASUREMENT_C_

/*---------------------------------------------------------------------------*/

FILE *file;
static INT32 line = 0;

/* Measurement method evaluates data
 *
 */
void data_evaluation_INT16(const char* id, INT16* input, INT32 dim) {
	INT32 i;
	static INT32 local_cnt = 0;
	fprintf(file, "#<---I16_%s_%d--->\n", id, local_cnt++);
	for (i = 0; i < dim; i++) {
		fprintf(file, "%d,%5d\n", line++, input[i]);
	}
}

void data_evaluation_INT32(const char* id, INT32* input, INT32 dim) {
	INT32 i;
	static INT32 local_cnt = 0;
	fprintf(file, "#<---I32_%s_%d--->\n", id, local_cnt++);
	for (i = 0; i < dim; i++) {
		fprintf(file, "%d,%10d\n", line++, input[i]);
	}
}

void data_evaluation_FLOAT64(const char* id, FLOAT64* input, INT32 dim) {
	INT32 i;
	static INT32 local_cnt = 0;
	fprintf(file, "#<---F64_%s_%d--->\n", id, local_cnt++);
	for (i = 0; i < dim; i++) {
		fprintf(file, "%d,%.20E\n", line++, input[i]);
	}
}

void data_evaluation_init(const char* file_path) {
	file = fopen(file_path, "w");
	if (file == NULL) {
		printf("Error opening file!\n");
		exit(1);
	}
	line = 0;
//	fprintf(file, "sep=,\n");
	fprintf(file, "line,data\n");
}

void data_evaluation_free() {
	fclose(file);
}

#endif /* BASE_DLPMATH_MEASUREMENT_MEASUREMENT_C_ */
