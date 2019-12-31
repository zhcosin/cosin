
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<ctype.h>

#include "xml/utils.h"
#include "xml/xmlparser.h"

int xml_angle_bracket_bound(const char *content, const char **begin, const char **end)
{
	const char *the_begin = NULL, *the_end = NULL;
	unsigned int len = 0;

	*begin = NULL;
	*end = NULL;

	if (content == NULL || begin == NULL || end == NULL)
		return xml_bad_parameter;

	len = strlen(content);
	if (len == 0)
		return xml_bad_parameter;

	the_begin = strchr(content, '<');
	if (the_begin == NULL)
		return xml_bad_format;

	the_end = strchr(the_begin, '>');
	if (the_end == NULL)
		return xml_bad_format;

	*begin = the_begin;
	*end = ++the_end;

	return xml_ok;
}

int xml_get_block_name(const char *str_begin, const char *str_end, char **name)
{
	int len = 0;
	int off = 0;
	char *the_name = NULL;

	*name = NULL;

	if (str_begin == NULL || str_end == NULL || name == NULL)
		return xml_bad_parameter;

	len = str_end - str_begin;
	if (len <= 0)
		return xml_bad_parameter;

	the_name = (char *)utils_malloc(len * sizeof(char));
	if (the_name == NULL)
		return xml_no_memory;

	off = *(str_begin + 1) == '/' ? 2 : 1;
	if (sscanf(str_begin + off, "%[^ />]", the_name) != 1)
	{
		utils_free(&the_name);
		return xml_bad_format;
	}

	*name = the_name;

	return xml_ok;
}

int xml_find_first_last_char(const char *str_beg, const char *str_end, const char **first, const char **last)
{
	const char *valid_first = NULL, *valid_last = NULL;

	*first = NULL;
	*last = NULL;

	if (str_beg == NULL || str_end == NULL)
		return xml_bad_parameter;

	if (str_beg >= str_end)
		return xml_bad_parameter;

	if (first == NULL || last == NULL)
		return xml_bad_parameter;

	if (*str_beg != '<' || *(str_end - 1) != '>')
		return xml_bad_parameter;

	valid_first = str_beg + 1;
	while (isspace(*valid_first))
	{
		++valid_first;
	}

	valid_last = str_end - 2;
	while (isspace(*valid_last))
	{
		--valid_last;
	}

	if (valid_first > str_end - 2 || valid_last < str_beg + 1)
		return xml_bad_format;

	if (valid_first > valid_last)
		return xml_unknown_error;

	*first = valid_first;
	*last = valid_last;

	return xml_ok;
}

int xml_find_block(const char *content, const char **begin, const char **end, int *block_type)
{
	const char *the_begin = NULL, *the_end = NULL;
	const char *the_valid_begin = NULL, *the_valid_end = NULL;
	unsigned int len = 0;
	int status = xml_ok;

	*begin = NULL;
	*end = NULL;
	*block_type = xml_unknown_type;

	if (content == NULL || begin == NULL || end == NULL || block_type == NULL)
		return xml_bad_parameter;

	len = strlen(content);
	if (len == 0)
		return xml_bad_parameter;

	status = xml_angle_bracket_bound(content, &the_begin, &the_end);
	if (status != xml_ok)
		return status;

	/* find the first and the last valid character. */
	status = xml_find_first_last_char(the_begin, the_end, &the_valid_begin, &the_valid_end);
	if (status != xml_ok)
		return status;

	if (*the_valid_begin == '!')
	{
		*begin = the_begin;
		*end = the_end;
		*block_type = xml_comment_type;
		return xml_ok;
	}
	else if (*the_valid_begin == '?' && *the_valid_end == '?')
	{
		*begin = the_begin;
		*end = the_end;
		*block_type = xml_header_type;
		return xml_ok;
	}
	else
	{
		if (*the_valid_end == '/')
		{
			*begin = the_begin;
			*end = the_end;
			*block_type = xml_element_type;
			return xml_ok;
		}
		else
		{
			char *name = NULL;
			char *current_name = NULL;
			const char *current_beg = NULL;
			const char *current_end = NULL;
			char *the_next_begin = NULL, *the_next_end = NULL;
			
			status = xml_get_block_name(the_begin, the_end, &name);
			if (status != xml_ok)
				return status;

			the_next_end = the_end;
			do 
			{
				current_end = the_next_end;
				status = xml_angle_bracket_bound(current_end, &the_next_begin, &the_next_end);
				if (status != xml_ok)
					return status;
				
				status = xml_get_block_name(the_next_begin, the_next_end, &current_name);
				if (status != xml_ok)
					return status;
			} while (strcmp(name, current_name) != 0);

			the_end = the_next_end;

			*begin = the_begin;
			*end = the_end;
			*block_type = xml_element_type;
			return xml_ok;
		}
	}
}

