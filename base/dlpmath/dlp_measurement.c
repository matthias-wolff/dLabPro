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

/*---------------------------------------------------------------------------*/

///* data logging object */
//typedef struct S {
//	FILE *file;
//	INT32 line;
//	const char* file_path;
//} dataLog;

/**
 * optional path to a text file containing the hash of the last commit
 */
const char* hashFilePath = "../../../dLabPro/last_hash.txt";

/**
 * Data logging function for datatype INT16.
 *
 * @param log	- data logging instance of type dataLog
 * @param id	- string to identify the measurement
 * @param input	- pointer to the input data
 * @param dim	- size of the input data array
 */
void data2csv_INT16(DataLog* log, const char* id, INT16* input, INT32 dim) {
	INT32 i;
	fprintf(log->file, "#<---I16---%s--->\n", id);
	for (i = 0; i < dim; i++) {
		fprintf(log->file, "%d,%08x,%d\n", i, input[i], input[i]);
	}
}

/**
 * Data logging function for datatype INT32.
 *
 * @param log	- data logging instance of type dataLog
 * @param id	- string to identify the measurement
 * @param input	- pointer to the input data
 * @param dim	- size of the input data array
 */
void data2csv_INT32(DataLog* log, const char* id, INT32* input, INT32 dim) {
	INT32 i;
	fprintf(log->file, "#<---I32---%s--->\n", id);
	for (i = 0; i < dim; i++) {
		fprintf(log->file, "%d,%08x,%d\n", i, input[i], input[i]);
	}
}

/**
 * Data logging function for datatype FLOAT64.
 *
 * @param log	- data logging instance of type dataLog
 * @param id	- string to identify the measurement
 * @param input	- pointer to the input data
 * @param dim	- size of the input data array
 */
void data2csv_FLOAT64(DataLog*log, const char* id, FLOAT64* input, INT32 dim) {
	INT32 i;
	fprintf(log->file, "#<---F64---%s--->\n", id);
	for (i = 0; i < dim; i++) {
		fprintf(log->file, "%d,%016llx,%.11g\n", i, input[i], input[i]);
	}
}

/**
 * This Init function has to be called before using the logger.
 *
 * @param log	- data logging instance of type dataLog
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
//		printf("Error opening hash file!\n");
	} else {
		fscanf(hashFile,"%s", hashString);
		fclose(hashFile);
	}
	fprintf(log->file, "### HASH=%s, FILE_PATH=%s ###\n", hashString, log->file_path);

//	log->line = 0;

//	fprintf(file, "sep=,\n");
//	fprintf(log->file, "line,data\n");
}

/**
 * Call this function after using the data logger to free allocated memory and
 * to close the logging file.
 *
 * @param log	- data logging instance of type dataLog
 */
void data2csv_free(DataLog* log) {
	fclose(log->file);
}

