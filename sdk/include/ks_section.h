


#pragma  once

#include "ks_datatypes.h"

#define CODE_SECTION_START         __text_start__
#define CODE_SECTION_END           __text_end__

extern const int CODE_SECTION_START;
extern const int CODE_SECTION_END;


#define DATA_SECTION_START         __data_start__
#define DATA_SECTION_END           __data_end__

extern const int DATA_SECTION_START;
extern const int DATA_SECTION_END;


#define BSS_SECTION_START         __bss_start__
#define BSS_SECTION_END           __bss_end__


#define AP_SECTION_END 		__end__



extern const int BSS_SECTION_START;
extern const int BSS_SECTION_END;
extern const int AP_SECTION_END;

