#include <string.h>
#include <stdio.h>
#include <iconv.h>
#include <errno.h>
#include <unistd.h>

int main(void)
{
    unsigned char sen[20] = "我是中国人";
    unsigned char out[20] = {0};
    unsigned char *outp = out;
    unsigned char *inp = sen;
    int utf_len = 20;
    size_t in_len = utf_len;
    size_t out_len = 20;
    int ret, index;
    int out_byte_len;
    iconv_t cd;

    printf("[utf-gbk] utf-src:%s and len[%d]\n", sen, (int)strlen(sen));

    cd = iconv_open("gbk", "utf-8");
    if (cd == -1)
    {
        printf("[utf-gbk] iconv_open fail, errno=%d\n", errno);
    }
    printf("[utf-gbk] begin to transform\n");
    ret = iconv(cd, &inp, &in_len,  &outp, &out_len);
    if (ret == -1)
    {
        printf("[utf-gbk] iconv fail, errno=%d\n", errno);
        return -1;
    }

    iconv_close(cd);

    printf("[utf-gbk] end of transform\n");

    out_byte_len = (int)strlen(out);

    printf("[utf-gbk] gbk-dst:%s chinese_char_len[%d] byte_lens[%d]\n", out, out_len, out_byte_len);

    printf("[utf-gbk] GBK dump Data: ");

    for (index = 0; index < out_byte_len; index++)
        printf("0x%.2X ", out[index]);

    printf("\n");

    return 0;
}
