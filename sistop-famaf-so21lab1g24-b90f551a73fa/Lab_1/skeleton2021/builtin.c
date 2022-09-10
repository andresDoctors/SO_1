#include <assert.h>
#include <errno.h>
#include <stdio.h>  // def NULL
#include <stdlib.h> // def EXIT_FAILURE
#include <string.h>
#include <unistd.h>

#include "builtin.h"
#include "tests/syscall_mock.h"


bool builtin_is_exit(pipeline pipe){
	assert(pipe != NULL);

	bool result = false;
	char* cmd = NULL;
	scommand sc = NULL;
	char str_exit[5] = "exit";

	if (! pipeline_is_empty(pipe)) {
		sc = pipeline_front(pipe);
		if (! scommand_is_empty(sc)) {
			cmd = scommand_front(sc);
			result = (strcmp(cmd, str_exit) == 0);
		}
	}
	
	return result;
}

bool builtin_is_cd(pipeline pipe){
	assert(pipe != NULL);

	bool result = false;
	char* cmd = NULL;
	scommand sc = NULL;
	char str_cd[3] = "cd";

	if (! pipeline_is_empty(pipe)) {
		sc = pipeline_front(pipe);
		if (! scommand_is_empty(sc)) {
			cmd = scommand_front(sc);
			result = (strcmp(cmd, str_cd) == 0);
		}
	}
	
	return result;
}

bool builtin_is_internal(pipeline pipe){
	return builtin_is_exit(pipe) || builtin_is_cd(pipe);
}

void builtin_exec(pipeline pipe){
	assert(builtin_is_internal(pipe));

	int sys_return;

	if (builtin_is_cd(pipe)){

		scommand sc = pipeline_front(pipe);
		scommand_pop_front(sc);

		char* direc;
		if (scommand_is_empty(sc)) {
			direc = "/home";
		}
		else {
			direc = scommand_front(sc);
		}
		
		sys_return = chdir(direc);
		if (sys_return == -1) {
			fprintf(stderr, "%s\n", strerror(errno));
		}
	}
	
	return;
}


