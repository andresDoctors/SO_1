#include <assert.h> // agregado por nosotros
#include <glib.h>
#include <stdio.h>

#include "command.h"
#include "strextra.h"

/* Memory leaks:
 * Las únicas funciones donde reservamos memoria son:
 * scommand_new, pipeline_new, scommand_to_string y pipeline_to_string */

/********** COMANDO SIMPLE **********/

/* Estructura correspondiente a un comando simple.
 * Es una 3-upla del tipo ([char*], char* , char*).
 */

struct scommand_s {
    GSList *args;
    char * redir_in;
    char * redir_out;
};

/* Editado por las especificaciones dadas en command.h
 * implementacion anterior: return NULL; */
scommand scommand_new(void){
    scommand result = (scommand) malloc(sizeof(struct scommand_s));

	/* No existe una funcion para crear un GSList
	 * NULL es considerada la lista vacia */
	result->args = NULL;
	result->redir_in  = NULL;
	result->redir_out = NULL;

	return result;
}

scommand scommand_destroy(scommand self){
	assert(self != NULL);
	
	g_slist_free_full(self->args, free); self->args = NULL;

	free(self->redir_in);  self->redir_in  = NULL; 
	free(self->redir_out); self->redir_out = NULL;

	free(self); self = NULL;

	return NULL;
}

void scommand_push_back(scommand self, char * argument){
	assert(self != NULL);
	assert(argument != NULL);

	self->args = g_slist_append(self->args, argument);

	return;
}

void scommand_pop_front(scommand self){
	assert(self != NULL);
	assert(! scommand_is_empty(self));

	gpointer head = NULL;

	head = g_slist_nth_data(self->args, 0u); 
	self->args = g_slist_remove(self->args, head); // Nota: remove NO libera memoria

	free(head); head = NULL;

	return;
}

void scommand_set_redir_in(scommand self, char * filename){
	assert(self != NULL);

	if (self->redir_in != NULL) {
		free(self->redir_in); self->redir_in = NULL;
	}
	self->redir_in = filename;
	
	return;
}

void scommand_set_redir_out(scommand self, char * filename){
	assert(self != NULL);

	if (self->redir_out != NULL) {
		free(self->redir_out); self->redir_out = NULL;
	}
	self->redir_out = filename;

	return;
}

bool scommand_is_empty(const scommand self){
	assert(self != NULL);

	return (self->args == NULL);
	/* Tal vez se deberia reemplazar por:
	 * (self->args == NULL) && (self->redir_in == NULL) && (self->redir_out == NULL) */
}

unsigned int scommand_length(const scommand self){
	assert(self != NULL);

	/* Devuelve la cantidad de strings en la lista self->args. 
	 * Tal vez deberian contarse la cantidad de chars en cada string
	 * y/o considerarse redir_in , redir_out */
	return g_slist_length(self->args);
}

char * scommand_front(const scommand self){
	assert(self != NULL);
	assert(! scommand_is_empty(self));

	char *result = NULL;
	result = g_slist_nth_data(self->args, 0u);

	return result;
}

char * scommand_get_redir_in(const scommand self){
	assert(self != NULL);

	return self->redir_in;
}

char * scommand_get_redir_out(const scommand self){
	assert(self != NULL);

	return self->redir_out;
}

