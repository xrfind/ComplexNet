#include "../inc/complexnet_file.h"

//basically, for different line styled file, I only need to change this function and struct LineInfo.
void fillLineInfo(char *line, struct LineInfo *LI, int *vtMaxId, int *vtMinId)
{

	//divide line to parts.
	char *delimiter="\t, ";
	char *partsLine[2];
	partsLine[0]=strtok(line, delimiter);
	assert(partsLine[0]!=NULL);
	partsLine[1]=strtok(NULL, delimiter);
	assert(partsLine[1]!=NULL);

	//transfor parts to num(double or int);
	char *pEnd;
	LI->vt1Id=strtol(partsLine[0], &pEnd, 10);
	assert(pEnd[0]=='\0');
	LI->vt2Id=strtol(partsLine[1], &pEnd, 10);
	assert(pEnd[1]=='\0');

	//max/min Id
	if (LI->vt1Id>LI->vt2Id) {
		*vtMaxId=(*vtMaxId)>LI->vt1Id?(*vtMaxId):LI->vt1Id;
		*vtMinId=(*vtMinId)<LI->vt2Id?(*vtMinId):LI->vt2Id;
	} else {
		*vtMaxId=(*vtMaxId)>LI->vt2Id?(*vtMaxId):LI->vt2Id;
		*vtMinId=(*vtMinId)<LI->vt1Id?(*vtMinId):LI->vt1Id;
	}

}

//if data is stored in each line and each line contain only num & delimiter, there is no need to change this function.
struct NetFile *readFileLBL(char *filename)
{
	//open file
	FILE *fp=fopen(filename,"r");
	fileError(fp, filename);

	struct LineInfo *LinesInfo=NULL;
	LinesInfo=malloc(LINES_LENGTH_EACH*sizeof(struct LineInfo));
	assert(LinesInfo!=NULL);

	int lineNum=0;
	int maxId=INT_MAX;
	int minId=0;

	char line[LINE_LENGTH_MAX];
	int each=1;
	while(fgets(line, LINE_LENGTH_MAX, fp)) {
		if (lineNum<LINES_LENGTH_EACH) {
			fillLineInfo(line, LinesInfo+lineNum+(each-1)*LINES_LENGTH_EACH, &maxId, &minId);
			++lineNum;
		} else {
			++each;
			printf("read file %s lines: %d\r", filename, (each-1)*LINES_LENGTH_EACH); fflush(stdout);
			struct LineInfo *temp=realloc(LinesInfo, each*LINES_LENGTH_EACH*sizeof(struct LineInfo));
			assert(temp!=NULL);
			LinesInfo=temp;
			lineNum=0;
			fillLineInfo(line, LinesInfo+lineNum+(each-1)*LINES_LENGTH_EACH, &maxId, &minId);
			++lineNum;
		}
	}
	lineNum+=(each-1)*LINES_LENGTH_EACH;
	printf("read file %s lines: %ld, Max: %ld, Min: %ld\n", filename, lineNum, maxId, minId); fflush(stdout);
	fclose(fp);

	struct NetFile *file=malloc(sizeof(struct NetFile));
	assert(file!=NULL);
	file->minId=minId;
	file->maxId=maxId;
	file->lines=LinesInfo;
	file->linesNum=lineNum;

	return file;
}
