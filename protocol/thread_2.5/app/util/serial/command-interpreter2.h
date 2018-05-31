/** @file command-interpreter2.h
 * @addtogroup command_interpreter
 * @brief Processes commands coming from the serial port.
 *
 * <!-- Culprit(s): Richard Kelsey, Matteo Paris -->
 *
 * <!-- Copyright 2004-2007 by Ember Corporation.  All rights reserved.  *80* -->
 */

#ifndef __COMMAND_INTERPRETER2_H__
#define __COMMAND_INTERPRETER2_H__

/** @addtogroup command_interpreter
 * Interpret serial port commands. See command-interpreter2.c for source code.
 *
 * See the following application usage example followed by a brief explanation.
 * @code
 *
 * // Usage: network form 22 0xAB12 -3 { 00 01 02 A3 A4 A5 A6 A7 }
 * void formCommand(void)
 * {
 *   uint8_t channel = emberUnsignedCommandArgument(0);
 *   uint16_t panId  = emberUnsignedCommandArgument(1);
 *   int8_t power   = emberSignedCommandArgument(2);
 *   uint8_t length;
 *   uint8_t *eui64  = emberStringCommandArgument(3, &length);
 *   ...
 *   ... call emberFormNetwork() etc
 *   ...
 * }
 *
 * // The table of network commands.
 * EmberCommandEntry networkCommands[] = {
 *   emberCommandEntryAction("form", formCommand, "uvsh", "Form a network"),
 *   emberCommandEntryAction("join", joinCommand, "uvsh", "join a network"),
 *   ...
 *   emberCommandEntryTerminator()
 * };
 *
 * EmberCommandEntry systemCommands[] = {
 *   emberCommandEntrySubMenu("network",  networkCommands, "Network form/join commands"),
 *   ...
 *   emberCommandEntryTerminator()
 * };
 *
 * // The main command table.  A SubMenu with an empty command name processes
 * // the commands in the subtable as if they were in the main table.
 * EmberCommandEntry emberCommandTable[] = {
 *   emberCommandEntrySubMenu("",  systemCommands, ""),
 *   emberCommandEntryAction("status",    statusCommand,   "Prints application status),
 *   ...
 *   emberCommandEntryTerminator()
 * };
 *
 * void main(void)
 * {
 *    emberCommandReaderInit();
 *    while(0) {
 *      ...
 *      // Process input and print prompt if it returns true.
 *      if (emberProcessCommandInput(serialPort)) {
 *         emberSerialPrintf(1, "%p>", PROMPT);
 *      }
 *      ...
 *    }
 * }
 * @endcode
 *
 * -# Applications specify the commands that can be interpreted
 *    by defining the emberCommandTable array of type ::EmberCommandEntry.
 *    The table includes the following information for each command:
 *   -# The full command name.
 *   -# Your application's function name that implements the command.
 *   -# An ::EmberCommandEntry::argumentTypes string specifies the number and types of arguments
 *      the command accepts.  See ::argumentTypes for details.
 *   -# A description string explains the command.
 *   .
 * -# A default error handler ::emberCommandErrorHandler() is provided to
 *    deal with incorrect command input. Applications may override it.
 * -# The application calls ::emberCommandReaderInit() to initalize, and
 *    ::emberProcessCommandInput() in its main loop.
 * -# Within the application's command functions, use emberXXXCommandArgument()
 *    functions to retrieve command arguments.
 *
 * The command interpreter does extensive processing and validation of the
 * command input before calling the function that implements the command.
 * It checks that the number, type, syntax, and range of all arguments
 * are correct.  It performs any conversions necessary (for example,
 * converting integers and strings input in hexadecimal notation into
 * the corresponding bytes), so that no additional parsing is necessary
 * within command functions.  If there is an error in the command input,
 * ::emberCommandErrorHandler() is called rather than a command function.
 *
 * The command interpreter allows inexact matches of command names.  The
 * input command may be either shorter or longer than the actual command.
 * However, if more than one inexact match is found and there is no exact
 * match, an error of type EMBER_CMD_ERR_NO_SUCH_COMMAND will be generated.
 * To disable this feature, define EMBER_REQUIRE_EXACT_COMMAND_NAME in the
 * application configuration header.
 *
 *@{
 */

/** @name Command Table Settings
 *@{
 */
#ifndef EMBER_MAX_COMMAND_ARGUMENTS
/** The maximum number of arguments a command can have.  A nested command
 * counts as an argument.
 */
#define EMBER_MAX_COMMAND_ARGUMENTS 14
#endif

#ifndef EMBER_COMMAND_BUFFER_LENGTH
#define EMBER_COMMAND_BUFFER_LENGTH 100
#endif

/** The maximum message size for custom commands reserves three bytes for
 * the length (1 bytes) and the custom command identifier (2 bytes).
 */
