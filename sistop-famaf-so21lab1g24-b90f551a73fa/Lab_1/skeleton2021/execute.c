#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "builtin.h"
#include "command.h"
#include "execute.h"
#include "tests/syscall_mock.h"


static void execute_scommand(pipeline apipe, Parser parser) {
    assert(apipe != NULL);
    assert(! pipeline_is_empty(apipe));

    int sys_return;
    scommand sc = pipeline_front(apipe);
    char* str_in = scommand_get_redir_in(sc);
    char* str_out = scommand_get_redir_out(sc);

    if (str_in != NULL) {
        int fd_in = open(str_in, O_RDONLY, 0);
        if (fd_in < 0) {
            fprintf(stderr, "%s\n", strerror(errno));
            return;
        }

        sys_return = dup2(fd_in, STDIN_FILENO);
        if (sys_return == -1) {
            fprintf(stderr, "%s\n", strerror(errno));
            apipe = pipeline_destroy(apipe);
            if (parser != NULL) {
                parser = parser_destroy(parser);
            }
            exit(EXIT_FAILURE);
        }

        sys_return = close(fd_in);
        if (sys_return == -1) {
            fprintf(stderr, "%s\n", strerror(errno));
            apipe = pipeline_destroy(apipe);
            if (parser != NULL) {
                parser = parser_destroy(parser);
            }
            exit(EXIT_FAILURE);
        }
    }

    if (str_out != NULL) {
        int fd_out = open(str_out, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
        if (fd_out < 0) {
            fprintf(stderr, "%s\n", strerror(errno));
            return;
        }

        sys_return = dup2(fd_out, STDOUT_FILENO);
        if (sys_return == -1) {
            fprintf(stderr, "%s\n", strerror(errno));
            apipe = pipeline_destroy(apipe);
            if (parser != NULL) {
                parser = parser_destroy(parser);
            }
            exit(EXIT_FAILURE);
        }

        sys_return = close(fd_out);
        if (sys_return == -1) {
            fprintf(stderr, "%s\n", strerror(errno));
            apipe = pipeline_destroy(apipe);
            if (parser != NULL) {
                parser = parser_destroy(parser);
            }
            exit(EXIT_FAILURE);
        }
    }

    unsigned int len_sc = scommand_length(sc);

    /*char* args_sc[len_sc + 1];*/
    char** args_sc = (char**) calloc(len_sc + 1u, sizeof(char*));

    char* arg;
    for (unsigned int i = 0u; i < len_sc; i++) {
        arg = scommand_front(sc);
        args_sc[i] = (char*) calloc(strlen(arg) + 1u, sizeof(char));
        args_sc[i] = strcpy(args_sc[i], arg);
        scommand_pop_front(sc);
    }
    args_sc[len_sc] = NULL;

    char* cmd = args_sc[0];
    sys_return = execvp(cmd, args_sc);
    if (sys_return == -1) {
        fprintf(stderr, "%s\n", strerror(errno));
    }   

    for (unsigned int i = 0u; i < len_sc; i++) {
        free(args_sc[i]); args_sc[i] = NULL;
    }
    free(args_sc); args_sc = NULL;
    
    
    return;
}

void _execute_pipeline(pipeline apipe, Parser parser) {
    assert(apipe != NULL);
    
    bool wait_apipe = pipeline_get_wait(apipe);
    unsigned int len_apipe = pipeline_length(apipe);
    int sys_return;
    pid_t pid_sys_return;
    pid_t* pid = (pid_t*) calloc(len_apipe, sizeof(pid_t));

    if (len_apipe > 1u) {
        
        /* int array_pipes[(len_apipe - 1) * 2]; */
        int* array_pipes = (int*) calloc((len_apipe - 1u) * 2u, sizeof(int));

        /* Crea las puntas de escritura */
        for (unsigned int i = 0u; i < len_apipe - 1u; i++) {
            sys_return = pipe(array_pipes + 2u * i);
            if (sys_return == -1) {
                fprintf(stderr, "%s\n", strerror(errno));
                free(array_pipes); array_pipes = NULL;
                free(pid); pid = NULL;
                return;
            }
        }
        
        /* Ciclo gordo */
        for (unsigned int i = 0u; i < len_apipe; i++) {
            pid[i] = fork();
            if (pid[i] < 0) {
                fprintf(stderr, "%s\n", strerror(errno));
                free(array_pipes); array_pipes = NULL;
                free(pid); pid = NULL;
                apipe = pipeline_destroy(apipe);
                if (parser != NULL) {
                    parser = parser_destroy(parser);
                }
                exit(EXIT_FAILURE);
            }

            else if (pid[i] == 0) {
                
                /* Redireccionamos los pipes */
                if (i != 0u) {
                    sys_return = dup2(array_pipes[2u * i - 2u], STDIN_FILENO);
                    if (sys_return == -1) {
                        fprintf(stderr, "%s\n", strerror(errno));
                        free(pid); pid = NULL;
                        free(array_pipes); array_pipes = NULL;
                        apipe = pipeline_destroy(apipe);
                        if (parser != NULL) {
                            parser = parser_destroy(parser);
                        }
                        exit(EXIT_FAILURE);
                    }
                }
                
                if (i != len_apipe - 1u) {
                    sys_return = dup2(array_pipes[2u * i + 1u], STDOUT_FILENO);
                    if (sys_return == -1) {
                        fprintf(stderr, "%s\n", strerror(errno));
                        free(pid); pid = NULL;
                        free(array_pipes); array_pipes = NULL;
                        apipe = pipeline_destroy(apipe);
                        if (parser != NULL) {
                            parser = parser_destroy(parser);
                        }
                        exit(EXIT_FAILURE);
                    }
                }
                
                /* Cerramos TODAS las puntas de escritura */
                for (unsigned int j = 0u; j < (len_apipe - 1u) * 2u; j++) {
                    sys_return = close(array_pipes[j]);
                    if (sys_return == -1) {
                        fprintf(stderr, "%s\n", strerror(errno));
                        free(pid); pid = NULL;
                        free(array_pipes); array_pipes = NULL;
                        apipe = pipeline_destroy(apipe);
                        if (parser != NULL) {
                            parser = parser_destroy(parser);
                        }
                        exit(EXIT_FAILURE);
                    }
                }

                /* Ejecutamos el comando al frente de apipe */
                if (! builtin_is_internal(apipe)) {
                    execute_scommand(apipe, parser);
                }

                free(pid); pid = NULL;
                free(array_pipes); array_pipes = NULL;
                apipe = pipeline_destroy(apipe);
                if (parser != NULL) {
                    parser = parser_destroy(parser);
                }

                exit(EXIT_SUCCESS);
            }

            else {
                /* Quitamos el comando ejecutado de nuestra pipeline */
                pipeline_pop_front(apipe);
            }
        }

        /* Cerramos TODAS las puntas de escritura */
        for (unsigned int i = 0u; i < (len_apipe - 1u) * 2u; i++) {
            sys_return = close(array_pipes[i]);
            if (sys_return == -1) {
                fprintf(stderr, "%s\n", strerror(errno));
                free(array_pipes); array_pipes = NULL;
                free(pid); pid = NULL;
                return;
            }
        }

        // Wait
        if (wait_apipe) {
            for (unsigned int i = 0u; i < len_apipe; i++) {
                pid_sys_return = waitpid(pid[i], NULL, 0); // Nota: la entrada del final puede ser cualquier int
                if (pid_sys_return == (pid_t) -1) {
                    fprintf(stderr, "%s\n", strerror(errno));
                    free(array_pipes); array_pipes = NULL;
                    free(pid); pid = NULL;
                    return;
                }
            }
        }
        
        free(pid); pid = NULL;
        free(array_pipes); array_pipes = NULL;
    }   

    else if (len_apipe == 1u) {

        if (builtin_is_internal(apipe)) {
            builtin_exec(apipe);
        }

        else {
            pid[0] = fork();
            if (pid[0] < 0) {
                fprintf(stderr, "%s\n", strerror(errno));
                free(pid); pid = NULL;
                apipe = pipeline_destroy(apipe);
                if (parser != NULL) {
                    parser = parser_destroy(parser);
                }
                exit(EXIT_FAILURE);
            }
            
            else if (pid[0] == 0) {
                execute_scommand(apipe, parser);
                free(pid); pid = NULL;
                apipe = pipeline_destroy(apipe);
                if (parser != NULL) {
                    parser = parser_destroy(parser);
                }
                exit(EXIT_SUCCESS);
            }
            
            if (wait_apipe) {
                pid_sys_return = wait(NULL);
                if (pid_sys_return == (pid_t) -1) {
                    fprintf(stderr, "%s\n", strerror(errno));
                    free(pid); pid = NULL;
                    return;
                }
            }
        }

        free(pid); pid = NULL;
    }

    return;
}

void execute_pipeline(pipeline apipe) {
    assert(apipe != NULL);
    _execute_pipeline(apipe, NULL);
    return;
}