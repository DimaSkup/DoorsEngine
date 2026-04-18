// =================================================================================
// Filename: Log.cpp
// =================================================================================
#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#pragma warning (disable : 4996)


char g_String    [LOG_BUF_SIZE]{ '\0' };         // global buffer for characters

static bool               s_IsInit = false;      // a flag to define if the logger is initialized already

static FILE*              s_pLogFile = nullptr;  // a static descriptor of the log file
static LogMsgsCharsBuffer s_LogMsgsCharsBuf;     // a static buffer for log messages chars (is used to prevent dynamic allocations)
static LogStorage         s_LogStorage;


//---------------------------------------------------------
// Desc:   set console color to some particular by input code
// Args:   - keyColor: key code to change color
//---------------------------------------------------------
void SetConsoleColor(const char* keyColor)
{
    printf("%s", keyColor);
}

//-----------------------------------------------------
// return relative path from the project root
//-----------------------------------------------------
void GetPathFromProjRoot(const char* fullPath, char* outPath)
{
    if ((!fullPath) || (fullPath[0] == '\0'))
    {
        LogErr(LOG, "empty path");
        return;
    }

    if (!outPath)
    {
        LogErr(LOG, "in-out path == nullptr");
        return;
    }

    const char* found = strstr(fullPath, "DoorsEngine\\");

    // if we found the substring we copy all the text after "DoorsEngine\"
    if (found != nullptr)
        strcpy(outPath, found + strlen("DoorsEngine\\"));

    else
        outPath[0] = '\0';
}

//---------------------------------------------------------
// Desc:   add a new message into the log storage (simply log history);
//         this storage is used for printing log messages into
//         the editors GUI
// Args:   - msg:  a string with text message
//         - type: what kind of log we want to add
//---------------------------------------------------------
void AddMsgIntoLogStorage(const char* msg, const eLogType type)
{
    if (!msg || msg[0] == '\0')
    {
        printf("%s%s%s\n", RED, "(log.cpp) AddMsgIntoLogStorage: input msg is empty!", RESET);
        return;
    }

    // dirty HACK: we need to create a log file and init memory for logs
    // but some global instances of engine calls log functions before logger initialization
    if (!s_IsInit)
    {
        InitLogger("log.txt");
        s_IsInit = true;
    }

    int& numLogs = s_LogStorage.numLogs;
    if (numLogs >= LOG_STORAGE_SIZE)
    {
        printf("%s can't put a new log msg into the log storage:"
               "storage overflow (its limit: %d logs)%s\n", RED, (int)LOG_STORAGE_SIZE, RESET);
        return;
    }

    LogMsgsCharsBuffer& charsBuf = s_LogMsgsCharsBuf;
    if (charsBuf.buf == nullptr)
        return;

    LogMessage& newLog = s_LogStorage.logs[numLogs];
    newLog.size = (int)strlen(msg) + 1;          // +1 because of null-terminator

    bool canWriteNewLog = (charsBuf.currSize + newLog.size >= charsBuf.maxSize);
    if (canWriteNewLog)
    {
        printf("%s can't put a new log msg into the log storage: "
               "if we do there will be a buffer overflow%s\n", RED, RESET);
        return;
    }

    newLog.startIdx = charsBuf.currSize;
    newLog.type = type;

    strcpy(charsBuf.buf + newLog.startIdx, msg);

    charsBuf.currSize += newLog.size;
    ++numLogs;
}

//---------------------------------------------------------
// Desc:   a helper for printing messages into the console
//         and into the logger file
//---------------------------------------------------------
void PrintHelper(
    const char* fileName,        // path to the caller file
    const char* funcName,        // name of the caller function/method
    const char* text,            // log message content
    const int codeLine,          // line of code where logger was called
    const eLogType type)         // a type of the log message
{
    const char* fmt = "[%05ld] %s %s: %s() (line: %d): %s\n";
    const time_t t = clock();

    const char* levels[] =
    {
        "",          // simple message
        "DEBUG:",
        "ERROR:",
        "",          // formatted message
        "FATAL:",
    };

    // create a final string
    char buf[1024];
    snprintf(buf, sizeof(buf), fmt, t, levels[type], fileName, funcName, codeLine, text);

    // print a message into the console and log-file
    printf(buf);
    AddMsgIntoLogStorage(buf, type);

    if (s_pLogFile)
        fprintf(s_pLogFile, buf);
}

