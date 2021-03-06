#include <stdlib.h>
#include "debug.h"
#include <string.h>
#include <sys/time.h>
#include <ctype.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>

FILE *myDebugFile = NULL;
static enum DebugLevel myDebugLevel = LV_ERROR_LEVEL;
#define LOG_THRESHOLD 100000


int DebugEnabled(enum DebugLevel level)
{
  if (!myDebugFile)
    DebugFileInit();
  else if (ftell(myDebugFile) >= LOG_THRESHOLD) {
      rewind(myDebugFile);
  }
  
  return (myDebugFile && (level <= myDebugLevel)); 
}


void DebugDisplayTime()
{
  struct timeval tv;
  if (!myDebugFile)
    DebugFileInit();

  if (!myDebugFile)
    return;
  
  gettimeofday(&tv, NULL);
  fprintf(myDebugFile, "%03ld.%06ld ", tv.tv_sec%1000, tv.tv_usec);
}


void DebugDump(const char *label, uint8_t *buf, size_t size)
{
#define MAX_BUF_SIZE 1024 
  size_t i;
  char line[20];

  if (!buf || !label)
    return;

  if (size > MAX_BUF_SIZE)
    size = MAX_BUF_SIZE;

  if (!myDebugFile)
    DebugFileInit();
  
  if (!myDebugFile)
    return;
  
  memset(line ,0, sizeof(line));
  fprintf(myDebugFile, "\n-------------------\n");
  fprintf(myDebugFile, "\nDump %s", label);

  for (i=0; i<size; i++) {
    if (!(i%16)) {
      fprintf(myDebugFile, "  %s", line);
      memset(line, 0, sizeof(line));
      fprintf(myDebugFile, "\n%p  ", buf+i);
    }
    fprintf(myDebugFile, "%02x ", buf[i]);
    line[i%16] = isprint(buf[i]) ? buf[i] : '.';
  }

  fprintf(myDebugFile, "\n");
}


void DebugFileInit()
{
#define MAX_FILENAME 40
  char filename[MAX_FILENAME+1];
  mode_t old_mask;
  struct stat buf;
  
  if (myDebugFile)
    fclose(myDebugFile);

  myDebugFile = NULL;

  if (!debug)
    return;

  if (snprintf(filename, MAX_FILENAME, VOXINUPLOG, getpid()) >= MAX_FILENAME)
    return;

  // the debug file must be read by the user only
  unlink(filename);
  old_mask = umask(0077);
  myDebugFile = fopen(filename, "w");
  umask(old_mask);
  if (!myDebugFile)
    return;
  
  if (fstat(fileno(myDebugFile), &buf) || buf.st_mode & 0077) {
    err("mode=%o", buf.st_mode);
    fclose(myDebugFile);
    myDebugFile = NULL;
    return;
  }
  
  setbuf(myDebugFile, NULL);
  myDebugLevel = (debug <= LV_DEBUG_LEVEL) ? debug : LV_DEBUG_LEVEL;  
}


void DebugFileFinish()
{
  if (myDebugFile)
    fclose(myDebugFile);  

  myDebugFile = NULL;
}