int xml_build_att_list(const char *content, struct xml_attribute ** att_list, int *att_count)
{
	int len = 0;
	int the_att_count = 0;
	struct xml_attribute *the_att_list = NULL;
	int att_index = 0;
	const char *first_valid_char = NULL, *last_valid_char = NULL;
	const char *curt_att_beg = NULL, *curt_att_end = NULL;
	char *curt_att_name = NULL, *curt_att_value = NULL;
	const char *lookahead = NULL;
	const char *sep = NULL;
	const char *curt_att_name_begin = NULL, *curt_att_name_end = NULL;
	const char *curt_att_value_begin = NULL, *curt_att_value_end = NULL;
	int index = 0;
	int status = 0;

	*att_list = NULL;
	*att_count = 0;

	if (content == NULL || att_list == NULL || att_count == NULL)
		return xml_bad_parameter;

	len = strlen(content);
	if (len == 0)
		return xml_bad_parameter;

	status = xml_find_first_last_char(content, content + len, &first_valid_char, &last_valid_char);
	if (status != xml_ok)
		return status;

	if (*first_valid_char == '/')
		return xml_bad_parameter;

	the_att_count = total_char_count(content, '=');
	if (the_att_count == 0)
		return xml_ok;

	the_att_list = (struct xml_attribute *)utils_malloc(the_att_count * sizeof(struct xml_attribute));
	if (the_att_list == NULL)
		return xml_no_memory;

	/* ignore the name of the node. */
	lookahead = first_valid_char;
	while (!isspace(*lookahead) && *lookahead != '\0')
	{
		++lookahead;
	}

	if (*lookahead == '\0')
	{
		utils_free(&the_att_list);
		return xml_bad_format;
	}

	for (index = 0; index < the_att_count; ++index)
	{
		/* ignore the space after the name of the node. */
		while (isspace(*lookahead) && *lookahead != '\0')
		{
			++lookahead;
		}

		if (*lookahead == '\0')
		{
			if (index > 0)
			{
				int j;
				for (j = 0; j < index; ++j)
				{
					xml_uninit_attribute(&the_att_list[j]);
				}
				utils_free(&the_att_list);
			}

			return xml_bad_format;
		}

		curt_att_beg = lookahead;
		curt_att_name_begin = curt_att_beg;
		curt_att_name_end = curt_att_name_begin;
		while (!isspace(*curt_att_name_end) && *curt_att_name_end != '\0' && *curt_att_name_end != '=')
		{
			++curt_att_name_end;
		}
		if (curt_att_name_end == NULL)
		{
			if (index > 0)
			{
				int j;
				for (j = 0; j < index; ++j)
				{
					xml_uninit_attribute(&the_att_list[j]);
				}
				utils_free(&the_att_list);
			}

			return xml_bad_format;
		}

		sep = strchr(curt_att_name_end, '=');
		curt_att_end = strchr(sep, '\"');
		if (curt_att_end == NULL)
		{
			if (index > 0)
			{
				int j;
				for (j = 0; j < index; ++j)
				{
					xml_uninit_attribute(&the_att_list[j]);
				}
				utils_free(&the_att_list);
			}

			return xml_bad_format;
		}

		++curt_att_end;
		curt_att_value_begin = curt_att_end;

		curt_att_end = strchr(curt_att_end, '\"');
		if (curt_att_end == NULL)
		{
			if (index > 0)
			{
				int j;
				for (j = 0; j < index; ++j)
				{
					xml_uninit_attribute(&the_att_list[j]);
				}
				utils_free(&the_att_list);
			}

			return xml_bad_format;
		}
		curt_att_value_end = curt_att_end;
		++curt_att_end;

		the_att_list[index].name = (char *)utils_malloc((curt_att_name_end - curt_att_name_begin + 1) * sizeof(char));
		if (the_att_list[index].name == NULL)
		{
			if (index > 0)
			{
				int j;
				for (j = 0; j < index; ++j)
				{
					xml_uninit_attribute(&the_att_list[j]);
				}
				utils_free(&the_att_list);
			}

			return xml_bad_format;
		}

		the_att_list[index].value = (char *)utils_malloc((curt_att_value_end - curt_att_value_begin + 1)* sizeof(char));
		if (the_att_list[index].value == NULL)
		{
			xml_uninit_attribute(&the_att_list[index]);
			if (index > 0)
			{
				int j;
				for (j = 0; j < index; ++j)
				{
					xml_uninit_attribute(&the_att_list[j]);
				}
				utils_free(&the_att_list);
			}

			return xml_bad_format;
		}

		strncpy(the_att_list[index].name, curt_att_name_begin, curt_att_name_end - curt_att_name_begin);
		strncpy(the_att_list[index].value, curt_att_value_begin, curt_att_value_end - curt_att_value_begin);

		lookahead = curt_att_end;
	}

	*att_list = the_att_list;
	*att_count = the_att_count;

	return xml_ok;
}

