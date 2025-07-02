#include "drv_csv.h"
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>


static void destroy_field(CSV_FIELD *field);
static int append_row(CSV_BUFFER *buffer);
static int append_field(CSV_BUFFER *buffer, size_t row);
static CSV_FIELD *create_field();
static int set_field(CSV_FIELD *field, char *text);

#define FVAR fres
static FRESULT FVAR = FR_OK;
//#define FWRITE(a, b, c, d) ((FVAR = f_write(a,b,c,d)) ? (return FVAR) : (return FR_OK))

#define FWRITE(a, b, c, d) { 			\
	FVAR = f_write(a,b,c,d); 				\
	if (FVAR != FR_OK) return FVAR; \
}

//#define FWRITE(a, b, c, d) ( if ((FVAR = f_write(a,b,c,d)) != FR_OK) return FVAR )


/* Function: destroy_field
 * ---------------------------
 * Frees CSV_FIELD memory. If the field has been initialized
 * but not set, field->text has not been malloc'd, and so
 * is not freed.
 * 
 * Returns:
 * 0: success
 * 1: error realloc'ing field's char array
 */
static void
	destroy_field(CSV_FIELD *field) {
	
	if (field->text != NULL) {
		free(field->text);
		field->text = NULL;
	}
	free(field);
	field = NULL;
}


/* Function: append_row
 * -------------------------------
 * Adds a "row" to the end of a CSV_BUFFER. The row is 
 * initialized with no fields. Also adds an integer to the
 * width array and sets width = 0.
 *
 * Returns:
 * 0: success
 * 1: error allocating width memory
 * 2: error allocating field memory
 */
static int
	append_row(CSV_BUFFER *buffer) {
	
	size_t *temp_width;
	CSV_FIELD ***temp_field;

	size_t row  = buffer->rows;

	temp_width = realloc(buffer->width, (buffer->rows + 1) * sizeof(size_t));
	if (temp_width != NULL)	{
			buffer->width = temp_width;
			buffer->width[row] = 0;
	} else return 1;

	temp_field = realloc(buffer->field, (buffer->rows + 1) * sizeof(CSV_FIELD**));
	if (temp_field != NULL) {
			buffer->field = temp_field;
			buffer->field[row] = NULL;
	}	else {
			free(temp_width);
			return 2;
	}

	buffer->rows++;
	append_field(buffer, row);
	return 0;
}


/* Function: append_field
 * ---------------------------------
 * Adds a field to the end of a given row in a CSV_BUFFER. 
 * The field is initialized using csv_create_buffer.
 *
 * Returns:
 * 0: success
 * 1: the given row does not exist
 * 2: memory allocation error 
 */
static int
	append_field(CSV_BUFFER *buffer, size_t row) {

	CSV_FIELD **temp_field;

	if (buffer->rows < row + 1)
		return 1;

	/* Set col equal to the index of the new field */
	int col = buffer->width[row];

	temp_field = realloc(buffer->field[row], (col + 1) * sizeof(CSV_FIELD*));
	if (temp_field == NULL)	{
		return 2;
	}	else	{
		buffer->field[row] = temp_field;
		buffer->field[row][col] = create_field(); 
		buffer->width[row]++;
	} 

	return 0;
}


/* Function: create_field 
 * ------------------------
 * Should be called once on every CSV_FIELD used. Allocates
 * memory for the field. Length is set to 0 and text to NULL
 * 
 * Returns NULL on error via malloc.
 */
static CSV_FIELD *
	create_field(void) {
	
	CSV_FIELD *field = malloc(sizeof(CSV_FIELD));
	field->length = 0;
	field->text = NULL;
	set_field(field, "\0");
	return field;
}


/* Function: set_field
 * -----------------------
 * Sets a field text to the string provided. Adjusts field
 * length accordingly. 
 * 
 * Returns:
 *  0: success
 *  1: error allocating space to the string
 */
static int 
	set_field(CSV_FIELD *field, char *text) {        
	
	char *tmp;

	field->length = strlen(text) + 1;
	tmp = realloc(field->text, field->length);
	if (tmp == NULL)
			return 1;
	field->text = tmp;
	strcpy(field->text, text);

	return 0;
}


