
#include "xml/xmlparser.h"
#include "xml/utils.h"
#include <stdio.h>

int main(int argc, char *argv[])
{
	struct xml_header header;
	char xml_header_str[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
	char *xml_header_str2;
	int status = 0;

	struct xml_attribute att;
	char xml_att_str[] = "Code=\"100110000103040001\"";
	char *xml_att_str2;

	const char *xml_content = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
		"<SIP_XML EventType=\"Notify_Status\">"
		"<Item Code=\"100110000103040001\" Name=\"HIK\" Status=\"1\"/>"
		"<Item Code=\"100110000103040002\" Name=\"DAHUA\" Status=\"1\"/>"
		"</SIP_XML>";
	const char *begin = NULL, *end = NULL;

	const char *test_str = "<Item Code=\"100110000103040001\" Name=\"HIK\" Status=\"1\"/>";

	const char *xml_node_str = 
		"<SIP_XML EventType=\"Notify_Status\">"
		"<Item Code=\"100110000103040001\" Name=\"HIK\" Status=\"1\"/>"
		"<Item Code=\"100110000103040002\" Name=\"DAHUA\" Status=\"1\"/>"
		"</SIP_XML>";

	struct xml_attribute *att_list = NULL;
	int att_count = 0;

	struct xml_node node;
	struct xml_tree tree;

	status = xml_angle_bracket_bound(xml_content, &begin, &end);

	

	// ²âÊÔ xml_header
	xml_init_header(&header);

	status = xml_parse_header(xml_header_str, &header);
	if (status != xml_ok)
	{
		xml_uninit_header(&header);
		return -1;
	}

	status = xml_unparse_header(&header, &xml_header_str2);
	if (status != xml_ok)
	{
		xml_uninit_header(&header);
		return -2;
	}

	utils_free(&xml_header_str2);

	xml_uninit_header(&header);

	// ²âÊÔ xml_attribute
	xml_init_attribute(&att);
	status = xml_parse_attribute(xml_att_str, &att);
	if (status != xml_ok)
	{
		xml_uninit_attribute(&att);
		return -1;
	}

	status = xml_unparse_attribute(&att, &xml_att_str2);
	if (status != xml_ok)
	{
		xml_uninit_attribute(&att);
		return -1;
	}

	utils_free(&xml_att_str2);
	status = xml_uninit_attribute(&att);

	status = xml_build_att_list(test_str, &att_list, &att_count);

	status = xml_init_node(&node);
	status = xml_parse_node(xml_node_str, &node);
	status = xml_uninit_node(&node);

	status = xml_init_tree(&tree);
	status = xml_parse(xml_content, &tree);
	status = xml_uninit_tree(&tree);

	return 0;
}