#include <ezxml.h>
#include <api/debug.h>

/**
 * @brief xml 객체를 생성한다.
 *
 * @param rtag root tag (<root></root>)
 *
 * @return 성공 시 객체 포인터 반환, 실패 시 NULL 포인터 반환
 */
ezxml_t create_xml(const char *rtag)
{
	return ezxml_new(rtag);
}



/**
 * @brief xml string 데이터를 파싱하여 xml 객체를 리턴한다.
 *
 * @param buf xml string 데이터
 * @param len xml string 데이터 길이
 *
 * @return 성공 시 객체 포인터 반환, 실패 시 NULL 포인터 반환
 */
ezxml_t parse_xml(char *buf, int len)
{
	ezxml_t xml;

	xml = ezxml_parse_str(buf, len);
	if (!xml) {
		return NULL;
	}

	// check parsing error string
	// check xml frame error
	// if 0, no xml error, success parsing
	if (strlen(ezxml_error(xml))) {
		printf("%s\n", ezxml_error(xml));
		ezxml_free(xml);
		return NULL;
	}

	return xml;
}


/**
 * @brief 주어진 xml 객체로 부터 하위 태그 xml 객체들을 반환한다.
 *
 * @param xml xml 객체
 * @param tag 태그명
 *
 * @return 하위 태그가 존재하여 성공 시 객체 포인터 반환, 실패 시 NULL 포인터 반환
 */
ezxml_t read_xml(ezxml_t xml, const char *tag)
{
	if (!xml) {
		dlp_err("xml is null");
		return NULL;
	}
	return ezxml_child(xml, tag);
}



/**
 * @brief 주어진 xml 객체에 태그와 태그 내용을 추가하여 객체를 반환한다.
 *
 * @param xml xml 객체
 * @param tag 추가할 태그명
 * @param txt 추가할 태그내용
 *
 * @return 성공 시 객체 포인터 반환, 실패 시 NULL 포인터 반환
 */
ezxml_t wrtie_xml(ezxml_t xml, const char *tag, const char *txt)
{
	ezxml_t child_xml;

	child_xml = ezxml_add_child(xml, tag, 0);
	ezxml_set_txt(child_xml, txt);
	return child_xml;
}


/**
 * @brief 주어진 xml 객체의 동적 메모리를 해제한다.
 *
 * @param xml xml 객체
 */
void free_xml(ezxml_t xml)
{
	ezxml_free(xml);
}
