#include <stdio.h>
#include "${name}.h"

#if OFFSET_GEN
int main()
{
%for field_name, field_data in data.items():
<% field_data["size"] = field_data["size"] if "size" in field_data else 1 %>\
    printf("%s,%d,%d\n", "${field_name}", sizeof(${field_data["type"]}) * ${field_data["size"]}, offsetof(${name}_config_t, ${field_name}));
%endfor

    return 0;
}
#endif
