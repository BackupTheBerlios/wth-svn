/* 
    sample program to fiddle with rrd's
    directly from within wth

    Volker Jahns, May 2007
*/
#include "rrd_tool.h"

char** string_to_argv(char *argstring, int *argc);
void Free_argv(char** argv);



int main ( int argc, char **argv) {
    int rrd_argc, ret;
    char **rrd_argv;
    char *argstr = "PressureI.rrd --start 973709203 \
       DS:PressureI:GAUGE:600:800:1100 \
       RRA:AVERAGE:0.5:1:600 \
       RRA:AVERAGE:0.5:6:700 \
       RRA:AVERAGE:0.5:24:775 \
       RRA:AVERAGE:0.5:288:797 \
       RRA:LAST:0.5:1:1";

    if((rrd_argv = string_to_argv(argstr, &rrd_argc)) != NULL)
        {
                optind=0; /* reset gnu getopt */
                opterr=0; /* no error messages */

                ret = rrd_create(rrd_argc, rrd_argv);

                //      free up the memory
                Free_argv(rrd_argv);

                return(ret);
        }
    return(0);
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
 compile command: gcc -L/usr/local/lib -lrrd -o wth_rrdtool wth_rrdtool.c
 please check for location of librrd.a
*/
