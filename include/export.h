#ifndef EXPORT_H
#define EXPORT_H

#include "types.h"

/* Export current viewport to ASCII art file */
/* Returns 0 on success, -1 on failure */
int export_viewport_to_file(const Canvas *canvas, const Viewport *vp, const char *filename);

#endif /* EXPORT_H */