//---------------------------------------------------------
// Desc:   print a message into the console and log-file
// Args:   - msg:  text content of the log message
//         - type: a type of this log message
//---------------------------------------------------------
void PrintHelper(const char* msg, const eLogType type)
{
    printf("%s\n", msg);
    AddMsgIntoLogStorage(msg, type);

    if (s_pLogFile)
        fprintf(s_pLogFile, "%s\n", msg);
}

//---------------------------------------------------------
// Desc:   create a logger file into which we will write messages
// Args:   - filename:  path to logger file relatively to the working directory
// Ret:    1 if everything is OK, and 0 if something went wrong
//---------------------------------------------------------
int InitLogger(const char* filename)
{
    if (s_IsInit)
        return 1;

    if (!filename || filename[0] == '\0')
    {
        printf("%s Logger ERROR (%s): input name for the log file is empty!%s\n", RED, __func__, RESET);
        return 0;
    }

#if _WIN32
    // to make possible console color chanding
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    GetConsoleMode(hConsole, &mode);
    SetConsoleMode(hConsole, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#endif

    if ((s_pLogFile = fopen(filename, "w")) == NULL)
    {
        printf("%sInitLogger(): can't initialize the logger %s\n", RED, RESET);
        return 0;   // false
    }

    // alloc memory for a log messages chars buffer
    s_LogMsgsCharsBuf.buf = new char[s_LogMsgsCharsBuf.maxSize]{ '\0' };

    // generate and store a message about successful initialization
    time_t rawTime;
    struct tm* info = NULL;
    char buffer[80];

    memset(buffer, 0, 80);
    time(&rawTime);
    info = localtime(&rawTime);
    strftime(buffer, 80, "%x - %I:%M%p", info);

    printf("%s| the log file is created!\n", buffer);
    printf("-------------------------\n\n");

    fprintf(s_pLogFile, "%s| the log file is created!\n", buffer);
    fprintf(s_pLogFile, "-------------------------\n\n");

    return 1;   // true
}

//---------------------------------------------------------
// Desc:   print msg about closing of the log file and close it
//---------------------------------------------------------
void CloseLogger()
{
    // release the memory from log messages chars buffer
    if (s_LogMsgsCharsBuf.buf)
    {
        delete[] s_LogMsgsCharsBuf.buf;
        s_LogMsgsCharsBuf.buf = nullptr;
    }

    return;

#if 0
    time_t rawTime;
    struct tm* info = NULL;
    char buffer[80];

    memset(buffer, 0, 80);
    time(&rawTime);
    info = localtime(&rawTime);
    strftime(buffer, 80, "%x -%I:%M%p", info);

    fprintf(s_pLogFile, "\n--------------------------------\n");
    fprintf(s_pLogFile, "%s| this is the end, my only friend, the end\n", buffer);

    fclose(s_pLogFile);
#endif
}

//---------------------------------------------------------
// Desc:  get a ptr to the logs storage (logs history)
//---------------------------------------------------------
const LogStorage* GetLogStorage()
{
    return &s_LogStorage;
}

//---------------------------------------------------------
// Desc:   return the current number of all the log messages
//---------------------------------------------------------
int GetNumLogMsgs()
{
    return s_LogStorage.numLogs;
}

//---------------------------------------------------------
// Desc:   return a ptr to the beginning of the log message by index 
//---------------------------------------------------------
const char* GetLogTextByIdx(const int idx)
{
    if (idx >= s_LogStorage.numLogs)
    {
        printf("%s (log.cpp) GetLogTextByIdx: input idx is too big! (idx_value: %d; max: %d); return NULL %s\n", RED, idx, s_LogStorage.numLogs, RESET);
        return nullptr;
    }

    return s_LogMsgsCharsBuf.buf + s_LogStorage.logs[idx].startIdx;
}

//---------------------------------------------------------
// Desc:   return a type of the log message by index
//---------------------------------------------------------
eLogType GetLogTypeByIdx(const int idx)
{
    if (idx >= s_LogStorage.numLogs)
    {
        printf("%s (log.cpp) GetLogTextByIdx: input idx is too big! (idx_value: %d; max: %d); return LOG_TYPE_MESSAGE %s\n", RED, idx, s_LogStorage.numLogs, RESET);
        return LOG_TYPE_MESSAGE;
    }
 
    return s_LogStorage.logs[idx].type;
}

//---------------------------------------------------------
// Desc:   print a usual message into console but without
//         info about the caller
// Args:   - format:   format string for variadic aruments
//         - ...:      variadic arguments
//---------------------------------------------------------
void LogMsg(const char* format, ...)
{
    const char* fmt = "[%05ld] %s";
    const time_t t = clock();

    va_list args;
    va_start(args, format);

    // make a string with input log-message
    char buf[512];
    vsnprintf(buf, sizeof(buf), format, args);

    // create a final string and print it
    char finalStr[512];
    snprintf(finalStr, sizeof(finalStr), fmt, t, buf);
    PrintHelper(finalStr, LOG_TYPE_FORMATTED);

    va_end(args);
}

//---------------------------------------------------------
// Desc:   print a usual message into console
//---------------------------------------------------------
void LogMsg(
    const char* fullFilePath,     // path to the caller file
    const char* funcName,         // name of the caller function
    const int codeLine,           // line of code where logger was called
    const char* format,           // format string for variadic arguments
    ...)                          // variadic arguments
{
    va_list args;
    va_start(args, format);

    char buf[256];
    char fileName[128]{'\0'};

    // make a string with input log-message
    vsnprintf(buf, sizeof(buf), format, args);

    // get a relative path to the caller's file
    GetPathFromProjRoot(fullFilePath, fileName);

    // print a message into the console and log file
    SetConsoleColor(GREEN);
    PrintHelper(fileName, funcName, buf, codeLine, LOG_TYPE_MESSAGE);
    SetConsoleColor(RESET);

    va_end(args);
}

//---------------------------------------------------------
// Desc:   print a debug message into console
//---------------------------------------------------------
void LogDbg(
    const char* fullFilePath,     // path to the caller file
    const char* funcName,         // name of the caller function
    const int codeLine,           // line of code where logger was called
    const char* format,           // format string for variadic arguments
    ...)                          // variadic arguments
{
    va_list args;
    va_start(args, format);

    char buf[256];
    char fileName[128]{'\0'};

    // make a string with input log-message
    vsnprintf(buf, sizeof(buf), format, args);

    // get a relative path to the caller's file
    GetPathFromProjRoot(fullFilePath, fileName);

    // print a message into the console and log file
    SetConsoleColor(RESET);
    PrintHelper(fileName, funcName, buf, codeLine, LOG_TYPE_DEBUG);

    va_end(args);
}

//---------------------------------------------------------
// Desc:   print an error message into console
//---------------------------------------------------------
void LogErr(
    const char* fullFilePath,     // path to the caller file
    const char* funcName,         // name of the caller function
    const int codeLine,           // line of code where logger was called
    const char* format,           // format string for variadic arguments
    ...)                          // variadic arguments
{
    va_list args;
    va_start(args, format);

    char buf[256];
    char fileName[256]{'\0'};
    char finalStr[512];

    // make a string with input log-message
    vsnprintf(buf, sizeof(buf), format, args);

    // get a relative path to the caller's file
    GetPathFromProjRoot(fullFilePath, fileName);

    const time_t time = clock();
    const char*  fmt =
        "[%05ld] ERROR:\n"
        "FILE:  %s\n"
        "FUNC:  %s()\n"
        "LINE:  %d\n"
        "MSG:   %s\n";

    snprintf(
        finalStr,
        sizeof(finalStr),
        fmt,
        time,
        fileName,                               // relative path to the caller file
        funcName,                               // a function name where we called this log-function
        codeLine,                               // at what line
        buf);

    // print a message into the console and log file
    SetConsoleColor(RED);
    PrintHelper(finalStr, LOG_TYPE_ERROR);
    SetConsoleColor(RESET);

    va_end(args);
}

//---------------------------------------------------------
// Desc:   print a FATAL error message into console and crash the app
//--------------------------------------------------------
void LogFatal(
    const char* fullFilePath,     // path to the caller file
    const char* funcName,         // name of the caller function
    const int codeLine,           // line of code where logger was called
    const char* format,           // format string for variadic arguments
    ...)                          // variadic arguments
{
    va_list args;
    va_start(args, format);

    char buf[256];
    char fileName[256]{'\0'};
    char finalStr[512];

    // make a string with input log-message
    vsnprintf(buf, sizeof(buf), format, args);

    // get a relative path to the caller's file
    GetPathFromProjRoot(fullFilePath, fileName);

    const time_t time = clock();
    const char* fmt =
        "[%05ld] FATAL:\n"
        "FILE:  %s\n"
        "FUNC:  %s()\n"
        "LINE:  %d\n"
        "MSG:   %s\n";

    snprintf(
        finalStr,
        sizeof(finalStr),
        fmt,
        time,
        fileName,                               // relative path to the caller file
        funcName,                               // a function name where we called this log-function
        codeLine,                               // at what line
        buf);

    // print a message into the console and log file
    SetConsoleColor(RED);
    PrintHelper(finalStr, LOG_TYPE_FATAL);

    va_end(args);

    // crash the fucking app
    exit(-1);
}