int xml_init_header( struct xml_header *header )
{
	if (header == NULL)
		return xml_bad_parameter;

	header->version = NULL;
	header->encode = NULL;

	return xml_ok;
}

int xml_parse_header( const char *xml_header_content, struct xml_header *header )
{
	char *version = NULL, *encode= NULL;
	unsigned int len = 0;
	int format_count = 0;
	const char *header_format = "<?xml version=\"%[^\"]\" encoding=\"%[^\"]\"?>";

	if (xml_header_content == NULL || header == NULL)
		return xml_bad_parameter;

	len = strlen(xml_header_content);
	if (len == 0)
		return xml_bad_parameter;

	version = (char *)utils_malloc(len * sizeof(char));
	if (version == NULL)
		return xml_no_memory;

	encode = (char *)utils_malloc(len * sizeof(char));
	if (encode == NULL)
	{
		utils_free(&version);
		return xml_no_memory;
	}

	format_count = sscanf(xml_header_content, header_format, version, encode);
	if (format_count != 2)
	{
		utils_free(&version);
		utils_free(&encode);
		return xml_bad_format;
	}

	header->version = version;
	header->encode = encode;

	return xml_ok;
}

int xml_unparse_header(const struct xml_header *header, char **xml_header_content)
{
	if (header == NULL || xml_header_content == NULL)
		return xml_bad_parameter;

	if (header->version == NULL || header->encode == NULL)
		return xml_bad_parameter;

	if (strlen(header->version) > 8 || strlen(header->encode) > 8)
		return xml_bad_parameter;

	*xml_header_content = (char *)utils_malloc(64 * sizeof(char));
	if (*xml_header_content == NULL)
		return xml_no_memory;

	sprintf(*xml_header_content, "<?xml version=\"%s\" encoding=\"%s\"?>", header->version, header->encode);

	return xml_ok;
}

int xml_uninit_header( struct xml_header *header )
{
	if (header == NULL)
		return xml_bad_parameter;

	if (header->version != NULL)
		utils_free(&(header->version));

	if (header->encode != NULL)
		utils_free(&(header->encode));

	return xml_ok;
}

int xml_init_attribute( struct xml_attribute *attribute )
{
	if (attribute == NULL)
		return xml_bad_parameter;

	attribute->name = NULL;
	attribute->value = NULL;

	return xml_ok;
}

