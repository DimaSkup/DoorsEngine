// =================================================================================
// Filename:   raw_file.h
// Desc:       loader/saver for the .RAW files
//
// Created:    22.06.2025  by DimaSkup
// =================================================================================
#pragma once

#include "types.h"

// save input data into RAW file
bool SaveRAW(const char* filename, const uint8* data, const int numBytes);

// load data from RAW file
bool LoadRAW(const char* filename, uint8** outData, int& outSize);
