/*
 * measurement.c
 *
 * utility functions for measurements
 *
 *  Created on: 07.07.2015
 *      Author: arn85980
 */

#include "dlp_kernel.h"
#include "dlp_base.h"
#include "dlp_math.h"

//#ifndef BASE_DLPMATH_MEASUREMENT_MEASUREMENT_C_
//#define BASE_DLPMATH_MEASUREMENT_MEASUREMENT_C_

/*---------------------------------------------------------------------------*/

///* data logging object */
//typedef struct S {
//	FILE *file;
//	INT32 line;
//	const char* file_path;
//} dataLog;

const char* hashFilePath = "../../../dLabPro/last_hash.txt";

/* Measurement method evaluates data
 *
 */
void data2csv_INT16(DataLog* log, const char* id, INT16* input, INT32 dim) {
	INT32 i;
	fprintf(log->file, "#<---I16---%s--->\n", id);
	for (i = 0; i < dim; i++) {
		fprintf(log->file, "%d,%08x,%d\n", i, input[i], input[i]);
	}
}

/**
 * @param log - data logging instance of type dataLog
 * @param id  - string to identify
 */
void data2csv_INT32(DataLog* log, const char* id, INT32* input, INT32 dim) {
	INT32 i;
	fprintf(log->file, "#<---I32---%s--->\n", id);
	for (i = 0; i < dim; i++) {
		fprintf(log->file, "%d,%08x,%d\n", i, input[i], input[i]);
	}
}

void data2csv_FLOAT64(DataLog*log, const char* id, FLOAT64* input, INT32 dim) {
	INT32 i;
	fprintf(log->file, "#<---F64---%s--->\n", id);
	for (i = 0; i < dim; i++) {
		fprintf(log->file, "%d,%016llx,%.11g\n", i, input[i], input[i]);
	}
}

/**
 * Init function has to be called before using the logger
 */
void data2csv_init(DataLog* log) {
	FILE* hashFile = fopen(hashFilePath, "r");
	char hashString[8] = { "no_hash" };

	log->file = fopen(log->file_path, "w");
	if (log->file == NULL) {
		printf("Error opening logging file!\n");
		exit(1);
	}

	if(hashFile == NULL) {
		printf("Error opening hash file!\n");
	} else {
		fscanf(hashFile,"%s", hashString);
		fprintf(log->file, "### HASH=%s, FILE_PATH=%s ###\n", hashString, log->file_path);
		fclose(hashFile);
	}

//	log->line = 0;

//	fprintf(file, "sep=,\n");
//	fprintf(log->file, "line,data\n");
}

void data2csv_free(DataLog* log) {
	fclose(log->file);
}

//#endif /* BASE_DLPMATH_MEASUREMENT_MEASUREMENT_C_ */

