#ifndef INPUT_H
#define INPUT_H

#include "types.h"
#include "joystick.h"
#include "config.h"

/* Process keyboard and mouse input, update viewport and canvas accordingly */
/* Returns 0 to continue, 1 to quit */
int handle_input(Canvas *canvas, Viewport *vp, JoystickState *js, const AppConfig *config);

/* Process joystick input based on current mode */
/* Returns 0 to continue, 1 to quit */
int handle_joystick_input(Canvas *canvas, Viewport *vp, JoystickState *js, const AppConfig *config);

#endif /* INPUT_H */
