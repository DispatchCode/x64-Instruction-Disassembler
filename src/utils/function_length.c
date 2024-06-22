#include "function_length.h"


vector* instrFlowLength(char *pMemory, enum supported_architecture arch)
{
    int bytes_len = 0;
    uint64_t addr = ((int64_t)pMemory);

    queue *future_paths = queue_init();
    vector *visited = vector_init();

    char *tmp_addr = pMemory;
    while(true)
    {
        struct instruction instr;
        x64id_decode(&instr, arch,tmp_addr,0);

        for (int i = 0; i < instr.length; i++) {
            printf("%X ", instr.instr[i]);
        }
        printf("\n");

        if(instr.op == 0xc3 || instr.op == 0xCC)
        {
            if(queue_empty(future_paths))
            {
                if(instr.op == 0xC3)
                    vector_push_back(visited,addr+instr.length);

                queue_free(future_paths);
                return visited;
            }
            vector_push_back(visited,addr);

            tmp_addr = 0;
            uint64_t tmp = queue_dequeue(future_paths);
            tmp_addr += tmp;
            addr = tmp;
        }
        else
        {
            if(vector_find(visited, addr))
            {
                if(queue_empty(future_paths)) {
                    queue_free(future_paths);
                    return visited;
                }

                tmp_addr = 0;
                uint64_t tmp = queue_dequeue(future_paths);
                tmp_addr += tmp;
                addr = tmp;
            }
            else
            {

                vector_push_back(visited, addr);

                if (instr.jcc_type == JCC_FAR || instr.jcc_type == JCC_SHORT)
                {
                    if (queue_find(future_paths, instr.label) == 0) {
                        queue_enqueue(future_paths, instr.label);
                    }
                }

                bytes_len += instr.length;
                addr += instr.length;
                tmp_addr += instr.length;

                if (instr.jcc_type == JMP_FAR || instr.jcc_type == JMP_SHORT)
                {
                    if (queue_find(future_paths,addr))
                        queue_enqueue(future_paths,addr);

                    tmp_addr = 0;
                    tmp_addr += instr.label;
                    addr = instr.label;
                }
            }
        }
    }
}

int compare(const void * n1, const void * n2)
{
    return (int)( *(uint64_t*)n1 - *(uint64_t*)n2 );
}

pFunctionInfo getFunctionLength(char *buffer, enum supported_architecture arch)
{
    pFunctionInfo f_info = calloc(1,sizeof(functionInfo));
    vector *visited = instrFlowLength(buffer, arch);

    qsort(visited->vect, visited->tos, sizeof(uint64_t), compare);

    uint64_t min = visited->vect[0];
    uint64_t max = visited->vect[visited->tos-1];

    f_info->pVisited = visited;
    f_info->length = (int)(max-min);

    return f_info;
}