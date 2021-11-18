#include <stdio.h>
#include <string.h>

/* prtspool by Tim Pinkawa (http://timpinkawa.net/hercules)
 * Written in December 2006, released in June 2007
 */

int main(int argc, char* argv[])
{
  if(argc != 3 && argc != 4)
  {
	  printf("prtspool - a simple print spooler for emulated 1403 printers\n");
	  printf("By Tim Pinkawa (http://www.timpinkawa.net/hercules)\n\n");
	  printf("usage: %s {msgclass} {output_directory} [command]\n", argv[0]);
	  return 0;
  }
  int job = 1;
  while(!feof(stdin))
  {
    int endCount = 0;
    char line[200];
    char ss[200];
    fgets(line, 200, stdin);
    if(strcmp(line, "\f") == 0)
      break;

	 char jobEnd[15];
	 snprintf(jobEnd, 15, "****%s   END", argv[1]);

	 char cmd[225];
	 if(argc == 4)
		 snprintf(cmd, 225, "%s %s %s", argv[3], argv[1], argv[2]);
	 
    int i;
    for(i = 1; i < 200; i++)
      ss[i - 1] = line[i];
    FILE* jobfp;

    char path[250];
    snprintf(path, 250, "%s%s%i%s", argv[2], "job-", job, ".txt");

    jobfp = fopen(path, "w");
    fprintf(jobfp, ss);
    int endOfJob = 0;
    while(!endOfJob && !feof(stdin))
    {
      fgets(line, 200, stdin);
      if(strstr(line, jobEnd) != NULL)
        endCount++;
      if(endCount == 4)
        endOfJob = 1;
      fprintf(jobfp, line);
    }
    fclose(jobfp);

    if(argc == 4)
       system(cmd);

    job++;
  }
}