#define EMBER_CUSTOM_COMMAND_BUFFER_LENGTH (EMBER_COMMAND_BUFFER_LENGTH - 3)

/** Whether or not the command entry structure will include descriptions for
 * the commands and error information. This consumes additional CONST space.
 * By default descriptions are not included.
 */
#if defined(DOXYGEN_SHOULD_SKIP_THIS)
#define EMBER_COMMAND_INTERPRETER_HAS_DESCRIPTION_FIELD
#endif

/** Whether or not error messages are included. This consumes additional
 * CONST space. By default error messages are included.
 */
#if defined(DOXYGEN_SHOULD_SKIP_THIS)
#define EMBER_COMMAND_INTERPRETER_NO_ERROR_MESSAGE
#endif

/** @} // END name group
 */

// The (+ 1) takes into account the leading command.
#define MAX_TOKEN_COUNT (EMBER_MAX_COMMAND_ARGUMENTS + 1)

// The cost of each entry is one pointer on the stack, so there is no
// reason to make this small.
#define MAX_COMMAND_TABLE_NESTING 16

typedef void (*CommandAction)(void);

#ifdef DOXYGEN_SHOULD_SKIP_THIS
/**@brief Command entry for a command table.
 */
typedef struct {
#else
typedef PGM struct {
#endif

  union {
    /** Use letters, digits, and underscores, '_', for the command name.
     * Command names are case-sensitive.
     */
    PGM_P name;

    /** A two-byte identifier for serial communication */
    uint16_t id;
  } identifier;

  /** A reference to a function in the application that implements the
   *  command.
   *  If this entry refers to a nested command, then action field
   *  has to be set to NULL.
   */
  CommandAction action;

  /**
   * In case of normal (non-nested) commands, argumentTypes is a
   * string that specifies the number and types of arguments the
   *  command accepts.  The argument specifiers are:
   *  - u:   one-byte unsigned integer.
   *  - v:   two-byte unsigned integer
   *  - w:   four-byte unsigned integer
   *  - s:   one-byte signed integer
   *  - r:   two-byte signed integer
   *  - q:   four-byte signed integer
   *  - b:   string.  The argument can be entered in ascii by using
   *         quotes, for example: "foo".  Or it may be entered
   *         in hex by using curly braces, for example: { 08 A1 f2 }.
   *         There must be an even number of hex digits, and spaces
   *         are ignored.
   *  - *:   zero or more of the previous type.
   *         If used, this must be the last specifier.
   *  - ?:   Unknown number of arguments. If used this must be the only
   *         character. This means, that command interpreter will not
   *         perform any validation of arguments, and will call the
   *         action directly, trusting it that it will handle with
   *         whatever arguments are passed in.
   *  Integer arguments can be either decimal or hexadecimal.
   *  A 0x prefix indicates a hexadecimal integer.  Example: 0x3ed.
   *
   *  In case of a nested command (action is NULL), then argumentTypes will
   *  contain a pointer to the nested commands.
   */
  PGM_P argumentTypes;

  /** A description of the command.
   */

#ifdef EMBER_COMMAND_INTERPRETER_HAS_DESCRIPTION_FIELD
  PGM_P description;
#endif
} EmberCommandEntry;

// deprecated names
  #define emberBinaryCommand        emberBinaryCommandEntryAction
  #define emberBinaryNestedCommand  emberBinaryCommandEntrySubMenu

#ifdef EMBER_COMMAND_INTERPRETER_HAS_DESCRIPTION_FIELD
  #define emberBinaryCommandEntryAction(identifier, command, arguments, description) \
  { { (PGM_P)identifier }, command, arguments, description }
  #define emberBinaryCommandEntrySubMenu(identifier, nestedCommands, description) \
  { { (PGM_P)identifier }, NULL, description, nestedCommands }
  #define emberCommandEntryAction(name, command, arguments, description) \
  { { name }, command, arguments, description }
  #define emberCommandEntrySubMenu(name, nestedCommands, description) \
  { { name }, NULL, (PGM_P)nestedCommands, description }
  #define emberCommandEntryTerminator() \
  { { NULL }, NULL, NULL, NULL }
#else // EMBER_COMMAND_INTERPRETER_HAS_DESCRIPTION_FIELD
  #define emberBinaryCommandEntryAction(identifier, command, arguments, description) \
  { { (PGM_P)identifier }, command, arguments }
  #define emberBinaryCommandEntrySubMenu(identifier, nestedCommands, description) \
  { { (PGM_P)identifier }, NULL, NULL, nestedCommands }
  #define emberCommandEntryAction(name, command, arguments, description) \
  { { name }, command, arguments }
  #define emberCommandEntrySubMenu(name, nestedCommands, description) \
  { { name }, NULL, (PGM_P)nestedCommands }
  #define emberCommandEntryTerminator() \
  { { NULL }, NULL, NULL }
#endif // EMBER_COMMAND_INTERPRETER_HAS_DESCRIPTION_FIELD

// deprecated names
#define emberCommand        emberCommandEntryAction
#define emberNestedCommand  emberCommandEntrySubMenu

extern EmberCommandEntry emberCommandTable[];

#ifdef DOXYGEN_SHOULD_SKIP_THIS
/** @brief Command error states.
 *
 * If you change this list, ensure you also change the strings that describe
 * these errors in the array emberCommandErrorNames[] in
 * command-interpreter2-error.c.
 */
enum EmberCommandStatus
#else
typedef uint8_t EmberCommandStatus;
enum
#endif
{
  EMBER_CMD_SUCCESS,
  EMBER_CMD_ERR_PORT_PROBLEM,
  EMBER_CMD_ERR_NO_SUCH_COMMAND,
  EMBER_CMD_ERR_WRONG_NUMBER_OF_ARGUMENTS,
  EMBER_CMD_ERR_ARGUMENT_OUT_OF_RANGE,
  EMBER_CMD_ERR_ARGUMENT_SYNTAX_ERROR,
  EMBER_CMD_ERR_STRING_TOO_LONG,
  EMBER_CMD_ERR_INVALID_ARGUMENT_TYPE
};

/** @name Functions to Retrieve Arguments
 * Use the following functions in your functions that process commands to
 * retrieve arguments from the command interpreter.
 * These functions pull out unsigned integers, signed integers, and strings,
 * and hex strings.  Index 0 is the first command argument.
 *@{
 */

/** This function returns the number of arguments for the current command. */
uint8_t emberCommandArgumentCount(void);

/** This function retrieves unsigned integer arguments. */
uint32_t emberUnsignedCommandArgument(uint8_t argNum);

/** This function retrieves signed integer arguments. */
int32_t emberSignedCommandArgument(uint8_t argNum);

/** This function retrieves quoted string or hexadecimal string arguments.
 * Hexadecimal strings have already been converted into binary.
 * ASCII strings have been null terminated.  The null terminator
 * is not included in the returned length argument.
 * To retrieve the name of the command, use an argNum of -1.
 * For example, to retrieve the first character of the command, do:
 * uint8_t firstChar = emberStringCommandArgument(-1, NULL)[0].
 * If the command is nested, an index of -2, -3, will work to retrieve
 * the higher level command names.
 * Note that [-1] only returns the text entered. If an abbreviated command
 * name is entered, only the text entered will be returned with [-1].
 */
uint8_t *emberStringCommandArgument(int8_t argNum, uint8_t *length);

uint8_t *emberLongStringCommandArgument(int8_t argNum, uint16_t *length);

// emberStringCommandArgument(-1, NULL) returns a pointer to the string
// with the command line name, but the caller must use the length to print it;
// it's not NULL-terminated, unless it happens to be the only text entered
// on the command line.
// emberCommandName() returns a NULL-terminated command name, which is easier
// than requiring every caller to know about the command name length.
const char *emberCommandName(void);

/** This function copies the string argument to the given destination up to maxLength.
 * If the argument length is nonzero but less than maxLength
 * and leftPad is true, leading zeroes are prepended to bring the
 * total length of the target up to maxLength.  If the argument
 * is longer than the maxLength, it is truncated to maxLength.
 * This function returns the minimum of the argument length and maxLength.
 * ASCII strings are null terminated, but the null terminator
 * is not included in the returned length.
 *
 * This function is commonly used for reading in hexadecimal strings
 * such as EUI64 or key data and left padding them with zeroes.
 * See ::emberGetKeyArgument and ::emberGetEui64Argument for
 * convenience macros for this purpose.
 */
uint8_t emberGetStringArgument(int8_t argNum,
                               uint8_t *destination,
                               uint8_t maxLength,
                               bool leftPad);

/** This function parses and returns, via target, an IP address at the provided index.
 * Returns true if an IP address was successfully parsed
 * Return false otherwise
 */
bool emberGetIpArgument(uint8_t index, uint8_t *target);

/** @brief This function parses an IPv6 address in a CLI command.
 *
 * @param index An index of the IPv6 address to parse
 * @param dst the ::EmberIpv6Address a location where the address will be written
 *
 * @return true if the IPv6 address was successfully parsed.
 */
bool emberGetIpv6AddressArgument(uint8_t index, EmberIpv6Address *dst);

/** @brief This function parses an IPv6 prefix in a CLI command.
 *
 * @param index An index of the IPv6 prefix to parse
 * @param dst the ::EmberIpv6Address a location where the address will be written
 * @param prefixBits the number of prefix bits in the string
 *
 * @return true if the IPv6 prefix was successfully parsed.
 */
bool emberGetIpv6PrefixArgument(uint8_t index,
                                EmberIpv6Address *dst,
                                uint8_t *dstPrefixBits);

/** A convenience macro for copying security key arguments to an
 * EmberKeyData pointer.
 */
#define emberGetKeyArgument(index, keyDataPointer)            \
  (emberGetStringArgument((index),                            \
                          emberKeyContents((keyDataPointer)), \
                          EMBER_ENCRYPTION_KEY_SIZE,          \
                          true))

/** This function copies the EUI64 argument to the given EmberEui64 destination, reversing the
 * bytes. EUI64's are stored little endian so reversing the bytes means they are
 * big endian in the input command string.
 */
#define emberGetEui64Argument(index, eui64) \
  emberGetExtendedPanIdArgument(index, (eui64)->bytes)

/** This function copies the extended PAN ID argument to the given destination, reversing the
 * bytes. Extended PAN ids are stored little endian so reversing the bytes
 * means they are big endian in the input command string.
 */
void emberGetExtendedPanIdArgument(int8_t index, uint8_t *extendedPanId);

/** @} // END name group
 */

void emberCommandErrorHandler(EmberCommandStatus status,
                              EmberCommandEntry *command);
void emberPrintCommandUsage(EmberCommandEntry *entry);
void emberPrintCommandUsageNotes(void);
void emberPrintCommandTable(void);
void emberCommandClearBuffer(void);

/** @brief This function nitializes the command interpreter.
 */
void emberCommandReaderInit(void);

/** @brief This function processes the given string as a command.
 */
bool emberProcessCommandString(const uint8_t *input, uint16_t sizeOrPort);

/** @brief This function processes input coming in on the given serial port.
 * @return true if an end of line character was read.
 * If the application uses a command line prompt,
 * this indicates it is time to print the prompt.
 * @code
 * void emberProcessCommandInput(uint8_t port);
 * @endcode
 */
#define emberProcessCommandInput(port) \
  emberProcessCommandString(NULL, port)

/** @brief For use when declaring a separate command streams.  The fields
 * are not accessed directly by the application.
 */
typedef struct {
  // Finite-state machine's current state.
  uint8_t state;

  // The command line is stored in this buffer.
  // Spaces and trailing '"' and '}' characters are removed,
  // and hex strings are converted to bytes.
  uint8_t buffer[EMBER_COMMAND_BUFFER_LENGTH];

  // Indices of the tokens (command(s) and arguments) in the above buffer.
  // The (+ 1) lets us store the ending index.
  uint16_t tokenIndices[MAX_TOKEN_COUNT + 1];

  // The number of tokens read in, including the command(s).
  uint8_t tokenCount;

  // Used while reading in the command line.
  uint16_t index;

  // First error found in this command.
  uint8_t error;

  // Storage for reading in a hex string. A value of 0xFF means unused.
  uint8_t hexHighNibble;

  // The token number of the first true argument after possible nested commands.
  uint8_t argOffset;

  // Remember the previous character seen by emberProcessCommandString()
  // to ignore an LF following a CR.
  uint8_t previousCharacter;

  // Unlike the above values, this does not change as commands are read.
  uint8_t defaultBase;

  // Keep a pointer to the current command the user ran.
  EmberCommandEntry *currentCommand;
} EmberCommandState;

/** @brief This function must be called to initialize a command state before passing it to
 *   emberRunBinaryCommandInterpreter() or emberRunAsciiCommandInterpreter().
 */
void emberInitializeCommandState(EmberCommandState *state);

/** @brief Type of error handlers; the command argument is currently
 *   always NULL.
 */

typedef void EmberCommandErrorHandler (EmberCommandStatus status,
                                       EmberCommandEntry *command);

/** @brief For use to process binary commands when additional different command
 * streams are being used.
 */
bool emberRunBinaryCommandInterpreter(EmberCommandState *state,
                                      EmberCommandEntry *commands,
                                      EmberCommandErrorHandler *errorHandler,
                                      const uint8_t *input,
                                      uint16_t sizeOrPort);

/** @brief For use to process ASCII commands when additional different command
 * streams are being used.
 */
bool emberRunAsciiCommandInterpreter(EmberCommandState *state,
                                     EmberCommandEntry *commands,
                                     EmberCommandErrorHandler *errorHandler,
                                     const uint8_t *input,
                                     uint16_t sizeOrPort);

// This function declaration really belongs in command-interpreter2-util.h, but
// I'm putting it here in command-interpreter2.h so that znet sees it as well
// zip. (znet doesn't have command-interpreter2-util.*)
void emberCommandReaderSetDefaultBase(uint8_t base);
/** @} // END addtogroup
 */

#endif
