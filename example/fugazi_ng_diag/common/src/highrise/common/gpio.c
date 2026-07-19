/*------------------------------------------------------------------
 *
 * gpio.c - gpio control APIs base on GPIO Sysfs interface
 *          platform indendent
 *
 * May 2019, markzha
 *
 * Copyright (c) 2019-2020 by Cisco Systems, Inc.
 * All rights reserved.
 *
 *------------------------------------------------------------------
 */
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include "gpio.h"

int gpio_has_exported (int pin)
{
    char buffer[PATH_MAX];
    DIR *dir = NULL;

    snprintf(buffer, PATH_MAX, "/sys/class/gpio/gpio%d", pin);

    dir = opendir(buffer);
    if (dir) {
        return (1);
    } else {
        return (0);
    }
}

int gpio_export (int pin)
{
    char buffer[BUFFER_MAX];
    ssize_t bytes_written;
    int fd;

    if (gpio_has_exported(pin)) {
        return (0);
    }

    fd = open("/sys/class/gpio/export", O_WRONLY);
    if (-1 == fd) {
        fprintf(stderr, "Failed to open export for writing, err(%s, %d)!\n", strerror(errno), errno);
        return (-1);
    }

    bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
    if (-1 == write(fd, buffer, bytes_written)) {
        fprintf(stderr, "Failed to write export, err(%s, %d)!\n", strerror(errno), errno);
        return (-1);
    }
    close(fd);
    return (0);
}

int gpio_unexport (int pin)
{
    char buffer[BUFFER_MAX];
    ssize_t bytes_written;
    int fd;

    if (!gpio_has_exported(pin)) {
        return (0);
    }

    fd = open("/sys/class/gpio/unexport", O_WRONLY);
    if (-1 == fd) {
        fprintf(stderr, "Failed to open unexport for writing, err(%s, %d)!\n", strerror(errno), errno);
        return (-1);
    }

    bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
    if (-1 == write(fd, buffer, bytes_written)){
        fprintf(stderr, "Failed to write unexport, err(%s, %d)!\n", strerror(errno), errno);
        return (-1);
    }

    close(fd);
    return(0);
}

int gpio_direction (int pin, int dir)
{
    static const char s_directions_str[]  = "in\0out";

    char path[DIRECTION_MAX];
    int fd;

    snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/direction", pin);
    fd = open(path, O_WRONLY);
    if (-1 == fd) {
        fprintf(stderr, "Failed to open gpio direction for writing, err(%s, %d)!\n", strerror(errno), errno);
        return(-1);
    }

    if (-1 == write(fd, &s_directions_str[IN == dir ? 0 : 3], IN == dir ? 2 : 3)) {
        fprintf(stderr, "Failed to set direction, err(%s, %d)!\n", strerror(errno), errno);
        return(-1);
    }

    close(fd);
    return(0);
}

int gpio_read (int pin, int *value)
{
    char path[VALUE_MAX];
    char value_str;
    int fd;

    snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
    fd = open(path, O_RDONLY);
    if (-1 == fd) {
        fprintf(stderr, "Failed to open gpio%d value for reading, err(%s, %d)!\n", pin, strerror(errno), errno);
        return(-1);
    }

    if (-1 == read(fd, &value_str, 1)) {
        fprintf(stderr, "Failed to read gpio%d value, err(%s, %d)!\n", pin, strerror(errno), errno);
        return(-1);
    }

    if (value_str == '0') {
        *value = 0;
    } else {
        *value = 1;
    }

    close(fd);

    return(0);
}

int gpio_write (int pin, int value)
{
    static const char s_values_str[] = "01";

    char path[VALUE_MAX];
    int fd;

    snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
    fd = open(path, O_WRONLY);
    if (-1 == fd) {
        fprintf(stderr, "Failed to open gpio value for writing, err(%s, %d)!\n", strerror(errno), errno);
        return(-1);
    }

    if (1 != write(fd, &s_values_str[LOW == value ? 0 : 1], 1)) {
        fprintf(stderr, "Failed to write value, err(%s, %d)!\n", strerror(errno), errno);
        return(-1);
    }

    close(fd);
    return(0);
}
