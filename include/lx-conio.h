#ifndef WINDOWS

/* Initialize new terminal i/o settings */
void InitTermios(int echo) ;

/* Restore old terminal i/o settings */
void ResetTermios(void);

/* Read 1 character - echo defines echo mode */
char getch_(int echo);

/* Read 1 character without echo */
char _getch(void);

/* Read 1 character with echo */
char getche(void);

#endif
