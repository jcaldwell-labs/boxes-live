#ifndef INPUT_H
#define INPUT_H

#include "types.h"

/* Process input and update viewport and canvas accordingly */
/* Returns 0 to continue, 1 to quit */
int handle_input(Canvas *canvas, Viewport *vp);

#endif /* INPUT_H */
