#ifndef COMPLEXNET_LINEFILE_H
#define COMPLEXNET_LINEFILE_H

//if define VEXTER_FILE_DIRECTION_LEFT, data file's column 2 point to column 1, direction is left.
//if not define, column 1 points column 2. that's normal.
#define VEXTER_FILE_DIRECTION_LEFT

//LINE_LENGTH_MAX is a serious constant, you should be sure a line's length not exceed this value.
#define LINE_LENGTH_MAX 10000

//LINES_LENGTH_EACH is the stepLength. 
//now it's 1e7, means, if a file contains less than 1e7 lines, malloc will be called only one time.
//if a file contans 1e8 lines, malloc will be called ten times.
//of course, if a file contains 1e8 lines, maybe you want to set LINES_LENGTH_EACH to 5e7 or 1e8. that's depend on you.
//you don't need to know the exactly line num of the file.
#define LINES_LENGTH_EACH 1000000

// for file containing one int in one line.
struct iLine {
	int i1;
};
struct iLineFile {
	int iMax;
	int iMin;
	long linesNum;
	struct iLine *lines;
};
void free_iLineFile(struct iLineFile *file);
struct iLineFile *create_iLineFile(const char * const filename);

//for file containing two int in one line.
struct iiLine {
	int i1;
	int i2;
};
struct iiLineFile {
	int iMax;
	int iMin;
	long linesNum;
	struct iiLine *lines;
};
void free_iiLineFile(struct iiLineFile *file);
struct iiLineFile *create_iiLineFile(const char * const filename);

//for file containing unlimited int in one line.
struct innLine{
	int lineId;
	int num;
	int *inn;
};
struct innLineFile {
	long linesNum;
	struct innLine *lines;
};
void free_innLineFile(struct innLineFile *file);
struct innLineFile *create_innLineFile(const char * const filename);

//for file containing four int in one line.
struct i4Line {
	int i1;
	int i2;
	int i3;
	int i4;
};
struct i4LineFile {
	int i1Max;
	int i1Min;
	int i2Max;
	int i2Min;
	long linesNum;
	struct i4Line *lines;
};
void free_i4LineFile(struct i4LineFile *file);
struct i4LineFile *create_i4LineFile(const char * const filename);
void print_i4LineFile(struct i4LineFile *file, char *filename);
void print_2_i4LineFile(struct i4LineFile *file1, struct i4LineFile *file2, char *filename);
struct i4LineFile *divide_i4LineFile(struct i4LineFile *file, double rate);


//for file containing int/double/int 
struct idiLine{
	int i1;
	double d2;
	int i3;
};
struct idiLineFile {
	int iMax;
	int iMin;
	long linesNum;
	struct idiLine *lines;
};
void free_idiLineFile(struct idiLineFile *file);
struct idiLineFile *create_idiLineFile(const char *const filename);

//for file containing 5 int, char *, double
struct i5sdLine {
	int i1;
	int i2;
	int i3;
	int i4;
	int i5;
	char s6[1000];
	double  d7;
};
struct i5sdLineFile {
	int iMax;
	int iMin;
	long linesNum;
	struct i5sdLine *lines;
};
void free_i5sdLineFile(struct i5sdLineFile *file);
struct i5sdLineFile *create_i5sdLineFile(const char *const filename);

//for file containing int,int, double,int
struct iidiLine{
	int i1;
	int i2;
	double d3;
	int i4;
};
struct iidiLineFile {
	int iMax;
	int iMin;
	long linesNum;
	struct iidiLine *lines;
};
void free_iidiLineFile(struct iidiLineFile *file);
struct iidiLineFile *create_iidiLineFile(const char *const filename);

//for file containing 5 int, char *, char *
struct i5ssLine {
	int i1;
	int i2;
	int i3;
	int i4;
	int i5;
	char s6[100];
	char s7[1000];
};
struct i5ssLineFile {
	int iMax;
	int iMin;
	long linesNum;
	struct i5ssLine *lines;
};
void free_i5ssLineFile(struct i5ssLineFile *file);
struct i5ssLineFile *create_i5ssLineFile(const char *const filename);

//for file containing int/int/int/int/int/int/int/double/double
struct i7ddsLine {
	int i1;
	int i2;
	int i3;
	int i4;
	int i5;
	int i6;
	int i7;
	double d8;
	double d9;
	char s10[100];
};
struct i7ddsLineFile {
	int iMax;
	int iMin;
	long linesNum;
	struct i7ddsLine *lines;
};
void free_i7ddsLineFile(struct i7ddsLineFile *file);
struct i7ddsLineFile *create_i7ddsLineFile(const char *const filename);

//for file containing int/int/double/double/double
struct iid3Line {
	int i1;
	int i2;
	double d3;
	double d4;
	double d5;
};
struct iid3LineFile {
	int iMax;
	int iMin;
	long linesNum;
	struct iid3Line *lines;
};
void free_iid3LineFile(struct iid3LineFile *file);
struct iid3LineFile *create_iid3LineFile(const char * const filename);

//for file containing int/int/int
struct i3Line{
	int i1;
	int i2;
	int i3;
};
struct i3LineFile{
	int iMax;
	int iMin;
	long linesNum;
	struct i3Line *lines;
};
void free_i3LineFile(struct i3LineFile *file);
struct i3LineFile *create_i3LineFile(const char * const filename);

#endif