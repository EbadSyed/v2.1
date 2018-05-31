// Copyright 2015 Silicon Laboratories, Inc.                                *80*

#ifndef ZCLIP_STRUCT_H
#define ZCLIP_STRUCT_H

// Needed:
//  variable length strings - decoded point to strings in encoded data
//  substructs
//  arrays
//  storage allocation
//  decoding size determination
//  parse failure reporting

//----------------------------------------------------------------
// The types of values that can be found in structs.

enum {
  EMBER_ZCLIP_TYPE_BOOLEAN,

  EMBER_ZCLIP_TYPE_INTEGER,
  EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER,

  EMBER_ZCLIP_TYPE_BINARY,
  EMBER_ZCLIP_TYPE_FIXED_LENGTH_BINARY,

  EMBER_ZCLIP_TYPE_STRING,
  EMBER_ZCLIP_TYPE_MAX_LENGTH_STRING,

  // These are used for struct-based encoding/decoding from/to an EmberZclStringType_t substructure.
  EMBER_ZCLIP_TYPE_UINT8_LENGTH_STRING,
  EMBER_ZCLIP_TYPE_UINT16_LENGTH_STRING,

  // Additional types for non-struct internal use.
  //
  // These allow direct encoding/decoding from/to a buffer containing a length-prefixed
  // binary or text string.
  EMBER_ZCLIP_TYPE_UINT8_LENGTH_PREFIXED_BINARY,
  EMBER_ZCLIP_TYPE_UINT16_LENGTH_PREFIXED_BINARY,
  EMBER_ZCLIP_TYPE_UINT8_LENGTH_PREFIXED_STRING,
  EMBER_ZCLIP_TYPE_UINT16_LENGTH_PREFIXED_STRING,

  // Markers that are not really value types.
  EMBER_ZCLIP_START_MARKER,
  EMBER_ZCLIP_ARRAY_MARKER
};

// Top bit of EMBER_ZCLIP_TYPE (unused by above enum) is used to
// indicate that the type is a mandatory field in the structure.
#define EMBER_ZCLIP_TYPE_MODIFIER_MANDATORY  0x80

//----------------------------------------------------------------
// Information about structs is encoded in arrays of uint32_t.  This makes
// it easy to define macros for defining different kinds of fields.

typedef unsigned long ZclipStructSpec;

// EMBER_ZCLIP_STRUCT is defined to the name of the current struct type.
//
// A simple struct definition and its encoding look like:
//
// typedef struct {
//   bool     field0;
//   uint16_t field1;
//   uint8_t  field2;
// } SomeStruct;
//
// #define EMBER_ZCLIP_STRUCT ZclipTestStruct
//
// unsigned long zclipTestStructSpec[] = {
//   EMBER_ZCLIP_OBJECT(sizeof(EMBER_ZCLIP_STRUCT),
//                      3,         // fieldCount
//                      NULL),     // names
//   EMBER_ZCLIP_FIELD_INDEXED(EMBER_ZCLIP_TYPE_BOOLEAN,          field0),
//   EMBER_ZCLIP_FIELD_INDEXED(EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER, field1),
//   EMBER_ZCLIP_FIELD_NAMED(  EMBER_ZCLIP_TYPE_UNSIGNED_INTEGER, field2, "e")
// };
//
// #undef EMBER_ZCLIP_STRUCT
//
// This struct will be encoded to or decoded from the CBOR map
// {0: <field0>, 1: <field1>, "e": <field2>}.
//
// Indexed fields must appear first in the spec and are automatically numbered
// starting from 0.  Named fields must appear after any indexed fields.
//
//----------------------------------------------------------------
// Macros for encoding struct information

// The offset of a field within EMBER_ZCLIP_STRUCT.
#define ZCLIP_FIELD_OFFSET(field) \
  ((uint16_t) (long) &(((EMBER_ZCLIP_STRUCT *) NULL)->field))

// The size of a field within EMBER_ZCLIP_STRUCT.
#define ZCLIP_FIELD_SIZE(field) \
  (sizeof(((EMBER_ZCLIP_STRUCT *)NULL)->field))

// A field spec is a single uint32_t:
//  [offset:16][size:8][type:8]
// where 'offset' is the offset of the field within the struct.

#define EMBER_ZCLIP_FIELD_SPEC(type, size, offset) \
  ((type) | ((size) << 8) | ((offset) << 16))

#define EMBER_ZCLIP_FIELD_NAMED(type, field, name)   \
  EMBER_ZCLIP_FIELD_SPEC(type,                       \
                         ZCLIP_FIELD_SIZE(field),    \
                         ZCLIP_FIELD_OFFSET(field)), \
  (ZclipStructSpec)(name)

#define EMBER_ZCLIP_FIELD_NAMED_MANDATORY(type, field, name)            \
  EMBER_ZCLIP_FIELD_NAMED((type) | EMBER_ZCLIP_TYPE_MODIFIER_MANDATORY, \
                          field,                                        \
                          name)

#define EMBER_ZCLIP_FIELD_INDEXED(type, field) \
  EMBER_ZCLIP_FIELD_NAMED(type, field, NULL)

// For backward compatibility.
#define EMBER_ZCLIP_FIELD EMBER_ZCLIP_FIELD_INDEXED

// The first few uint32_ts encode information about the struct itself.
// The start marker is for safety - we don't really need it.  The other
// values are the number of fields, the size of the struct, and a string
// holding the names of the struct and its fields (not yet implemented).

#define EMBER_ZCLIP_OBJECT(size, fieldCount, names)                \
  (EMBER_ZCLIP_START_MARKER | ((fieldCount) << 8) | (size) << 16), \
  (ZclipStructSpec) (names)

//----------------------------------------------------------------
// To avoid having too much code depend on the exact encoding, information
// about structs and fields can be expanded.

typedef struct {
  const ZclipStructSpec *spec;
  uint16_t size;                // size of the struct in bytes
  uint8_t  fieldCount;          // num of fields
  // These two fields will be needed if we add subtyping.
  // ZclipStructData *parent;
  // uint8_t  ancestorElements; // number of elements to be found higher up

  // Values used for iterating through the fields.
  uint16_t fieldIndex;          // the index of the next field in the struct
  const ZclipStructSpec *next;  // next field to be processed
} ZclipStructData;

bool emExpandZclipStructData(const ZclipStructSpec *spec,
                             ZclipStructData *structData);

typedef struct {
  uint8_t valueType;
  uint8_t valueSize;              // 0xFF for strings and substructs
  // uint8_t optionIndex;            // 0xFF for required,
  uint16_t valueOffset;
  bool valueMandatory;            // if true, the field must be present in the structure being decoded.

  // ZclipStructData *substructType; // for substructs

  // uint16_t minCount;              // for arrays of values
  // uint16_t maxCount;
  // uint16_t countOffset;
  const char *name;
} ZclipFieldData;

void emResetZclipFieldData(ZclipStructData *structData);
bool emZclipFieldDataFinished(ZclipStructData *structData);
void emGetNextZclipFieldData(ZclipStructData *structData,
                             ZclipFieldData *fieldData);

//----------------------------------------------------------------
// Utilities for reading and writing integer fields.  These should probably
// go somewhere else.

uint32_t emFetchInt32uValue(const uint8_t *valueLoc, uint16_t valueSize);
int32_t emFetchInt32sValue(const uint8_t *valueLoc, uint16_t valueSize);
void emStoreInt32sValue(uint8_t* valueLoc, int32_t value, uint8_t valueSize);
void emStoreInt32uValue(uint8_t* valueLoc, uint32_t value, uint8_t valueSize);

#endif // ZCLIP_STRUCT_H
