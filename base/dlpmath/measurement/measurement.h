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

/* data logging object */
typedef struct S {
	FILE *file;
	INT32 line;
	const char* file_path;
} dataLog;

/* Measurement method evaluates data
 *
 */
void data2csv_INT16(dataLog* log, const char* id, INT16* input, INT32 dim) {
	INT32 i;
	static INT32 local_cnt = 0;
	fprintf(log->file, "#<---I16_%s_%d--->\n", id, local_cnt++);
	for (i = 0; i < dim; i++) {
		fprintf(log->file, "%d,%5d\n", log->line++, input[i]);
	}
}

void data2csv_INT32(dataLog* log, const char* id, INT32* input, INT32 dim) {
	INT32 i;
	static INT32 local_cnt = 0;
	fprintf(log->file, "#<---I32_%s_%d--->\n", id, local_cnt++);
	for (i = 0; i < dim; i++) {
		fprintf(log->file, "%d,%10d\n", log->line++, input[i]);
	}
}

void data2csv_FLOAT64(dataLog*log, const char* id, FLOAT64* input, INT32 dim) {
	INT32 i;
	static INT32 local_cnt = 0;
	fprintf(log->file, "#<---F64_%s_%d--->\n", id, local_cnt++);
	for (i = 0; i < dim; i++) {
		fprintf(log->file, "%d,%.20E\n", log->line++, input[i]);
	}
}

void data2csv_init(dataLog* log) {
	log->file = fopen(log->file_path, "w");
	if (log->file == NULL) {
		printf("Error opening file!\n");
		exit(1);
	}
	log->line = 0;
//	fprintf(file, "sep=,\n");
//	fprintf(log->file, "line,data\n");
}

void data2csv_free(dataLog* log) {
	fclose(log->file);
}

#endif /* BASE_DLPMATH_MEASUREMENT_MEASUREMENT_C_ */
