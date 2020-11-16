int		fopen_rbf			( char*, char* );
int		fclose_rbf			( int file_id );
int		fseek_rbf			( int, int, int );
int		ftell_rbf			( int finputid );
int		fgetc_rbf			( int finputid );

void	processFileInput	( int finputid );
int 	prepareLoading		();
void 	programByte			( int one_byte );