int xml_parse_attribute( const char *xml_attribute_content, struct xml_attribute *attribute )
{
	char *name = NULL, *value = NULL;
	unsigned int len = 0;
	const char *attribute_format = "%[^ =]=\"%[^\"]\"";

	if (xml_attribute_content == NULL || attribute == NULL)
		return xml_bad_parameter;

	len = strlen(xml_attribute_content);
	if (len == 0)
		return xml_bad_parameter;

	name = (char *)utils_malloc(len * sizeof(char));
	if (name == NULL)
		return xml_no_memory;

	value = (char *)utils_malloc(len * sizeof(char));
	if (value == NULL)
	{
		utils_free(&name);
		return xml_no_memory;
	}

	if (sscanf(xml_attribute_content, attribute_format, name, value) != 2)
	{
		utils_free(&name);
		utils_free(&value);
		return xml_bad_format;
	}

	attribute->name = name;
	attribute->value = value;

	return xml_ok;
}

int xml_unparse_attribute(const struct xml_attribute *attribute, char **xml_attribute_content)
{
	char *dest = NULL;
	int len_name = 0, len_value = 0;

	if (attribute == NULL || xml_attribute_content == NULL)
		return xml_bad_parameter;

	if (attribute->name == NULL || attribute->value == NULL)
		return xml_bad_parameter;

	len_name = strlen(attribute->name);
	len_value = strlen(attribute->value);
	if (len_name == 0 || len_value == 0)
		return xml_bad_parameter;

	dest = (char *)utils_malloc((len_name + len_value + 4) * sizeof(char));
	if (dest == NULL)
		return xml_no_memory;

	sprintf(dest, "%s=\"%s\"", attribute->name, attribute->value);
	*xml_attribute_content = dest;

	return xml_ok;
}

int xml_uninit_attribute( struct xml_attribute *attribute )
{
	if (attribute == NULL)
		return xml_bad_parameter;

	if (attribute->name != NULL)
		utils_free(&(attribute->name));

	if (attribute->value != NULL)
		utils_free(&(attribute->value));

	return xml_ok;
}

int xml_init_node( struct xml_node *node )
{
	if (node == NULL)
		return xml_bad_parameter;

	node->name = NULL;
	node->value = NULL;

	node->attributes = NULL;
	node->attribute_count = 0;

	node->children = NULL;
	node->child_count = 0;

	return xml_ok;
}

