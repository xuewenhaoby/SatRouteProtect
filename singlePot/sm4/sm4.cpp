    #include "table.h"
     
    void four_uCh2uLong(unsigned char *in , unsigned long *out);
    void uLong2four_uCh(unsigned long in , unsigned char *out);
    unsigned long move(unsigned long data , int length);
    unsigned long func_key(unsigned long input);
    unsigned long func_data(unsigned long input);
    int proc_enc_dec(char *type , unsigned char *key , unsigned char *input , unsigned char *output);
    void print_hex(unsigned char *data , int len);
    int hex_str2bytes(unsigned char *in ,  unsigned char *out , int *outLen);
     
    //4字节无符号数组转无符号long型
    void four_uCh2uLong(unsigned char *in , unsigned long *out)
    {
        int i = 0;
        *out = 0;
        for(i = 0 ; i < 4 ; i++)
            *out = ((unsigned long)in[i] << (24-i*8)) ^ *out;
    }
     
    //无符号long型转4字节无符号数组
    void uLong2four_uCh(unsigned long in , unsigned char *out)
    {
        int i = 0;
        //从32位unsigned long的高位开始取
        for(i = 0 ; i < 4 ; i++)
            *(out+i) = (unsigned char )(in >> (24-i*8));
    }
     
    //左移，保留丢弃位放置尾部
    unsigned long move(unsigned long data , int length)
    {
        unsigned long result = 0;
        result = (data << length) ^ (data >> (32-length));
     
        return result;
    }
     
    //秘钥处理函数
    unsigned long func_key(unsigned long input)
    {
        int i = 0;
        unsigned long ulTmp = 0;
        unsigned char ucIndexList[4] = {0};
        unsigned char ucSboxValueList[4] = {0};
        uLong2four_uCh(input , ucIndexList);
        for(i = 0 ; i < 4 ; i++)
        {
            ucSboxValueList[i] = TBL_SBOX[ucIndexList[i]];
        }
        four_uCh2uLong(ucSboxValueList , &ulTmp);
        ulTmp = ulTmp ^ move(ulTmp , 13) ^ move(ulTmp , 23);
     
        return ulTmp;
    }
     
    //加解密数据处理函数
    unsigned long func_data(unsigned long input)
    {
        int i = 0;
        unsigned long ulTmp = 0;
        unsigned char ucIndexList[4] = {0};
        unsigned char ucSboxValueList[4] = {0};
        uLong2four_uCh(input , ucIndexList);
        for(i = 0 ; i < 4 ; i++)
        {
            ucSboxValueList[i] = TBL_SBOX[ucIndexList[i]];
        }
        four_uCh2uLong(ucSboxValueList , &ulTmp);
        ulTmp = ulTmp ^ move(ulTmp , 2) ^ move(ulTmp , 10) ^ move(ulTmp , 18) ^ move(ulTmp , 24);
     
        return ulTmp;
    }
     
    //加解密函数
    int proc_enc_dec(char *type , unsigned char *key , unsigned char *input , unsigned char *output)
    {
        int i = 0;
        unsigned long ulKeyTmpList[4] = {0};
        unsigned long ulKeyList[36] = {0};
        unsigned long ulDataList[36] = {0};
     
        /*开始生成子秘钥*/
        four_uCh2uLong(key , &(ulKeyTmpList[0]));
        four_uCh2uLong(key+4 , &(ulKeyTmpList[1]));
        four_uCh2uLong(key+8 , &(ulKeyTmpList[2]));
        four_uCh2uLong(key+12 , &(ulKeyTmpList[3]));
     
        ulKeyList[0] = ulKeyTmpList[0] ^ TBL_SYS_PARAMS[0];
        ulKeyList[1] = ulKeyTmpList[1] ^ TBL_SYS_PARAMS[1];
        ulKeyList[2] = ulKeyTmpList[2] ^ TBL_SYS_PARAMS[2];
        ulKeyList[3] = ulKeyTmpList[3] ^ TBL_SYS_PARAMS[3];
        
        for(i = 0 ; i < 32 ; i++)
        {
            //5-36为32个子秘钥
            ulKeyList[i+4] = ulKeyList[i] ^ func_key(ulKeyList[i+1] ^ ulKeyList[i+2] ^ ulKeyList[i+3] ^ TBL_FIX_PARAMS[i]);
        }
        /*生成32轮32位长子秘钥结束*/
     
     
        /*开始处理加解密数据*/
        four_uCh2uLong(input , &(ulDataList[0]));
        four_uCh2uLong(input+4 , &(ulDataList[1]));
        four_uCh2uLong(input+8 , &(ulDataList[2]));
        four_uCh2uLong(input+12 , &(ulDataList[3]));
     
        if(strcmp(type , "1") == 0)
        {
            //加密
            for(i = 0 ; i < 32 ; i++)
            {
                ulDataList[i+4] = ulDataList[i] ^ func_data(ulDataList[i+1] ^ ulDataList[i+2] ^ ulDataList[i+3] ^ ulKeyList[i+4]);
            }
        }
        else if(strcmp(type , "2") == 0)
        {
            //解密
            for(i = 0 ; i < 32 ; i++)
            {
                ulDataList[i+4] = ulDataList[i] ^ func_data(ulDataList[i+1] ^ ulDataList[i+2] ^ ulDataList[i+3] ^ ulKeyList[35-i]);
            }
        }
     
        uLong2four_uCh(ulDataList[35] , output);
        uLong2four_uCh(ulDataList[34] , output+4);
        uLong2four_uCh(ulDataList[33] , output+8);
        uLong2four_uCh(ulDataList[32] , output+12);
     
        return 0;
    }
     
    //无符号字符数组转16进制打印
    void print_hex(unsigned char *data , int len)
    {
        int i = 0;
        char alTmp[16] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
        char output[32];
        for(i = 0 ; i < len ; i++)
        {
            // printf("%c" , alTmp[data[i]/16]);
            // printf("%c" , alTmp[data[i]%16]);
            output[i*2] = alTmp[data[i]/16];
            output[i*2+1] = alTmp[data[i]%16];
    //        putchar(' ');
        }
        printf("%s",output);
        putchar('\n');
    }
     
    //16进制数组转无符号字符数组
    int hex_str2bytes(unsigned char *in ,  unsigned char *out , int *outLen)
    {
        int i = 0 , j = 0;
        int num = 0;
        int tmp = 0;
        for (i = 0 ; i < strlen((char *)in) ; i++)
        {
            if(in[i] < 58 && in[i] > 47) //0-9
                tmp = in[i]-48;
            else if(in[i] < 71 && in[i] > 64) //A-F
                tmp = 10+in[i]-65;
            else if(in[i] < 103 && in[i] > 96) //a-f
                tmp = 10+in[i]-97;
            else
            {
                printf("Invalid arg!\n");
                return -1;
            }
     
            if(i%2 == 0)
                num = tmp*16;
            else
            {
                num += tmp;
                out[j++] = num;
            }
        }
        *outLen = j;
        return 0;
    }
     
    int main(int argc , char **argv)
    {
        unsigned char alResult[16] = {0};
        unsigned char alKey[16] = {0};
        int ilKeyLen = 0;
        unsigned char alData[16] = {0};
        int ilDataLen = 0;
     
        // if(argc != 4 || strlen(argv[2]) != 32 || strlen(argv[3]) != 32 || 
        //     (strcmp(argv[1] , "1") != 0 && strcmp(argv[1] , "2") != 0) )
        // {
        //     printf("Usage: ./sm4_tool  [1/2]  hex32_key  hex32_data\n1:encrypt , 2:decrypt\n");
        //     return -1;
        // }
        
        if(hex_str2bytes((unsigned char *)argv[2] , alKey , &ilKeyLen))
            return -1;
        if(hex_str2bytes((unsigned char *)argv[3] , alData, &ilDataLen))
            return -1;
     
        proc_enc_dec(argv[1] , alKey , alData , alResult);
        print_hex(alResult , 16);
     
        return 0;
    }
