all: assignment1 run Selftest Selfrun

assignment1:
	gcc -o test_assign1 test_assign1_1.c dberror.c storage_mgr.c 

run: assignment1
	./test_assign1

Selftest:
	gcc -o test_assign2 test_additional.c dberror.c storage_mgr.c 

Selfrun: Selftest
	./test_assign2
