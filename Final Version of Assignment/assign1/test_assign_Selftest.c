
//to do these tests it will have to be copied back into the storage_mgr.c code to function
//this function is to check some of the cases
int main()
{
	SM_FileHandle fh;
	SM_PageHandle ph;
	int a, i;
	ph = (SM_PageHandle)malloc(PAGE_SIZE);

	initStorageManager();

	a = createPageFile("Texting.txt");
	printf("RC: %d Create Page file\n", a);

	a = readBlock(0, &fh, ph);
	printf("RC: %d read block\n", a);

	a = getBlockPos(&fh);
	printf("RC: %d block Position\n", a);

	a = readFirstBlock(&fh, ph);
	printf("RC: %d read First block\n", a);

	a = readPreviousBlock(&fh, ph);
	printf("RC: %d read Previous block\n", a);

	a = readLastBlock(&fh, ph);
	printf("RC: %d read Last block\n", a);

}
