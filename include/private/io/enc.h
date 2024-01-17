#pragma once

#ifndef FILE_ENC_H
#define FILE_ENC_H

#include <io/file.h>

const char* detect_file_encoding(file_t* _fd);

#endif