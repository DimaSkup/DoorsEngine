// =================================================================================
// Filename:   raw_file.h
// Desc:       loader/saver for the .RAW files
//
// Created:    22.06.2025  by DimaSkup
// =================================================================================
#pragma once

#include <stdint.h>

// save input data into RAW file
bool SaveFileRAW(const char* filename, const uint8_t* data, const int numBytes);

// load data from RAW file
bool LoadFileRAW(const char* filename, uint8_t** outData, int& outSize);
