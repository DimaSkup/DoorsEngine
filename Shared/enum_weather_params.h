/**********************************************************************************\

    ******     ******    ******   ******    ********
    **    **  **    **  **    **  **    **  **    **
    **    **  **    **  **    **  **    **  **
    **    **  **    **  **    **  **    **  ********
    **    **  **    **  **    **  ******          **
    **    **  **    **  **    **  **  ***   **    **
    ******     ******    ******   **    **  ********

    Filename: enum_weather_params.h
    Desc:     enumeration of weather parameters
              (for instance: wind parameters for grass/bushes/branches/foliage movement)

    Created:  18.11.2025 by DimaSkup
\**********************************************************************************/
#pragma once

enum eWeatherParam
{
    WEATHER_WIND_DIR_X,
    WEATHER_WIND_DIR_Y,
    WEATHER_WIND_DIR_Z,
    WEATHER_WIND_SPEED,

    // swaying
    WEATHER_WIND_STRENGTH,
    WEATHER_WAVE_AMPLITUDE,
    WEATHER_TURBULENCE,
    WEATHER_GUST_DECAY,
    WEATHER_GUST_POWER,
    WEATHER_WAVE_FREQUENCY,
    WEATHER_BEND_SCALE,
    WEATHER_SWAY_DISTANCE,

    NUM_WEATHER_PARAMS,
};