int xml_parse_node( const char *xml_node_content, struct xml_node *node )
{
	unsigned int len = 0;
	const char *first_valid_char = NULL, *last_valid_char = NULL;
	const char *first_angle_beg = NULL, *first_angle_end = NULL;
	char *node_header = NULL;
	struct xml_attribute *att_list = NULL;
	int att_count = 0;
	char *node_name = NULL;
	int status = 0;

	if (xml_node_content == NULL || node == NULL)
		return xml_bad_parameter;

	len = strlen(xml_node_content);
	if (len == 0)
		return xml_bad_parameter;

	status = xml_angle_bracket_bound(xml_node_content, &first_angle_beg, &first_angle_end);
	if (status != xml_ok)
		return status;

	status = xml_find_first_last_char(xml_node_content, xml_node_content + len, &first_valid_char, &last_valid_char);
	if (status != xml_ok)
		return status;

	if (*first_valid_char == '/')
		return xml_bad_parameter;

	/* parse attribute list. */
	node_header = (char *)utils_malloc((first_angle_end - first_angle_beg + 1) * sizeof(char));
	if (node_header == NULL)
		return xml_no_memory;

	utils_strncpy(node_header, first_angle_beg, first_angle_end - first_angle_beg);
	if (strchr(node_header, '=') != NULL)
	{
		status = xml_build_att_list(node_header, &(node->attributes), &(node->attribute_count));
		if (status != xml_ok)
		{
			utils_free(&node_header);
			return xml_bad_format;
		}
	}
	
	utils_free(&node_header);

	status = xml_get_block_name(xml_node_content, xml_node_content + len, &node_name);
	if (status != xml_ok)
	{
		utils_free(&node_header);
		return status;
	}

	node->name = node_name;

	if (*last_valid_char == '/')
	{
		return xml_ok;
	}
	else
	{
		const char *next_angle_beg = NULL, *next_angle_end = NULL;
		const char *next_first_valid_char = NULL, *next_last_valid_char = NULL;
		char *next_block_name = NULL;

		status = xml_angle_bracket_bound(first_valid_char, &next_angle_beg, &next_angle_end);
		if (status != xml_ok)
		{
			xml_uninit_node(node);
			return xml_bad_format;
		}

		status = xml_find_first_last_char(next_angle_beg, next_angle_end, &next_first_valid_char, &next_last_valid_char);
		if (status != xml_ok)
		{
			xml_uninit_node(node);
			return xml_bad_format;
		}

		if (*next_first_valid_char == '/')
		{
			char *node_value = NULL;

			status = xml_get_block_name(next_angle_beg, next_angle_end, &next_block_name);
			if (status != xml_ok)
			{
				xml_uninit_node(node);
				return xml_bad_format;
			}

			if (strcmp(node_name, next_block_name) != 0)
			{
				xml_uninit_node(node);
				utils_free(&next_block_name);
				return xml_bad_format;
			}

			node_value = (char *)utils_malloc((next_angle_beg - first_angle_end + 1) * sizeof(char));
			if (node_value == NULL)
			{
				xml_uninit_node(node);
				utils_free(&next_block_name);
				return xml_no_memory;
			}

			utils_strncpy(node_value, first_angle_end, next_angle_beg - first_angle_end);
			utils_clrspace(node_value);

			if (strlen(node_value) > 0)
				node->value = node_value;
			else
			{
				utils_free(&node_value);
			}
		} 
		else
		{
			const char *current_block_end = NULL;
			const char *next_block_beg = NULL, *next_block_end = NULL;
			char *next_block_content = NULL;
			char *next_angle_name = NULL;
			const char *next_angle_beg = NULL, *next_anble_end = NULL;
			int next_block_type = 0;

			struct xml_node *child_nodes = NULL;
			int child_node_real_count = 0;
			int child_node_init_count = 0;
			int child_node_allow_count = 0;
			int child_node_increament = 20;

			if (child_node_init_count > 0)
			{
				child_nodes = (struct xml_node *)utils_malloc(child_node_init_count * sizeof(struct xml_node));
				if (child_nodes == NULL)
				{
					xml_uninit_node(node);
					return xml_no_memory;
				}
				child_node_allow_count = child_node_init_count;
			}

			next_block_end = first_angle_end;

			do 
			{
				current_block_end = next_block_end;

				status = xml_angle_bracket_bound(current_block_end, &next_angle_beg, &next_angle_end);
				if (status != xml_ok)
				{
					xml_uninit_node(node);
					return xml_bad_format;
				}

				status = xml_get_block_name(next_angle_beg, next_angle_end, &next_angle_name);
				if (status != xml_ok)
				{
					xml_uninit_node(node);
					return xml_bad_format;
				}

				if (strcmp(next_angle_name, node_name) == 0)
				{
					utils_free(&next_angle_name);
					break;
				}

				utils_free(&next_angle_name);

				if (child_node_real_count == child_node_allow_count)
				{
					struct xml_node *new_buffer = (struct xml_node *)utils_malloc((child_node_real_count + child_node_increament) * sizeof(struct xml_node));
					if (new_buffer == NULL)
					{
						int i;
						for (i = 0; i < child_node_real_count; ++i)
						{
							xml_uninit_node(&(child_nodes[i]));
						}
						xml_uninit_node(node);
						return xml_no_memory;
					}

					memmove(new_buffer, child_nodes, child_node_real_count * sizeof(struct xml_node));

					child_nodes = new_buffer;
					child_node_allow_count += child_node_increament;
				}

				status = xml_find_block(current_block_end, &next_block_beg, &next_block_end, &next_block_type);
				if (status != xml_ok)
				{
					xml_uninit_node(node);
					return xml_bad_format;
				}

				switch (next_block_type)
				{
				case xml_header_type:
					{
						xml_uninit_node(node);
						return xml_bad_format;
					}
				case xml_comment_type:
					{
						current_block_end = next_block_end;
						continue;
						break;
					}
				case xml_element_type:
					{
						next_block_content = (char *)utils_malloc((next_block_end - next_block_beg + 1) * sizeof(char));
						if (next_block_content == NULL)
						{
							xml_uninit_node(node);
							return xml_no_memory;
						}

						utils_strncpy(next_block_content, next_block_beg, next_block_end - next_block_beg);

						xml_init_node(&(child_nodes[child_node_real_count]));
						status = xml_parse_node(next_block_content, &(child_nodes[child_node_real_count]));
						if (status != xml_ok)
						{
							xml_uninit_node(node);
							utils_free(&next_block_content);
							xml_uninit_node(&(child_nodes[child_node_real_count]));
							return status;
						}

						child_node_real_count ++;

						break;
					}
				case xml_unknown_type:
					{
						xml_uninit_node(node);
						return xml_unknown_error;
					}
				}
			} while (1);

			if (child_node_real_count > 0)
			{
				node->child_count = child_node_real_count;
				node->children = child_nodes;
			}
		}
	}

	return xml_ok;
}