/*
// info sobre strings en C: https://www.tutorialspoint.com/cprogramming/c_strings.htm
char * scommand_to_string(const scommand self){
	assert(self != NULL);

	 Primero calculamos el largo de cada string en 'self' (incluyendo los que estan dentro de la lista)
	unsigned int len_in = 0u;
	if (self->redir_in != NULL) {
		len_in = strlen(self->redir_in) + 3u;   // +3 porque dejamos espacio para " < "
	}

	unsigned int len_out = 0u;
	if (self->redir_out != NULL) {
		len_out = strlen(self->redir_out) + 3u; // +3 porque dejamos espacio para " > "
	}

	char *string_aux1;
	unsigned int len_args = 0u;
	for (unsigned int i = 0u; i < g_slist_length(self->args); i++) { // para cada elemento de la lista...

		string_aux1 = g_slist_nth_data(self->args, i);  // tomamos el string
		len_args += strlen(string_aux1);                // calculamos su largo y se lo añadimos a 'len_args'
		
		if (i != g_slist_length(self->args) - 1u) { // para todos menos el ultimo elemento...
			len_args += 1u;                         // agregamos 1 al largo para el espacio en blanco
		}
	}
	

	 Declaramos nuestro string y le asignamos espacio en el heap 
	char * result = calloc(len_args + len_out + len_in + 1u, sizeof(char));
	 Nota: el string tiene +1 de largo porque debe terminar en el char nulo '\0' 

	result[0] = '\0'; // vaciamos el string

	char space[2] = " ";
	for (unsigned int i = 0u; i < g_slist_length(self->args); i++) { // para cada elemento de la lista...

		string_aux1 = g_slist_nth_data(self->args, i); // tomamos el string
		result = strcat(result, string_aux1);          // concatenamos

		if(i != g_slist_length(self->args) - 1u){ // para todo elemento menos el ultimo...
			result = strcat(result, space);       // agregamos un espacio en blanco
		}
	}


	char string_aux2[4] = " c ";
	if (len_in > 0u) {
		string_aux2[1] = '<'; // string_aux2 == " < "

		result = strcat(result, string_aux2);
		result = strcat(result, self->redir_in);
	}
	if (len_out > 0u) {
		string_aux2[1] = '>'; // string_aux2 == " > "

		result = strcat(result, string_aux2);     
		result = strcat(result, self->redir_out); 
	}


	return result;
}
*/

char * scommand_to_string(const scommand self){
	assert(self != NULL);

	char* result  = strdup("");

	char* result_aux = result;


	char* space = strdup(" ");
	char* redir_in  = strdup(" < ");
	char* redir_out = strdup(" > ");

	unsigned int len_self = scommand_length(self);
	for (unsigned int i = 0u; i < len_self; i++) {
		result = strmerge(result, (char*) g_slist_nth_data(self->args, i));
		free(result_aux); result_aux = result;

		if (i != len_self - 1u) {
			result = strmerge(result, space);
			free(result_aux); result_aux = result;
		}
	}
		
	if (self->redir_in != NULL) {
		result = strmerge(result, redir_in);
		free(result_aux); result_aux = result;

		result = strmerge(result, self->redir_in);
		free(result_aux); result_aux = result;
	}
	
	if (self->redir_out != NULL) {
		result = strmerge(result, redir_out);
		free(result_aux); result_aux = result;

		result = strmerge(result, self->redir_out);
		free(result_aux); result_aux = result;
	}

	free(space); space = NULL;
	free(redir_in); redir_in = NULL;
	free(redir_out); redir_out = NULL;

	return result;
}


/********** COMANDO PIPELINE **********/

/* Estructura correspondiente a un comando pipeline.
 * Es un 2-upla del tipo ([scommand], bool)
 */

struct pipeline_s {
    GSList *scmds;
    bool wait;
};



pipeline pipeline_new(void){
	pipeline result = (pipeline) malloc(sizeof(struct pipeline_s));
	result->scmds = NULL; result->wait = true;
	return result;
}

/* Para que hagan match los tipos en la proxima funcion */
static void scommand_destroy_void(gpointer self){	
	self = (gpointer) scommand_destroy((scommand) self);
	return;
}

pipeline pipeline_destroy(pipeline self){
	assert(self != NULL);

	g_slist_free_full(self->scmds, scommand_destroy_void); self->scmds = NULL; // destruye la lista

	free(self); self = NULL; // destruye la estructura

	return NULL;
}

