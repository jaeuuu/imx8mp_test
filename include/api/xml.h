#ifndef API_EZXML_H_
#define API_EZXML_H_

#include <stdlib.h>
#include <string.h>
#include "ezxml.h"


/**
 * @brief xml 객체를 string으로 변환
 *
 * @param x xml 객체
 *
 * @return 성공 시 char 형 포인터 반환
 */
#define XML_TO_STRING(x) ezxml_toxml(x)

ezxml_t create_xml(const char *rtag);
ezxml_t parse_xml(char *buf, int len);
ezxml_t read_xml(ezxml_t xml, const char *tag);
ezxml_t write_xml(ezxml_t xml, const char *tag, const char *txt);
void free_xml(ezxml_t xml);

#endif
