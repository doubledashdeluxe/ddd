#pragma once

#ifdef __CWCC__
#pragma section RX "replacements"

#define REPLACE __declspec(section "replacements")
#else
#define REPLACE
#endif
#define REPLACED(function) thunk_replaced_##function
