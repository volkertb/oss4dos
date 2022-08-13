/*
 * This file is part of Open Sound System
 *
 * Copyright (C) 4Front Technologies 1996-2008.
 *
 * This software is released under the BSD license.
 * See the COPYING file included in the main directory of this source
 * distribution for the license terms and conditions
 */


#include <stdio.h>
#include <oss_version.h>

int
main (void)
{
  printf (OSS_VERSION_STRING "\n");
  return 0;
}
