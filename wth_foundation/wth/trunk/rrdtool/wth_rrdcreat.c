/* 
    sample program to test creating rrd's
    directly from with wth

    Volker Jahns, May 2007
*/
#include "rrd_tool.h"

char** string_to_argv(char *argstring, int *argc);
void Free_argv(char** argv);



int main ( int argc, char **argv) {
    int rrd_argc,i;
    char **rrd_argv;
    char *argstr = "test.rrd --start 920804400 DS:speed:COUNTER:600:U:U RRA:AVERAGE:0.5:1:24 RRA:AVERAGE:0.5:6:10";

    if((rrd_argv = string_to_argv(argstr, &rrd_argc)) != NULL)
        {
                optind=0; /* reset gnu getopt */
                opterr=0; /* no error messages */

                i = rrd_create(rrd_argc, rrd_argv);

                //      free up the memory
                Free_argv(rrd_argv);

                return i;
        }
}



char** string_to_argv(char *argstring, int *argc)
{
	char
		inquotes = 0,
		*workstring,
		**argv;

	int 
		i, nchars;

	if((nchars = strlen(argstring)) > 0)
	{
		workstring = (char*)calloc((nchars + 2), sizeof(char*));

		//	fill the array with space characters
		for(i=0; i < (nchars + 2); i++)
			workstring[i] = ' '; 

		//	copy the argstring into the workstring (padded with spaces on both ends)
		for(i=0; i < nchars; i++)
			workstring[i+1] = argstring[i]; 

		for((*argc) = 1, i=0; i < nchars + 2; i++)
		{
			//	count the number of arguements
			if(isgraph(workstring[i]) > 0)
			{
				//	if current character is not whitespace and previous character is whitespace, increment (*argc)
				if((isgraph(workstring[i-1]) == 0) && (inquotes == 0))
					(*argc)++;

				//	check for the quote character
				if(workstring[i] == '"')
				{
					//	toggle inquotes flag
					inquotes = (char)(!inquotes);
				}

			}
			else
			{
				//	convert whitespace to null characters
				if(!inquotes)
					workstring[i] = '\0';
			}
		}

		//	no arguements in the string
		if((*argc) == 0)
		{
			free(workstring);
			return NULL;
		}
		else
		{
			inquotes = 0;

			argv = (char**)calloc((*argc), sizeof(char**));

			argv[0] = &workstring[0];

			for((*argc) = 1, i=1; i < nchars + 2; i++)
			{
				//	count the number of arguements
				if(isgraph(workstring[i]) > 0)
				{
					//	if current character is not whitespace and previous character is whitespace, increment (*argc)
					if((isgraph(workstring[i-1]) == 0) && (inquotes == 0))
					{
						argv[(*argc)] = &workstring[i];
						(*argc)++;
					}

					//	check for the quote character
					if(workstring[i] == '"')
					{
						//	toggle inquotes flag
						inquotes = (char)(!inquotes);
					}
				}
			}

			return argv;
		}
	}
	else 
	{
		(*argc) = 0;
		return NULL;
	}
}

void Free_argv(char** argv)
{
        free(argv[0]);
        free(argv);
}
/*
 compile command: gcc -L/usr/local/lib -lrrd -o wth_rrdcreat wth_rrdcreat.c
 please check for location of librrd.a
*/
