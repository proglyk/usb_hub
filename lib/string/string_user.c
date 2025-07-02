#include "string_user.h"

char *
	strcpy(char* dest, char const* src) {
	
	char *tmp = dest;
	
	if (!dest)
		return 0;
	if (!src)
		return 0;
	
	while ((*dest++ = *src++) != '\0');
	
	return tmp;
	//return 0;
}


char *
	strcat(char* dest,  char const* src) {
	
	char *tmp = dest;
		
	if (!dest)
		return 0;
	if (!src)
		return 0;	

	while (*dest)	dest++;
	while ((*dest++ = *src++) != '\0');

	return tmp;
	//return 0;
}


//------------------------------------------------------------------------------
/// Функция сравнивает в лексикографическом порядке две строки и возвращает 
///	целое значение, зависящее следующим образом от результата сравнения.
/// Return 0 if equals
/// Return >0 if cs > ct
/// Return <0 if cs < ct
/// \param cs Pointer to the start of the source string.
/// \param ct Pointer to the start of the target string.
//------------------------------------------------------------------------------
int
	strcmp(char const *cs, char const *ct) {
	
	signed char __res;

	while (1) {
		if ((__res = *cs - *ct++) != 0 || !*cs++)
			break;
	}

	return __res;
}