// BeagleBone GPIO Pin Configurator
// Compatible with the config-pin script available elsewhere

// Copyright (C)2018, Philip Munts, President, Munts AM Corp.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/param.h>

#define NAMES		"/sys/firmware/devicetree/base/ocp/%s_pinmux/pinctrl-names"
#define STATE		"/sys/devices/platform/ocp/ocp:%s_pinmux/state"
#define DIRECTION "/sys/class/gpio/gpio%d/direction"
#define DELIMITERS	" \t\r\n"

static void fixup_pin_name(char *s)
{
  for (; *s != 0; s++)
  {
    unsigned char c = *s;

    if ((c >= 'a') && (c <= 'z'))
      *s = toupper(c);
    else if (c == '.')
      *s = '_';
  }
}

// Only stores the first line of cmd's output
// Returns -1 if fails
static int RunShellCmd(char *cmd, char *buff, const int BUFF_SIZE) {
  FILE *fp;
  int status;

  fp = popen(cmd, "r");
  if(fp == NULL)
    return -1;
  
  fgets(buff, BUFF_SIZE, fp);
  strtok(buff, "\n"); // Remove newline

  status = pclose(fp);

  return status;
}

// Returns -1 if fails
static int GetGpio(char *pin) {
  char cmd[255];
  char pin_name[255];
  char chip[255];
  char gpio_chip[255];
  int gpio, status;

  fixup_pin_name(pin);

  // Ensure pin name is valid (prevent OS injections)
  // Must have format P[0-9]_[0-9][0-9]
  if(strlen(pin) != 5 || pin[0] != 'P' || !isdigit(pin[1]) || pin[2] != '_' || !isdigit(pin[3]) || !isdigit(pin[4]))
  {
    fprintf(stderr, "WARNING: invalid pin name, %s", pin,
    strerror(errno));
    return -1;
  }

  // Get pin name
  strcpy(cmd, "gpioinfo | grep ");
  strcat(cmd, pin);
  strcat(cmd, " -m 1 | awk '{print substr($3,2,length($3) - 2)}'");
  status = RunShellCmd(cmd, pin_name, 255);

  if(status == -1 || pin_name[0] == '\0') // Check for error or empty string (meaning pin could not be found)
    return -1;

  // Get GPIO chip
  strcpy(cmd, "gpiofind ");
  strcat(cmd, pin_name);
  strcat(cmd, " | awk '{print substr($1,9)}'");
  RunShellCmd(cmd, chip, 255);

  if(status == -1 || chip[0] == '\0') // Check for error or empty string (meaning pin could not be found)
    return -1;

  // Get GPIO number on chip
  strcpy(cmd, "gpiofind ");
  strcat(cmd, pin_name);
  strcat(cmd, " | awk '{print $2}'");
  RunShellCmd(cmd, gpio_chip, 255);

  if(status == -1 || gpio_chip[0] == '\0') // Check for error or empty string (meaning pin could not be found)
    return -1;

  // Calculate sysfs GPIO number
  gpio = atoi(chip) * 32 + atoi(gpio_chip);
  return gpio;
}

static void ListModes(char *pin)
{
  char filename[MAXPATHLEN];
  int nf;
  char buf[256];
  ssize_t len;
  int i;

  // Sanitize the pin name parameter

  fixup_pin_name(pin);

  // Open the modes list file

  memset(filename, 0, sizeof(filename));
  snprintf(filename, sizeof(filename), NAMES, pin);

  nf = open(filename, O_RDONLY);
  if (nf < 1)
  {
    fprintf(stderr, "ERROR: open() for %s failed, %s\n", filename,
      strerror(errno));
    exit(1);
  }

  // Read the modes list file

  len = read(nf, buf, sizeof(buf));
  if (len < 0)
  {
    fprintf(stderr, "ERROR: read() from %s failed, %s\n", filename,
      strerror(errno));
    exit(1);
  }

  // Display the available modes

  printf("\nAvailable modes for %s are:", pin);

  for (i = 0; i < len; i++)
  {
    putchar(' ');
    printf(buf + i);
    i += strlen(buf + i);
  }

  puts("\n");
  fflush(stdout);

  // Close the modes list file

  close(nf);
}

