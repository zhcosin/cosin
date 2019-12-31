
#ifndef __COSIN_XML_PARSER_H_
#define __COSIN_XML_PARSER_H_

typedef enum xml_error_code
{
	xml_ok				= 0,
	xml_bad_parameter	= -1,
	xml_no_memory		= -2,
	xml_bad_format		= -3,
	xml_unknown_error	= -99
};

typedef enum xml_block_type
{
	xml_header_type,
	xml_comment_type,
	xml_element_type,
	xml_unknown_type
};

struct xml_header
{
	char *version;
	char *encode;
};

struct xml_attribute
{
	char *name;
	char *value;
};

struct xml_node
{
	char					*name;
	char					*value;		/* a node can not both with value and children. */

	struct xml_attribute	*attributes;
	int						attribute_count;

	struct xml_node			*children;
	int						child_count;
};

struct xml_tree
{
	struct xml_header	header;
	struct xml_node		root;
};

int xml_angle_bracket_bound(const char *content, const char **begin, const char **end);
int xml_get_block_name(const char *str_begin, const char *str_end, char **name);
int xml_find_first_last_char(const char *str_beg, const char *str_end, const char **first, const char **last);
int xml_find_block(const char *content, const char **begin, const char **end, int *block_type);
int xml_build_att_list(const char *content, struct xml_attribute ** att_list, int *att_count);

int xml_init_header(struct xml_header *header);
int xml_parse_header(const char *xml_header_content, struct xml_header *header);
int xml_unparse_header(const struct xml_header *header, char **xml_header_content);
int xml_uninit_header(struct xml_header *header);

int xml_init_attribute(struct xml_attribute *attribute);
int xml_parse_attribute(const char *xml_attribute_content, struct xml_attribute *attribute);
int xml_unparse_attribute(const struct xml_attribute *attribute, char **xml_attribute_content);
int xml_uninit_attribute(struct xml_attribute *attribute);

int xml_init_node(struct xml_node *node);
int xml_parse_node(const char *xml_node_content, struct xml_node *node);
int xml_unparse_node(const struct xml_node *node, char **xml_node_content);
int xml_uninit_node(struct xml_node *node);

int xml_init_tree(struct xml_tree *tree);
int xml_parse(const char *xml_content, struct xml_tree* tree);
int xml_unparse(const struct xml_tree *tree, char **xml_content);
int xml_uninit_tree(struct xml_tree *tree);


#endif
