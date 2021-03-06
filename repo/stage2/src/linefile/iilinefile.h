#ifndef CN_IILINEFILE_H
#define CN_IILINEFILE_H

//for file containing two int in one line.
struct iiLine {
	int i1;
	int i2;
};
struct iiLineFile {
	int i1Max;
	int i1Min;
	int i2Max;
	int i2Min;
	long linesNum;
	struct iiLine *lines;
};
void free_iiLineFile(struct iiLineFile *file);
struct iiLineFile *create_iiLineFile(const char * const filename);
void print_iiLineFile(struct iiLineFile *file, char *filename);


void divide_iiLineFile(struct iiLineFile *file, double rate, struct iiLineFile **first, struct iiLineFile **second);
void free_2_iiLineFile(struct iiLineFile *twofile);
//void print_2_iiLineFile(struct iiLineFile *file1, struct iiLineFile *file2, char *filename);

#endif