static void QueryMode(char *pin)
{
  char filename[MAXPATHLEN];
  int sf;
  char buf[256];
  ssize_t len;

  // Sanitize the pin name parameter

  fixup_pin_name(pin);

  memset(filename, 0, sizeof(filename));
  snprintf(filename, sizeof(filename), STATE, pin);

  // Open the current mode file

  sf = open(filename, O_RDONLY);
  if (sf < 1)
  {
    fprintf(stderr, "ERROR: open() for %s failed, %s\n", filename,
      strerror(errno));
    exit(1);
  }

  // Read the current mode file

  len = read(sf, buf, sizeof(buf));
  if (len < 0)
  {
    fprintf(stderr, "ERROR: read() from %s failed, %s\n", filename,
      strerror(errno));
    exit(1);
  }

  // Display the current mode

  printf("\nCurrent mode for %s is:     %s\n", pin, buf);

  // Close the current mode file

  close(sf);
}

static void ConfigDir(int gpio, char *dir) {
  char filename[MAXPATHLEN];
  int df;
  ssize_t len;

  memset(filename, 0, sizeof(filename));
  snprintf(filename, sizeof(filename), DIRECTION, gpio);

  // Open the current direction file

  df = open(filename, O_WRONLY);
  if (df < 1)
  {
    fprintf(stderr, "ERROR: open() for %s failed, %s\n", filename,
      strerror(errno));
    exit(1);
  }

  // Write to current direction file

  len = write(df, dir, strlen(dir));
  if (len < 0)
  {
    fprintf(stderr, "ERROR: write() to %s failed, %s\n", filename,
      strerror(errno));
    exit(1);
  }

  // Close the current direction file

  close(df);
}

static void ConfigureMode(char *pin, char *mode, bool quiet)
{
  char filename[MAXPATHLEN];
  int sf;
  ssize_t len;
  bool inout = !strcmp(mode, "out") || !strcmp(mode, "in");

  // Sanitize the pin name parameter

  fixup_pin_name(pin);

  memset(filename, 0, sizeof(filename));
  snprintf(filename, sizeof(filename), STATE, pin);

  // Open the current mode file

  sf = open(filename, O_WRONLY);
  if (sf < 1)
  {
    fprintf(stderr, "ERROR: open() for %s failed, %s\n", filename,
      strerror(errno));
    exit(1);
  }

  // Read the current mode file

  len = inout ? write(sf, "gpio", strlen("gpio")) : write(sf, mode, strlen(mode));
  if (len < 0)
  {
    fprintf(stderr, "ERROR: write() to %s failed, %s\n", filename,
      strerror(errno));
    exit(1);
  }

  // Display the current mode

  if (!quiet) printf("\nCurrent mode for %s is:     %s\n\n", pin, inout ? "gpio" : mode);

  // Close the current mode file

  close(sf);

  // Update GPIO direction (if applicable)

  if(inout) {
    int gpio = GetGpio(pin);

    if(gpio == -1)
      fprintf(stderr, "ERROR: Could not find GPIO number for pin %s", pin);
    else
      ConfigDir(gpio, mode);
  }
}

static void ConfigFile(char *filename)
{
  FILE *cf;
  char buf[256];
  char *pin, *mode;

  // Open the specified configuration file

  cf = fopen(filename, "r");
  if (cf == NULL)
  {
    fprintf(stderr, "ERROR: fopen() for %s failed, %s\n", filename,
      strerror(errno));
    exit(1);
  }

  // Read the configuration file

  while (fgets(buf, sizeof(buf), cf) != NULL)
  {
    pin = strtok(buf, DELIMITERS);
    if (pin == NULL) continue;
    if (pin[0] == '#') continue;

    mode = strtok(NULL, DELIMITERS);
    if (mode == NULL) continue;

    ConfigureMode(pin, mode, true);
  }

  // Close the configuration file

  fclose(cf);
}

int main(int argc, char *argv[])
{
  if (argc != 3)
  {
    fprintf(stderr, "\nGPIO Pin Configurator\n\n");
    fprintf(stderr, "Usage: config-pin -c <filename>\n");
    fprintf(stderr, "       config-pin -l <pin>\n");
    fprintf(stderr, "       config-pin -q <pin>\n");
    fprintf(stderr, "       config-pin <pin> <mode>\n\n");
    exit(1);
  }

  if (!strcasecmp(argv[1], "-c"))
    ConfigFile(argv[2]);
  else if (!strcasecmp(argv[1], "-l"))
    ListModes(argv[2]);
  else if (!strcasecmp(argv[1], "-q"))
    QueryMode(argv[2]);
  else
    ConfigureMode(argv[1], argv[2], false);

  exit(0);
}
