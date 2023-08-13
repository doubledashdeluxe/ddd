#pragma once

#pragma section RX "replacements"

#define REPLACE __declspec(section "replacements")
#define REPLACED(function) thunk_replaced_##function
