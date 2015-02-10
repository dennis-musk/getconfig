#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

/* 
 * The format is :
 * [section1]
 * param1  = value1
 * param2  = value2
 *
 * [section2]
 * param1  = value1
 * param2  = valule2
 *
 */

#define		MAX_SECTON_SIZE		4096
#define		MAX_NAME_SIZE		40
#define		MAX_VALUE_SIZE		40
#define 	MAX_ROW_SIZE		200

/* get_section - get the who section string from config file
 * @config_file: config file name, include path
 * @section:	 the section what is looking for from the config file
 * @revalue:	 pointer where the section string store if found in config file
 *
 * return: 0 if success, negative if error happend.
 * */
int get_section(const char *config_file, const char *section, char *revalue)
{
	FILE *fd;
	int start;
	char tmp[MAX_NAME_SIZE] = {0};
	char row[MAX_ROW_SIZE];

	fd = fopen(config_file, "r");
	if (fd == NULL) {
		perror("fopen():");
		return -1;
	}

	start = 0;

	sprintf(tmp, "[%s]", section);

	memset(row, 0, MAX_ROW_SIZE);
	while (fgets(row, MAX_ROW_SIZE, fd)) {
		if(row[0] == '[' && start) {
			break;
		}

		if (!memcmp(row, tmp, strlen(tmp))) {
			start = 1;
		}

		if (start) {
			if (strlen(revalue) + strlen(row) >= MAX_SECTON_SIZE) {
				return -ENOMEM;
			}	

			memcpy(revalue + strlen(revalue), row, strlen(row));
		}

		memset(row, 0, MAX_ROW_SIZE);
	}

	fclose(fd);
	return 0;
}


int get_value(const char *section, const char *node_name, char *revalue)
{
	int i;
	char *begin, *end, *p;
	char name_cmp[MAX_NAME_SIZE] = {0};
	char value_tmp[MAX_VALUE_SIZE] = {0};

	if (section == NULL || node_name == NULL) {
		printf("The section or node name pointer is null\n");
		return -EINVAL;
	}

	sprintf(name_cmp, "%s", node_name);

	p = value_tmp;

	begin = strstr(section, name_cmp);
	if (begin == NULL) {
		return -EFAULT;
	}

	begin += strlen(name_cmp); /* end of node_name */

	end = strchr(begin, '\n'); 

	while (isblank(*begin) || (*begin) == '=') {
		begin++;	
	} 

	memcpy(value_tmp, begin, end - begin);

	/* remove end blank from value_tmp */
	for (i = 0; i < strlen(value_tmp); ++i) {
		if (isblank(p[i]))
			p[i] = '\0';
	}

	memcpy(revalue, value_tmp, strlen(value_tmp));

	return 0;
}

int main(void)
{
	int ret;
	char section[MAX_SECTON_SIZE] = {0};
	char value[MAX_VALUE_SIZE] = {0};

	ret = get_section("./commModule.conf", "TcpClient", section);
	if (ret < 0) {
		perror("get_section()");
		exit(1);
	}

	printf("The section is:\n%s", section);

	ret = get_value(section, "LogicName", value);
	if (ret < 0) {
		perror("get_value()");
		exit(1);
	}
	printf("The LogicName is : %s\n", value);

	memset(value, 0, MAX_VALUE_SIZE);

	ret = get_value(section, "DestIp", value);
	if (ret < 0) {
		perror("get_value()");
		exit(1);
	}

	printf("The DestIp is : %s\n", value);

	memset(value, 0, MAX_VALUE_SIZE);
	ret = get_value(section, "DestPort", value);
	if (ret < 0) {
		perror("get_value()");
		exit(1);
	}

	printf("The DestPort is : %s\n", value);

	return 0;
}
