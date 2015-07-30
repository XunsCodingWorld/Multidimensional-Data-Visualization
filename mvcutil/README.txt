When adding more general mouse-based support to Controller -> ModelView interface, use another
polymorphic version of "handleCommand". Something like:

void handleCommand(std::string cmd, int flags, double ldsX, double ldsY);

"cmd" might be, for example, "select"
"flags" (or maybe other choices for "cmd") could indicate things like "left mouse", "one finger swipe",
	"double-click", modifier keys, etc.)
