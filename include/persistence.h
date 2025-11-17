#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include "types.h"

/* Save canvas to file */
/* Returns 0 on success, -1 on error */
int canvas_save(const Canvas *canvas, const char *filename);

/* Load canvas from file */
/* Returns 0 on success, -1 on error */
int canvas_load(Canvas *canvas, const char *filename);

/* Set the current file name (for reload) */
void persistence_set_current_file(const char *filename);

/* Get the current file name (for reload) */
const char *persistence_get_current_file(void);

#endif /* PERSISTENCE_H */
