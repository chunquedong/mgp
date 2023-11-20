#ifndef __runtime__utf8__
#define __runtime__utf8__

/*
 * Description: UTF-8 字符串的解码和编码函数
 *              unicode 字符处理函数
 *     History: yang@haipo.me, 2013/05/29, create
 */

# pragma once

# include <stdint.h>
# include <stddef.h>

/*
 * 标准 C 并没有规定 wchar_t 的位数。但 GNU C Lib 保证 wchar_t 是 32 位的，
 * 所以可以用 wchar.h 中定义的函数来像 wchar_t 一样操纵 ucs4_t.
 * http://www.gnu.org/software/libc/manual/html_node/Extended-Char-Intro.html
 */
typedef int32_t ucs4_t;

/*
 * 从 UTF-8 编码的字符串 *src 中读取一个 unicode 字符，并更新 *src 的值。
 *
 * 如果遇到非法 UTF-8 编码，则跳过非法部分。
 * 如果 illegal 参数不为 NULL, 则 *illegal 表示非法 UTF-8 编码字节数。
 */
ucs4_t getu8c(char **src, int *illegal);

/*
 * 将 src 指向的 UTF-8 编码字符串解码为 unicode，放在长度为 n 的数组 des 中，
 * 并在末尾添加 0. 如果 des 不足以存放所有的字符，则最多保存 n - 1 个 unicode
 * 字符并补 0.
 *
 * 如果遇到非法 UTF-8 编码，则跳过非法部分。
 * 如果 illegal 不为 NULL, 则 *illegal 表示非法 UTF-8 编码的字节数。
 */
size_t u8decode(char const *str, ucs4_t *des, size_t n, int *illegal);

/*
 * 将 unicode 字符 uc 编码为 UTF-8 编码，放到长度为 *left 的字符串 *des 中。
 *
 * 如果 *des 不足以存放 uc 对应的 UTF-8 字符串，返回一个负值。
 * 如果成功，更新 *des 和 *left 的值。
 */
int putu8c(ucs4_t uc, char **des, size_t *left);

/*
 * 将以 0 结尾的 unicode 数组 us 编码为 UTF-8 字符串，放到长度为 n 的字符串 des 中。
 *
 * 负数为非法的 unicode 字符。
 * 如果 illegal 不为 NULL，则 *illegal 表示非法的 unicode 字符数。
 */
size_t u8encode(ucs4_t *us, char *des, size_t n, int *illegal);

/*
 * 判断是否为全角字符
 */
int isufullwidth(ucs4_t uc);

/*
 * 判断是否为全角字母
 */
int isufullwidthalpha(ucs4_t uc);

/*
 * 判断是否为全角数字
 */
int isufullwidthdigit(ucs4_t uc);

/*
 * 全角字符转半角字符。
 * 如果 uc 为全角字符，则返回对应的半角字符，否则返回 uc 本身。
 */
ucs4_t ufull2half(ucs4_t uc);

/*
 * 半角字符转全角字符
 * 如果 uc 为半角字符，则返回对应的全角字符，否则返回 uc 本身。
 */
ucs4_t uhalf2full(ucs4_t uc);

/*
 * 判断是否为汉字字符（中日韩越统一表意文字）
 */
int isuchiness(ucs4_t uc);

/*
 * 判断是否为中文标点
 */
int isuzhpunct(ucs4_t uc);

/*
 * 判断是否为日文平假名字符
 */
int isuhiragana(ucs4_t uc);

/*
 * 判断是否为日文片假名字符
 */
int isukatakana(ucs4_t uc);

/*
 * 判断是否为韩文字符
 */
int isukorean(ucs4_t uc);

#endif /* defined(__runtime__utf8__) */