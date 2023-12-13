#include <platform.h>
#include <stdio.h>

%for name in cmd_map:
#include "${name}.h"
%endfor

#if OFFSET_GEN
int main(int argc, char* argv[])
{
    FILE *fp;
    printf("argc = %d\n", argc);
    if(argc == 2)
    {
        printf("argv = %s\n", argv[1]);
        fp = fopen(argv[1], "w");
    }
    else
    {
        fp = fopen("cmd_offset_size.h", "w");
    }
    fprintf(fp, "#ifndef CMD_OFFSET_SIZE_H\n");
    fprintf(fp, "#define CMD_OFFSET_SIZE_H\n\n");
%for name, data in cmd_map.items():
    %for field_name, field_data in data.items():
<% field_data["size"] = field_data["size"] if "size" in field_data else 1 %>\
    fprintf(fp, "#define OFFSET_${name}_${field_name} %u\n", offsetof(${name}_config_t, ${field_name}));
    fprintf(fp, "#define SIZEOF_${name}_${field_name} %u\n", sizeof(${field_data["type"]}) * ${field_data["size"]});
    fprintf(fp, "#define NUM_VALUES_${name}_${field_name} %u\n", ${field_data["size"]});
    fprintf(fp, "\n");
    %endfor
%endfor
    fprintf(fp, "#endif\n");
    fclose(fp);
    return 0;
}
#endif
