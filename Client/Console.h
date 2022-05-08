#ifndef CONSOLE_H
#define CONSOLE_H

#define NUM_COLUMNS	(81)
#define NUM_ROWS (23)

void CSInitial(void);
void CSMoveCursor(int x, int y);
void CSClearScreen(void);

#endif /* CONSOLE_H */