//
// Created by Jost on 20/09/2024.
//

#ifndef ERROR_CODES_H
#define ERROR_CODES_H

#include <string>

inline const std::string ERROR_INVALID_FILE = "[ERROR_R0] Failed to open file: {}";
inline const std::string ERROR_MISSING_PROJ = "[ERROR_R1] File misses projection reference, please manually specify one!";
inline const std::string ERROR_INVALID_PROJ = "[ERROR_R2] Provided projection reference does not discribe a valid projection:\n\n{}";
inline const std::string ERROR_FAILED_PROJ = "[ERROR_R3] Failed to project points, unknown reason!";
inline const std::string ERROR_NO_RASTER = "[ERROR_T0] No raster files were provided, at least one is needed!";
inline const std::string ERROR_NO_PATH = "[ERROR_T1] No paths were provided, at least one is needed!";

#endif // ERROR_CODES_H
