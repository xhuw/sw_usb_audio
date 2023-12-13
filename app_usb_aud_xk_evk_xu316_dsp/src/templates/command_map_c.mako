#include "cmd_offset_size.h"

static cmd_t commands[] =
{
%for name, data in cmd_map.items():
    %for field_name, field_data in data.items():
<%
    field_data["size"] = field_data["size"] if "size" in field_data else 1
    cmd_type = {'int32_t': 'TYPE_INT32', 'int16_t': 'TYPE_INT16', 'int8_t': 'TYPE_INT8'}
%>\
 {"${name}_${field_name}", ${cmd_type[field_data["type"]]}, NUM_VALUES_${name}_${field_name}, OFFSET_${name}_${field_name}, SIZEOF_${name}_${field_name}},
    %endfor

%endfor
}