/*******************************************************************************	
	Description:
	
	Arguments:
		
	Returns:
		
*******************************************************************************/
int
	csv_set_field(CSV_BUFFER *buffer, size_t row, size_t entry, char *field) {
  
	while (row >= buffer->rows) {
		append_row(buffer);
  } 
	
	while (entry >= buffer->width[row]) {
		append_field(buffer, row);
	}
       
	if (set_field(buffer->field[row][entry], field) == 0)
			return 0;
	else 
			return 1;
}


/*******************************************************************************	
	Description:
	
	Arguments:
		
	Returns:
		
*******************************************************************************/
CSV_BUFFER *
	csv_create_buffer(void) {
	
	CSV_BUFFER *buffer = malloc(sizeof(CSV_BUFFER));

	if (buffer != NULL)	{
		buffer->field = NULL;
		buffer->rows = 0;
		buffer->width = NULL;
		buffer->field_delim = ',';
		buffer->text_delim = '"';
	}

	return buffer;
}


/*******************************************************************************	
	Description:
	
	Arguments:
		
	Returns:
		
*******************************************************************************/
void
	csv_destroy_buffer(CSV_BUFFER *buffer) {
	
	int i, j;

	for (i = 0; i < buffer->rows; i++) {
		for (j = 0; j < buffer->width[i]; j++)
			destroy_field(buffer->field[i][j]);
		free(buffer->field[i]);
		buffer->field[i] = NULL;
	}

	if (buffer->field != NULL)
			free(buffer->field);

	if (buffer->width != NULL)
			free(buffer->width);

	free(buffer);
}


FRESULT 
	csv_save(FIL *csv_fp, CSV_BUFFER *buffer, UINT* pbytes) {
	
	int i, j;
	char *chloc; 
	UINT n;
	FRESULT res;

	// Rewind just in case
	/*res = f_rewind(csv_fp);
	
	if (res != FR_OK)
	{
		log_error("f_rewind pb: %d", res);
		return 1;
	}*/

	char text_delim = buffer->text_delim;
	char field_delim = buffer->field_delim;
	char new_line = '\n';
	*pbytes = 0;
	
	for(i = 0; i < buffer->rows; i++) {
		for(j = 0; j < buffer->width[i]; j++)	{
			
			//...
			chloc = strchr(buffer->field[i][j]->text, text_delim);
			if(chloc == NULL)
				chloc = strchr(buffer->field[i][j]->text, field_delim);
			if(chloc == NULL)
				chloc = strchr(buffer->field[i][j]->text, '\n');
			
			// if any of the above characters are found, chloc will be set
			// and we must use text delimiters.
			if(chloc != NULL) {
				//if ((FVAR = f_write(csv_fp, &text_delim, 1, &n)) != FR_OK) return FVAR;
				FWRITE(csv_fp, &text_delim, 1, &n);
				*pbytes += n;

				FWRITE(csv_fp, buffer->field[i][j]->text, buffer->field[i][j]->length - 1, &n)
				*pbytes += n;
				
				FWRITE(csv_fp, &text_delim, 1, &n);
				*pbytes += n;
				
				chloc = NULL;
			} else {
				FWRITE(csv_fp, buffer->field[i][j]->text, buffer->field[i][j]->length - 1, &n);
				*pbytes += n;
			}
			
			//...
			if(j < buffer->width[i] - 1) {
				FWRITE(csv_fp, &field_delim, 1, &n);
				*pbytes += n;
			}
			else if (i < buffer->rows - 1) {
				FWRITE(csv_fp, &new_line, 1, &n);
				*pbytes += n;
			}
		}
	}
	
	FWRITE(csv_fp, &new_line, 1, &n);
	*pbytes += n;
	
	f_truncate(csv_fp);

	// Sync (just in case)
	/*res = f_sync(csv_fp);
	if (res != FR_OK)
	{
		log_error("f_sync pb: %d", res);
		return 0;
	}*/

	return 0;
}

