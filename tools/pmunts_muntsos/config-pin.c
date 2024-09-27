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
#include <dirent.h>
#include <sys/param.h>

#define PLATFORMBASE "/sys/devices/platform/ocp/"
#define NAMES		"/sys/firmware/devicetree/base/ocp/%s_pinmux/pinctrl-names"
#define STATE_AI   "/sys/devices/platform/44000000.ocp/44000000.ocp:%s_pinmux/state"
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
static int GetGpio(char *pin, int print) {
  char cmd[255];
  char cmdout[255];
  char *temp_pin_name;
  char pin_name[32];
  char chip;
  char gpio_chip[3];
  int gpio, status;

  fixup_pin_name(pin);

  // Ensure pin name is valid (prevent OS injections)
  // (must have format P[0-9]_[0-9][0-9])

  if(strlen(pin) != 5 || pin[0] != 'P' || !isdigit(pin[1]) || pin[2] != '_' || !isdigit(pin[3]) || !isdigit(pin[4]))
  {
    fprintf(stderr, "WARNING: invalid pin name, %s", pin,
    strerror(errno));
    return -1;
  }

  // Run gpioinfo [pin]

  strcpy(cmd, "gpioinfo | grep ");
  strcat(cmd, pin);
  strcat(cmd, " -m 1");
  status = RunShellCmd(cmd, cmdout, sizeof(cmdout));

  if(status == -1 || cmdout[0] == '\0') // Check for error or empty string (meaning pin could not be found)
    return -1;

  // Get pin name (parse gpioinfo output)

  strtok(cmdout, " ");
  strtok(NULL, " ");
  temp_pin_name = strtok(NULL, " "); // Pin name is the third word of gpioinfo output
  
  strncpy(pin_name, temp_pin_name + 1, strlen(temp_pin_name) - 1); // Remove quotes surrounding pin name
  pin_name[strlen(temp_pin_name) - 2] = '\0'; // Need to manually add null terminator with strncpy

  // Run gpiofind [pin_name]

  strcpy(cmd, "gpiofind ");
  strcat(cmd, pin_name);
  RunShellCmd(cmd, cmdout, sizeof(cmdout));

  if(status == -1 || cmdout[0] == '\0') // Check for error or empty string (meaning pin could not be found)
    return -1;

  // Get GPIO chip (parse gpiofind output)

  chip = cmdout[8];

  // Get GPIO number on chip (parse gpiofind output)

  strtok(cmdout, " "); // Split gpiofind output at space
  strcpy(gpio_chip, strtok(NULL, " "));

  // Calculate sysfs GPIO number

  gpio = atoi(&chip) * 32 + atoi(gpio_chip);

  if (print == 1)
  {
    printf("Physical Pin: %s -> gpio%d\n",pin, gpio);
    return 0;
  }
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

  len = read(nf, buf, sizeof(buf) -1 );
  if (len < 0)
  {
    fprintf(stderr, "ERROR: read() from %s failed, %s\n", filename,
      strerror(errno));
    exit(1);
  }

  buf[len] = '\0';

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

static void QueryMode(char *pin, int newline)
{
  char filename[MAXPATHLEN];
  int sf;
  char buf[256];
  ssize_t len;

  // Sanitize the pin name parameter

  fixup_pin_name(pin);

  memset(filename, 0, sizeof(filename));
  DIR* dir = opendir("/sys/devices/platform/ocp/");
  if (dir) {
    /* AM335x */
    closedir(dir);
    snprintf(filename, sizeof(filename), STATE, pin);
  } else {
    /* AM572x  */
    snprintf(filename, sizeof(filename), STATE_AI, pin);
  }

  // Open the current mode file

  sf = open(filename, O_RDONLY);
  if (sf < 1)
  {
    fprintf(stderr, "ERROR: open() for %s failed, %s\n", filename,
      strerror(errno));
    exit(1);
  }
  // Read the current mode file

  len = read(sf, buf, sizeof(buf) - 1 ); // Leave space for null terminator
  if (len < 0)
  {
    fprintf(stderr, "ERROR: read() from %s failed, %s\n", filename,
      strerror(errno));
    exit(1);
  }

  // Add null terminator to the buffer, as read system call does not add a null terminator
  buf[len] = '\0';

  // Display the current mode

  if (newline == 1)
  {
    printf("\nCurrent mode for %s is:     %s", pin, buf);
  }
  else {
    printf("\nCurrent mode for %s is:     %s\n", pin, buf);
  }
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
  bool inout = !strcmp(mode, "out") || !strcmp(mode, "output") || !strcmp(mode, "in") || !strcmp(mode, "input");

  // Allow for both in/out and input/output

  mode = !strcmp(mode, "output") ? "out" : mode;
  mode = !strcmp(mode, "input") ? "in" : mode;

  // Sanitize the pin name parameter

  fixup_pin_name(pin);

  memset(filename, 0, sizeof(filename));
  DIR* dir = opendir("/sys/devices/platform/ocp/");
  if (dir) {
    /* AM335x */
    closedir(dir);
    snprintf(filename, sizeof(filename), STATE, pin);
  } else {
    /* AM572x  */
    snprintf(filename, sizeof(filename), STATE_AI, pin);
  }

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
    int gpio = GetGpio(pin,0);

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

// Function to extract pin name between ":" and "pinmux"
static void extract_pin_name(char *full_string, char *pin_name) {
    char *start, *end;

    // Find the part of the string after the colon ":"
    start = strchr(full_string, ':');
    if (start == NULL) {
        fprintf(stderr, "ERROR: Could not find colon in the string.\n");
        return;
    }

    // Move pointer after the colon
    start++;

    // Find the part of the string before "pinmux"
    end = strstr(start, "_pinmux");
    if (end == NULL) {
        fprintf(stderr, "ERROR: Could not find '_pinmux' in the string.\n");
        return;
    }

    // Calculate the length of the pin name and copy it to pin_name
    size_t length = end - start;
    strncpy(pin_name, start, length);
    pin_name[length] = '\0'; // Null-terminate the string
}

// List all available pins with extracted pin names
static void ListAllPins() {
    DIR *d;
    struct dirent *dir;
    char filename[MAXPATHLEN];
    char pin_name[32];

    // Open the directory containing pin information
    d = opendir(PLATFORMBASE);
    if (d == NULL) {
        fprintf(stderr, "ERROR: Unable to open %s, %s\n", PLATFORMBASE, strerror(errno));
        exit(1);
    }

    printf("\nListing all pins with current mode and available modes:\n");

    // Traverse the directory to find relevant pin files
    while ((dir = readdir(d)) != NULL) {
        // Look for entries that contain "pinmux" in the name
        if (strstr(dir->d_name, "pinmux") != NULL) {
            // Construct the full filename path (for debugging or further use)
            snprintf(filename, sizeof(filename), PLATFORMBASE"%s", dir->d_name);

            // Extract the pin name between ":" and "_pinmux"
            extract_pin_name(filename, pin_name);
            
            // Physical to GPIO 
            GetGpio(pin_name,1);

            // Query the current mode of the pin
            QueryMode(pin_name,1);

            // List the available modes for the pin
            ListModes(pin_name);

            // Line Separator
            printf("----------------------------------------------------------------------------------------------------\n");
        }
    }

    closedir(d);
    printf("Finished listing all pins.\n");
}

int main(int argc, char *argv[])
{
  if (argc == 2 && !strcasecmp(argv[1], "-a"))
  {
    ListAllPins(); // List all pins with their modes
    exit(0);
  }

  if (argc != 3)
  {
    fprintf(stderr, "\nGPIO Pin Configurator\n\n");


    fprintf(stderr, "Usage: config-pin -c <filename>\n");
    fprintf(stderr, "       config-pin -l <pin>\n");
    fprintf(stderr, "       config-pin -q <pin>\n");
    fprintf(stderr, "       config-pin -g <pin>\n");
    fprintf(stderr, "       config-pin -a (to list all pins)\n\n");
    fprintf(stderr, "       config-pin <pin> <mode>\n\n");
    exit(1);
  }

  if (!strcasecmp(argv[1], "-c"))
    ConfigFile(argv[2]);
  else if (!strcasecmp(argv[1], "-l"))
    ListModes(argv[2]);
  else if (!strcasecmp(argv[1], "-q"))
    QueryMode(argv[2],0);
  else if (!strcasecmp(argv[1], "-g"))
    GetGpio(argv[2],1);
  else
    ConfigureMode(argv[1], argv[2], false);

  exit(0);
}