int xml_unparse_node(const struct xml_node *node, char **xml_node_content)
{
	if (node == NULL || xml_node_content == NULL)
		return xml_bad_parameter;

	return xml_ok;
}

int xml_uninit_node( struct xml_node *node )
{
	if (node == NULL)
		return xml_bad_parameter;

	if (node->name != NULL)
		utils_free(&(node->name));

	if (node->value != NULL)
		utils_free(&(node->value));

	if (node->attribute_count > 0)
	{
		int i;
		for (i = 0; i < node->attribute_count; ++i)
		{
			xml_uninit_attribute(&node->attributes[i]);
		}

		utils_free(&(node->attributes));
		node->attribute_count = 0;
	}

	if (node->child_count > 0)
	{
		int i;
		for (i = 0; i < node->child_count; ++i)
		{
			xml_uninit_node(&node->children[i]);
		}

		utils_free(&(node->children));
		node->child_count = 0;
	}

	return xml_ok;
}

int xml_init_tree( struct xml_tree *tree )
{
	int error_code = xml_ok;

	if (tree == NULL)
		return xml_bad_parameter;
	
	error_code = xml_init_header(&tree->header);
	if (error_code != xml_ok)
	{
		xml_uninit_header(&tree->header);
		return error_code;
	}

	error_code = xml_init_node(&tree->root);
	if (error_code != xml_ok)
	{
		xml_uninit_header(&tree->header);
		xml_init_node(&tree->root);
		return error_code;
	}

	return xml_ok;
}

int xml_parse(const char *xml_content, struct xml_tree* tree)
{
	int len = 0;
	const char *block_beg = NULL, *block_end = NULL;
	const char *current_block_end = NULL;
	int block_type = 0;
	int error_code = 0;
	char *header = NULL;
	char *root = NULL;

	if (xml_content == NULL || tree == NULL)
		return xml_bad_parameter;

	len = strlen(xml_content);
	if (len == 0)
		return xml_bad_parameter;

	error_code = xml_find_block(xml_content, &block_beg, &block_end, &block_type);
	if (error_code != xml_ok)
		return xml_bad_format;

	if (block_type != xml_header_type)
		return xml_bad_format;

	header = (char *)utils_malloc((block_end - block_beg + 1) * sizeof(char));
	if (header == NULL)
		return xml_no_memory;

	utils_strncpy(header, block_beg, block_end - block_beg);

	xml_init_header(&(tree->header));
	error_code = xml_parse_header(header, &(tree->header));
	if (error_code != xml_ok)
	{
		utils_free(&header);
		return error_code;
	}

	utils_free(&header);

	do 
	{
		current_block_end = block_end;

		error_code = xml_find_block(current_block_end, &block_beg, &block_end, &block_type);
		if (error_code != xml_ok)
			return error_code;
	} while (block_type != xml_element_type);

	root = (char *)utils_malloc((block_end - block_beg + 1) * sizeof(char));
	if (root == NULL)
	{
		xml_uninit_header(&(tree->header));
		return xml_no_memory;
	}
	utils_strncpy(root, block_beg, block_end - block_beg);

	xml_init_node(&(tree->root));
	error_code = xml_parse_node(root, &(tree->root));
	{
		utils_free(&root);
		return error_code;
	}

	utils_free(&root);

	return xml_ok;
}

int xml_unparse(const struct xml_tree *tree, char **xml_content)
{
	return xml_ok;
}

int xml_uninit_tree(struct xml_tree *tree)
{
	xml_uninit_header(&(tree->header));
	xml_uninit_node(&(tree)->root);

	return xml_ok;
}