#define _XOPEN_SOURCE 600

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_LEN 4096

static FILE *infile = NULL;
static size_t histogram[256] = {0};

static int
hist256 (void)
{
  size_t i;
  char *path;
  char buf[BUF_LEN];

  while (NULL != fgets (buf, BUF_LEN, infile)
      && NULL != (path = strtok (buf, "\n")))
    {
      size_t amount_read;
      FILE *file = fopen (path, "r");

      if (NULL == file)
        {
          fprintf (stderr, "%s\n", strerror (errno));
          return 1;
        }

      posix_fadvise (fileno (file), 0, 0, POSIX_FADV_WILLNEED);

      while (0 != (amount_read = fread (buf, 1, BUF_LEN, file)))
        for (i = 0; i < amount_read; i++)
          histogram[(uint8_t) buf[i]]++;

      fclose (file);
    }

  printf ("DEC VALUE\n");
  for (i = 0; i < 256; i++)
    printf ("%03lu %05lu\n", i, histogram[i]);

  return 0;
}

static void
usage (void)
{
  printf (
"hist256 -i input.txt\n"
"\n"
"-i, --input <string>     A new line delimited list of files to compute the\n"
"                           histogram with [default=stdin]\n"
"-h, --help               This help menu\n"
"\n");
}

int
main (int argc, char **argv)
{
  int err;
  static struct option long_options[] = {
    {"input", required_argument, 0, 'i'},
    {"help", no_argument, 0, 'h'},
    {0, 0, 0, 0}
  };

  for (;;)
    {
      int option_index = 0;
      int c = getopt_long (argc, argv, "i:h", long_options, &option_index);

      if (-1 == c)
        break;

      switch (c)
        {
          case 'i':
            if (optarg[0] == '-' && optarg[1] == '\0')
              infile = stdin;
            else
              {
                infile = fopen (optarg, "r");
                if (NULL == infile)
                  {
                    fprintf (stderr, "%s\n", strerror (errno));
                    usage ();
                    return 1;
                  }
                posix_fadvise (fileno (infile), 0, 0, POSIX_FADV_WILLNEED);
              }
            break;
          case 'h':
            usage ();
            return 0;
          default:
            usage ();
            return 1;
        }
    }

  if (NULL == infile)
    infile = stdin;

  err = hist256 ();
  fclose (infile);

  return err;
}
