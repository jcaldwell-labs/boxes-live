#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include "types.h"

/* Save canvas to file */
/* Returns 0 on success, -1 on error */
int canvas_save(const Canvas *canvas, const char *filename);

/* Load canvas from file */
/* Returns 0 on success, -1 on error */
int canvas_load(Canvas *canvas, const char *filename);

#endif /* PERSISTENCE_H */
