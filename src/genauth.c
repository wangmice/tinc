/*
    genauth.c -- generate a random passphrase
    Copyright (C) 1998,99 Ivo Timmermans <zarq@iname.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <xalloc.h>

#include "encr.h"

unsigned char initvec[] = { 0x22, 0x7b, 0xad, 0x55, 0x41, 0xf4, 0x3e, 0xf3 };

int main(int argc, char **argv)
{
  FILE *fp;
  int bits, c, i, bytes;
  unsigned char *p;

  if(argc != 2)
    {
      fprintf(stderr, "Usage: %s bits\n", argv[0]);
      return 1;
    }

  if(!(bits = atol(argv[1])))
    {
      fprintf(stderr, "Illegal number: %s\n", argv[1]);
      return 1;
    }

  bits = ((bits - 1) | 63) + 1;
  fprintf(stderr, "Generating %d bits number", bits);
  bytes = bits >> 3;

  if((fp = fopen("/dev/urandom", "r")) == NULL)
    {
      perror("Opening /dev/urandom");
      return 1;
    }

  p = xmalloc(bytes);

  setbuf(stdout, NULL);
  for(i = 0; i < bytes; i++)
    {
      c = fgetc(fp);
      if(feof(fp))
        {
          puts("");
          fprintf(stderr, "File was empty!\n");
        }
      p[i] = c;
    }
  fclose(fp);

  printf("%d ", bits);
  for(i = 0; i < bytes; i++)
    printf("%02x", p[i]);
  puts("");
  fprintf(stderr, ": done.\n");

  return 0;
}