void pipeline_push_back(pipeline self, scommand sc){
	assert(self != NULL);
	assert(sc != NULL);

	self->scmds = g_slist_append(self->scmds, sc);
	return;
}

void pipeline_pop_front(pipeline self){
	assert(self != NULL);
	assert(! pipeline_is_empty(self));

	gpointer head = NULL;

	head = g_slist_nth_data(self->scmds, 0u); 
	self->scmds = g_slist_remove(self->scmds, head); 

	head = scommand_destroy((scommand) head);

	return;
}

void pipeline_set_wait(pipeline self, const bool w){
	assert(self != NULL);

	self->wait = w;
	return;
}

bool pipeline_is_empty(const pipeline self){
	assert(self != NULL);

	return self->scmds == NULL;
}

unsigned int pipeline_length(const pipeline self){
	assert(self != NULL);

	return (unsigned int) g_slist_length(self->scmds);
}

scommand pipeline_front(const pipeline self){
	assert(self != NULL);
	assert(! pipeline_is_empty(self));

	scommand head = g_slist_nth_data(self->scmds, 0u); 
	return head;
}

bool pipeline_get_wait(const pipeline self){
	assert(self != NULL);

	return self->wait;
}

/*
char * pipeline_to_string(const pipeline self){
	assert(self != NULL);

	unsigned int len_string = 0u;
	scommand cd = NULL;

	for (unsigned int i = 0u; i < pipeline_length(self); i++) {// para cada elemento de la lista self->scmds

		cd = g_slist_nth_data(self->scmds, i);  // tomamos el comando
		len_string += scommand_length(cd);      // le agregamos su largo a len_string

		if (i != pipeline_length(self) - 1u) { // para todos menos el ultimo...
			len_string += 3u;                  // agregamos espacio para " | "
		}

		else {                    // para el ultimo...
			if (! self->wait) {   // si se ejecuta en el background...
				len_string += 2u; // agregamos espacio para " &"
			}			
		}
	}

	char *result = (char*) calloc(len_string + 1u, sizeof(char)); // reservamos espacio para nuestro string resultado
	result[0] = '\0'; // vaciamos el string

	char pipe[4] = " | ";
	char amperson[3] = " &";
	char *cd_string = NULL;
	for (unsigned int i = 0u; i < pipeline_length(self); i++) { // para cada elemento en la lista...

		cd = g_slist_nth_data(self->scmds, i); // tomamos el comando
		cd_string = scommand_to_string(cd);    // lo pasamos a un string
		
		result = strcat(result, cd_string);  // se lo concatenamos a result

		 Liberamos cd_string 
		 Causa errores MUY RAROS par test_pipeline_to_string 
		free(cd_string); cd_string = NULL;
		

		if (i != pipeline_length(self) - 1u) { // para todos menos el ultimo...
			result = strcat(result, pipe);     // agregamos " | "
		}

		else { // para el ultimo...

			if (! self->wait) {                    // si se ejecuta en el background...
				result = strcat(result, amperson); // agregamos " &"
			}
		}
	}

	return result;
}
*/

char * pipeline_to_string(const pipeline self) {
	assert(self != NULL);

	char* result = strdup("");
	char* result_aux = result;

	char* sc_string;
	char* pipe = strdup(" | ");
	char* amperson = strdup(" &");
	unsigned int len_self = pipeline_length(self);
	for (unsigned int i = 0u; i < len_self; i++) {
		sc_string = scommand_to_string((scommand) g_slist_nth_data(self->scmds, i));
		result = strmerge(result, sc_string);

		free(result_aux); result_aux = result;
		free(sc_string); sc_string = NULL;

		if (i != len_self - 1u) {
			result = strmerge(result, pipe);
			free(result_aux); result_aux = result;
		}
		else {
			if (! self->wait) {
				result = strmerge(result, amperson);
				free(result_aux); result_aux = result;
			}
		}
	}
			
	free(pipe); pipe = NULL;
	free(amperson); amperson = NULL;

	return result;
}