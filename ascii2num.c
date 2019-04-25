#include <rtthread.h>

rt_uint32_t str2dec(char* str)
{
    rt_uint32_t i;
    rt_uint32_t result = 0;
    char tmpchar;

    for(i=0; ;++i)
    {
        if (0 == (tmpchar = str[i])) 
        {
            break;
        }
        result *= 10;
        if(tmpchar>='0' && tmpchar<='9')
        {
            result += tmpchar - '0';
        }
        else /* any illegal char return 0 */
        {
            return 0; 
        }
    }
    return result;
}
